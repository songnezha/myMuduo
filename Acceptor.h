#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include <functional>
#include "Channel.h"
#include "Define.h"

using std::placeholders::_1;

class Acceptor{
public:
    typedef std::function<void(int)> AcceptorCallBack;

    Acceptor(int epollfd, int port);
    ~Acceptor() = default;
    void initEpoll(int sockfd);
    void setCallBack(const AcceptorCallBack& acceptorCallBack);
    void start();

private:
    int createAndListen(int port);
    int _port;
    int _epollfd;
    int _listenfd;
    Channel* _pAcceptorChannel;
    AcceptorCallBack _acceptorCallBack;

};

#endif