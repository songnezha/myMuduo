#ifndef CHANELL_H
#define CHANELL_H

#include <functional>

class  Channel{
public:
    typedef std::function<void(int, int)> CallBack;

    Channel(int epollfd, int sockfd);
    ~Channel() = default;
    void setCallBack(const CallBack &callBack);
    void handleEvent();
    void setRevents(int revent);
    int  getSockfd();
    void enableReading();

private:
    void update();
    int _epollfd;
    int _sockfd;
    int _events;
    int _revents;
    CallBack _callBack;
};

#endif