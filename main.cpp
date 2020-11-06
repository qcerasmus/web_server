#include <iostream>

#include "web_server.h"

void keep_running()
{
    auto key = std::getchar();
}

int main()
{
    try
    {
        std::cout << "[main] - Starting web server now\n";

        web_server ws("127.0.0.1", 80);
        ws.start();
        ws.register_function(METHODS::GET, "/", [&](const web_request& request_header, web_response& response)
            {
                response.status_code = 200;
                response.status = "OK";
                response.host = "localhost";
                response.content_type = "text/html";
                response.body = "<html><body><h1>HELLO WORLD from root!</h1></body></html>";
            });
        ws.register_function(METHODS::GET, "/api/asdf", [&](const web_request& request_header, web_response& response)
            {
                std::cout << "FROM api/asdf: " << request_header.url << std::endl;

                response.status_code = 200;
                response.status = "OK";
                response.host = "localhost";
                response.content_type = "text/html";
                response.body = "<html><body><h1>HELLO WORLD! from /api/asdf</h1></body></html>";
            });
        ws.register_function(METHODS::POST, "/api/post_test", [&](const web_request& request_header, web_response& response)
            {
                std::cout << request_header.body << std::endl;
                response.status_code = 200;
                response.status = "OK";
                response.host = "localhost";

                response.content_type = "application/json";
                response.body = "{\"asdf\":\"asdf\"}";
            });

        std::thread t([]()
            {
                keep_running();
            });

        t.join();
        ws.stop();
    }
    catch(const std::exception& e)
    {
        std::cerr << "[main] - Exception: " << e.what() << std::endl;
    }
}
