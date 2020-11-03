#pragma once

#include <memory>
#include <string>
#include <thread>
#include <map>
#include <algorithm>

#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

#include "connection.h"

class web_server
{
public:
    explicit web_server(const std::string& ip_address, const short port);
    ~web_server();

    bool start();
    void register_function(METHODS method, const std::string& url, const std::function<void(const web_request&, web_response&)>& function);
    void stop();

protected:
    void wait_for_client();

private:
    std::string _ip_address;
    unsigned short _port;
    bool _shutdown = false;

    asio::io_context _io_context;
    asio::ip::tcp::endpoint _endpoint;
    std::unique_ptr<asio::ip::tcp::acceptor> _acceptor;
    std::thread _context_thread;
    std::vector<std::shared_ptr<connection>> _connections;

    std::map<METHODS, std::vector<std::pair<std::string, std::function<void(const web_request&, web_response&)>>>> _functions;
    std::chrono::high_resolution_clock::time_point _start_time;
    std::thread _delete_connections_thread;
    std::mutex _connection_mutex;
};

inline web_server::web_server(const std::string& ip_address, const short port)
    : _ip_address(ip_address),
    _port(port)
{
    try
    {
        _endpoint = asio::ip::tcp::endpoint(asio::ip::make_address(_ip_address), port);
        _acceptor = std::make_unique<asio::ip::tcp::acceptor>(_io_context, _endpoint);
        const auto option = asio::socket_base::reuse_address(true);
        std::error_code ec;
        _acceptor->set_option(option, ec);
        if (ec)
        {
            std::cout << ec.message();
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "[web_server] Error: " << e.what() << std::endl;
    }
}

inline web_server::~web_server()
{
    stop();
    _connections.clear();
    if (_delete_connections_thread.joinable())
        _delete_connections_thread.join();
}

inline bool web_server::start()
{
    try
    {
        wait_for_client();
        _context_thread = std::thread([&]() { _io_context.run(); });
        _delete_connections_thread = std::thread([&]()
            {
                while (!_shutdown)
                {
                    try
                    {
                        if (!_connections.empty())
                        {
                            std::lock_guard<std::mutex> guard(_connection_mutex);
                            _connections.erase(std::remove_if(_connections.begin(), _connections.end(), [](const std::shared_ptr<connection>& connection)
                                {
                                    if (connection)
                                        return connection->close_me;
                                    return false;
                                }), _connections.end());
                        }
                    }
                    catch(const std::exception&)
                    {
                        
                    }

                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            });
    }
    catch(const std::exception& e)
    {
        std::cerr << "[web_server] Error: " << e.what() << std::endl;
        return false;
    }

    std::cout << "[web_server] Started..." << std::endl;
    return true;
}

inline void web_server::register_function(const METHODS method, const std::string& url, const std::function<void(const web_request&, web_response&)>& function)
{
    _functions[method].push_back(std::make_pair(url, function));
}

inline void web_server::stop()
{
    _shutdown = true;
    _io_context.stop();
    if (_context_thread.joinable())
        _context_thread.join();

    std::cout << "[web_server] Stopped..." << std::endl;
}

inline void web_server::wait_for_client()
{
    _acceptor->async_accept([&](std::error_code ec, asio::ip::tcp::socket socket)
        {
            socket.set_option(asio::socket_base::reuse_address(true));
            if (!ec)
            {
                _start_time = std::chrono::high_resolution_clock::now();
                //std::cout << "[web_server] someone connected: " << socket.remote_endpoint() << std::endl;
                auto new_connection = std::make_shared<connection>(_io_context, std::move(socket));
                new_connection->done_reading_function = [&](const web_request& request, web_response& response)
                {
                    if (_functions.find(request.method) != _functions.end())
                    {
                        auto found = false;
                        for (const auto& [url, function] : _functions[request.method])
                        {
                            if (request.url == url)
                            {
                                found = true;
                                function(request, response);
                                break;
                            }
                        }
                        if (!found)
                        {
                            response.status_code = 404;
                            response.status = "NOT FOUND";
                        }
                    }
                    else
                    {
                        response.status_code = 404;
                        response.status = "NOT FOUND";
                    }
                    const auto end_time = std::chrono::high_resolution_clock::now();
                    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - _start_time);
                    //std::cout << "It took: " << duration.count() << " us\n";
                };
                std::lock_guard<std::mutex> guard(_connection_mutex);
                _connections.emplace_back(new_connection);
                wait_for_client();
            }
            else
            {
                std::cerr << "[web_server] Error with client connection: " << ec.message() << std::endl;
            }
        });
}
