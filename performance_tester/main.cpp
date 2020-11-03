#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>
#include <numeric>

#include "client.h"

inline bool SHUTDOWN = false;

void keep_running()
{
    auto bla = std::getchar();
}

std::vector<int> times_;

void print_average_times_for_last_second()
{
    while (!SHUTDOWN)
    {
        const auto sum = std::accumulate(times_.begin(), times_.end(), 0);
        std::cout << "In the last second we have received " << times_.size() << " responses with an average of: " << static_cast<double>(sum) / static_cast<double>(times_.size()) << " microseconds\n";
        times_.clear();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main()
{
    const auto number_of_clients = 16;

    std::thread performance_print_thread([&]()
        {
            print_average_times_for_last_second();
        });

    std::vector<std::thread> clients(number_of_clients);

    for (auto i = 0; i < number_of_clients; i++)
    {
        std::thread t([&]()
            {
                while (!SHUTDOWN)
                {
                    const auto start_time = std::chrono::high_resolution_clock::now();
                    client c("127.0.0.1", 80);
                    times_.push_back(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start_time).count());
                }
            });

        clients.emplace_back(std::move(t));
    }

    std::thread dont_stop_believing([&]()
        {
            keep_running();
        });
    dont_stop_believing.join();
    SHUTDOWN = true;

    for (auto& t : clients)
    {
        if (t.joinable())
            t.join();
    }

    performance_print_thread.join();

    clients.clear();
}