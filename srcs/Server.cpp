#include "../include/Server.h"



Server::Server()
{
    max_body_size = 5000000;
}

Server::Error::Error()
{

}

int Server::Error::get_length()
{
    return (v_error.size());
}

Server::Error::~Error()
{
    
}