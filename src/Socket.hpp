#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <string>
#include <stdexcept>

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>


#include <cerrno>
#include <cstdio>

class Socket
{

private:
    int _fd;
    void setNonBlocking();

public:
    Socket();
    ~Socket();
    Socket(const Socket&);
    Socket& operator=(const Socket&);

    void create();
    void bind(int port);
    void bind(int port, const std::string& ip_address);
    void listen(int backlog);
    int accept();

    // getter
    int getFd() const;

};

#endif


