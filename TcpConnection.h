#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include "Channel.h"
#include "Define.h"

using std::placeholders::_1;
using std::placeholders::_2;

class TcpConnection{
public:
    TcpConnection(int epollfd, int sockfd);
    ~TcpConnection() = default;
    void readEpoll(int sockfd, int events);

private:
    int _epollfd;
    int _sockfd;
    char line[MAX_LINE];
    Channel* _pChannel;
};

#endif