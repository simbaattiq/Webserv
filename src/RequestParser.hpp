#ifndef REQUEST_PARSER_HPP
#define REQUEST_PARSER_HPP


#include <string>
#include <map>
#include <stdexcept>
#include <vector>
#include "ResponseBuilder.hpp"
#include "../include/Server.h"
#include <fstream>
#include "../include/Parser.h"
#include <sys/wait.h>
// #include <cstdio>


using namespace std;



extern Server *srv;


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
    bool _Extract_Request_Data(vector <string> v_request);
    bool _Check_Get_Method(ResponseBuilder & response);
    bool _Check_Post_Method(ResponseBuilder & response);
    bool _isHttpSupported ();
    bool handleUploadData( int & statuscode, string &fullpath);
    string GenerateUploadFile();
    bool saveBodyToFile(const string filepath);
    bool _Check_Delete_Method(ResponseBuilder & response);
    bool _Delete_Content(vector <string > uri);


    // added
    struct UploadedFile
    {
        std::string fieldName;
        std::string fileName;
        std::string contentType;
        std::string content;
    };
        
    std::vector<UploadedFile> _uploadedFiles;
        
    bool parseMultipartFormData(const std::string& contentType);
    std::string extractBoundary(const std::string& contentType);
    void parseMultipartParts(const std::string& boundary);
    


public:
    RequestParser();
    ~RequestParser();

    bool parse(const std::string& raw_request);
    void clear();
    std::vector<std::string> _split(const std::string& str, char delimiter);
    const std::string& getMethod() const;
    const std::string& getUri() const;
    const std::string& getHttpVersion() const;
    const std::map<std::string, std::string>& getHeaders() const;
    const std::string& getBody() const;
    bool ValidateDataForResponse(ResponseBuilder &response);

    

};

#endif