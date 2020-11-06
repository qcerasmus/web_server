#pragma once

#include <asio/asio.hpp>

/**
 * \brief A simple client class to do requests to the root of a server.
 * This is only being used by the performance_tester
 */
class client
{
public:
    /**
     * \brief Connects to the ip-address and port and does a GET request.
     * \param ip_address The ip-address to connect to.
     * \param port The port the server is listening on. Usually port 80.
     */
    client(const std::string& ip_address, unsigned short port = 80);
    ~client() = default;

private:
    /**
     * \brief Writes the GET request to the socket.
     */
    void write_header();

    asio::io_context _context;
    std::unique_ptr<asio::ip::tcp::socket> _socket = nullptr;
    std::vector<unsigned char> _response;
    std::string _request;
    bool still_running = true;
};

