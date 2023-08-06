#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "event.h"

const int RW_BUFF_SIZE = 8192;
const uint32_t SOCK_EVENT = 0;
const uint32_t SHELL_EVENT = 1;

const uint32_t DATA_IN_EVENT = 0;
const uint32_t DATA_OUT_EVENT = 1;
const uint32_t MODE_SHELL = 0;
const uint32_t MODE_DEBUG = 1;

class client {
public:
    client(int epfd, int connfd);
    ~client();
    uint32_t start();
    uint32_t on_event(uint32_t type, uint32_t event);
private:
    uint32_t create_shell();
    bool switch_mode(char *buff, uint32_t lens);
    uint32_t shell_mode_event(uint32_t type, uint32_t event);
    uint32_t debug_mode_event(uint32_t type, uint32_t event);
    uint32_t handle_debug(char *buff, uint32_t size);
    uint32_t handle_cmd(char *buff, uint32_t size);
    int redir_stdout();
    int resore_stdout();
private:
	int sockfd = -1;
    int ptyfd = -1;
    int epfd = -1;
	int shell_pid = -1;
    uint32_t mode = 0;
    int stdout_bak = -1;
	/* two circular buffers */
	char sockbuf[RW_BUFF_SIZE];
    char shellbuf[RW_BUFF_SIZE];
	int rdidx1, wridx1, size1;
	int rdidx2, wridx2, size2;

    event_handler *shell = nullptr;
    event_handler *sock = nullptr;
};


#endif
