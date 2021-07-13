#include <iostream>

//#define OPEN_SSL

#include "web_server.h"

/**
 * \brief Start a thread in the main function which won't die until we enter a character to stop the server.
 */
void keep_running()
{
    std::getchar();
}

int main()
{
    try
    {
        std::cout << "[main] - Starting web server now\n";

        //Create a web_server which will listen on port 80 on the localhost.
        web_server ws("127.0.0.1", 4445, "cert.pem", "key.pem");
        //We can start it immediately.
        ws.start();
        //We can add endpoints at runtime (which some libraries can't do)
        //In this example we create a GET method on the root path.
        //When someone connects on the path '/' we respond with a bit of HTML.
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
                response.status_code = 200;
                response.status = "OK";
                response.host = "localhost";
                response.content_type = "text/html";
                response.body = "<html><body><h1>HELLO WORLD! from /api/asdf</h1></body></html>";
            });
        //This is a POST example where we reply with some JSON.
        ws.register_function(METHODS::POST, "/api/post_test", [&](const web_request& request_header, web_response& response)
            {
                std::cout << request_header.body << std::endl;
                response.status_code = 200;
                response.status = "OK";
                response.host = "localhost";

                response.content_type = "application/json";
                response.body = "{\"asdf\":\"asdf\"}";
            });

        //This thread is only to keep the server from closing until we enter a character.
        std::thread t([]()
            {
                keep_running();
            });

        t.join();
        //It is VERY important to call the stop function to exit cleanly.
        ws.stop();
    }
    catch(const std::exception& e)
    {
        std::cerr << "[main] - Exception: " << e.what() << std::endl;
    }
}
