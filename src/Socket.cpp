#include "Socket.hpp"
#include <iostream>

Socket::Socket() : _fd(-1) {}

Socket::~Socket() 
{
    if (_fd != -1) 
    {
        close(_fd);
    }
}

void Socket::create() 
{
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd == -1) 
    {
        perror("socket");
        throw std::runtime_error("Failed to create socket.");
    }
    setNonBlocking();

    // allow reuse of local addresses
    int optval = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
    {
        perror("setsockopt SO_REUSEADDR");
        close(_fd);
        throw std::runtime_error("Failed to set SO_REUSEADDR.");
    }
}

void Socket::bind(int port)
{
    if (_fd == -1)
    {
        throw std::runtime_error("Socket not created. Call create() first.");
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (::bind(_fd, (struct sockaddr*)&address, sizeof(address)) == -1)
    {
        perror("bind");
        close(_fd);
        throw std::runtime_error("Failed to bind socket.");
    }
}

void Socket::listen(int backlog) 
{
    if (_fd == -1) 
    {
        throw std::runtime_error("Socket not created. Call create() first.");
    }

    if (::listen(_fd, backlog) == -1)
    {
        perror("listen");
        close(_fd);
        throw std::runtime_error("Failed to listen on socket.");
    }
}


// returns new client socket fd
int Socket::accept() 
{
    if (_fd == -1) 
    {
        throw std::runtime_error("Socket not created. Call create() first.");
    }

    sockaddr_in client_address;
    socklen_t client_len = sizeof(client_address);
    int client_fd = ::accept(_fd, (struct sockaddr*)&client_address, &client_len);

    if (client_fd == -1) 
    {
        // perror("accept"); // don't print error for non-blocking accept returning -1
        return -1; // error no connection accepted yet
    }
    
    // set the accepted client socket to non-blocking as well
    int flags = fcntl(client_fd, F_GETFL, 0);
    if (flags == -1)
    {
        perror("fcntl F_GETFL (client_fd)");
        close(client_fd);
        return -1;
    }
    if (fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        perror("fcntl F_SETFL O_NONBLOCK (client_fd)");
        close(client_fd);
        return -1;
    }

    return client_fd;
}

int Socket::getFd() const
{
    return _fd;
}

void Socket::setNonBlocking()
{
    int flags = fcntl(_fd, F_GETFL, 0);
    if (flags == -1)
    {
        perror("fcntl F_GETFL");
        close(_fd);
        throw std::runtime_error("Failed to get socket flags.");
    }
    if (fcntl(_fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        perror("fcntl F_SETFL O_NONBLOCK");
        close(_fd);
        throw std::runtime_error("Failed to set socket to non-blocking.");
    }
}
