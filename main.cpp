#include <iostream>
#include "include/Parser.h"


using namespace std;


int main (int argc, char **argv)
{
    if (argc != 2)
    {
        cout << "Wrong Arguments : ./server <conf_file>\n";
        return (1);
    }


    Parser parser (argv[1]);
    
    Server *srv = parser.Parse();


    if (srv == NULL)
    {
        std::cerr << "error parsing Config File\n";
        return (1);
    }


    if (!srv->Setup())
    {
        cerr << "Cannot Setup Server\n\n";
    }
    


    // mainloop;

    while (1)
    {
        int client_fd = accept (srv->server_fd, NULL, NULL);

        if (client_fd < 0)
            continue;

        cout << "request : \n" << srv->ReadRequest(client_fd) << endl;

        const char* response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "Hello, World!";

        send (client_fd, response, strlen (response), 0);
        close (client_fd);

        // tasks : parse request, generate response; cgi
    }

    if (srv)
    {
        delete srv;
    }


    
    
    (void)argc;
    (void)argv;
    return (0);
}