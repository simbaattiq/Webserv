#include "RequestParser.hpp"
#include <sstream>
#include <iostream>
#include <algorithm>

RequestParser::RequestParser() {
    clear();
}

RequestParser::~RequestParser() {
    // Destructor
}

void RequestParser::clear()
{
    _method.clear();
    _uri.clear();
    _httpVersion.clear();
    _headers.clear();
    _body.clear();
}

size_t RequestParser::find_crlf(const std::string& s, size_t pos) const
{
    return s.find("\r\n", pos);
}

size_t RequestParser::find_crlfcrlf(const std::string& s, size_t pos) const
{
    return s.find("\r\n\r\n", pos);
}


std::string RequestParser::trim(const std::string& str) const 
{
    size_t first = str.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) 
    {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}


bool RequestParser::parse(const std::string& raw_request)
{
    clear();

    size_t first_crlf = find_crlf(raw_request);
    if (first_crlf == std::string::npos)
    {
        return false;
    }

    std::string request_line = raw_request.substr(0, first_crlf);
    std::istringstream iss(request_line);

    std::string method_str, uri_str, httpVersion_str;

    iss >> method_str >> uri_str >> httpVersion_str;

    _method = trim(method_str);
    _uri = trim(uri_str);
    _httpVersion = trim(httpVersion_str);

    if (_method.empty() || _uri.empty() || _httpVersion.empty())
    {
        return false;
    }

    size_t header_start = first_crlf + 2; // skip first CRLF
    size_t body_start = find_crlfcrlf(raw_request, header_start);

    if (body_start == std::string::npos)
    {
        // if no double CRLF found == no body or malformed request
        body_start = raw_request.length(); // treat the rest as headers
    }


    size_t headers_length = 0;
    if (body_start > header_start) {
        headers_length = body_start - header_start;
    }

    std::string headers_str = raw_request.substr(header_start, headers_length);
    std::istringstream header_iss(headers_str);
    std::string line;

    while (std::getline(header_iss, line) && line != "\r")
    {
        if (line.length() < 2) 
            continue;

        // remove trailing '\r' if present (from \r\n)
        if (line[line.length() - 1] == '\r')
        {
            line = line.substr(0, line.length() - 1);
        }

        size_t colon_pos = line.find(":");
        if (colon_pos != std::string::npos)
        {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);

            _headers[trim(key)] = trim(value);
        }
    }

    if (body_start != raw_request.length()) 
    {
        _body = raw_request.substr(body_start + 4); // skip double CRLF
    }

    return true;
}

const std::string& RequestParser::getMethod() const { return _method; }

const std::string& RequestParser::getUri() const { return _uri; }

const std::string& RequestParser::getHttpVersion() const { return _httpVersion; }

const std::map<std::string, std::string>& RequestParser::getHeaders() const { return _headers; }

const std::string& RequestParser::getBody() const { return _body; }

