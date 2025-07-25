#ifndef SERVER_H
#define SERVER_H


#include <iostream>
#include <vector>
using namespace std;

class Server
{
    //  struct sterror;


    public:

        struct sterror
        {
           int err_number;
           string html_path;
        };

    private :
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
            vector <Server::sterror> v_error;
            int get_length();
            ~Error();
    };

    class Location
    {
        public :
            string root;
            string index;
            bool autoindex;
            vector <string> methods;
    };

    class Location_Upload
    {
        public :
            string root;
            string upload_store;
            bool autoindex;
            vector <string> methods;
    };


    class Cgi_Bin
    {
        public :
            string root;
            string cgi_pass;
            vector <string> methods;
    };


    public : 


    Server();
    Listening listening;
    Error     error;
    Location  location;
    Location_Upload location_upload;
    size_t          max_body_size;
    Cgi_Bin cgi_bin;

};

# endif