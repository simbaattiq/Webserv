#ifndef EVENT_HANDLER_HPP
#define EVENT_HANDLER_HPP

#include <vector>
#include <poll.h>
#include <stdexcept>

#include <cerrno>
#include <cstdio>

class EventHandler 
{

private:
    std::vector<pollfd> _pollfds;

public:
    EventHandler();
    ~EventHandler();
    EventHandler(const EventHandler&);
    EventHandler& operator=(const EventHandler&);

    void addFd(int fd, short events);
    void removeFd(int fd);
    int pollEvents(int timeout_ms);
    const std::vector<pollfd>& getPollFds() const;
    void modifyFdEvents(int fd, short events);


};

#endif