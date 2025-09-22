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

    if (getMethod() != "POST")
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
    vector<string> v_lines;
    string line = raw_request;
    
    size_t tmp_pos = 0;
    size_t pos = 0;
    while ((pos = line.find("\r\n", tmp_pos)) != string::npos)
    {
        string temp = line.substr(tmp_pos, pos - tmp_pos);
        v_lines.push_back(temp);

        tmp_pos = pos + 2;

        // check for end of headers (empty line)
        if (tmp_pos < line.length() && line.substr(tmp_pos, 2) == "\r\n")
        {
            v_lines.push_back("");
            tmp_pos += 2;

            // Add remaining content as body
            if (tmp_pos < line.length())
            {
                string rest = line.substr(tmp_pos);
                v_lines.push_back(rest);
            }
            break;
        }
    }

    if (tmp_pos < line.length() && pos == string::npos)
    {
        string rest = line.substr(tmp_pos);
        if (!rest.empty())
            v_lines.push_back(rest);
    }

    if (!_Extract_Request_Data(v_lines))
        return false;

    //// // /
    if (_method == "POST" && _headers.find("Content-Type") != _headers.end() && 
        _headers["Content-Type"].find("multipart/form-data") != std::string::npos)
    {
        // Parse multipart form data
        if (!parseMultipartFormData(_headers["Content-Type"]))
        {
            cout << "Failed to parse multipart form data" << endl;
            return false;
        }
        // std::cout << "==> \n" << _headers["Content-Type"] << "\n";
    }
    // std::cout << "[BODY]:\n" << _body << "\n";
    
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

