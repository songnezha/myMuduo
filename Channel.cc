#include "Channel.h"
#include <sys/epoll.h>
#include <iostream>

Channel::Channel(int epollfd, int sockfd)
    :_epollfd(epollfd)
    ,_sockfd(sockfd)
    ,_events(0)
    ,_revents(0)
    ,_initCallBack(nullptr)
    ,_readCallBack(nullptr)
{
}

void Channel::setInitCallBack(const InitCallBack &initCallBack){
    _initCallBack = initCallBack;
}

void Channel::setReadCallBack(const ReadCallBack &readCallBack){
    _readCallBack = readCallBack;
}

void Channel::setRevents(int revents){
    _revents = revents;
}

void Channel::setListenfd(int listenfd){
    _listenfd = listenfd;
}

void Channel::handleEvent(){
    if(_listenfd == _sockfd){
        _initCallBack(_sockfd);
    }
    else if(_revents & (EPOLLIN | EPOLLOUT | EPOLLHUP | EPOLLRDHUP)){
        _readCallBack(_sockfd, _events);
    }
}

int Channel::getSockfd(){
    return _sockfd;
}

void Channel::enableReading(){
    _events |= EPOLLIN ;
    update();
}

void Channel::update(){
    struct epoll_event ev;
    ev.data.ptr = this;
    ev.events = _events;
    epoll_ctl(_epollfd, EPOLL_CTL_ADD, _sockfd, &ev);
}
