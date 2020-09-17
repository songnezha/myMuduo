#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "TcpConnection.h"
#include "Channel.h"
#include "Define.h"

#include <string.h> //for bzero
#include <iostream>

TcpConnection::TcpConnection(int epollfd, int sockfd)
    :_epollfd(epollfd)
    ,_sockfd(sockfd)
{
    _pChannel = new Channel(_epollfd, _sockfd);
    _pChannel->setReadCallBack(std::bind(&TcpConnection::readEpoll, this, _1, _2));
    _pChannel->enableReading();
}

void TcpConnection::readEpoll(int sockfd, int events){
    int n;
    //EPOLLIN
    if(events & EPOLLIN){
        if( sockfd < 0){
            std::cout << "[+] error: EPOLLIN socket < 0" <<std::endl;
            return;
        } 

        bzero(line, sizeof(line));
        
        if( (n = read(sockfd, line, sizeof(line))) < 0){
            std::cout<< "[+] Read error" << std::endl;
            close(sockfd);
        }
        else if(n == 0){
            std::cout << "[+] Read complete, close sockfd " << sockfd << std::endl;
            close(sockfd);
        }
        else{
            std::cout << "[+] Read message:" << line;

            if( (n = write(sockfd ,line, strlen(line))) < 0){
                std::cout << "[+] write error" << std::endl;
                close(sockfd);
            }
        }
    }
    //连接断开
    if(events & (EPOLLHUP | EPOLLRDHUP)){
        struct epoll_event ev;
        ev.data.fd = sockfd;
        ev.events = events;
        std::cout << "[+] Sockfd "<< sockfd <<" connection closed" << std::endl;
        epoll_ctl(_epollfd, EPOLL_CTL_DEL, sockfd, &ev);
        close(sockfd);
        return;
    }
}