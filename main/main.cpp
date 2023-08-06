#include "listen_event.h"
#include <stdlib.h> 
#include <unistd.h> 
#include "symbol.h" // tomoved

bool g_running = 1;

int main(int argc, char *argv[])
{
    uint16_t sport = UINT16_MAX;

    pid_t pid = getpid();
    colect_process_symbol(pid);
    if(argc == 2){
        sport = atoi(argv[1]);
    }

    int epfd = create_epfd();

    listen_event listener(epfd, sport);
    listener.start_server();
    
    process_event(epfd, g_running);

    close(epfd);
    return 0;
}