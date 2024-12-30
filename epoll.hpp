#include <iostream>
#include <cstring>
#include <string>
#include <cstdlib>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include "Log.hpp"

class epollevent{
    private:
        int epoll_fd;
        static epollevent* eventptr;
    private:
        epollevent(int _fd)
            :epoll_fd(_fd)
        {}
        epollevent(const epollevent&) = delete;
        epollevent* operator=(const epollevent&) = delete; 
    public:
        static epollevent* createpoll(){
            if(eventptr == nullptr){
                int epollfd = epoll_create1(0);
                if (epollfd < 0) {
                    LOG(FATAL, "epoll error!");
                    exit(1);
                }
                eventptr = new epollevent(epollfd);
                LOG(INFO, "epoll success!");
            }
            return eventptr;
        }
        int returnfd(){
            return epoll_fd;
        }
        void addsockettoepoll(int epoll_fd, int socket_fd){

            // 设置 socket 为非阻塞模式
            if (fcntl(socket_fd, F_SETFL, O_NONBLOCK) < 0) {
                LOG(FATAL,"fcntl failed");
                return;
            }

            struct epoll_event event;
            event.events = EPOLLIN | EPOLLET;
            event.data.fd = socket_fd;
            int epoll_ctl_result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event);
            if (epoll_ctl_result < 0) {
                LOG(FATAL, "socket add error!");
                exit(1);
            }
        }
        ~epollevent(){
            if(epoll_fd >= 0){
                close(epoll_fd);
            }
        }

};
epollevent* epollevent::eventptr = nullptr;