bool CanWeWriteFile(string s)
{
    if (access(s.c_str(), W_OK) == 0)
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

std::vector<std::string> RequestParser::_split(const std::string& str, char delimiter)
 {
    std::vector<std::string> result;
    std::string temp;
    for (size_t i = 0; i < str.length(); ++i)
    {
        if (str[i] == delimiter){
            if (!temp.empty())
                result.push_back(temp);
            temp.clear();
        } else {
            temp += str[i];
        }
    }
    if (!temp.empty())
        result.push_back(temp);
    return result;
}


bool _isFileOpend (string filename)
{
    
    ifstream File (filename.c_str());

    if (!File.is_open())
    {
        std::cerr << "Failed to open file : " 
            << filename << "\n";
        return (false);
    }

    return (true);
}


bool execute_cgi(string  & response, string arg, string path)
{
    pid_t pid  ;
    int fd[2];
    int status = 200;
    // int stdin = dup (  STDIN_FILENO);
    // int stdout = dup (  STDOUT_FILENO);

    path = srv->cgi_bin.root + '/' + path;


    try
    {

        if (pipe(fd) == -1)
        {
            status = 500; // internal;
            return (false);
        }

       pid  = fork();
       if (pid == 0)
       {
            cout << "i will execute \n";
            close (fd[0]);
            dup2 (fd[1], STDOUT_FILENO);
            close (fd[1]);
            char *argv[] = { (char*)srv->cgi_bin.cgi_pass.c_str(), (char*)path.c_str(), (char*)arg.c_str(),  (char*)NULL };
            char *envp[] = { NULL };
            execve((char*)srv->cgi_bin.cgi_pass.c_str(), argv, envp);
            perror("execve");
            exit(1);
       }
       else if (pid > 0)
       {
            close (fd[1]);
            ssize_t n;
            char buffer[4096];
            response.clear();

            while ((n = read(fd[0], buffer, sizeof(buffer))) > 0)
            {
                response.append(buffer, n);
            }

            close (fd[0]);
            int status;
            waitpid(pid, &status, 0) ;

            return (true);
       }
       else
       {
            cout << "fork failed\n";
            return (false);
       }
    }
    catch (exception &e)
    {
        cout << e.what() << endl;
        return (false);
    }
    (void)status;
    return (true);
}


bool RequestParser::_Check_Get_Method(ResponseBuilder & response)
{
    string imagedata = "";
    bool isimagerequested = false;
    bool iscgi              =  false;
    string output="";


    vector <string> v =  _split (_uri,'/' );

    (void)response;
    int statuscode = 200;


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

        cout << "status code for " << _uri << " is " << statuscode << endl;
    }
    else if (v[0] == "images")
    {   
        isimagerequested = true;
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
        
        else
        {
            string path=srv->location_images.root+ '/';
            if (v.size() == 1)
                path += "defaultimg.jpeg";
            else
            {
                for (size_t i = 1; i < v.size(); i++)
                    path += v[i];
            }

            std::ifstream imagefile(path.c_str(), std::ios::binary);

            if (imagefile.is_open())
            {
                std::ostringstream ss;
                ss << imagefile.rdbuf();
                imagedata = ss.str();
                imagefile.close();
            }
            else
            {
                cout << "cannot open " << path << endl;
                statuscode = 404;
            }
        }
    }
    else if (_uri.find("cgi-bin") != std::string::npos)
    {

         if (!isMethodAuthorised(_method, srv->cgi_bin.methods ))
            statuscode = 405;
        else if (!isFileAccessible(srv->cgi_bin.root))
            statuscode = 404;
        else if (!CanWeReadAFile(srv->location.root + '/'  + srv->location.index))
            statuscode = 403;
        else if (!_isHttpSupported())
            statuscode = 505;
        else if (!isHeaderNameExist("Host", _headers))
            statuscode = 400;

        else
        {

        
                iscgi = true;

                string scriptpath = _uri.substr(strlen("/cgi-bin/"));
                size_t querypos = scriptpath.find('?');
                string arg = "";

                if (querypos != string::npos)
                {
                    scriptpath = scriptpath.substr(0, querypos);
                }

            
                string pathscript = srv->cgi_bin.root + "/" + scriptpath;
                if (!_isFileOpend(pathscript))
                {
                    std::cout << "cannot open script: " << pathscript << std::endl;
                    statuscode = 404;
                }
                else
                {
                    size_t pos = _uri.find('?');
                    string arg = "";
                
                    if (pos != string::npos)
                    {
                        arg = _uri.substr(pos + 1, _uri.length());
                    }
                
                    vector <string> v_arg = _split (arg, '=');
                
                    if (v_arg.size() != 2)
                    {
                        statuscode = 400;
                    }
                    else
                    {
                        if (execute_cgi(output, v_arg[1], scriptpath))
                        {
                            cout << "execution pass\n";
                        }
                        else
                        {
                            cout << "cgi error ";
                            statuscode = 400;
                        }
                    }
                }
            }
    }
    else if (_uri == "favicon.ico")
    {
        cout << "hello this is favicon.ico" << endl;
    }
    else
    {
        cout << "URL not found\n";
        statuscode = 400;
    }

    string MessageStatus = StatusCodes::getStatusMessage(statuscode);
    response.setStatus(statuscode, MessageStatus);
    

    if (isimagerequested)
    {
        if (statuscode == 200)
        {
            std::string contentType = "application/octet-stream";
            if (imagedata.find(".jpg") != std::string::npos || imagedata.find(".jpeg") != std::string::npos)
                response.addHeader("Content-Type",  "image/jpeg");
            else if (imagedata.find(".png") != std::string::npos)
                response.addHeader("Content-Type" , "image/png");
            else if (imagedata.find(".gif") != std::string::npos)
                response.addHeader("Content-Type", "image/gif");
            
            response.setBody(imagedata);
        }
        else
        {
            response.addHeader("Content-Type", "text/html");
            string body = response.Replace_html_error_message(srv->error.error.html_content, 
                                statuscode, MessageStatus);
            response.setBody(body);
        }
    }

    else if (statuscode == 200)
    {
        response.addHeader("Content-Type", "text/html");
        if (iscgi)
        {
            iscgi =false;
            string body = response.Replace_html_cgi_message(srv->cgi_bin.htmlcontent, output );
            response.setBody(body);

        }
        else
        {
            response.addHeader("Content-Type", "text/html");
            response.setBody(srv->location.index_content);
        }
        
    }

    else
    {
        string body = response.Replace_html_error_message(srv->error.error.html_content, 
                                statuscode, MessageStatus);
        response.setBody(body);
    }
    
    return (true);
}

string RequestParser::GenerateUploadFile()
{
    string filename;

    time_t now = time(0);
    ostringstream oss;
    oss << "upload_" << now;

    string contenttype = _headers["Content-Type"];

    if (contenttype.find("text/plain") != string::npos)
        oss << ".txt";
    else if (contenttype.find("text/html") != string::npos)
        oss << ".html";
    else if (contenttype.find("application/json") != string::npos)
        oss << ".json";
    else if (contenttype.find("image/jpeg") != string::npos)
        oss << ".jpeg";
    else if (contenttype.find("image/png") != string::npos)
        oss << ".png";
    else
        oss << ".bin";
    return (oss.str());
}

