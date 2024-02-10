#pragma once

#include <stdexcept>
#include <string>

#undef DELETE

enum class METHODS
{
    GET,
    POST,
    PUT,
    DELETE
};

namespace methods
{
inline METHODS string_to_method(const std::string &method_string)
{
    if (method_string == "GET")
        return METHODS::GET;
    else if (method_string == "POST")
        return METHODS::POST;
    else if (method_string == "PUT")
        return METHODS::PUT;
    else if (method_string == "DELETE")
        return METHODS::DELETE;
    else
        throw std::runtime_error("That method is not supported");
}

inline std::string method_to_string(const METHODS &method)
{
    switch (method)
    {
    case METHODS::GET:
        return "GET";
    case METHODS::POST:
        return "POST";
    case METHODS::PUT:
        return "PUT";
    case METHODS::DELETE:
        return "DELETE";
    }

    return "UNKNOWN";
}
} // namespace methods

#define DELETE
