#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <iostream>

#include "Acceptor.h"

Acceptor::Acceptor(int epollfd, int port)
    :_port(port)
    ,_epollfd(epollfd)
    ,_listenfd(-1)
    ,_pAcceptorChannel(nullptr)
    ,_acceptorCallBack(nullptr)
{
}

int Acceptor::createAndListen(int port){
    struct sockaddr_in serveraddr;

    _listenfd = socket(AF_INET, SOCK_STREAM, 0);
    //non blocking
    fcntl(_listenfd, F_SETFL, O_NONBLOCK);

    //打开地址复用
    int on = 1;
    setsockopt(_listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(port);

    int ret = bind(_listenfd, (sockaddr *)&serveraddr, sizeof(serveraddr));
    assert(ret != -1);
    ret = listen(_listenfd, MAX_LISTENFD);
    assert(ret != -1);

    return _listenfd;
}

void Acceptor::initEpoll(int sockfd){
    struct sockaddr_in clientaddr;
    socklen_t clinetlen = sizeof(clientaddr);
    int connfd = accept(_listenfd, (sockaddr *)&clientaddr, &clinetlen);
    assert(connfd >= 0);

    std::cout << "[+] New connection from " << inet_ntoa(clientaddr.sin_addr) 
            << ":" << ntohs(clientaddr.sin_port) << std::endl 
            << "[+] Accept socket fd is " << connfd << std::endl;

    //non blocking
    fcntl(connfd, F_SETFL, O_NONBLOCK);

    //注册epoll事件
    _acceptorCallBack(connfd);
}

void Acceptor::setCallBack(const AcceptorCallBack& acceptorCallBack){
    _acceptorCallBack = acceptorCallBack;
}

void Acceptor::start(){
    _listenfd = createAndListen(_port);
    _pAcceptorChannel = new Channel(_epollfd, _listenfd);
    _pAcceptorChannel->setListenfd(_listenfd);
    _pAcceptorChannel->setInitCallBack(std::bind(&Acceptor::initEpoll, this, _1));
    _pAcceptorChannel->enableReading();
}

