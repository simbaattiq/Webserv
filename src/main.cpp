
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
#include "../include/globals.h"

#include <cerrno>
#include <cstdio>

std::map<int, RequestParser> clientParsers;
std::map<int, std::string> clientBuffers;
std::vector<Socket*> serverSockets;

bool isServerSocket(int fd)
{
    for (size_t i = 0; i < serverSockets.size(); ++i)
    {
        if (serverSockets[i]->getFd() == fd)
            return true;
    }
    return false;
}

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
    bool conection = false;
    
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


        ResponseBuilder responseBuilder;
        
        
        if (isComplete || contentLength == 0)
        {
            try
            {
                bool isparsed = false;
                Server *selectedServer=NULL;
                if (parser.parse(clientBuffers[client_fd]))
                {
                    isparsed = true;
                }

                if (isparsed)
                {
                    string host = parser.getHeader("Host");
                    selectedServer = Server::select_correspondent_server (host);

                    if (!selectedServer)
                    {
                        cout << "cannot select a server\n";
                        isparsed = false;
                    }
                }

                if (isparsed && selectedServer)
                {
                    

                    parser.ValidateDataForResponse(responseBuilder,  selectedServer);
                    std::string response = responseBuilder.build();

                    send(client_fd, response.c_str(), response.length(), 0);
                    
                    clientBuffers.erase(client_fd);
                    parser.clear();
                    if (responseBuilder.Connection == responseBuilder.CLOSE)
                    {
                        eventHandler.removeFd(client_fd);
                        close(client_fd);
                        clientParsers.erase(client_fd);
                        clientBuffers.erase(client_fd);
                        cout << "connection just closed for " << client_fd << endl;
                        conection = true;
                    }
                    else
                        conection = false;
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
                    eventHandler.removeFd(client_fd);
                    close (client_fd);
                    clientBuffers.erase(client_fd);
                    cout << "connection just closed for " << client_fd << endl;
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
                eventHandler.removeFd(client_fd);
                close (client_fd);
                clientBuffers.erase(client_fd);
                cout << "connection just closed for " << client_fd << endl;
            }
        }
        // If not complete, wait for more data
    }
    else if (bytes_read == 0)
    {
        if (conection)
        {
            std::cout << "Client " << client_fd << " disconnected" << std::endl;
            eventHandler.removeFd(client_fd);
            close(client_fd);
            clientParsers.erase(client_fd);
            clientBuffers.erase(client_fd);
        }
        
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


static void printStringVector(const std::vector<std::string>& v)
{
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i)
    {
        std::cout << v[i];
        if (i + 1 < v.size()) std::cout << ", ";
    }
    std::cout << "]";
}

// Main debug function to print servers
void printServers(const std::vector<Server>& servers)
{
    using std::cout;
    using std::endl;

    cout << "=== v_srv (" << servers.size() << ") ===" << endl;
    for (size_t si = 0; si < servers.size(); ++si)
    {
        const Server& s = servers[si];
        cout << "Server #" << si << ":" << endl;

      
        for (size_t i = 0; i < s.v_listening.size(); ++i)
            cout << "    - " << s.v_listening[i].ip_addr << ":" << s.v_listening[i].Port << endl;

        
        cout << "  error_page.path: " << s.error.error.html_path << endl;

       
        cout << "  max_body_size: " << s.max_body_size << endl;

       
        cout << "  location.root: " << s.location.root << endl;
        cout << "  location.index: " << s.location.index << endl;
        cout << "  location.autoindex: " << (s.location.autoindex ? "ON" : "OFF") << endl;
        cout << "  location.methods: "; printStringVector(s.location.methods); cout << endl;

       
        cout << "  upload.root: " << s.location_upload.root << endl;
        cout << "  upload.upload_store: " << s.location_upload.upload_store << endl;
        cout << "  upload.methods: "; printStringVector(s.location_upload.methods); cout << endl;


        cout << "  images.root: " << s.location_images.root << endl;
        cout << "  images.methods: "; printStringVector(s.location_images.methods); cout << endl;

        cout << "  cgi_bin.root: " << s.cgi_bin.root << endl;
        cout << "  cgi_bin.cgi_pass: " << s.cgi_bin.cgi_pass << endl;
        cout << "  cgi_bin.methods: "; printStringVector(s.cgi_bin.methods); cout << endl;

        cout << "------------------------------" << endl;
    }
    cout << "=== end v_srv ===" << endl;
}


vector <Server>  v_srv;

int main(int argc, char **argv)
{
    // update : address.sin_addr.s_addr = INADDR_ANY;   to srv->listennig.ip_addr;
    if (argc != 2)
    {
        cout << "Wrong Arguments : ./server <conf_file>\n";
        return (1);
    }

    Parser parser (argv[1]);

    v_srv = parser.getServers();

    if (v_srv.empty())
    {
        cout << "No servers found in configuration\n";
        return 1;
    }

    try
    {
        EventHandler eventHandler;
        int backlog = 10;

        // Create server sockets for each listening address/port
        for (size_t j = 0; j < v_srv.size(); j++)
        {
            for (size_t i = 0; i < v_srv[j].v_listening.size(); ++i)
            {
                Socket* serverSocket = new Socket();

                try
                {
                    serverSocket->create();
                    serverSocket->bind(v_srv[j].v_listening[i].Port, v_srv[j].v_listening[i].ip_addr);
                    serverSocket->listen(backlog);
                }
                catch (const std::exception& e) 
                {
                    if (serverSocket)
                        delete serverSocket;
                    std::cerr << "Error: " << e.what() << std::endl;

                    throw std::runtime_error("Failed to set up  socket.");
                }

                std::cout << "Server listening on " << v_srv[j].v_listening[i].ip_addr 
                          << ":" << v_srv[j].v_listening[i].Port << std::endl;

                eventHandler.addFd(serverSocket->getFd(), POLLIN);
                serverSockets.push_back(serverSocket);
            }
        }

        cout << "##Start main loop##\n";
        while (true)
        {
            int num_events = eventHandler.pollEvents(-1);

            if (num_events > 0)
            {
                const std::vector<pollfd>& fds = eventHandler.getPollFds();
                for (size_t i = 0; i < fds.size(); ++i)
                {
                    if (isServerSocket(fds[i].fd))
                    {
                        if (fds[i].revents & POLLIN)
                        {
                            for (size_t j = 0; j < serverSockets.size(); ++j)
                            {
                                if (serverSockets[j]->getFd() == fds[i].fd)
                                {
                                    handleNewConnection(*serverSockets[j], eventHandler);
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        if (fds[i].revents & POLLIN)
                        {
                            handleClientData(fds[i].fd, eventHandler);
                        }
                        if (fds[i].revents & (POLLHUP | POLLERR))
                        {
                            std::cout << "Client " << fds[i].fd << " hung up or error." << std::endl;
                            eventHandler.removeFd(fds[i].fd);
                            close(fds[i].fd);
                            clientParsers.erase(fds[i].fd);
                            clientBuffers.erase(fds[i].fd);
                            cout << "connection just closed for " << fds[i].fd << endl;
                        }
                    }
                }
            }
        }
    } 
    catch (const std::exception& e) 
    {
        std::cerr << "Server error: " << e.what() << std::endl;
        
        for (size_t i = 0; i < serverSockets.size(); ++i)
        {
            delete serverSockets[i];
        }

        while (v_srv.size() != 0)
            v_srv.pop_back ();
        
        return 1;
    }
    for (size_t i = 0; i < serverSockets.size(); ++i)
    {
        delete serverSockets[i];
    }
     while (v_srv.size() != 0)
            v_srv.pop_back ();

    return 0;
}