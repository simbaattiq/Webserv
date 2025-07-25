#ifndef REQUEST_H
# define REQUEST_H


#include <iostream>

using namespace std;


class Request
{
    private : 

        string _request;


    public : 

        string get_request_string();
        Request (string request);
        ~Request ();
        bool Parse();


        

};


#endif