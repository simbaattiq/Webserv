#ifndef RESPONSE_BUILDER_HPP
#define RESPONSE_BUILDER_HPP

#include <string>
#include <vector>
#include <utility>
#include <sstream>

class ResponseBuilder
{
private:
    std::string _statusCode;
    std::string _statusMessage;
    std::vector<std::pair<std::string, std::string> > _headers;
    std::string _body;


public:
    ResponseBuilder();
    ~ResponseBuilder();
    ResponseBuilder(const ResponseBuilder&);
    ResponseBuilder& operator=(const ResponseBuilder&);

    void setStatus(int statusCode, const std::string& statusMessage);
    void addHeader(const std::string& name, const std::string& value);
    void setBody(const std::string& body);

    std::string build() const;
    void clear();

};

#endif