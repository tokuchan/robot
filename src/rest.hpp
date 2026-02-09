static_assert( __cplusplus > 2020'00 );
#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/json.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <unordered_set>

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
            {
                std::cerr << "REST read error: " << ec.message() << std::endl;
                return self->do_close();
            }
            std::cout << "REST request: " << self->req_.method_string() << " " << self->req_.target() << std::endl;
            try
            {
                self->handle_request();
            }
            catch( const std::exception & e )
            {
                std::cerr << "REST request error: " << e.what() << std::endl;
                self->send_response( http::status::internal_server_error, "Internal Server Error" );
            }
            catch( ... )
            {
                std::cerr << "REST request error: unknown exception" << std::endl;
                self->send_response( http::status::internal_server_error, "Internal Server Error" );
            }
        } );
    }

    void handle_request()
    {
        auto target = req_.target();
        // Strip query parameters (e.g., ?id=...&vscodeBrowserReqId=...) from the path
        auto target_view = target;
        auto query_pos = target_view.find( '?' );
        if( query_pos != std::string_view::npos )
        {
            target_view = target_view.substr( 0, query_pos );
        }

        if( target_view == "/input" && req_.method() == http::verb::post )
        {
            handle_input();
        }
        else if( target_view == "/output" && req_.method() == http::verb::get )
        {
            handle_output();
        }
        else if( ( target_view == "/client" || target_view == "/" ) && req_.method() == http::verb::get )
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
            std::lock_guard< std::mutex > lock( store_mutex_ );
            auto body = req_.body();
            auto jv = boost::json::parse( body );
            auto obj = jv.as_object();
            // Only extract x and y - ignore any other fields like IDs
            float x = boost::json::value_to< float >( obj.at( "x" ) );
            float y = boost::json::value_to< float >( obj.at( "y" ) );

            // std::cout << "REST input: entity 0 <- PlayerInput(" << x << ", " << y << ")" << std::endl;

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
            std::cerr << "REST input error: " << e.what() << std::endl;
            send_response( http::status::bad_request, std::string( e.what() ) );
        }
    }

    void handle_output()
    {
        try
        {
            std::lock_guard< std::mutex > lock( store_mutex_ );
            boost::json::object scene;
            boost::json::array geometries;

            // Iterate over entities but don't expose entity_id to client
            for( auto [ entity_id, polygon ] : entity_store_.get< Polygon >() )
            {
                boost::json::object geo;
                boost::json::array vertices;
                for( auto [ x, y ] : polygon )
                {
                    vertices.push_back( boost::json::array{ x, y } );
                }
                // Only send vertices and position - no entity IDs
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
            std::cerr << "REST output error: " << e.what() << std::endl;
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
            height: 100vh;
            box-sizing: border-box;
        }
        h1 {
            margin: 0 0 20px 0;
            font-size: 24px;
        }
        #canvas-wrapper {
            width: 100%;
            max-width: 1200px;
            flex: 1;
            display: flex;
        }
        #canvas {
            border: 2px solid #4a9eff;
            background: #0a0a0a;
            box-shadow: 0 0 20px rgba(74, 158, 255, 0.3);
            width: 100%;
            height: 100%;
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
    <div id="canvas-wrapper">
        <canvas id="canvas"></canvas>
    </div>
    <div class="controls">
        <div class="info">
            Use <span class="key">W</span><span class="key">A</span><span class="key">S</span><span class="key">D</span> to move
        </div>
        <div class="info" id="status">Input: (0.0, 0.0)</div>
    </div>

    <script>
        const canvasWrapper = document.getElementById('canvas-wrapper');
        const canvas = document.getElementById('canvas');
        const ctx = canvas.getContext('2d');
        const statusEl = document.getElementById('status');
        let viewWidth = 1;
        let viewHeight = 1;
                function resizeCanvas()
        {
            const rect = canvasWrapper.getBoundingClientRect();
            const dpr = window.devicePixelRatio || 1;
            let w = Math.max( 100, Math.floor( rect.width ) );
            let h = Math.max( 100, Math.floor( rect.height ) );
            viewWidth = w;
            viewHeight = h;
            canvas.width = Math.floor( w * dpr );
            canvas.height = Math.floor( h * dpr );
            ctx.setTransform( dpr, 0, 0, dpr, 0, 0 );
            console.log( `Canvas resized to ${w}x${h} (dpi: ${dpr})` );
        }

        
        // Input state
        const keys = { KeyW: false, KeyA: false, KeyS: false, KeyD: false };
        let currentInput = { x: 0, y: 0 };
        
        // Keyboard handling
        document.addEventListener('keydown', (e) => {
            if (e.code in keys) {
                e.preventDefault();
                if (!keys[e.code]) {
                    keys[e.code] = true;
                    updateInput();
                }
            }
        });
        
        document.addEventListener('keyup', (e) => {
            if (e.code in keys) {
                e.preventDefault();
                keys[e.code] = false;
                updateInput();
            }
        });
        
        function updateInput()
        {
            let x = 0, y = 0;
            if( keys.KeyA )
                x -= 1;
            if( keys.KeyD )
                x += 1;
            if( keys.KeyW )
                y += 1;
            if( keys.KeyS )
                y -= 1;

            // Normalize diagonal movement
            if( x !== 0 && y !== 0 )
            {
                const len = Math.sqrt( x * x + y * y );
                x /= len;
                y /= len;
            }

            currentInput = { x, y };
            statusEl.textContent = `Input: (${x.toFixed( 1 )}, ${y.toFixed( 1 )})`;
            console.log(`updateInput: sending (${x.toFixed(2)}, ${y.toFixed(2)})`);
            sendInput( x, y );
        }
        
        async function sendInput(x, y) {
            try {
                console.log(`sendInput: POSTing (${x.toFixed(2)}, ${y.toFixed(2)}) to /input`);
                // Explicitly send only x and y coordinates - no IDs
                await fetch('/input', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ x: Number(x), y: Number(y) })
                });
            } catch (err) {
                console.error('Failed to send input:', err);
            }
        }
        
        async function fetchScene() {
            try {
                const response = await fetch('/output');
                if (!response.ok) {
                    console.error(`fetch /output failed: status ${response.status}`);
                    return null;
                }
                const data = await response.json();
                console.log('fetchScene succeeded:', data);
                return data;
            } catch (err) {
                console.error('Failed to fetch scene:', err);
                return null;
            }
        }
        
        function computeSceneBounds(scene) {
            let minX = Infinity;
            let minY = Infinity;
            let maxX = -Infinity;
            let maxY = -Infinity;

            if (!scene || !scene.geometries) {
                return { minX: -10, minY: -10, maxX: 10, maxY: 10 };
            }

            scene.geometries.forEach((geo) => {
                const pos = geo.position || [0, 0];
                const vertices = geo.vertices || [];
                vertices.forEach((v) => {
                    const wx = pos[0] + v[0];
                    const wy = pos[1] + v[1];
                    minX = Math.min(minX, wx);
                    minY = Math.min(minY, wy);
                    maxX = Math.max(maxX, wx);
                    maxY = Math.max(maxY, wy);
                });
            });

            if (!isFinite(minX) || !isFinite(minY) || !isFinite(maxX) || !isFinite(maxY)) {
                return { minX: -10, minY: -10, maxX: 10, maxY: 10 };
            }

            let width = Math.max(1, maxX - minX);
            let height = Math.max(1, maxY - minY);
            const padX = width * 0.1 + 1;
            const padY = height * 0.1 + 1;
            return {
                minX: minX - padX,
                minY: minY - padY,
                maxX: maxX + padX,
                maxY: maxY + padY
            };
        }

        function niceGridStep( rawStep )
        {
            if( rawStep <= 0 )
                rawStep = 1;
            const power = Math.pow( 10, Math.floor( Math.log10( rawStep ) ) );
            const scaled = rawStep / power;
            if( scaled <= 1 )
                return 1 * power;
            if( scaled <= 2 )
                return 2 * power;
            if( scaled <= 5 )
                return 5 * power;
            return 10 * power;
        }

        function drawScene(scene) {
            // Clear canvas
            ctx.fillStyle = '#0a0a0a';
            ctx.fillRect(0, 0, viewWidth, viewHeight);

            const bounds = computeSceneBounds(scene);
            const worldWidth = Math.max(1, bounds.maxX - bounds.minX);
            const worldHeight = Math.max(1, bounds.maxY - bounds.minY);
            const zoom = 2;
            const scale = Math.min(viewWidth / worldWidth, viewHeight / worldHeight) * zoom;
            const centerX = viewWidth / 2;
            const centerY = viewHeight / 2;
            const worldCenterX = (bounds.minX + bounds.maxX) / 2;
            const worldCenterY = (bounds.minY + bounds.maxY) / 2;

            console.log(`Drawing: bounds=[${bounds.minX.toFixed(1)}, ${bounds.minY.toFixed(1)}] to [${bounds.maxX.toFixed(1)}, ${bounds.maxY.toFixed(1)}], scale=${scale.toFixed(2)}`);

            const worldToScreen = (x, y) => {
                return {
                    x: centerX + (x - worldCenterX) * scale,
                    y: centerY - (y - worldCenterY) * scale
                };
            };

            // Draw grid
            ctx.strokeStyle = '#1a3a5a';
            ctx.lineWidth = 1;
            const targetGridPixels = 80;
            const gridStep = niceGridStep(targetGridPixels / scale);
            const startX = Math.floor(bounds.minX / gridStep) * gridStep;
            const endX = Math.ceil(bounds.maxX / gridStep) * gridStep;
            const startY = Math.floor(bounds.minY / gridStep) * gridStep;
            const endY = Math.ceil(bounds.maxY / gridStep) * gridStep;

            for (let x = startX; x <= endX; x += gridStep) {
                const a = worldToScreen(x, bounds.minY);
                const b = worldToScreen(x, bounds.maxY);
                ctx.beginPath();
                ctx.moveTo(a.x, a.y);
                ctx.lineTo(b.x, b.y);
                ctx.stroke();
            }
            for (let y = startY; y <= endY; y += gridStep) {
                const a = worldToScreen(bounds.minX, y);
                const b = worldToScreen(bounds.maxX, y);
                ctx.beginPath();
                ctx.moveTo(a.x, a.y);
                ctx.lineTo(b.x, b.y);
                ctx.stroke();
            }

            if (!scene || !scene.geometries) return;

            // Draw geometries
            scene.geometries.forEach((geo, idx) => {
                const pos = geo.position || [0, 0];
                const vertices = geo.vertices || [];

                if (vertices.length < 3) return;

                ctx.save();

                // Draw filled polygon
                ctx.fillStyle = idx === 0 ? '#4a9eff' : '#ff6b6b';
                ctx.strokeStyle = '#fff';
                ctx.lineWidth = 2;
                ctx.beginPath();
                vertices.forEach((v, i) => {
                    const worldX = pos[0] + v[0];
                    const worldY = pos[1] + v[1];
                    const screen = worldToScreen(worldX, worldY);
                    if (i === 0) ctx.moveTo(screen.x, screen.y);
                    else ctx.lineTo(screen.x, screen.y);
                });
                ctx.closePath();
                ctx.fill();
                ctx.stroke();

                ctx.restore();
            });
        }
        
        let lastScene = null;
        let fetchCount = 0;

        async function updateSceneData()
        {
            try
            {
                lastScene = await fetchScene();
                fetchCount++;
                // Always log first fetch, then occasionally
                if( fetchCount === 1 || fetchCount % 60 === 0 )
                {
                    if( !lastScene )
                    {
                        console.warn( 'fetchScene returned null' );
                    }
                    else if( !lastScene.geometries )
                    {
                        console.warn( 'Scene missing geometries:', lastScene );
                    }
                    else
                    {
                        console.log( `Fetched scene with ${lastScene.geometries.length} geometries` );
                    }
                }
            }
            catch( err )
            {
                console.error( 'updateSceneData error:', err );
            }
        }

        function animate()
        {
            if( lastScene )
            {
                drawScene( lastScene );
            }
            else
            {
                // Show loading state
                ctx.fillStyle = '#0a0a0a';
                ctx.fillRect(0, 0, viewWidth, viewHeight);
                ctx.fillStyle = '#4a9eff';
                ctx.font = '20px "Courier New", monospace';
                ctx.textAlign = 'center';
                ctx.textBaseline = 'middle';
                ctx.fillText('Loading...', viewWidth / 2, viewHeight / 2);
            }
            requestAnimationFrame( animate );
        }

        // Fetch every 33ms (~30fps) instead of 16ms to reduce server load
        setInterval( updateSceneData, 33 );
        
        window.addEventListener( 'resize', resizeCanvas );

        // Initialize immediately - no delays
        resizeCanvas();
        updateSceneData();
        requestAnimationFrame( animate );
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
        http::async_write( stream_, *res, [ self, res ]( beast::error_code ec, std::size_t ) {
            if( ec )
            {
                std::cerr << "REST write error: " << ec.message() << std::endl;
                self->do_close();
            }
            else
            {
                self->do_read();
            }
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
        http::async_write( stream_, *res, [ self, res ]( beast::error_code ec, std::size_t ) {
            if( ec )
            {
                std::cerr << "REST write error: " << ec.message() << std::endl;
                self->do_close();
            }
            else
            {
                self->do_read();
            }
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
    std::unordered_set< std::string > known_clients_;

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
            {
                beast::error_code endpoint_ec;
                auto remote_endpoint = socket.remote_endpoint( endpoint_ec );
                if( endpoint_ec )
                {
                    std::cerr << "REST accept error: " << endpoint_ec.message() << std::endl;
                }
                else
                {
                    auto client_ip = remote_endpoint.address().to_string();
                    if( shared_this->known_clients_.insert( client_ip ).second )
                    {
                        std::cout << "REST client connected first time from " << client_ip << std::endl;
                    }
                }
                std::make_shared< Session >(
                    shared_this->ioc_,
                    std::move( socket ),
                    shared_this->store_mutex_,
                    shared_this->entity_store_ )
                    ->run();
            }
            else
            {
                std::cerr << "REST accept error: " << ec.message() << std::endl;
            }
            shared_this->do_accept();
        } );
    }
};
} // namespace robot::src::detail::rest::inline exports

namespace robot::src::inline exports::inline rest
{
using namespace detail::rest::exports;
}