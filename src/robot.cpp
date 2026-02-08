#include <iostream>
#include <csignal>
#include "mainloop.hpp"

// Global stop token
std::stop_source stop_source;

void signal_handler(int signal)
{
    if (signal == SIGINT)
    {
        stop_source.request_stop();
    }
}

int main(int argc, char *argv[])
{
    std::cout << "Robot application started." << std::endl;

    std::signal(SIGINT, signal_handler);
    mainloop(stop_source);

    std::cout << "Robot application exiting." << std::endl;

    return 0;
}