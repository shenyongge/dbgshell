#include "client.h"
#include <new>
#include <pty.h>
#include <unistd.h>
#include <errno.h> 
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include "io_event.h"
#include "logger.h"


client::~client()
{
    delete sock;
    delete shell;
    kill(shell_pid, SIGKILL);
}

client::client(int epfd, int connfd): epfd(epfd), sockfd(connfd)
{

}

int client::redir_stdout()
{
    stdout_bak = dup(STDOUT_FILENO);
    return dup2(sockfd, STDOUT_FILENO); 
}

int client::resore_stdout()
{   
    int fd = stdout_bak;
    stdout_bak = -1;
    return dup2(fd, STDOUT_FILENO); 
}

bool client::switch_mode(char *buff, uint32_t lens)
{
    uint32_t mode_change = 0;
    if (strcmp(buff, "shellmode\r\n") == 0) {
        mode = MODE_SHELL;
        log_info("switch_mode to shellmode!");
        return true;
    } else if (strcmp(buff, "debugmode\r\n") == 0)  {
        mode = MODE_DEBUG;
        log_info("switch_mode to shellmode!");
        return true;
    }
    return false;
}

uint32_t client::on_event(uint32_t type, uint32_t event)
{
    if (mode == MODE_SHELL) {
        return shell_mode_event(type, event);
    } else {
        return debug_mode_event(type, event);
    }
}

uint32_t client::handle_debug(char *buff, uint32_t size) 
{
    redir_stdout();
    handle_cmd(buff, size);
    resore_stdout();
}

uint32_t client::handle_cmd(char *buff, uint32_t size) 
{
    redir_stdout();
    buff[size] = '\0';
    printf("%s", buff);
    resore_stdout();
}

uint32_t client::debug_mode_event(uint32_t type, uint32_t event)
{
    if (event == DATA_IN_EVENT) {
        if (type == SOCK_EVENT) {
            int ret = read(sockfd, &sockbuf[0], RW_BUFF_SIZE);
            if (ret > 0) {
                if (switch_mode(&sockbuf[0], ret)) {
                    return 0;
                }
                return handle_debug(&sockbuf[0], ret);
            }else if (ret == 0) { // close
                return -1; 
            } else if (ret == EAGAIN) {
                return 0;
            } else {
                return 0;
            }
        } 
    }
}

uint32_t client::shell_mode_event(uint32_t type, uint32_t event)
{
    if (event == DATA_IN_EVENT) {
        if (type == SOCK_EVENT) {
            int ret = read(sockfd, &sockbuf[0], RW_BUFF_SIZE);
            if (ret > 0) {
                if (switch_mode(&sockbuf[0], ret)) {
                    return 0;
                }
                write(ptyfd, &sockbuf[0], ret);
            }else if (ret == 0) { // close
                return -1; 
            } else if (ret == EAGAIN) {
                return 0;
            } else {
                return 0;
            }
        } else if (type == SHELL_EVENT) {
            int ret = read(ptyfd, &shellbuf[0], RW_BUFF_SIZE);
            if (ret == 0) { // close
                return -1; 
            } else if (ret == EAGAIN) {
                return 0;
            } else if (ret > 0) {
                write(sockfd, &shellbuf[0], ret);
            } 
        }
    }
}

uint32_t client::create_shell()
{
    shell_pid = forkpty(&ptyfd, NULL, NULL, NULL);
    if(shell_pid == 0) { // child
        setsid();
        execl("/bin/bash", "bash", NULL);
        _exit(0);
        return 0;
    } else {
        shell = new io_event(epfd, ptyfd, SHELL_EVENT, *this); 
        if (shell == nullptr) {
            return -1;
        }
        int ret = shell->add_event();
        if (ret != 0) {
            return -1;
        }
    }
    return 0;
}

uint32_t client::start()
{
    uint32_t ret = create_shell();
    if (ret != 0) {
        return ret;
    }
    sock = new io_event(epfd, sockfd, SOCK_EVENT, *this); 
    if (sock == nullptr) {
        return -1;
    }

    ret = sock->add_event();
    if (ret != 0) {
        return -1;
    }
    return 0;
}