bool RequestParser::saveBodyToFile(const string filepath)
{
    ofstream file(filepath.c_str(), ios::binary);

    if (!file.is_open())
        return (false);


    file.write (_body.c_str(), _body.size());
    file.close();
    return (true);
}

bool RequestParser::handleUploadData( int & statuscode, string &fullpath)
{
    try
    {

        string filename = GenerateUploadFile();
        fullpath  = srv->location_upload.root + "/" + filename;

        if ((!saveBodyToFile(fullpath)))
        {
            cout << "Faild to save uploaded data" << endl;
            statuscode = 500;
            return (false);
        }

        cout << "Data saved to : " << fullpath << endl;
        statuscode = 201;
        return (true);


    }
    catch(const std::exception& e)
    {
        cout << "Upload processing error " <<  e.what() << endl;
        statuscode = 500;
        return (false);
    }
    
}




// new updated method
bool RequestParser::_Check_Post_Method(ResponseBuilder & response)
{
    int statuscode = 400;
    string fullpath = "";

    if (_uri == "/upload")
    {
        if (!isMethodAuthorised(_method, srv->location_upload.methods))
        {
            statuscode = StatusCodes::METHOD_NOT_ALLOWED;
            response.addHeader("Allow", "GET, POST");
        }
        else if (!isFileAccessible(srv->location_upload.root))
        {
            statuscode = StatusCodes::NOT_FOUND;
        }
        else if (!CanWeWriteFile(srv->location_upload.root))
        {
            statuscode = StatusCodes::FORBIDDEN;
        }
        else if (!_isHttpSupported())
        {
            statuscode = StatusCodes::HTTP_VERSION_NOT_SUPPORTED;
        }
        else if (!isHeaderNameExist("Host", _headers))
        {
            statuscode = StatusCodes::BAD_REQUEST;
        }
        else if (!isHeaderNameExist("Content-Length", _headers))
        {
            statuscode = StatusCodes::LENGTH_REQUIRED;
        }
        else if (!isHeaderNameExist("Content-Type", _headers))
        {
            statuscode = StatusCodes::BAD_REQUEST;
        }

        else
        {
            size_t expectedLength = static_cast<size_t>(atoi(_headers["Content-Length"].c_str()));
            
            if (_body.size() != expectedLength)
            {
                statuscode = StatusCodes::BAD_REQUEST;
            } 
            else
            {
                //multipart form data request
                if (_headers["Content-Type"].find("multipart/form-data") != std::string::npos)
                {
                    if (parseMultipartFormData(_headers["Content-Type"]))
                    {
                        // Save each uploaded file
                        bool allFilesSaved = true;
                        std::vector<std::string> savedPaths;
                        
                        for (size_t i = 0; i < _uploadedFiles.size(); i++)
                        {
                            if (!_uploadedFiles[i].fileName.empty())
                            {
                                std::string filePath = srv->location_upload.upload_store + "/" + _uploadedFiles[i].fileName;
                                std::ofstream file(filePath.c_str(), std::ios::binary);
                                
                                if (file.is_open())
                                {
                                    file.write(_uploadedFiles[i].content.c_str(), _uploadedFiles[i].content.length());
                                    file.close();
                                    savedPaths.push_back(filePath);
                                }
                                else
                                {
                                    allFilesSaved = false;
                                    break;
                                }
                            }
                        }
                        
                        if (allFilesSaved)
                        {
                            statuscode = StatusCodes::CREATED;
                            
                            // Add the list of saved files to the response
                            std::string responseBody = "<html><body><h1>Files uploaded successfully</h1><ul>";
                            for (size_t i = 0; i < savedPaths.size(); ++i) //// // /
                            {
                                responseBody += "<li>" + savedPaths[i] + "</li>";
                            }
                            responseBody += "</ul></body></html>";
                            
                            response.setBody(responseBody);
                            response.addHeader("Content-Type", "text/html");
                        }
                        else
                        {
                            statuscode = StatusCodes::INTERNAL_SERVER_ERROR;
                        }
                    }
                    else
                    {
                        statuscode = StatusCodes::BAD_REQUEST;
                    }
                }
                else
                {
                    // regular POST data as before
                    if (handleUploadData(statuscode, fullpath)) 
                    {
                        // Success
                    }
                }
            }
        }
    }
    else if (_uri.find("cgi-bin") != std::string::npos)
    {
        cout << "Les amis cgi bin by post just called .\n";

        if (!isMethodAuthorised(_method, srv->cgi_bin.methods))
        {
            statuscode = StatusCodes::METHOD_NOT_ALLOWED;
            response.addHeader("Allow", "GET, POST");
        }
        else if (!isFileAccessible(srv->cgi_bin.root))
        {
            statuscode = StatusCodes::NOT_FOUND;
        }
        else if (!_isHttpSupported())
        {
            statuscode = StatusCodes::HTTP_VERSION_NOT_SUPPORTED;
        }
        else if (!isHeaderNameExist("Host", _headers))
        {
            statuscode = StatusCodes::BAD_REQUEST;
        }
        else if (!isHeaderNameExist("Content-Length", _headers))
        {
            statuscode = StatusCodes::LENGTH_REQUIRED;
        }
        else if (!isHeaderNameExist("Content-Type", _headers))
        {
            statuscode = StatusCodes::BAD_REQUEST;
        }
        else
        {

            size_t expectedLength = static_cast<size_t>(atoi(_headers["Content-Length"].c_str()));
            
            if (_body.size() != expectedLength)
            {
                statuscode = StatusCodes::BAD_REQUEST;
            }
            
            else  if (_headers.find("Content-Type") != _headers.end())
            {
                string cgi_output;


                // CHECK SCRIPT PATH IF AVAILAIBLE AND CHECK FOR MSG;

                string scriptpath = _uri.substr(strlen("/cgi_bin/") , _uri.length());


                string pathscript = srv->cgi_bin.root + "/" + scriptpath;
                    // std::cout << " script path: " << pathscript << std::endl;

                if (!_isFileOpend(pathscript))
                {
                    std::cout << "cannot open script: " << pathscript << std::endl;
                    statuscode = 404;
                }
                else if (!_body.empty())
                {
                    vector <string> v_arg = _split(_body, '=');

                    if (v_arg.size() == 2 && v_arg[0] == "msg")
                    {

                        if (execute_cgi(cgi_output, v_arg[1], scriptpath))
                        {
                            statuscode = 200;
                            response.setBody(cgi_output);
                            response.addHeader("Content-Type", "text/plain");
                        }
                        else
                        {
                            statuscode = StatusCodes::INTERNAL_SERVER_ERROR;
                        }
                    }
                    else
                    {
                        statuscode = StatusCodes::BAD_REQUEST;
                    }
                }
                else
                {
                    statuscode = StatusCodes::BAD_REQUEST;
                }
            }
            else
            {
                statuscode = StatusCodes::UNSUPPORTED_MEDIA_TYPE;

            }
        }
    }
    else
    {
        statuscode = StatusCodes::NOT_FOUND;
    }

    if (statuscode == StatusCodes::CREATED)
    {
        response.setStatus(statuscode, StatusCodes::getStatusMessage(statuscode));
        if (fullpath != "")
            response.addHeader("Location", fullpath);
    }
    else if (statuscode >= 400)
    {
        std::string errorpage = srv->error.error.html_content;
        response.setStatus(statuscode, StatusCodes::getStatusMessage(statuscode));
        response.addHeader("Content-Type", "text/html");
        response.setBody(response.Replace_html_error_message(errorpage, statuscode, StatusCodes::getStatusMessage(statuscode)));
    }
    else
    {
        response.setStatus(statuscode, StatusCodes::getStatusMessage(statuscode));
    }

    return true;
}


