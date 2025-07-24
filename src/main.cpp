
#include <iostream>
#include <vector>
#include <algorithm>
#include <map>


#include "Socket.hpp"
#include "EventHandler.hpp"
#include "RequestParser.hpp"
#include "ResponseBuilder.hpp"

#include <cerrno>
#include <cstdio>

std::map<int, RequestParser> clientParsers;


void handleNewConnection(Socket& serverSocket, EventHandler& eventHandler) 
{
    int client_fd = serverSocket.accept();
    if (client_fd != -1)
    {
        std::cout << "Accepted new connection on fd: " << client_fd << std::endl;
        eventHandler.addFd(client_fd, POLLIN); // monitor client for read events
        clientParsers[client_fd] = RequestParser();
    }

}

void handleClientData(int client_fd, EventHandler& eventHandler)
{
    char buffer[2048]; // Increased buffer!!
    int bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_read > 0)
    {
        buffer[bytes_read] = '\0';
        std::string rawRequest(buffer);

        std::cout << "Received from client " << client_fd << ":\n" << rawRequest << std::endl;

        RequestParser& parser = clientParsers[client_fd];
        ResponseBuilder responseBuilder;

        if (parser.parse(rawRequest)) {
            std::cout << "Parsed Request - Method: " << parser.getMethod()
                      << ", URI: " << parser.getUri() << std::endl;

            // simple routing/response logic
            if (parser.getMethod() == "GET" && parser.getUri() == "/")
            {
                responseBuilder.setStatus(200, "OK");
                responseBuilder.addHeader("Content-Type", "text/html");
                responseBuilder.setBody("<html><body><h1>Hello from webserv!</h1><p>You requested: " + parser.getUri() + "</p></body></html>");
            }
            else if (parser.getMethod() == "GET" && parser.getUri() == "/info")
            {
                responseBuilder.setStatus(200, "OK");
                responseBuilder.addHeader("Content-Type", "text/plain");
                responseBuilder.setBody("This is a simple webserv written in C++98.");
            }
            else
            {
                responseBuilder.setStatus(404, "Not Found");
                responseBuilder.addHeader("Content-Type", "text/html");
                responseBuilder.setBody("<html><body><h1>404 Not Found</h1><p>The requested URL " + parser.getUri() + " was not found on this server.</p></body></html>");
            }
        }
        else
        {
            // Bad request
            std::cerr << "Failed to parse request from client " << client_fd << std::endl;
            responseBuilder.setStatus(400, "Bad Request");
            responseBuilder.addHeader("Content-Type", "text/plain");
            responseBuilder.setBody("Bad Request");
        }

        std::string httpResponse = responseBuilder.build();
        send(client_fd, httpResponse.c_str(), httpResponse.length(), 0);
        std::cout << "Sent response to client " << client_fd << ":\n" << httpResponse << std::endl;

        // note: after sending response, we might want to close connection for simple http/1.0
        // or keep-alive for http/1.1. For now, we'll close it.
        eventHandler.removeFd(client_fd);
        close(client_fd);
        clientParsers.erase(client_fd);

    }
    else if (bytes_read == 0)
    {
        // client closed connection
        std::cout << "Client " << client_fd << " disconnected." << std::endl;
        eventHandler.removeFd(client_fd);
        close(client_fd);
        clientParsers.erase(client_fd);
    }
    else
    {
        // Error or no data (EAGAIN/EWOULDBLOCK for non-blocking)
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            perror("recv");
            std::cerr << "Error reading from client " << client_fd << std::endl;
            eventHandler.removeFd(client_fd);
            close(client_fd);
            clientParsers.erase(client_fd);
        }
    }
}

int main()
{
    try
    {
        Socket serverSocket;
        EventHandler eventHandler;
        int port = 8080;
        int backlog = 10;

        serverSocket.create();
        serverSocket.bind(port);
        serverSocket.listen(backlog);

        std::cout << "Server listening on port " << port << std::endl;

        // adding the server socket to the event handler
        eventHandler.addFd(serverSocket.getFd(), POLLIN);

        while (true)
        {
            int num_events = eventHandler.pollEvents(-1); // wait indefinitely for events

            if (num_events > 0)
            {
                const std::vector<pollfd>& fds = eventHandler.getPollFds();
                for (size_t i = 0; i < fds.size(); ++i)
                {
                    // Events on the server socket (new connections)
                    if (fds[i].fd == serverSocket.getFd())
                    {
                        if (fds[i].revents & POLLIN)
                        {
                            handleNewConnection(serverSocket, eventHandler);
                        }
                    }
                    else
                    {
                        // check for events on client sockets
                        if (fds[i].revents & POLLIN)
                        {
                            handleClientData(fds[i].fd, eventHandler);
                        }
                        // client disconnection or errors
                        if (fds[i].revents & (POLLHUP | POLLERR))
                        {
                            std::cout << "Client " << fds[i].fd << " hung up or error." << std::endl;
                            eventHandler.removeFd(fds[i].fd);
                            close(fds[i].fd);
                            clientParsers.erase(fds[i].fd); // clean up parser for this client
                        }
                    }
                }
            }
        }
    } 
    catch (const std::exception& e) 
    {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
