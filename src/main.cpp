
#include <iostream>
#include <vector>
#include <algorithm>
#include <map>


#include "Socket.hpp"
#include "EventHandler.hpp"
#include "RequestParser.hpp"
#include "ResponseBuilder.hpp"
#include "../include/Server.h"
#include "../include/Parser.h"

#include <cerrno>
#include <cstdio>

std::map<int, RequestParser> clientParsers;
std::map<int, std::string> clientBuffers;



void handleNewConnection(Socket& serverSocket, EventHandler& eventHandler) 
{
    int client_fd = serverSocket.accept();
    if (client_fd != -1)
    {
        std::cout << "Accepted new connection on fd: " << client_fd << std::endl;
        eventHandler.addFd(client_fd, POLLIN); // monitor client for read events // maybe we need write checking too??
        clientParsers[client_fd] = RequestParser();
    }

}


void handleClientData(int client_fd, EventHandler& eventHandler)
{
    char buffer[8192];
    int bytes_read = recv(client_fd, buffer, sizeof(buffer), 0);


    // cout << "buff*****\n";
    // cout << buffer << endl;
    // cout << "buff*****\n";
    if (bytes_read > 0)
    {
        if (clientBuffers.find(client_fd) == clientBuffers.end())
        {
            clientBuffers[client_fd] = "";
        }
        clientBuffers[client_fd].append(buffer, bytes_read);
        
        // getting parser for this client
        RequestParser& parser = clientParsers[client_fd];
        
        // Check if we have a complete request
        bool isComplete = false;
        size_t contentLength = 0;
        size_t headerEnd = clientBuffers[client_fd].find("\r\n\r\n");
        
        if (headerEnd != std::string::npos)
        {
            // Extract just the headers to look for Content-Length
            std::string headers = clientBuffers[client_fd].substr(0, headerEnd);
            size_t contentLengthPos = headers.find("Content-Length:");
            
            if (contentLengthPos != std::string::npos)
            {
                size_t valueStart = headers.find_first_not_of(" \t", contentLengthPos + 15);
                size_t valueEnd = headers.find("\r\n", valueStart);
                if (valueStart != std::string::npos && valueEnd != std::string::npos)
                {
                    std::string lengthStr = headers.substr(valueStart, valueEnd - valueStart);
                    contentLength = std::strtoul(lengthStr.c_str(), NULL, 10); ///// // /
                }
            }
            
            // Check if we have the complete body
            if (headerEnd + 4 + contentLength <= clientBuffers[client_fd].length())
            {
                isComplete = true;
            }
        }

        // std::cout << "((((((((((((((()))))))))))))))\n";
        // std::cout << clientBuffers[client_fd] << std::endl;
        // std::cout << "((((((((((((((()))))))))))))))\n";
        
        if (isComplete || contentLength == 0)
        {
            try
            {
                if (parser.parse(clientBuffers[client_fd])) //// // /
                {
                    ResponseBuilder responseBuilder;
                    parser.ValidateDataForResponse(responseBuilder);
                    std::string response = responseBuilder.build();
                    
                    send(client_fd, response.c_str(), response.length(), 0);
                    
                    clientBuffers.erase(client_fd);
                    parser.clear();
                }
                else
                {
                    std::cout << "Failed to parse request from client " << client_fd << std::endl;
                    
                    ResponseBuilder errorResponse;
                    errorResponse.setStatus(400, "Bad Request");
                    errorResponse.addHeader("Content-Type", "text/html");
                    errorResponse.setBody("<html><body><h1>400 Bad Request</h1><p>Your browser sent a request that this server could not understand.</p></body></html>");
                    std::string response = errorResponse.build();
                    
                    send(client_fd, response.c_str(), response.length(), 0);
                    
                    clientBuffers.erase(client_fd);
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << "Exception handling client " << client_fd << ": " << e.what() << std::endl;
                
                ResponseBuilder errorResponse;
                errorResponse.setStatus(500, "Internal Server Error");
                errorResponse.addHeader("Content-Type", "text/html");
                errorResponse.setBody("<html><body><h1>500 Internal Server Error</h1></body></html>");
                std::string response = errorResponse.build();
                
                send(client_fd, response.c_str(), response.length(), 0);
                
                clientBuffers.erase(client_fd);
            }
        }
        // If not complete, wait for more data
    }
    else if (bytes_read == 0)
    {
        std::cout << "Client " << client_fd << " disconnected" << std::endl;
        eventHandler.removeFd(client_fd);
        close(client_fd);
        clientParsers.erase(client_fd);
        clientBuffers.erase(client_fd);
    }
    else
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            std::cerr << "Error reading from client " << client_fd << ": " << strerror(errno) << std::endl;
            eventHandler.removeFd(client_fd);
            close(client_fd);
            clientParsers.erase(client_fd);
            clientBuffers.erase(client_fd);
        }
        // If EAGAIN/EWOULDBLOCK, just wait for more data
    }
}


Server *srv = NULL;

int main(int argc, char **argv)
{
    // update : address.sin_addr.s_addr = INADDR_ANY;   to srv->listennig.ip_addr;
    if (argc != 2)
    {
        cout << "Wrong Arguments : ./server <conf_file>\n";
        return (1);
    }


    Parser parser (argv[1]);
    srv = parser.Parse();


    if (srv == NULL)
    {
        std::cerr << "error parsing Config File\n";
        return (1);
    }

    try
    {
        Socket serverSocket;
        EventHandler eventHandler;
        int backlog = 10;

        serverSocket.create();
        serverSocket.bind(srv->listening.Port);
        serverSocket.listen(backlog);

        std::cout << "Server listening on port " << srv->listening.Port << std::endl;

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


    if (srv)
        delete (srv);

    return 0;
}