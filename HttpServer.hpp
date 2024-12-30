#pragma once

#include <iostream>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include "TcpServer.hpp"
#include "Task.hpp"
#include "ThreadPool.hpp"
#include "Log.hpp"
#include "epoll.hpp"
#define PORT 8081

const int MAX_EVENTS = 1024;
//HTTP服务器
class HttpServer{
    private:
        int _port; //端口号
    public:
        HttpServer(int port)
            :_port(port)
        {}

        //初始化服务器
        void InitServer()
        {
            signal(SIGPIPE, SIG_IGN); //忽略SIGPIPE信号，防止写入时崩溃
        }

        //启动服务器
        void Loop()
        {
            LOG(INFO, "loop begin");
            TcpServer* tsvr = TcpServer::GetInstance(_port); //获取TCP服务器单例对象
            int listen_sock = tsvr->Sock(); //获取监听套接字
            epollevent* epollinstance = epollevent::createpoll();
            int epoll_fd = epollinstance->returnfd();
            epollinstance->addsockettoepoll(epoll_fd, listen_sock);
            while (true) {
                struct epoll_event events[MAX_EVENTS];
                int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
                if (num_events < 0) {
                    LOG(FATAL,"Error in epoll_wait");
                    break;
                }
                for (int i = 0; i < num_events; ++i) {
                    int current_fd = events[i].data.fd;
                    if (current_fd == listen_sock) {
                        struct sockaddr_in peer;
                        memset(&peer, 0, sizeof(peer));
                        socklen_t len = sizeof(peer);
                        int sock = accept(listen_sock, (struct sockaddr*)&peer, &len); //获取新连接
                        if(sock < 0){
                            continue; //获取失败，继续获取
                        }
                        epollinstance->addsockettoepoll(epoll_fd,sock);

                        //打印客户端相关信息
                        std::string client_ip = inet_ntoa(peer.sin_addr);
                        int client_port = ntohs(peer.sin_port);
                        LOG(INFO, "get a new link: ["+client_ip+":"+std::to_string(client_port)+"]");
                    }
                    else {
                        LOG(INFO,"new task");
                        Task task(current_fd);
                        ThreadPool::GetInstance()->PushTask(task);
                    }
                }
            }
                //构建任务并放入任务队列中

                //创建新线程处理新连接发起的HTTP请求
                //int* p = new int(sock);
                //pthread_t tid;
                //pthread_create(&tid, nullptr, CallBack::HandlerRequest, (void*)p);
                //pthread_detach(tid); //线程分离
        }
        ~HttpServer()
        {}
};
