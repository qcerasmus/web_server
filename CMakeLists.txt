cmake_minimum_required(VERSION 3.16)

add_definitions(-DASIO_STANDALONE)

if (WIN32)
    add_definitions(-D_WIN32_WINNT=0x0A00)
endif(WIN32)

project(web_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(third_party)

include_directories(third_party)
include_directories(third_party/asio/asio/include)
include_directories(src)

add_subdirectory(performance_tester)

file(GLOB web_server_headers
    src/*.h)

add_executable(web_server main.cpp ${web_server_headers} ${handler_source})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(web_server Threads::Threads)