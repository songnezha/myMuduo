#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <sys/epoll.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

#include "Define.h"
#include "Channel.h"
#include "Acceptor.h"
#include "TcpConnection.h"

using namespace std;
using std::placeholders::_1;

typedef vector<struct epoll_event> EventList;

class TcpServer{
public:
    TcpServer();
    TcpServer(int argc, char **argv);
    ~TcpServer();
    void newConnection(int sockfd);
    void start();

private:
    int _argc;
    vector<string> _argv;

    int _port;
    int _epollfd;
    int _listenfd;
    unique_ptr<Acceptor> _pAcceptor;
    //Acceptor* _pAcceptor;
    unordered_map<int, shared_ptr<TcpConnection>> _connections;
    EventList _events = vector<struct epoll_event>(MAX_EVENTS);
};

#endif