cmake_minimum_required(VERSION 3.9)

add_executable(performance_tester client.cpp client.h main.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(performance_tester Threads::Threads)