#include "StatusCodes.hpp"

std::string StatusCodes::getStatusMessage(int statusCode) 
{
    static std::map<int, std::string> statusMessages;
    
    if (statusMessages.empty()) 
    {
        // 2xx Success
        statusMessages[200] = "OK";
        statusMessages[201] = "Created";
        statusMessages[204] = "No Content";
        
        // 3xx Redirection
        statusMessages[301] = "Moved Permanently";
        statusMessages[302] = "Found";
        statusMessages[304] = "Not Modified";
        
        // 4xx Client Errors
        statusMessages[400] = "Bad Request";
        statusMessages[401] = "Unauthorized";
        statusMessages[403] = "Forbidden";
        statusMessages[404] = "Not Found";
        statusMessages[405] = "Method Not Allowed";
        statusMessages[408] = "Request Timeout";
        statusMessages[411] = "Length Required";
        statusMessages[413] = "Payload Too Large";
        statusMessages[414] = "URI Too Long";
        statusMessages[415] = "Unsupported Media Type";
        
        // 5xx Server Errors
        statusMessages[500] = "Internal Server Error";
        statusMessages[501] = "Not Implemented";
        statusMessages[502] = "Bad Gateway";
        statusMessages[503] = "Service Unavailable";
        statusMessages[504] = "Gateway Timeout";
        statusMessages[505] = "HTTP Version Not Supported";
    }
    
    std::map<int, std::string>::const_iterator it = statusMessages.find(statusCode);
    if (it != statusMessages.end()) 
    {
        return it->second;
    }
    return "Unknown Status";
}

// std::string StatusCodes::getErrorPage(int statusCode, const std::string& message) 
// {
//     std::ostringstream html;
//     std::string statusMsg = getStatusMessage(statusCode);
//     std::string customMsg = message.empty() ? statusMsg : message;
    
//     html << "<!DOCTYPE html>\n"
//          << "<html>\n"
//          << "<head>\n"
//          << "    <title>" << statusCode << " " << statusMsg << "</title>\n"
//          << "</head>\n"
//          << "<body>\n"
//          << "    <h1>" << statusCode << " " << statusMsg << "</h1>\n"
//          << "    <p>" << customMsg << "</p>\n"
//          << "    <hr>\n"
//          << "    <p>webserv/1.0</p>\n"
//          << "</body>\n"
//          << "</html>";
    
//     return html.str();
// }
