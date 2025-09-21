#include "EventHandler.hpp"
#include <iostream>

EventHandler::EventHandler() {}

EventHandler::~EventHandler() {}

void EventHandler::addFd(int fd, short events) 
{
    pollfd pfd;
    pfd.fd = fd;
    pfd.events = events;
    pfd.revents = 0;
    _pollfds.push_back(pfd);
}

void EventHandler::removeFd(int fd)
{
    for (std::vector<pollfd>::iterator it = _pollfds.begin(); it != _pollfds.end(); ++it)
    {
        if (it->fd == fd)
        {
            _pollfds.erase(it);
            return;
        }
    }
}

void EventHandler::modifyFdEvents(int fd, short events)
{
    for (size_t i = 0; i < _pollfds.size(); ++i)
    {
        if (_pollfds[i].fd == fd)
        {
            _pollfds[i].events = events;
            return;
        }
    }
}

int EventHandler::pollEvents(int timeout_ms)
{
    int num_events = poll(&_pollfds[0], _pollfds.size(), timeout_ms);
    if (num_events == -1)
    {
        perror("poll");
        throw std::runtime_error("Poll failed.");
    }
    return num_events;
}

const std::vector<pollfd>& EventHandler::getPollFds() const
{
    return _pollfds;
}
