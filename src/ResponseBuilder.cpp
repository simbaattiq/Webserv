#include "ResponseBuilder.hpp"
#include <cstdlib>



ResponseBuilder::ResponseBuilder() 
{
    clear();
}

ResponseBuilder::~ResponseBuilder() 
{
    // Destructor
}


std::string intToString(int value) 
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

void ResponseBuilder::clear() 
{
    _statusCode.clear();
    _statusMessage.clear();
    _headers.clear();
    _body.clear();
}

void ResponseBuilder::setStatus(int statusCode, const std::string& statusMessage) 
{
    _statusCode = intToString(statusCode);
    _statusMessage = statusMessage;
}

void ResponseBuilder::addHeader(const std::string& name, const std::string& value) 
{
    _headers.push_back(std::make_pair(name, value));
}

void ResponseBuilder::setBody(const std::string& body) 
{
    _body = body;
}

std::string ResponseBuilder::build() const 
{
    std::ostringstream oss;

    oss << "HTTP/1.1 " << _statusCode << " " << _statusMessage << "\r\n";

    // Headers
    // add Content-Length header if body exists and not already present
    bool contentLengthSet = false;
    for (size_t i = 0; i < _headers.size(); ++i) 
    {
        if (_headers[i].first == "Content-Length") 
        {
            contentLengthSet = true;
            break;
        }
    }
    if (!contentLengthSet && !_body.empty()) 
    {
        oss << "Content-Length: " << intToString(_body.length()) << "\r\n";
    }

    for (size_t i = 0; i < _headers.size(); ++i) 
    {
        oss << _headers[i].first << ": " << _headers[i].second << "\r\n";
    }

    // End of headers
    oss << "\r\n";

    oss << _body;

    return oss.str();
}
