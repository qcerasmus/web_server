#include "client.h"

#include <iostream>

client::client(const std::string& ip_address, unsigned short port)
{
    asio::ip::tcp::endpoint endpoint(asio::ip::make_address(ip_address), port);
    _socket = std::make_unique<asio::ip::tcp::socket>(_context);

    _socket->connect(endpoint);

    _request = "GET / HTTP/1.0\r\n"
        "Host: localhost\r\n"
        "Accept: */*\r\n"
        "Connection: close\r\n\r\n";

    write_header();
    _context.run();
    while (still_running)
        std::this_thread::sleep_for(std::chrono::nanoseconds(10));
}

client::~client()
{

}

void client::write_header()
{
    asio::async_write(*_socket, asio::buffer(_request), [&](const std::error_code& ec, const std::size_t length)
        {
            asio::async_read(*_socket, asio::dynamic_buffer(_response), [&](const std::error_code& ec, const std::size_t length)
                {
                    still_running = false;
                });
        });
}

