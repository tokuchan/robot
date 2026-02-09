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
        else if( target == "/client" && req_.method() == http::verb::get )
        {
            handle_client();
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

    void handle_client()
    {
        constexpr std::string_view html = R"html(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Robot Control</title>
    <style>
        body {
            margin: 0;
            padding: 20px;
            background: #1a1a1a;
            color: #fff;
            font-family: 'Courier New', monospace;
            display: flex;
            flex-direction: column;
            align-items: center;
        }
        h1 {
            margin: 0 0 20px 0;
            font-size: 24px;
        }
        #canvas {
            border: 2px solid #4a9eff;
            background: #0a0a0a;
            box-shadow: 0 0 20px rgba(74, 158, 255, 0.3);
        }
        .controls {
            margin-top: 20px;
            text-align: center;
        }
        .info {
            margin-top: 15px;
            padding: 10px;
            background: #2a2a2a;
            border-radius: 5px;
            font-size: 14px;
        }
        .key {
            display: inline-block;
            padding: 5px 10px;
            margin: 0 2px;
            background: #4a9eff;
            border-radius: 3px;
            font-weight: bold;
            color: #000;
        }
    </style>
</head>
<body>
    <h1>🤖 Robot Control Interface</h1>
    <canvas id="canvas" width="800" height="600"></canvas>
    <div class="controls">
        <div class="info">
            Use <span class="key">W</span><span class="key">A</span><span class="key">S</span><span class="key">D</span> to move
        </div>
        <div class="info" id="status">Input: (0.0, 0.0)</div>
    </div>

    <script>
        const canvas = document.getElementById('canvas');
        const ctx = canvas.getContext('2d');
        const statusEl = document.getElementById('status');
        
        // Input state
        const keys = { w: false, a: false, s: false, d: false };
        let currentInput = { x: 0, y: 0 };
        
        // Keyboard handling
        document.addEventListener('keydown', (e) => {
            const key = e.key.toLowerCase();
            if (key in keys && !keys[key]) {
                keys[key] = true;
                updateInput();
            }
        });
        
        document.addEventListener('keyup', (e) => {
            const key = e.key.toLowerCase();
            if (key in keys) {
                keys[key] = false;
                updateInput();
            }
        });
        
        function updateInput() {
            let x = 0, y = 0;
            if (keys.a) x -= 1;
            if (keys.d) x += 1;
            if (keys.w) y += 1;
            if (keys.s) y -= 1;
            
            // Normalize diagonal movement
            if (x !== 0 && y !== 0) {
                const len = Math.sqrt(x * x + y * y);
                x /= len;
                y /= len;
            }
            
            currentInput = { x, y };
            statusEl.textContent = `Input: (${x.toFixed(1)}, ${y.toFixed(1)})`;
            sendInput(x, y);
        }
        
        async function sendInput(x, y) {
            try {
                await fetch('/input', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ x, y })
                });
            } catch (err) {
                console.error('Failed to send input:', err);
            }
        }
        
        async function fetchScene() {
            try {
                const response = await fetch('/output');
                return await response.json();
            } catch (err) {
                console.error('Failed to fetch scene:', err);
                return null;
            }
        }
        
        function drawScene(scene) {
            // Clear canvas
            ctx.fillStyle = '#0a0a0a';
            ctx.fillRect(0, 0, canvas.width, canvas.height);
            
            // Draw grid
            ctx.strokeStyle = '#1a3a5a';
            ctx.lineWidth = 1;
            const gridSize = 50;
            for (let x = 0; x < canvas.width; x += gridSize) {
                ctx.beginPath();
                ctx.moveTo(x, 0);
                ctx.lineTo(x, canvas.height);
                ctx.stroke();
            }
            for (let y = 0; y < canvas.height; y += gridSize) {
                ctx.beginPath();
                ctx.moveTo(0, y);
                ctx.lineTo(canvas.width, y);
                ctx.stroke();
            }
            
            if (!scene || !scene.geometries) return;
            
            // Transform: world coordinates to screen coordinates
            const centerX = canvas.width / 2;
            const centerY = canvas.height / 2;
            const scale = 50; // pixels per world unit
            
            // Draw geometries
            scene.geometries.forEach((geo, idx) => {
                const pos = geo.position || [0, 0];
                const vertices = geo.vertices || [];
                
                if (vertices.length < 3) return;
                
                ctx.save();
                ctx.translate(centerX + pos[0] * scale, centerY - pos[1] * scale);
                
                // Draw filled polygon
                ctx.fillStyle = idx === 0 ? '#4a9eff' : '#ff6b6b';
                ctx.strokeStyle = '#fff';
                ctx.lineWidth = 2;
                ctx.beginPath();
                vertices.forEach((v, i) => {
                    const x = v[0] * scale;
                    const y = -v[1] * scale; // Invert Y for screen coordinates
                    if (i === 0) ctx.moveTo(x, y);
                    else ctx.lineTo(x, y);
                });
                ctx.closePath();
                ctx.fill();
                ctx.stroke();
                
                ctx.restore();
            });
        }
        
        // Main render loop
        async function animate() {
            const scene = await fetchScene();
            drawScene(scene);
            requestAnimationFrame(animate);
        }
        
        // Start
        animate();
    </script>
</body>
</html>)html";

        send_html_response( http::status::ok, html );
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

    void send_html_response( http::status status, std::string_view body )
    {
        auto res = std::make_shared< http::response< http::string_body > >();
        res->result( status );
        res->set( http::field::content_type, "text/html" );
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