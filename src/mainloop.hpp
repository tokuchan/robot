#pragma once

#include <thread>
#include <stop_token>
#include <atomic>

void mainloop(std::stop_source &stop_source)
{
    std::jthread loop_thread(
        [&stop_source](std::stop_token stop_token)
        {
            std::cout << "Main loop started. Press Ctrl+C to stop." << std::endl;

            while (!stop_token.stop_requested())
            {
                // Event loop body
                // Process events here
            }

            std::cout << "\rMain loop exiting..." << std::endl;
        },
        stop_source.get_token());
}