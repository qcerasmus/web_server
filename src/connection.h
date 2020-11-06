#pragma once

#include <asio.hpp>

#include "structs/request.h"
#include "structs/response.h"

class connection : public std::enable_shared_from_this<connection>
{
public:
    connection(asio::io_context& io_context, asio::ip::tcp::socket socket);
    connection(const connection& c) = delete;
    connection(const connection&& c) = delete;
    connection& operator=(const connection& other) = delete;
    connection& operator=(const connection&& other) = delete;
    ~connection();

    std::function<void(web_request&, web_response&)> done_reading_function;
    bool close_me = false;

protected:
    asio::io_context& _io_context;
    asio::ip::tcp::socket _socket;

private:
    void read_header();
    void read_body(const std::size_t& body_length, web_request& request_header) const;

    static void header_helper(std::string& header, web_request& request_header);

    asio::streambuf _socket_buffer;
    std::string _response_string;
};

inline connection::connection(asio::io_context& io_context, asio::ip::tcp::socket socket)
    : _io_context(io_context),
    _socket(std::move(socket))
{
    read_header();
}

inline connection::~connection()
{
    while (!close_me)
        std::this_thread::sleep_for(std::chrono::nanoseconds(10));
    _socket_buffer.consume(_socket_buffer.size());
    if (_socket.is_open())
        _socket.close();
}

inline void connection::read_header()
{
    asio::async_read_until(_socket, _socket_buffer, "\r\n\r\n",
        [&](std::error_code ec, std::size_t length_read)
        {
            //We have read some data.
            if (!ec)
            {
                if (length_read >= 0)
                {
                    auto header = std::string(
                        asio::buffer_cast<const char*>(_socket_buffer.data()),
                        length_read);
                    _socket_buffer.consume(length_read);

                    web_request request_header{};
                    header_helper(header, request_header);
                    //std::cout << request_header;

                    std::size_t length = 0;
                    if (request_header.header_values.find("Content-Length") != request_header.header_values.end())
                    {
                        length = std::stol(request_header.header_values["Content-Length"]);
                    }

                    read_body(length, request_header);
                    web_response response;
                    response.protocol = request_header.protocol;
                    response.version = request_header.version;
                    done_reading_function(request_header, response);
                    _response_string = response.to_string();
                    asio::async_write(_socket, asio::buffer(_response_string, _response_string.length()),
                        [&](std::error_code ec, std::size_t bytes_written)
                        {
                            if (!ec)
                            {
                                //std::cout << "[connection] replied to client" << std::endl;
                                
                            }
                            else
                                std::cerr << "[connection] Error while replying: " << ec.message() << std::endl;

                            _socket.close();
                            close_me = true;
                        });
                }   
            }
            else
            {
                std::cerr << "[connection] Error: " << ec.message() << std::endl;
                close_me = true;
                _socket.close();
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

        //std::cout << "[connection] body: " << body << std::endl;

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

