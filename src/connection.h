#pragma once

#include <asio.hpp>
#ifdef OPEN_SSL
#include <asio/ssl.hpp>
#endif

#include "structs/request.h"
#include "structs/response.h"

/**
 * \brief This class is responsible for reading and writing to the socket.
 */
class connection : public std::enable_shared_from_this<connection>
{
public:
    /**
     * \brief This class moves the socket locally and is responsible to read and write to it.
     * \param socket the socket is moved to this class
     */
#ifdef OPEN_SSL
    connection(asio::ssl::stream<asio::ip::tcp::socket> socket);
#else
    connection(asio::ip::tcp::socket socket);
#endif
    
    connection(const connection& c) = delete;
    connection(const connection&& c) = delete;
    connection& operator=(const connection& other) = delete;
    connection& operator=(const connection&& other) = delete;
    ~connection() = default;

    /**
     * \brief After the request has been read, we call this function which is set by the web_server class.
     * This is to allow the server to call a function set by the end user.
     */
    std::function<void(web_request&, web_response&)> done_reading_function;
    /**
     * \brief This will be set to true so that the connection can be deleted from the web_server class.
     */
    bool close_me = false;

protected:
#ifdef OPEN_SSL
    asio::ssl::stream<asio::ip::tcp::socket> _socket;
#else
    asio::ip::tcp::socket _socket;
#endif

private:
    /**
     * \brief This is the first function that is called to get the request header.
     * It happens directly after a connection is established.
     */
    void read_header();
    /**
     * \brief Adds the body to the web_request object.
     * \param body_length The length of the body
     * \param request_header The web_request object by reference to set the body
     */
    void read_body(const std::size_t& body_length, web_request& request_header) const;

    /**
     * \brief Sets the header content that was sent.
     * \param header The string of headers that was sent from the client.
     * \param request_header The web_request object has a vector of headers that are set here.
     */
    static void header_helper(std::string& header, web_request& request_header);
#ifdef OPEN_SSL
    void do_handshake();
    void ssl_shutdown(const asio::error_code& ec);
#endif

    asio::streambuf _socket_buffer;
    std::string _response_string;
};

#ifdef OPEN_SSL
inline connection::connection(asio::ssl::stream<asio::ip::tcp::socket> socket)
    : _socket(std::move(socket))
{
    do_handshake();
}
#else
inline connection::connection(asio::ip::tcp::socket socket)
    : _socket(std::move(socket))
{
    read_header();
}
#endif

inline void connection::read_header()
{
    async_read_until(_socket, _socket_buffer, "\r\n\r\n",
        [&](const std::error_code ec, const std::size_t length_read)
        {
            //We have read some data.
            if (!ec)
            {
                if (length_read != 0)
                {
                    auto header = std::string(
                        asio::buffer_cast<const char*>(_socket_buffer.data()),
                        length_read);
                    _socket_buffer.consume(length_read);

                    web_request request_header{};
                    header_helper(header, request_header);

                    std::size_t length = 0;
                    if (request_header.header_values.find("Content-Length") != request_header.header_values.end())
                    {
                        length = std::stol(request_header.header_values["Content-Length"]);
                    }

                    read_body(length, request_header);
                    web_response response;
                    response.protocol = request_header.protocol;
                    response.version = request_header.version;
                    response.host = request_header.header_values["Host"];
                    done_reading_function(request_header, response);
                    _response_string = response.to_string();
                    asio::async_write(_socket, asio::buffer(_response_string, _response_string.length()),
                        [&](const std::error_code ec_write, const std::size_t bytes_written)
                        {
                            if (ec_write || bytes_written != _response_string.length())
                                std::cerr << "[connection] Error while replying: " << ec.message() << std::endl;
#ifdef OPEN_SSL
                            _socket.lowest_layer().cancel();
                            _socket.async_shutdown(std::bind(&connection::ssl_shutdown, this->shared_from_this(), std::placeholders::_1));
#else
                            _socket.close();
#endif
                            close_me = true;
                        });
                }   
            }
            else
            {
                std::cerr << "[connection] Error: " << ec.message() << std::endl;
#ifdef OPEN_SSL
                _socket.lowest_layer().cancel();
                _socket.async_shutdown(std::bind(&connection::ssl_shutdown, this->shared_from_this(), std::placeholders::_1));
#else
                _socket.close();
#endif
                close_me = true;
            }
        });
}

inline void connection::read_body(const std::size_t& body_length, web_request& request_header) const
{
    if (body_length > 0)
    {
        const auto body = std::string(
            asio::buffer_cast<const char*>(_socket_buffer.data()),
            body_length);

        request_header.body = body;
    }
}

inline void connection::header_helper(std::string& header, web_request& request_header)
{
    auto sub_position = header.find(' ');
    const auto temp_header = header.substr(0, sub_position);
    header = header.substr(sub_position + 1);
    request_header.method = methods::string_to_method(temp_header);

    sub_position = header.find(' ');
    const auto temp_url = header.substr(0, sub_position);
    header = header.substr(sub_position + 1);
    request_header.url = temp_url;

    sub_position = header.find('/');
    const auto temp_protocol = header.substr(0, sub_position);
    header = header.substr(sub_position + 1);
    request_header.protocol = temp_protocol;

    sub_position = header.find("\r\n");
    const auto temp_version = header.substr(0, sub_position);
    header = header.substr(sub_position + 2);
    request_header.version = temp_version;

    while (header.length() > 0)
    {
        std::string key;
        const auto key_end_position = header.find(':');
        key = header.substr(0, key_end_position);
        if (key == "\r\n")
            break;
        header = header.substr(key_end_position + 2);
        const auto value_end_position = header.find("\r\n");
        const auto value = header.substr(0, value_end_position); //this could be an issue...
        header = header.substr(value_end_position + 2);

        request_header.header_values[key] = value;
    }
}

#ifdef OPEN_SSL
inline void connection::do_handshake()
{
    _socket.async_handshake(asio::ssl::stream_base::server,
        [this](const asio::error_code& error)
        {
            if (!error)
            {
                read_header();
            }
            else
            {
                std::cerr << "[connection] Error: " << error.message() << std::endl;
            }
        });
}

inline void connection::ssl_shutdown(const asio::error_code& ec)
{
    if (ec && ec != asio::error::eof)
        std::cout << ec.message() << std::endl;

    asio::error_code ec2;
    _socket.lowest_layer().close(ec2);
    if (ec2)
        std::cout << ec2.message() << std::endl;
    
}
#endif
