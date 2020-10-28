#pragma once

#include <map>
#include <string>
#include <iostream>

#include "src/enums/method.h"

/*
 POST / HTTP/1.1
Content-Type: application/javascript
User-Agent: PostmanRuntime/7.26.5
Accept: 
Cache-Control: no - cache
Postman-Token: 697f6e75-406f-4ab2-8270-b54276990603
Host: localhost
Accept-Encoding: gzip, deflate, br
Connection: keep-alive
Content-Length : 25
 */

struct web_request
{
    METHODS method;
    std::string url;
    std::string protocol;
    std::string version;
    std::map<std::string, std::string> header_values;

    std::string body;

    friend std::ostream& operator<< (std::ostream& os, web_request& request_header)
    {
        os << "Method: " << methods::method_to_string(request_header.method) << std::endl <<
            "URL: " << request_header.url << std::endl <<
            "PROTOCOL: " << request_header.protocol << std::endl <<
            "VERSION: " << request_header.version << std::endl << 
            "Extra headers: " << std::endl;

        for (const auto& header_value : request_header.header_values)
        {
            os << "\t" << header_value.first << ": " << header_value.second << std::endl;
        }

        os << "Body: " << std::endl << request_header.body << std::endl;

        return os;
    }
};
