#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <functional>

#include "Channel.h"

#define MAX_LINE 1024
#define MAX_EVENTS 500
#define MAX_LISTENFD 50

using namespace std;
using std::placeholders::_1;
using std::placeholders::_2;

typedef vector<struct epoll_event> EventList;

class TcpServer{
public:
    TcpServer();
    TcpServer(int argc, char **argv);
    ~TcpServer();
    int createAndListen(int port);
    void initEpoll(int sockfd, int events);
    void start();

private:
    int _argc;
    vector<string> _argv;

    int _port;
    int _epollfd;
    int _listenfd;
    char line[MAX_LINE];
    EventList _events = vector<struct epoll_event>(MAX_EVENTS);
};

#endif