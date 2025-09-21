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



struct ClientState
{
    std::string responseBuffer;
    size_t bytesWritten;
    bool hasResponse;
    
    ClientState() : bytesWritten(0), hasResponse(false) {}
};

std::map<int, ClientState> clientStates;

std::map<int, RequestParser> clientParsers;
std::map<int, std::string> clientBuffers;

void handleNewConnection(Socket& serverSocket, EventHandler& eventHandler) 
{
    int client_fd = serverSocket.accept();
    if (client_fd != -1)
    {
        std::cout << "Accepted new connection on fd: " << client_fd << std::endl;
        eventHandler.addFd(client_fd, POLLIN);
        clientParsers[client_fd] = RequestParser();
    }
}

void handleClientData(int client_fd, EventHandler& eventHandler)
{
    char buffer[8192];
    int bytes_read = recv(client_fd, buffer, sizeof(buffer), 0);

    if (bytes_read > 0)
    {
        if (clientBuffers.find(client_fd) == clientBuffers.end())
        {
            clientBuffers[client_fd] = "";
        }
        clientBuffers[client_fd].append(buffer, bytes_read);
        
        RequestParser& parser = clientParsers[client_fd];
        
        bool isComplete = false;
        size_t contentLength = 0;
        size_t headerEnd = clientBuffers[client_fd].find("\r\n\r\n");
        
        if (headerEnd != std::string::npos)
        {
            std::string headers = clientBuffers[client_fd].substr(0, headerEnd);
            size_t contentLengthPos = headers.find("Content-Length:");
            
            if (contentLengthPos != std::string::npos)
            {
                size_t valueStart = headers.find_first_not_of(" \t", contentLengthPos + 15);
                size_t valueEnd = headers.find("\r\n", valueStart);
                if (valueStart != std::string::npos && valueEnd != std::string::npos)
                {
                    std::string lengthStr = headers.substr(valueStart, valueEnd - valueStart);
                    contentLength = std::strtoul(lengthStr.c_str(), NULL, 10);
                }
            }
            
            if (headerEnd + 4 + contentLength <= clientBuffers[client_fd].length())
            {
                isComplete = true;
            }
        }
        
        if (isComplete || contentLength == 0)
        {
            try
            {
                if (parser.parse(clientBuffers[client_fd]))
                {
                    ResponseBuilder responseBuilder;
                    parser.ValidateDataForResponse(responseBuilder);
                    std::string response = responseBuilder.build();
                    
                    // BUFFER THE RESPONSE - NO DIRECT SEND
                    clientStates[client_fd].responseBuffer = response;
                    clientStates[client_fd].bytesWritten = 0;
                    clientStates[client_fd].hasResponse = true;
                    eventHandler.modifyFdEvents(client_fd, POLLOUT);

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
                    
                    // BUFFER THE ERROR RESPONSE - NO DIRECT SEND
                    clientStates[client_fd].responseBuffer = response;
                    clientStates[client_fd].bytesWritten = 0;
                    clientStates[client_fd].hasResponse = true;
                    eventHandler.modifyFdEvents(client_fd, POLLOUT);

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
                
                // FIXED: BUFFER INSTEAD OF DIRECT SEND
                clientStates[client_fd].responseBuffer = response;
                clientStates[client_fd].bytesWritten = 0;
                clientStates[client_fd].hasResponse = true;
                eventHandler.modifyFdEvents(client_fd, POLLOUT);
                
                clientBuffers.erase(client_fd);
            }
        }
    }
    else if (bytes_read == 0)
    {
        std::cout << "Client " << client_fd << " disconnected" << std::endl;
        eventHandler.removeFd(client_fd);
        close(client_fd);
        clientParsers.erase(client_fd);
        clientBuffers.erase(client_fd);
        clientStates.erase(client_fd);
    }
}

void handleClientWrite(int client_fd, EventHandler& eventHandler)
{
    ClientState& state = clientStates[client_fd];
    
    if (!state.hasResponse || state.responseBuffer.empty())
    {
        return;
    }
    
    size_t remaining = state.responseBuffer.size() - state.bytesWritten;
    ssize_t bytes_sent = send(client_fd, 
                             state.responseBuffer.data() + state.bytesWritten, 
                             remaining, 0);
    
    if (bytes_sent > 0)
    {
        state.bytesWritten += bytes_sent;
        
        if (state.bytesWritten >= state.responseBuffer.size())
        {
            // Response fully sent - close connection
            eventHandler.removeFd(client_fd);
            close(client_fd);
            clientParsers.erase(client_fd);
            clientBuffers.erase(client_fd);
            clientStates.erase(client_fd);
        }
    }
    else if (bytes_sent == 0)
    {
        // Connection closed
        eventHandler.removeFd(client_fd);
        close(client_fd);
        clientParsers.erase(client_fd);
        clientBuffers.erase(client_fd);
        clientStates.erase(client_fd);
    }
}

Server *srv = NULL;

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return 1;
    }

    Parser parser(argv[1]);
    srv = parser.Parse();

    if (srv == NULL)
    {
        std::cerr << "Error parsing config file" << std::endl;
        return 1;
    }

    try
    {
        Socket serverSocket;
        EventHandler eventHandler;
        int backlog = 10;

        serverSocket.create();
        serverSocket.bind(srv->listening.Port);
        serverSocket.listen(backlog);

        std::cout << "Server listening on " << srv->listening.ip_addr 
                  << ":" << srv->listening.Port << std::endl;

        eventHandler.addFd(serverSocket.getFd(), POLLIN);

        while (true)
        {
            int num_events = eventHandler.pollEvents(-1);

            if (num_events > 0)
            {
                const std::vector<pollfd>& fds = eventHandler.getPollFds();
                for (size_t i = 0; i < fds.size(); ++i)
                {
                    if (fds[i].revents & (POLLHUP | POLLERR | POLLNVAL))
                    {
                        if (fds[i].fd != serverSocket.getFd())
                        {
                            std::cout << "Client " << fds[i].fd << " error/hangup" << std::endl;
                            eventHandler.removeFd(fds[i].fd);
                            close(fds[i].fd);
                            clientParsers.erase(fds[i].fd);
                            clientBuffers.erase(fds[i].fd);
                            clientStates.erase(fds[i].fd);
                        }
                        continue;
                    }

                    // Server socket - new connections
                    if (fds[i].fd == serverSocket.getFd())
                    {
                        if (fds[i].revents & POLLIN)
                        {
                            handleNewConnection(serverSocket, eventHandler);
                        }
                    }
                    else
                    {
                        // Client sockets
                        if (fds[i].revents & POLLIN)
                        {
                            handleClientData(fds[i].fd, eventHandler);
                        }
                        if (fds[i].revents & POLLOUT)
                        {
                            handleClientWrite(fds[i].fd, eventHandler);
                        }
                    }
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Server error: " << e.what() << std::endl;
        if (srv) delete srv;
        return 1;
    }

    if (srv) delete srv;
    return 0;
}