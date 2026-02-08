static_assert( __cplusplus > 2020'00 );
#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/json.hpp>
#include <memory>
#include <thread>

#include "component_types.hpp"

namespace robot::src::detail::rest::inline exports
{
namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class Session : public std::enable_shared_from_this< Session >
{
private:
    net::io_context & ioc_;
    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    std::mutex & store_mutex_;
    EntityStore & entity_store_;
    http::request< http::string_body > req_;

public:
    Session( net::io_context & ioc, tcp::socket socket, std::mutex & store_mutex, EntityStore & entity_store )
        : ioc_( ioc )
        , stream_( std::move( socket ) )
        , store_mutex_( store_mutex )
        , entity_store_( entity_store )
    {}

    void run()
    {
        do_read();
    }

private:
    void do_read()
    {
        auto self = shared_from_this();
        http::async_read( stream_, buffer_, req_, [ self ]( beast::error_code ec, std::size_t ) {
            if( ec == http::error::end_of_stream )
                return self->do_close();
            if( ec )
                return;
            self->handle_request();
            self->do_read();
        } );
    }

    void handle_request()
    {
        auto target = req_.target();
        if( target == "/input" && req_.method() == http::verb::post )
        {
            handle_input();
        }
        else if( target == "/output" && req_.method() == http::verb::get )
        {
            handle_output();
        }
        else
        {
            send_response( http::status::not_found, "Not Found" );
        }
    }

    void handle_input()
    {
        try
        {
            auto body = req_.body();
            auto jv = boost::json::parse( body );
            auto obj = jv.as_object();
            float x = boost::json::value_to< float >( obj.at( "x" ) );
            float y = boost::json::value_to< float >( obj.at( "y" ) );

            auto & inputs = entity_store_.get< PlayerInput >();

            if( inputs.contains( 0 ) )
            {
                inputs[ 0 ] = PlayerInput( x, y );
            }
            else
            {
                inputs.insert( 0, PlayerInput( x, y ) );
            }
            send_response( http::status::ok, R"({"status":"ok"})" );
        }
        catch( const std::exception & e )
        {
            send_response( http::status::bad_request, std::string( e.what() ) );
        }
    }

    void handle_output()
    {
        try
        {
            boost::json::object scene;
            boost::json::array geometries;

            for( auto [ entity_id, polygon ] : entity_store_.get< Polygon >() )
            {
                boost::json::object geo;
                boost::json::array vertices;
                for( auto [ x, y ] : polygon )
                {
                    vertices.push_back( boost::json::array{ x, y } );
                }
                geo[ "vertices" ] = vertices;
                if( entity_store_.get< Position >().contains( entity_id ) )
                {
                    auto pos = entity_store_.get< Position >()[ entity_id ];
                    geo[ "position" ] = boost::json::array{ pos.x, pos.y };
                }
                geometries.push_back( geo );
            }

            scene[ "geometries" ] = geometries;
            send_response( http::status::ok, boost::json::serialize( scene ) );
        }
        catch( const std::exception & e )
        {
            send_response( http::status::internal_server_error, std::string( e.what() ) );
        }
    }

    void send_response( http::status status, std::string_view body )
    {
        auto res = std::make_shared< http::response< http::string_body > >();
        res->result( status );
        res->set( http::field::content_type, "application/json" );
        res->body() = std::string( body );
        res->prepare_payload();

        auto self = shared_from_this();
        http::async_write( stream_, *res, [ self ]( beast::error_code ec, std::size_t ) {
            if( ec )
                self->do_close();
        } );
    }

    void do_close()
    {
        beast::error_code ec;
        stream_.socket().shutdown( tcp::socket::shutdown_both, ec );
    }
};

class RESTServer : public std::enable_shared_from_this< RESTServer >
{
private:
    net::io_context & ioc_;
    tcp::acceptor acceptor_;
    std::mutex & store_mutex_;
    EntityStore & entity_store_;

public:
    RESTServer( net::io_context & ioc, std::mutex & store_mutex, EntityStore & entity_store, unsigned short port )
        : ioc_( ioc )
        , acceptor_( ioc, tcp::endpoint( tcp::v4(), port ) )
        , store_mutex_( store_mutex )
        , entity_store_( entity_store )
    {}

    void run()
    {
        do_accept();
    }

private:
    void do_accept()
    {
        acceptor_.async_accept( [ shared_this = shared_from_this() ]( beast::error_code ec, tcp::socket socket ) {
            if( !ec )
                std::make_shared< Session >(
                    shared_this->ioc_,
                    std::move( socket ),
                    shared_this->store_mutex_,
                    shared_this->entity_store_ )
                    ->run();
            shared_this->do_accept();
        } );
    }
};
} // namespace robot::src::detail::rest::inline exports

namespace robot::src::inline exports::inline rest
{
using namespace detail::rest::exports;
}