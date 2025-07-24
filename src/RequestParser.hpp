#ifndef REQUEST_PARSER_HPP
#define REQUEST_PARSER_HPP

#include <string>
#include <map>
#include <stdexcept>

class RequestParser 
{

private:
    std::string _method;
    std::string _uri;
    std::string _httpVersion;
    std::map<std::string, std::string> _headers;
    std::string _body;

    size_t find_crlf(const std::string& s, size_t pos = 0) const;
    size_t find_crlfcrlf(const std::string& s, size_t pos = 0) const;
    std::string trim(const std::string& str) const;


public:
    RequestParser();
    ~RequestParser();

    bool parse(const std::string& raw_request);
    void clear();

    const std::string& getMethod() const;
    const std::string& getUri() const;
    const std::string& getHttpVersion() const;
    const std::map<std::string, std::string>& getHeaders() const;
    const std::string& getBody() const;



};

#endif