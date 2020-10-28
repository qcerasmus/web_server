#pragma once

struct web_response
{
    int status_code = 404;
    std::string status;
    std::string content_type;
    std::string body;
    std::string host;

    std::string protocol;
    std::string version;

    inline std::string to_string()
    {
        std::string response = protocol + "/" + version + " " + std::to_string(status_code) + 
            " " + status;
        response += "\r\nServer: " + host +
            "\r\nContent-Type: " + content_type + 
            "\r\nContent-Length: " + std::to_string(body.length()) +
            "\r\n\r\n" + body;

        return response;
    }
};
