#include "RequestParser.hpp"
#include <sstream>
#include <iostream>
#include <algorithm>
#include "StatusCodes.hpp"
#include <dirent.h>
#include <sys/stat.h>
#include <ctime>
#include <iomanip>


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


// bool RequestParser::_Extract_Request_Data(std::vector<std::string> v_request) {
//     if (v_request.empty())
//         return false;

//     std::stringstream iss(v_request[0]);
//     iss >> _method >> _uri >> _httpVersion;

//     if (_method.empty() || _uri.empty() || _httpVersion.empty()) {
//         std::cerr << "Invalid request line!" << std::endl;
//         return false;
//     }

//     std::cout << "Method: " << _method << ", URI: " << _uri << ", HTTP Version: " << _httpVersion << std::endl;

//     size_t i = 1;
//     bool endofheaders = false;
//     for (; i < v_request.size(); i++) {
//         std::string line = v_request[i];

//         if (line.empty()) {
//             endofheaders = true;
//             i++;
//             break;
//         }

//         size_t pos = line.find(':');
//         if (pos != std::string::npos) {
//             std::string header_name = trim(line.substr(0, pos));
//             std::string header_value = trim(line.substr(pos + 1));
//             _headers[header_name] = header_value;
//         }
//     }

//     // std::map<std::string, std::string>::iterator it;
//     // for (it = _headers.begin(); it != _headers.end(); ++it) {
//     //     // std::cout << "Header: " << it->first << " = " << it->second << std::endl;
//     // }

//     if (_method != "POST")
//         return true;

//     if (endofheaders && i < v_request.size()) {
//         for (; i < v_request.size(); i++) {
//             if (!_body.empty())
//                 _body += '\n';
//             _body += v_request[i];
//         }
//     } else {
//         std::cout << "Header not closed properly\n";
//         return false;
//     }

//     std::cout << "Body: " << _body << std::endl;
//     return true;
// }


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


