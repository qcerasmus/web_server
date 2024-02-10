#pragma once

#include <string>
/**
 * \brief This struct is mostly set by the end user to send a response.
 * We default the status_code to 404 and the status to "Not Found".
 * We also default the host, protocol and version to whatever was sent in the request.
 */
struct web_response
{
    int status_code = 404;
    std::string status = "Not Found";
    std::string host;
    std::string protocol;
    std::string version;
    std::string content_type;
    std::string body;

    inline std::string to_string()
    {
        std::string response = protocol + "/" + version + " " + std::to_string(status_code) + " " + status;
        response += "\r\nServer: " + host + "\r\nContent-Type: " + content_type +
                    "\r\nContent-Length: " + std::to_string(body.length()) + "\r\n\r\n" + body;

        return response;
    }
};
