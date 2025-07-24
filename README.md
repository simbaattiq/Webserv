# Webserv

A simple Server which starts by:

Listening for connections.

Then accepts new connections non-blockingly style.

Parsing incoming HTTP requests.

Constructing and sending Simple HTTP responses.
[...]
    

/* A Simple map to hold RequestParser instances for each client. */
std::map<int, RequestParser> clientParsers;



// if client_fd is -1, it means no pending connection (non-blocking accept)
// or an error occurred, which is handled by Socket::accept()
void handleNewConnection(Socket& serverSocket, EventHandler& eventHandler);


Still Needs Configuration file support and more
