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

#define MAX_LINE 1024
#define MAX_EVENTS 500
#define MAX_LISTENFD 50

using namespace std;

typedef vector<struct epoll_event> EventList;

class TcpServer{
public:
    TcpServer();
    TcpServer(int argc, char **argv);
    ~TcpServer();
    void start();

private:
    int _argc;
    int port;
    vector<string> _argv;

    void modify_event(int epollfd, int fd, int state);
};

#endif