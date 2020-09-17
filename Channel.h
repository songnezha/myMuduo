#ifndef CHANELL_H
#define CHANELL_H

#include <functional>

class  Channel{
public:
    typedef std::function<void(int)> InitCallBack;
    typedef std::function<void(int, int)> ReadCallBack;

    Channel(int epollfd, int sockfd);
    ~Channel() = default;
    void setInitCallBack(const InitCallBack &initCallBack);
    void setReadCallBack(const ReadCallBack &readCallBack);
    void handleEvent();
    void setRevents(int revent);
    void setListenfd(int listenfd);
    int  getSockfd();
    void enableReading();

private:
    void update();
    int _epollfd;
    int _sockfd;
    int _events;
    int _revents;
    int _listenfd;
    InitCallBack _initCallBack;
    ReadCallBack _readCallBack;
};

#endif