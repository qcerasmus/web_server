#pragma once

#include <map>
#include <string>
#include <iostream>

#include "../enums/method.h"

/**
 * \brief This struct will contain everything that was sent in the request.
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
