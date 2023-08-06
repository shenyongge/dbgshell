#include "event.h"
#include <sys/epoll.h>
#include "logger.h"

event_handler::event_handler(int epfd, int selfd): epfd(epfd), selfd(selfd) {

}

event_handler::~event_handler()
{
    rmv_event();
    epfd = -1;
    selfd = -1;
}

int event_handler::add_event()
{
    struct epoll_event ev;
    // 设置与要处理的事件相关的文件描述符     
    ev.data.ptr = this;     
    // 设置要处理的事件类型     
    ev.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET;  
    log_info("epoll_add: epfd = %d, fd = %d, event = 0x%x, data = %p\n",
        epfd, selfd, ev.events, ev.data.ptr);    
    // 注册epoll事件   
    return epoll_ctl(epfd, EPOLL_CTL_ADD, selfd, &ev);
}

int event_handler::rmv_event()
{
    struct epoll_event ev;
    // 设置与要处理的事件相关的文件描述符     
    ev.data.ptr = this;     
    // 设置要处理的事件类型     
    ev.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET; 
    log_info("epoll_del: epfd = %d, fd = %d, event = 0x%x, data = %p\n",
        epfd, selfd, ev.events, ev.data.ptr);       
    // DEL epoll事件   
    return epoll_ctl(epfd, EPOLL_CTL_DEL, selfd, &ev);
}



void process_event(int epfd, bool running)
{
    const int MAXEVENTS = 16;                //最大事件数
    struct epoll_event events[MAXEVENTS];    //监听事件数组
    while (running)  {
        //等待epoll事件的发生,如果当前有信号的句柄数大于输出事件数组的最大大小,超过部分会在下次epoll_wait时输出,事件不会丢        
        int nfds = epoll_wait(epfd, events, MAXEVENTS, 500);
        for (uint32_t i = 0; i < nfds; i++) {
            uint32_t eventid = events[i].events;
            event_handler *handler = (event_handler *)events[i].data.ptr;
            handler->on_event(eventid);
        }
    }
}


int create_epfd()
{
    const int EPOLL_NUM = 100;
   //创建epoll,对2.6.8以后的版本,其参数无效,只要大于0的数值就行,内核自己动态分配
    int epfd = epoll_create(EPOLL_NUM);
    log_warn("create_epfd: epfd = %d\n", epfd);  
    return epfd;
}

