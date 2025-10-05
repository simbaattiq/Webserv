#include "../include/Server.h"
#include "../include/globals.h"


std::string trim( std::string& str)  
{
    size_t first = str.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) 
    {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

std::vector<std::string> _split( std::string& str, char delimiter)
 {
    std::vector<std::string> result;
    std::string temp;


    str = trim (str);
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

Server* Server::select_correspondent_server ( std::string host)
{
    // int  index = -1;
    if (host.empty())
        return (NULL);

    vector <string> v_host  = _split(host, ':');

    if (v_host.size() != 2)
        return (NULL);


    string ip_addr = v_host[0];
    int port = atoi (v_host[1].c_str());
    

    for (size_t i = 0; i < v_srv.size(); i++)
    {
        for (size_t z = 0; z < v_srv[i].v_listening.size(); z++)
        {
            if (v_srv[i].v_listening[z].Port == port)
                return (&v_srv[i]);
        }
    }
    return NULL;
}



Server::Server()
{
    max_body_size = 5000000;
    server_fd = -1;
}

Server::Error::Error()
{

}


Server::Error::~Error()
{
    
}


string Server::ReadRequest(int client_Id)
{
    char buff[4096];

    string request = "";
    int byte = 0;

    while ((byte = recv (client_Id, buff, sizeof(buff) - 1, 0)) > 0)
    {
        buff[byte] = '\0';
        string tmp (buff);


        request += tmp;
    }

    return (request);
}


Server& Server::operator=(const Server& other) {
    if (this != &other) {
        
        this->location = other.location;
        this->error = other.error;
        this->max_body_size = other.max_body_size;
        this->server_fd = other.server_fd;
        this->v_listening = other.v_listening;
        this->location_upload = other.location_upload;
        this->location_images = other.location_images;
        this->cgi_bin = other.cgi_bin;

    }
    return *this;
}