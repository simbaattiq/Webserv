#include "RequestParser.hpp"
#include <sstream>
#include <iostream>
#include <algorithm>
#include "StatusCodes.hpp"

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



int findindex( string s   , char  c)
{
    for (size_t i = 0; i < s.length(); i++)
    {
        if (s[i] == c)
            return (i);
    }
    return (-1);
}


bool RequestParser::_Extract_Request_Data(vector <string> v_request)
{

    if (v_request.empty())
        return (false);

    stringstream iss(v_request[0]);

    iss >> _method >> _uri >> _httpVersion;

    if (_method.empty() || _uri.empty() || _httpVersion.empty())
    {
        cerr << "Invalid request line!" << std::endl;
        return false;
    }
    else
    {
        cout << "Request line : \n";
        cout << getMethod() << " " << getUri()  << " " << getHttpVersion() << endl;
    }

    size_t i = 1;
    bool endofheaders = false;
    for ( ;  i < v_request.size(); i++)
    {
        string line = v_request[i];

        if (line.empty())
        {
            endofheaders = true;
            i++;
            break;
        }

        size_t pos = line.find(':');

        if (pos != string::npos)
        {
            string header_name = trim(line.substr(0, pos));
            string header_value = trim (line.substr(pos + 1));

            _headers[header_name] = header_value;
        }
    }

    if (getMethod() != "Post")
        return (true);

    if (endofheaders && i < v_request.size())
    {
        for (; i < v_request.size(); i++)
        {
            if (!_body.empty())
                _body += '\n';
            _body += v_request[i];
        }
    }
    else
    {
        cout << "Header not closed proprerly\n";
        return (false);
    }
    return (true);
}


bool RequestParser::parse(const std::string& raw_request)
{
    clear();
    vector <string> v_lines;
    string line =  raw_request;
    
    size_t tmp_pos = 0;
    size_t pos = 0;
    while (( pos = line.find("\r\n", tmp_pos)) != string::npos)
    {
        string temp = line.substr(tmp_pos, pos - tmp_pos);
        v_lines.push_back(temp);

        tmp_pos = pos + 2;


        if (tmp_pos < line.length() &&   line.substr(tmp_pos, 2)  == "\r\n" )
        {
            v_lines.push_back( "");
            tmp_pos += 2;

            if (tmp_pos < line.length())
            {
                string rest = line.substr(tmp_pos);
                v_lines.push_back(rest);
            }
            break;
        }
    }

    if (tmp_pos < line.length() && pos==string::npos)
    {
        string rest = line.substr(tmp_pos);

        if (!rest.empty())
            v_lines.push_back(rest);
    }

    if (!_Extract_Request_Data(v_lines))
        return (false);

    return true;
}


bool isFileAccessible(string s)
{
    if (access(s.c_str(), F_OK) == 0)
        return (true);
    else
        return (false);
}

bool CanWeReadAFile(string s)
{
    if (access(s.c_str(), R_OK) == 0)
        return (true);
    else
        return (false);
}


bool RequestParser::_isHttpSupported ()
{
    if (_httpVersion.empty())
        return (false);

    if (!(_httpVersion == "HTTP/1.1"))
        return (false);
    return (true);
}

bool isMethodAuthorised(string Method , vector <string> methods )
{
    for (size_t i = 0; i < methods.size(); i++)
    {
        if (methods[i] == Method)
            return (true);
    }
    return (false);
}


bool isHeaderNameExist(string HeaderName, map <string, string> headers)
{

    if (headers.find(HeaderName) == headers.end() 
        || headers[HeaderName].empty())
        return (false);

    return (true);
}





bool RequestParser::_Check_Get_Method(ResponseBuilder & response)
{

    (void)response;
    int statuscode = 200;
    // chech auto index but is insecure; unsafe  to list 

    if (_uri == "/")
    {
        if (!isMethodAuthorised(_method, srv->location.methods ))
            statuscode = 405;
        else if (!isFileAccessible(srv->location.root))
            statuscode = 404;
        else if (!CanWeReadAFile(srv->location.root + '/'  + srv->location.index))
            statuscode = 403;
        else if (!_isHttpSupported())
            statuscode = 505;
        else if (!isHeaderNameExist("Host", _headers))
            statuscode = 400;
    }
    else
    {
        cout << "URL not found\n";
        statuscode = 400;
    }

    string MessageStatus = StatusCodes::getStatusMessage(statuscode);
    response.setStatus(statuscode, MessageStatus);
    response.addHeader("Content-Type", "text/html");

    if (statuscode == 200)
    {
        response.setBody(srv->location.index_content);
    }
    else
    {
        string body = response.Replace_html_error_message(srv->error.error.html_content, 
                                statuscode, MessageStatus);
        response.setBody(body);
        // serve the error html;
    }
    
    return (true);
}


bool   RequestParser:: ValidateDataForResponse(ResponseBuilder &response)
{

    if (_method == "GET")
    {
        response.Method = response.GET;
        _Check_Get_Method(response);
        
    }

    else if (_method == "DELETE")
    {
        response.Method = response.DELETE;
    }
    else if (_method == "POST")
    {
        response.Method = response.POST;
    }
    else
    {
        cerr << "error in METHOD\n";
        response.Method = response.ERROR;
        return (false);
    }

    if (!_headers["Connection"].empty())
    {
        if (_headers["Connection"] == "close")
            response.Connection = response.CLOSE;
    }
    return (true);
}





const std::string& RequestParser::getMethod() const { return _method; }

const std::string& RequestParser::getUri() const { return _uri; }

const std::string& RequestParser::getHttpVersion() const { return _httpVersion; }

const std::map<std::string, std::string>& RequestParser::getHeaders() const { return _headers; }

const std::string& RequestParser::getBody() const { return _body; }

