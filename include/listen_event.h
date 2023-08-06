#ifndef _LISTEN_EVENT_H_
#define _LISTEN_EVENT_H_

#include "event.h"

class listen_event : public event_handler {
public:
    listen_event(int epfd, uint16_t listen_port = -1): event_handler(epfd, -1), port(listen_port) {}
    ~listen_event();

    uint32_t start_server();
    void stop_server();
public:
    uint32_t on_event(uint32_t event);
private:
    void handle_client(int fd, struct sockaddr_in &remote);
    void processError();
    void processNewConn();
private:

    uint16_t port;
};
#endif