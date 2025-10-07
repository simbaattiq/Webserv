#ifndef SERVER_H
#define SERVER_H


#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <queue>
#include <cstdlib>

using namespace std;

class Server
{
    //  struct sterror;


    public:

        struct sterror
        {
           string html_path;
           string index;
           string html_content;
        };

    public :
        class Listening
        {
            public : 
                string ip_addr;
                int   Port;
        }; 

    class Error
    {
        public :      
            Error ();
            sterror error;
            ~Error();
    };

    class Location
    {
        public :
            string root;
            string index;
            string index_content;
            bool autoindex;
            vector <string> methods;
    };

    class Location_Upload
    {
        public :
            string root;
            string upload_store;
            bool autoindex;
            string index;
            vector <string> methods;
            string index_content;
    };

    class Location_Images
    {
        public :
            string root;
            bool autoindex;
            vector <string> methods;
            string default_img;
    };


    class Cgi_Bin
    {
        public :
            string root;
            string cgi_pass;
            vector <string> methods;
            string htmlcontent;
    };


    public : 


    Server();
    Server& operator=(const Server& other);
    Listening listening; // will be deleted;
    vector <Listening > v_listening; // new one;
    Error     error;
    Location  location;
    Location_Upload location_upload;
    Location_Upload location_images;
    size_t          max_body_size;
    Cgi_Bin cgi_bin;
    int     server_fd;
    bool Setup();
    string ReadRequest(int client_Id);
    static Server* select_correspondent_server ( std::string host);

};

# endif