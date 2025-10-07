#ifndef STATUS_CODES_HPP
#define STATUS_CODES_HPP

#include <map>
#include <string>

class StatusCodes 
{
public:
    // 2xx Success
    static const int OK = 200;
    static const int CREATED = 201;
    static const int NO_CONTENT = 204;
    
    // 3xx Redirection
    static const int MOVED_PERMANENTLY = 301;
    static const int FOUND = 302;
    static const int NOT_MODIFIED = 304;
    
    // 4xx Client Errors
    static const int BAD_REQUEST = 400;
    static const int UNAUTHORIZED = 401;
    static const int FORBIDDEN = 403;
    static const int NOT_FOUND = 404;
    static const int METHOD_NOT_ALLOWED = 405;
    static const int REQUEST_TIMEOUT = 408;
    static const int LENGTH_REQUIRED = 411;
    static const int PAYLOAD_TOO_LARGE = 413;
    static const int URI_TOO_LONG = 414;
    static const int UNSUPPORTED_MEDIA_TYPE = 415;
    
    // 5xx Server Errors
    static const int INTERNAL_SERVER_ERROR = 500;
    static const int NOT_IMPLEMENTED = 501;
    static const int BAD_GATEWAY = 502;
    static const int SERVICE_UNAVAILABLE = 503;
    static const int GATEWAY_TIMEOUT = 504;
    static const int HTTP_VERSION_NOT_SUPPORTED = 505;

    static std::string getStatusMessage(int statusCode);
    static std::string getErrorPage(int statusCode, const std::string& message = "");
};

#endif
