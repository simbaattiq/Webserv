#include "ResponseBuilder.hpp"
#include <cstdlib>
#include <string>




ResponseBuilder::ResponseBuilder()
{
    Connection = KEEP_ALIVE;
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

string ResponseBuilder::Replace_html_error_message(string body, int statuscode, string statusMessage)
{
    std::string html = body;

    size_t pos;
    pos = html.find("{{STATUS}}");
    if (pos != std::string::npos)
    {
        string statusStr = intToString(statuscode);
        html.replace(pos, 10, statusStr);
    }
    else
    {
        cout << "{{STATUS}} not found in HTML!" << endl;
    }

    pos = html.find("{{MESSAGE}}");
    if (pos != std::string::npos)
    {
        html.replace(pos, 11, statusMessage);
    }

    return html;
}

string ResponseBuilder::Replace_html_cgi_message(string body, string output)
{
    std::string html = body;

    size_t pos;
    pos = html.find("{{MESSAGE}}");
    if (pos != std::string::npos)
    {
        html.replace(pos, 11, output);
    }

    return html;
}
