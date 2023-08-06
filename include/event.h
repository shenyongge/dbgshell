#ifndef _EPOLL_EVENT_H_
#define _EPOLL_EVENT_H_

#include <cstdint>

class event_handler {
public:
    event_handler(int epfd, int selfd);
    event_handler() = default;
    virtual ~event_handler();
    virtual uint32_t on_event(uint32_t event) = 0;

    int add_event();
    int rmv_event();

    int get_selfd() const
    {
        return selfd;
    }
    void set_selfd(int fd) 
    {
        selfd = fd;
    }
    int get_epfd() const 
    {
        return epfd;
    }
    void set_epfd(int fd) 
    {
        epfd = fd;
    }
private:
    int epfd = -1;
    int selfd = -1;
};

int create_epfd();
void process_event(int epfd, bool running);

#endif