#pragma once

#include <asio/asio.hpp>

class client
{
public:
    client(const std::string& ip_address, unsigned short port);
    ~client();

private:
    void write_header();

    asio::io_context _context;
    std::unique_ptr<asio::ip::tcp::socket> _socket = nullptr;
    std::vector<unsigned char> _response;
    std::string _request;
    bool still_running = true;
};

