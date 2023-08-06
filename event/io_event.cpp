#include "io_event.h"
#include <sys/epoll.h>
#include <unistd.h> 
#include "logger.h"

io_event::~io_event()
{
    close_conn();
}

void io_event::close_conn()
{
    int connfd = get_selfd();
    rmv_event();
    close(connfd);
}
uint32_t io_event::on_event(uint32_t event)
{
    if (event & EPOLLIN) {
        cli.on_event(type, DATA_IN_EVENT);
    } else if (event & EPOLLOUT) {
        cli.on_event(type, DATA_OUT_EVENT);
    }

    return 0;
}

void io_event::process_data()
{
    int connfd = get_selfd();
}
