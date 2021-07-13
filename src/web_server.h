#pragma once

#include <memory>
#include <string>
#include <thread>
#include <map>
#include <algorithm>

#include <asio.hpp>
#include <asio/ts/internet.hpp>

#include "connection.h"

/**
 * \brief This is the main class someone should use.
 * NB! Remember to call the stop() function or you will not exit cleanly.
 */
class web_server
{
public:
    /**
     * \brief Constructor
     * This will set up some asio stuff for the tcp-layer.
     * \param ip_address to listen on.
     * \param port to listen on.
     */
    explicit web_server(std::string_view ip_address, short port = 80, std::string_view certificate_file = "", std::string_view key_file = "");
    web_server(const web_server& other) = delete;
    web_server(const web_server&& other) = delete;
    web_server& operator=(const web_server& other) = delete;
    web_server& operator=(const web_server&& other) = delete;
    ~web_server() = default;

    /**
     * \brief Calls the wait_for_client function.
     * Starts the context thread.
     * Starts a thread to delete stale connections.
     * \return true if the server started.
     */
    bool start();
    /**
     * \brief Registers a new endpoint that clients can connect to.
     * \param method Which HTTP Action to listen on.
     * \param url The url the client must hit to call this method.
     * \param function A lambda that will be called once the METHOD and url is hit.
     */
    void register_function(METHODS method, const std::string& url, const std::function<void(const web_request&, web_response&)>& function);
    /**
     * \brief Stops the io_context thread
     */
    void stop();

protected:
    /**
     * \brief Waits for clients to connect.
     * When a client connects, we create a connection object and add it to the _connections vector.
     */
    void wait_for_client();

private:
    /**
     * \brief The IpAddress the server is listening on.
     */
    std::string _ip_address;
    /**
     * \brief The port the server is listening on.
     */
    unsigned short _port;
    /**
     * \brief Whether or not the server should shut down.
     */
    bool _shutdown = false;

    std::string _certificate_file;
    std::string _key_file;

#ifdef OPEN_SSL
    asio::ssl::context _io_context;
#endif
    asio::io_context _basic_context;
    asio::ip::tcp::endpoint _endpoint;
    std::unique_ptr<asio::ip::tcp::acceptor> _acceptor;
    std::thread _context_thread;
    /**
     * \brief A list of active connections
     */
    std::vector<std::shared_ptr<connection>> _connections;

    /**
     * \brief The list of active end points a client can connect to.
     */
    std::map<METHODS, std::vector<std::pair<std::string, std::function<void(const web_request&, web_response&)>>>> _functions;
    /**
     * \brief A thread that deletes stale connections from the list.
     */
    std::thread _delete_connections_thread;
    /**
     * \brief The list is changed in a thread so we have to be thread safe.
     */
    std::mutex _connection_mutex;
};

inline web_server::web_server(const std::string_view ip_address, const short port, const std::string_view certificate_file, const std::string_view key_file)
    : _ip_address(ip_address),
    _port(port),
#ifdef OPEN_SSL
    _io_context(asio::ssl::context::sslv23),
#endif
    _certificate_file(certificate_file),
    _key_file(key_file)
{
    try
    {
#ifdef OPEN_SSL
        _io_context.set_options(
            asio::ssl::context::default_workarounds
            | asio::ssl::context::no_sslv2);
        _io_context.use_certificate_chain_file(_certificate_file);
        _io_context.use_private_key_file(_key_file, asio::ssl::context::pem);
#endif
        _endpoint = asio::ip::tcp::endpoint(asio::ip::make_address(_ip_address), _port);
        _acceptor = std::make_unique<asio::ip::tcp::acceptor>(_basic_context, _endpoint);
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

inline bool web_server::start()
{
    try
    {
        wait_for_client();
        _context_thread = std::thread([&]()
        {
            try
            {
                _basic_context.run();
            }
            catch (const asio::error_code& e)
            {
                std::cout << "Error: " << e.message() << std::endl;
            }
            catch (const std::exception& e)
            {
                std::cout << "Error: " << e.what() << std::endl;
            }
        });
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
    _basic_context.stop();
    if (_context_thread.joinable())
        _context_thread.join();
    _connections.clear();
    if (_delete_connections_thread.joinable())
        _delete_connections_thread.join();

    std::cout << "[web_server] Stopped..." << std::endl;
}

inline void web_server::wait_for_client()
{
    _acceptor->async_accept([&](const std::error_code ec, asio::ip::tcp::socket socket)
        {
            socket.set_option(asio::socket_base::reuse_address(true));
            if (!ec)
            {
#ifdef OPEN_SSL
                auto new_connection = std::make_shared<connection>(asio::ssl::stream<asio::ip::tcp::socket>(std::move(socket), _io_context));
#else
                auto new_connection = std::make_shared<connection>(std::move(socket));
#endif
                new_connection->done_reading_function = [&](const web_request& request, web_response& response)
                {
                    if (_functions.find(request.method) != _functions.end())
                    {
                        for (const auto& [url, function] : _functions[request.method])
                        {
                            if (request.url == url)
                            {
                                function(request, response);
                                break;
                            }
                        }
                    }
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
