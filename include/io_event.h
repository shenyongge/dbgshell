#ifndef _CONN_EVENT_H_
#define _CONN_EVENT_H_

#include "client.h"

class io_event: public event_handler
{
public:
    io_event(int epfd, int connfd, uint32_t type, client &cli): 
        event_handler(epfd, connfd), cli(cli), type(type) {}
    io_event(uint32_t type, client &cli): 
        event_handler(), cli(cli), type(type) {}
    ~io_event();
    uint32_t on_event(uint32_t event);
private:
    void process_data();
    void close_conn();  
private:
    client &cli;
    uint32_t type = 0;
};


#endif