bool execute_cgi(string  & response, string arg, string path, const  Server *srv)
{
    pid_t pid  ;
    int fd[2];
    int status = 200;

    path = srv->cgi_bin.root + '/' + path;


    try
    {

        if (pipe(fd) == -1)
        {
            status = 500;
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


bool RequestParser::_Check_Get_Method(ResponseBuilder & response, const Server *srv)
{
    // // Handle directories and index files

    std::string fullpath = srv->location.root + _uri;
    std::string checkPath = fullpath;
    if (_uri.size() > 1 && _uri[_uri.size() - 1] == '/') {
        checkPath = fullpath.substr(0, fullpath.size() - 1);
    }

    if (isDirectory(checkPath))
    {
        if (!isMethodAuthorised(_method, srv->location.methods))
        {
            response.setStatus(405, "Method Not Allowed");
            response.addHeader("Content-Type", "text/html");
            string body = response.Replace_html_error_message(srv->error.error.html_content, 
                            405, "Method Not Allowed");
            response.setBody(body);
            return false;
        }
        
        std::string indexPath = checkPath + "/" + srv->location.index;
        if (CanWeReadAFile(indexPath))
        {
            std::ifstream file(indexPath.c_str(), std::ios::binary);
            if (file.is_open())
            {
                std::ostringstream ss;
                ss << file.rdbuf();
                response.setStatus(200, "OK");
                
                std::string contentType = "text/html";
                if (indexPath.find(".css") != std::string::npos)
                    contentType = "text/css";
                else if (indexPath.find(".js") != std::string::npos)
                    contentType = "application/javascript";
                else if (indexPath.find(".jpg") != std::string::npos || 
                         indexPath.find(".jpeg") != std::string::npos)
                    contentType = "image/jpeg";
                else if (indexPath.find(".png") != std::string::npos)
                    contentType = "image/png";
                
                response.addHeader("Content-Type", contentType);
                response.setBody(ss.str());
                file.close();
                return true;
            }
        }
        
        if (srv->location.autoindex)
        {
            std::string dirListingHtml = make_autoindex_html(checkPath, _uri);
            if (!dirListingHtml.empty()) {
                response.setStatus(200, "OK");
                response.addHeader("Content-Type", "text/html");
                response.setBody(dirListingHtml);
                return true;
            }
            else
            {
                response.setStatus(403, "Forbidden");
                response.addHeader("Content-Type", "text/html");
                string body = response.Replace_html_error_message(srv->error.error.html_content, 
                            403, "Forbidden");
                response.setBody(body);
                return false;
            }
        }
        else
        {
            response.setStatus(403, "Forbidden");
            response.addHeader("Content-Type", "text/html");
            string body = response.Replace_html_error_message(srv->error.error.html_content, 
                        403, "Forbidden");
            response.setBody(body);
            return false;
        }
    }
    ///////////

    if (!srv->location.redirection.empty())
    {
        response.setStatus(StatusCodes::MOVED_PERMANENTLY,
                           StatusCodes::getStatusMessage(StatusCodes::MOVED_PERMANENTLY));
        // response.addHeader("Location", srv->location.redirection);
        string body = response.Replace_html_error_message(srv->error.error.html_content, 
                                StatusCodes::MOVED_PERMANENTLY,
                           StatusCodes::getStatusMessage(StatusCodes::MOVED_PERMANENTLY));
        response.setBody(body);
        return true;
    }
    string imagedata = "";
    bool isimagerequested = false;
    bool iscgi              =  false;
    bool is_upload_requested = false;
    bool is_error = false;
    string output="";
    // size_t pos=-1;


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
        {
            if (srv->location.autoindex)
            {
                std::string dirListingHtml = make_autoindex_html(srv->location.root, _uri);
                if (!dirListingHtml.empty())
                {
                    response.setStatus(200, "OK");
                    response.addHeader("Content-Type", "text/html");
                    response.setBody(dirListingHtml);
                    return true;
                }
                else
                {
                    response.setStatus(403, "Forbidden");
                    response.addHeader("Content-Type", "text/html");
                    response.setBody("<html><body><h1>403 Forbidden</h1><p>Permission denied.</p></body></html>");
                    return false;
                }
            }
            else
            {
                response.setStatus(403, "Forbidden");
                response.addHeader("Content-Type", "text/html");
                response.setBody("<html><body><h1>403 Forbidden</h1><p>Directory listing not allowed.</p></body></html>");
                return false;
            }
        }
        else if (!_isHttpSupported())
            statuscode = 505;
        else if (!isHeaderNameExist("Host", _headers))
            statuscode = 400;

        cout << "status code for " << _uri << " is " << statuscode << endl;
    }
    else if (_uri == ('/' + srv->location.root) || _uri == ('/' + srv->location.index) || 
    _uri == ('/' + srv->location.root)  + ('/' + srv->location.index) )
    {
        if (!isMethodAuthorised(_method, srv->location.methods ))
            statuscode = 405;
        else if (!isFileAccessible(srv->location.root))
            statuscode = 404;
        else if (!CanWeReadAFile(srv->location.root + '/'   + srv->location.index))
        {
            if (srv->location.autoindex)
            {
                std::string html = make_autoindex_html(srv->location.root, "/");
                response.addHeader("Content-Type", "text/html");
                response.setBody(html);
                response.setStatus(200, "OK");
                return true;
            }
            statuscode = 403;
        }
        else if (!_isHttpSupported())
            statuscode = 505;
        else if (!isHeaderNameExist("Host", _headers))
            statuscode = 400;

        cout << "status code for " << _uri << " is " << statuscode << endl;
    }
    else if (_uri == ('/' +  srv->location_upload.root) || _uri == ('/' + srv->location_upload.index) || 
    _uri == ( srv->location_upload.root)  + ('/' + srv->location_upload.index) )
    {
        is_upload_requested = true;
        if (!isMethodAuthorised(_method, srv->location_upload.methods ))
            statuscode = 405;
        else if (!isFileAccessible(srv->location_upload.root))
            statuscode = 404;
        else if (!CanWeReadAFile(srv->location_upload.root + '/'   + srv->location_upload.index))
        {
            if (srv->location_upload.autoindex)
            {
                std::string html = make_autoindex_html(srv->location_upload.root, ("/" + srv->location_upload.root));
                response.addHeader("Content-Type", "text/html");
                response.setBody(html);
                response.setStatus(200, "OK");
                return true;
            }
            statuscode = 403;
        }
        else if (!_isHttpSupported())
            statuscode = 505;
        else if (!isHeaderNameExist("Host", _headers))
            statuscode = 400;

        cout << "status code for " << _uri << " is " << statuscode << endl;
    }
    else if (_uri ==  ( '/' + srv->error.error.html_path) || _uri == ('/' + srv->error.error.index) )
    {
        is_error = true;
        if (!isMethodAuthorised(_method, srv->location.methods ))
            statuscode = 405;
        else if (!isFileAccessible(srv->error.error.html_path))
            statuscode = 404;
        else if (!CanWeReadAFile(srv->error.error.html_path ))
            statuscode = 403;
        else if (!_isHttpSupported())
            statuscode = 505;
        else if (!isHeaderNameExist("Host", _headers))
            statuscode = 400;

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
    else if (v[0] == "cgi-bin")
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
                        if (execute_cgi(output, v_arg[1], scriptpath, srv))
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
        else if (is_upload_requested)
        {
            is_upload_requested = false;
            response.setBody(srv->location_upload.index_content);
        }
        else if (is_error)
        {
            is_error = false;
            string body = response.Replace_html_error_message(srv->error.error.html_content, 
                                200, "OK");
            response.setBody(body);
        }
        else
        {
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

bool RequestParser::handleUploadData( int & statuscode, string &fullpath, const Server *srv)
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
bool RequestParser::_Check_Post_Method(ResponseBuilder & response, const Server *srv)
{
    int statuscode = 400;
    string fullpath = "";
    cout << "check post just called \n";

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
                    if (handleUploadData(statuscode, fullpath, srv)) 
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

                        if (execute_cgi(cgi_output, v_arg[1], scriptpath, srv))
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

bool RequestParser::_Delete_Content(vector < string > v_uri, const Server *srv)
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



bool RequestParser::_Check_Delete_Method(ResponseBuilder & response, const Server *srv)
{

    (void)response;
    int statuscode = 200;

    Parser prs ("");
    vector <string > v_uri =prs. _split(_uri, '/');

    if (v_uri.size() <= 1)
        statuscode=400;
    

    else if (v_uri[0] == "uploads")
    {
        cout << "in uploads  _uri is " << _uri << endl;
        if (!isMethodAuthorised(_method, srv->location_upload.methods ))
            statuscode = 405;

        else if (!isFileAccessible(srv->location_upload.root + '/' + v_uri[v_uri.size() - 1]))
        {
            statuscode = 404;
        }
        else if (!CanWeWriteFile(srv->location_upload.root ))
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
        if (!_Delete_Content(v_uri, srv))
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





bool   RequestParser:: ValidateDataForResponse(ResponseBuilder &response, const Server *srv)
{

    cout << "hey from validate data for response \n";

    if (_method == "GET")
    {
        response.Method = response.GET;
        _Check_Get_Method(response, srv);
        
    }
    else if (_method == "DELETE")
    {
        response.Method = response.DELETE;
        _Check_Delete_Method(response, srv);
    }
    else if (_method == "POST")
    {
        response.Method = response.POST;
        _Check_Post_Method(response, srv);
    }
    else
    {
        cerr << "error in METHOD\n";
        response.Method = response.ERROR;
        return (false);
    }
    if (!_headers["Connection"].empty())
    {
        if (_headers["Connection"] == "close" || _headers["Connection"] == "CLOSE")
            response.Connection = response.CLOSE;
        else if (_headers["Connection"] == "keep-alive" || _headers["Connection"] == "KEEP-ALIVE")
            response.Connection = response.KEEP_ALIVE;
    }
    else
        response.Connection = response.CLOSE;
    return (true);
}





const std::string& RequestParser::getMethod() const { return _method; }

const std::string& RequestParser::getUri() const { return _uri; }

const std::string& RequestParser::getHttpVersion() const { return _httpVersion; }

const std::map<std::string, std::string>& RequestParser::getHeaders() const { return _headers; }

const std::string RequestParser::getHeader(std::string name) 
{
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
    {
        if (it->first == name)
            return (it->second);
    }
    return ("");
}


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

std::string html_escape(const std::string &in)
{
    std::string out;
    for (size_t i = 0; i < in.size(); ++i) {
        unsigned char c = in[i];
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#39;"; break;
            default: out.push_back(c);
        }
    }
    return out;
}

// std::string RequestParser::make_autoindex_html(const std::string &dirpath, const std::string &request_path)
// {
//     std::ostringstream rows;
//     DIR *d = opendir(dirpath.c_str());
//     if (d)
//     {
//         struct dirent *ent;
//         while ((ent = readdir(d)) != NULL)
//         {
//             std::string name(ent->d_name);
//             if (name == ".") continue;
//             std::string href = name;
//             std::string display = name;
//             std::string sizeStr = "-";
//             std::string mtimeStr = "-";
//             struct stat st;
//             std::string full = dirpath + "/" + name;
//             if (stat(full.c_str(), &st) == 0)
//             {
//                 if (S_ISDIR(st.st_mode))
//                 {
//                     href += "/";
//                     display += "/";
//                 }
//                 else
//                 {
//                     std::ostringstream ss;
//                     double sz = static_cast<double>(st.st_size);
//                     const char* units[] = {"B","KB","MB","GB"};
//                     int ui = 0;
//                     while (sz >= 1024.0 && ui < 3) { sz /= 1024.0; ++ui; }
//                     ss << std::fixed << std::setprecision(sz>=100.0?0:1) << sz << " " << units[ui];
//                     sizeStr = ss.str();
//                     char buf[64];
//                     std::tm tm;
//                     localtime_r(&st.st_mtime, &tm);
//                     strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", &tm);
//                     mtimeStr = buf;
//                 }
//             }
//             std::string esc = html_escape(display);
//             rows << "<tr><td data-key=\"name\"><a href=\"" << href << "\">" << esc << "</a></td>";
//             rows << "<td data-key=\"size\" data-size=\"" << sizeStr << "\" class=\"small\">" << sizeStr << "</td>";
//             rows << "<td data-key=\"mtime\" class=\"small\">" << mtimeStr << "</td></tr>\n";
//         }
//         closedir(d);
//     }

//     std::ostringstream html;
//     html << "<!doctype html><html><head><meta charset=\"utf-8\"><title>Index of " << html_escape(request_path) << "</title>";
//     html << "<style>body{font-family:Arial;margin:20px}table{width:100%;border-collapse:collapse}th,td{padding:8px;border-bottom:1px solid #eee}th{background:#fafafa}</style></head><body>";
//     html << "<h1>Index of " << html_escape(request_path) << "</h1><table id=\"ix\"><thead><tr><th data-key=\"name\">Name</th><th data-key=\"size\">Size</th><th data-key=\"mtime\">Last modified</th></tr></thead><tbody>";
//     html << rows.str();
//     html << "</tbody></table></body></html>";
//     return html.str();
// }


bool RequestParser::isDirectory(const std::string& path)
{
    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) != 0)
    {
        return false;
    }
    return S_ISDIR(statbuf.st_mode);
}

std::string RequestParser::make_autoindex_html(const std::string &dirpath, const std::string &request_path) 
{
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    std::stringstream html;

    dir = opendir(dirpath.c_str());
    if (!dir) {
        return ""; // Return empty string on error
    }

    html << "<!DOCTYPE html>\n"
         << "<html>\n"
         << "<head>\n"
         << "    <title>Index of " << request_path << "</title>\n"
         << "    <style>\n"
         << "        body { font-family: Arial, sans-serif; margin: 20px; }\n"
         << "        h1 { border-bottom: 1px solid #ccc; padding-bottom: 10px; }\n"
         << "        table { border-collapse: collapse; width: 100%; }\n"
         << "        th, td { text-align: left; padding: 8px; }\n"
         << "        tr:nth-child(even) { background-color: #f2f2f2; }\n"
         << "        th { background-color: #4CAF50; color: white; }\n"
         << "        a { text-decoration: none; color: #0366d6; }\n"
         << "        a:hover { text-decoration: underline; }\n"
         << "    </style>\n"
         << "</head>\n"
         << "<body>\n"
         << "    <h1>Index of " << request_path << "</h1>\n"
         << "    <table>\n"
         << "        <tr>\n"
         << "            <th>Name</th>\n"
         << "            <th>Last modified</th>\n"
         << "            <th>Size</th>\n"
         << "        </tr>\n";

    // add parent directory link unless at root
    if (request_path != "/")
    {
        html << "        <tr>\n"
             << "            <td><a href=\"..\">..</a></td>\n"
             << "            <td>-</td>\n"
             << "            <td>-</td>\n"
             << "        </tr>\n";
    }

    // collect all entries for sorting
    std::vector<std::string> entries;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;
        if (name != "." && name != "..")
        {
            entries.push_back(name);
        }
    }
    closedir(dir);

    std::sort(entries.begin(), entries.end());

    // Add each entry to the HTML
    for (std::vector<std::string>::const_iterator it = entries.begin(); it != entries.end(); ++it)
    {
        const std::string& name = *it;
        std::string fullPath = dirpath + "/" + name;
        
        if (stat(fullPath.c_str(), &file_stat) < 0)
        {
            continue; // Skip if stat fails
        }
        
        char timeStr[100];
        struct tm* timeinfo = localtime(&file_stat.st_mtime);
        strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeinfo);
        
        std::string sizeStr;
        if (S_ISDIR(file_stat.st_mode))
        {
            sizeStr = "-";
        }
        else
        {
            double size = file_stat.st_size;
            const char* units[] = {"B", "KB", "MB", "GB"};
            int unit = 0;
            
            while (size > 1024 && unit < 3)
            {
                size /= 1024;
                unit++;
            }
            
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(unit > 0 ? 1 : 0) << size << " " << units[unit];
            sizeStr = ss.str();
        }
        
        html << "        <tr>\n"
             << "            <td><a href=\"" << name << (S_ISDIR(file_stat.st_mode) ? "/" : "") << "\">" 
             << name << (S_ISDIR(file_stat.st_mode) ? "/" : "") << "</a></td>\n"
             << "            <td>" << timeStr << "</td>\n"
             << "            <td>" << sizeStr << "</td>\n"
             << "        </tr>\n";
    }

    html << "    </table>\n"
         << "</body>\n"
         << "</html>";

    return html.str();
}