string AssembleWord(vector < string > v_uri, string wordtonotadd)
{
    string path = "";


    for (size_t i=0; i < v_uri.size() ; i++)
    {
        if (v_uri[i] != wordtonotadd)
        {
             path += v_uri[i];

             if ( i < v_uri.size() -1)
                path += '/';
        }
           
        
    }
    return (path);
}

bool RequestParser::_Delete_Content(vector < string > v_uri)
{
    try
    {
        string fullpath = AssembleWord(v_uri, "uploads");

        if (remove((srv->location_upload.root + '/' + fullpath).c_str() )== 0)
        {
            cout << "File Deleted Succuss " << srv->location_upload.root + '/' + fullpath << endl;
            return (true);
        }
        else
        {
            cout << "Failed to delete file: " << srv->location_upload.root + '/' + fullpath << endl;
            return (false);
        }
    }
    catch (exception &e)
    {
        cout << "error  catched  : " << e.what() << endl;
        return (false);
    }
}



bool RequestParser::_Check_Delete_Method(ResponseBuilder & response)
{

    (void)response;
    int statuscode = 200;


    cout << "****************uri  : "  <<   _uri << endl;

    Parser prs ("");
    vector <string > v_uri =prs. _split(_uri, '/');

    
    if (v_uri.size() <= 1)
        statuscode=400;
    

    else if (v_uri[0] == "uploads")
    {
        if (!isMethodAuthorised(_method, srv->location_upload.methods ))
            statuscode = 405;



        /* we have an issue in this section */

        // // else if (!isFileAccessible("/var/www/" + _uri))
        // else if (!isFileAccessible("/var/www/" + _uri))
        // {
        //     cout << "**404** : _uri: " << _uri << '\n';
        //     statuscode = 404;
        // }
        // else if (!CanWeWriteFile("/var/www/" + _uri))
        //     statuscode = 403;


        else if (!_isHttpSupported())
            statuscode = 505;
        else if (!isHeaderNameExist("Host", _headers))
            statuscode = 400;
    }
    else
    {
        cout << "URL not found\n";
        // cout << "V_URL[0]: ==> " << v_uri[0] << "\n";  
        statuscode = 400;
    }

    string MessageStatus = StatusCodes::getStatusMessage(statuscode);
    response.setStatus(statuscode, MessageStatus);
    response.addHeader("Content-Type", "text/html");

    if (statuscode == 200)
    {
        if (!_Delete_Content(v_uri))
        {
            statuscode = 500;
        }
        else
            response.setBody("File Deleted Successfully\n");
    }
    // if (statuscode != 200)
    // {
    //     response.setBody("Cannot Delete content\n");
    // }

    prs.~Parser();
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
        _Check_Delete_Method(response);
    }
    else if (_method == "POST")
    {
        response.Method = response.POST;
        _Check_Post_Method(response);
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

///////////////////////

bool RequestParser::parseMultipartFormData(const std::string& contentType)
{
    std::string boundary = extractBoundary(contentType);
    if (boundary.empty())
    {
        std::cerr << "no boundry found\n";
        return false;
    }
    
    parseMultipartParts(boundary);
    return !_uploadedFiles.empty();
}

std::string RequestParser::extractBoundary(const std::string& contentType)
{
    size_t pos = contentType.find("boundary=");
    if (pos == std::string::npos)
    {
        return "";
    }
    
    return contentType.substr(pos + 9);
}

void RequestParser::parseMultipartParts(const std::string& boundary)
{
    std::string delimiter = "--" + boundary;
    size_t pos = _body.find(delimiter);
    
    while (pos != std::string::npos)
    {
        size_t nextPos = _body.find(delimiter, pos + delimiter.length());
        if (nextPos == std::string::npos)
        {
            break;
        }
        
        std::string part = _body.substr(pos + delimiter.length() + 2,
                                      nextPos - (pos + delimiter.length() + 2));
        
        // Parse the part headers and content
        size_t headerEnd = part.find("\r\n\r\n");
        if (headerEnd != std::string::npos)
        {
            std::string headers = part.substr(0, headerEnd);
            std::string content = part.substr(headerEnd + 4);
            
            size_t dispositionPos = headers.find("Content-Disposition:");

            if (dispositionPos != std::string::npos)
            {
                size_t filenamePos = headers.find("filename=\"", dispositionPos);
                size_t namePos = headers.find("name=\"", dispositionPos);
                
                UploadedFile file;
                
                if (namePos != std::string::npos)
                {
                    size_t nameEnd = headers.find("\"", namePos + 6);
                    if (nameEnd != std::string::npos)
                    {
                        file.fieldName = headers.substr(namePos + 6, nameEnd - (namePos + 6));
                    }
                }
                
                // Extract filename if it exists
                if (filenamePos != std::string::npos)
                {
                    size_t filenameEnd = headers.find("\"", filenamePos + 10);
                    if (filenameEnd != std::string::npos)
                    {
                        file.fileName = headers.substr(filenamePos + 10, filenameEnd - (filenamePos + 10));
                    }
                }
                
                size_t contentTypePos = headers.find("Content-Type:");
                if (contentTypePos != std::string::npos)
                {
                    size_t contentTypeEnd = headers.find("\r\n", contentTypePos);
                    if (contentTypeEnd != std::string::npos)
                    {
                        file.contentType = trim(headers.substr(contentTypePos + 13, contentTypeEnd - (contentTypePos + 13)));
                    }
                }
                
                // If there's a filename, it's a file upload
                if (!file.fileName.empty())
                {
                    file.content = content;
                    _uploadedFiles.push_back(file);
                }
            }
        }
        
        pos = nextPos;
    }
}