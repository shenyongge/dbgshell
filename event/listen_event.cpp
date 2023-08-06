#include "listen_event.h"
#include <cstdint>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netinet/tcp.h> 
#include <fcntl.h> 
#include <errno.h> 
#include <unistd.h> 
#include <stdlib.h> 
#include "logger.h"
#include "client.h"


void set_close_onfork(int fd)
{
    int flags = fcntl(fd, F_GETFD);  
    flags |= FD_CLOEXEC;  
    fcntl(fd, F_SETFD, flags);  
}

uint32_t setnonblocking(int fd) 
{     
    int opts;     
    opts = fcntl(fd, F_GETFL);     
    if (opts < 0)     
    {          
        log_error("fcntl(fd = %d, GETFL) return %d\n", fd, opts);   
        return -1;     
    }     
    opts = opts|O_NONBLOCK;     
    if (fcntl(fd, F_SETFL, opts) < 0)     
    {         
        log_error("fcntl(fd = %d,SETFL, opts = %d) failed\n", fd, opts);         
        return -1;       
    }  
}

uint32_t setreuseaddr(int fd)
{
    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(&opt)) < 0)     
    {         
        log_error("setsockopt(fd = %d, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(&opt))) failed\n", fd);       
        return -1;     
    }  
}



int create_listen_fd(uint16_t sport)
{
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd < 0)
    {
        log_error("create_listen_fd %d failed\n", listenfd);       
        return -1;
    }

    pid_t pid = getpid();
    if (sport == UINT16_MAX) {
        sport = 50000 + pid % 10000;
    }


    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    printf("PID = %d, port = %d\n", pid, sport);
    log_warn("PID = %d listening on port = %d\n", pid, sport);  
    addr.sin_port = htons(sport);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if(bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        log_error("listenfd = %d bind failed!\n", listenfd);   
        return -2;
    }

    if(listen(listenfd, 20) < 0)
    {
        log_error("listenfd = %d listen failed!\n", listenfd); 
        return -3;
    }

    //把监听socket设置为非阻塞方式     
    setnonblocking(listenfd);
    //设置监听socket为端口重用 
    setreuseaddr(listenfd);

    set_close_onfork(listenfd);
    return listenfd;
}

listen_event::~listen_event()
{
    stop_server();
}

uint32_t listen_event::on_event(uint32_t event)
{
    if ((event & EPOLLERR) || (event & EPOLLHUP)) { // //有异常发生
        processError();
    }
    if (event & EPOLLIN) {    // 有连接到来
        processNewConn();
    }
    log_warn("unkown event(%d)!\n", event);
    return 0;
}

uint32_t listen_event::start_server()
{
    int listenfd = create_listen_fd(port);
    if (listenfd < 0) {
        return -1;
    }
    set_selfd(listenfd);
    
    return add_event();
}

void listen_event::stop_server()
{
    int listenfd = get_selfd();
    rmv_event();
    close(listenfd);      
}

void listen_event::processError()
{
    stop_server();      
    // 重新初始化
    (void)start_server();
}

void listen_event::processNewConn()
{
    int listenfd = get_selfd();
    struct sockaddr_in remote;
    socklen_t addrlen = sizeof(remote);
    int conn_sock = 0;
    while ((conn_sock = accept(listenfd,(struct sockaddr *) &remote, &addrlen)) > 0) {
        handle_client(conn_sock, remote);
    }
    if (conn_sock == -1) {
        if (errno != EAGAIN && errno != ECONNABORTED && errno != EPROTO && errno != EINTR) {
            perror("accept");
        }

    }
}

void *get_in_addr(struct sockaddr *sa) // get sockaddr, IPv4 or IPv6:
{
    if(sa->sa_family == AF_INET) return &(((struct sockaddr_in*) sa)->sin_addr);
    else return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


void listen_event::handle_client(int fd, struct sockaddr_in &remote)
{
    setnonblocking(fd); 
    set_close_onfork(fd);
    int epfd = get_epfd();

    uint32_t addr = remote.sin_addr.s_addr;
    uint32_t ip[4] = {
        addr >> 24,
        (addr >> 16) & 0xff,
        (addr >> 8) & 0xff,
        addr & 0xff
    };
    log_warn("new connect (%d) form %d.%d.%d.%d:%d!\n", fd, 
        ip[0], ip[1], ip[2], ip[3], remote.sin_port);
    const char hello_msg[] = "Hello, you shell!\n";
    write(fd, hello_msg, sizeof(hello_msg) - 1);

    client *cli = new client(epfd, fd); 
    cli->start();
}



