#pragma once

#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <stop_token>
#include <thread>

#include "component_types.hpp"
#include "rest.hpp"
#include "systems.hpp"

namespace robot::src::detail::mainloop::inline exports
{
void runMainloop( std::stop_source & stop_source )
{
    // The REST server and main loop will both access the EntityStore, so we
    // need to protect it with a mutex.
    std::mutex store_mutex;
    EntityStore store;
    boost::asio::io_context ioc;

    std::jthread loop_thread(
        [ &stop_source, &store_mutex, &store ]( std::stop_token stop_token ) {
            std::cout << "Main loop started. Press Ctrl+C to stop." << std::endl;

            while( !stop_token.stop_requested() )
            {
                std::lock_guard< std::mutex > lock( store_mutex );
                handlePlayerInput( store );
                handleCollisions( store );
                updatePositions( store );
            }

            std::cout << "\rMain loop exiting..." << std::endl;
        },
        stop_source.get_token() );

    std::jthread rest_thread(
        [ &stop_source, &store_mutex, &store, &ioc ]( std::stop_token stop_token ) {
            std::cout << "REST server started on port 8080." << std::endl;
            // print a clickable URL if the terminal supports it
            std::cout << "Open http://localhost:8080 in your browser to control the robot." << std::endl;
            auto rest_server = std::make_shared< RESTServer >( ioc, store_mutex, store, 8080 );
            rest_server->run();

            // Register a callback to stop the io_context when the stop token is requested
            std::stop_callback stop_cb( stop_token, [ &ioc ]() {
                ioc.stop();
            } );

            // Run the io_context - this will block until ioc.stop() is called
            ioc.run();

            std::cout << "\rREST server exiting..." << std::endl;
        },
        stop_source.get_token() );
}
} // namespace robot::src::detail::mainloop::inline exports

namespace robot::src::inline exports::inline mainloop
{
using namespace detail::mainloop::exports;
}