#include "../include/Server.h"



Server::Server()
{
    max_body_size = 5000000;
    server_fd = -1;
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


string Server::ReadRequest(int client_Id)
{
    char buff[4096];

    int byte = recv (client_Id, buff, sizeof(buff) - 1, 0);

    if (byte > 0)
    {
        buff[byte] = '\0';

        string request (buff);
        return (request);
    }

    return (NULL);
}

bool Server::Setup()
{
    cout << "starting server setup" << endl;


    server_fd = socket (AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1)
    {
        cerr << "cannot open socket \n";
        return (false);
    }

    cout << "socket created \n";

    int opt = 1;

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt,sizeof(opt)) < 0)
    {
        cerr << "cannot controle socket \n";
        close(server_fd);
    }


    sockaddr_in addr;

    memset (&addr,0,  sizeof(sockaddr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(listening.Port); 
    addr.sin_addr.s_addr = inet_addr(listening.ip_addr.c_str());


    if (bind(server_fd, (struct   sockaddr *)&addr, sizeof(addr)) < 0)
    {
        cerr << " cannot bind socket to ip address\n";
        close (server_fd);
        return (false);
    }


    if (listen(server_fd, 10) < 0)
    {
        cerr << "cannot listen on socket\n";
        close (server_fd);
    }

    cout << "\nServer is Listenning\n";

    return (true);
}