#pragma once

#include <thread>
#include <stop_token>
#include <atomic>
#include <iostream>
#include <chrono>
#include <mutex>

#include "component_types.hpp"
#include "systems.hpp"
#include "rest.hpp"

namespace robot::src::detail::mainloop::inline exports
{
    void runMainloop(std::stop_source &stop_source)
    {
        // The REST server and main loop will both access the EntityStore, so we
        // need to protect it with a mutex.
        std::mutex store_mutex;
        EntityStore store;
        boost::asio::io_context ioc;

        std::jthread loop_thread(
            [&stop_source, &store_mutex, &store](std::stop_token stop_token)
            {
                std::cout << "Main loop started. Press Ctrl+C to stop." << std::endl;

                while (!stop_token.stop_requested())
                {
                    std::lock_guard<std::mutex> lock(store_mutex);
                    handlePlayerInput(store);
                    handleCollisions(store);
                    updatePositions(store);
                }

                std::cout << "\rMain loop exiting..." << std::endl;
            },
            stop_source.get_token());

        std::jthread rest_thread(
            [&stop_source, &store_mutex, &store, &ioc](std::stop_token stop_token)
            {
                std::cout << "REST server started on port 8080." << std::endl;
                auto rest_server = std::make_shared<RESTServer>(ioc, store_mutex, store, 8080);
                rest_server->run();
                ioc.run();

                // The REST server runs its own io_context loop, so we just wait
                // for the stop token.
                while (!stop_token.stop_requested())
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }

                std::cout << "\rREST server exiting..." << std::endl;
            },
            stop_source.get_token());
    }
}

namespace robot::src::inline exports::inline mainloop
{
    using namespace detail::mainloop::exports;
}