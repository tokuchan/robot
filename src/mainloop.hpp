#pragma once

#include <thread>
#include <stop_token>
#include <atomic>
#include "component_types.hpp"
#include "systems.hpp"

void mainloop(std::stop_source &stop_source)
{
    std::jthread loop_thread(
        [&stop_source](std::stop_token stop_token)
        {
            std::cout << "Main loop started. Press Ctrl+C to stop." << std::endl;
            EntityStore store;

            while (!stop_token.stop_requested())
            {
                handlePlayerInput(store);
                handleCollisions(store);
                updatePositions(store);
            }

            std::cout << "\rMain loop exiting..." << std::endl;
        },
        stop_source.get_token());
}