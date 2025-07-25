#include "../include/Request.h"


Request::Request(string request)
{
    _request = request;

    cout << _request << endl;
}

Request::~Request ()
{

}

string Request::get_request_string()
{
    return (_request);
}


bool Request::Parse()
{
    return  (true);
}