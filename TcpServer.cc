#include "TcpServer.h"

TcpServer::TcpServer(int argc, char **argv) 
    :_argc(argc)
    ,_argv(argv+1, argv+argc) 
    ,_epollfd(-1)
    ,_listenfd(-1)
{
    if( 2 == argc ){
        _port = atoi(argv[1]);
        cout << "[+] Listen from localhost:" << _port <<endl;
    }
    else{
        cout << "[+] Usage:" << argv[0] << " port" <<endl;
        exit(1);
    }
}

TcpServer::~TcpServer(){

}

int TcpServer::createAndListen(int port){
    struct sockaddr_in serveraddr;

    int _listenfd = socket(AF_INET, SOCK_STREAM, 0);
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

void TcpServer::initEpoll(int sockfd, int events){
    int n;

    if(sockfd == _listenfd){
        struct sockaddr_in clientaddr;
        socklen_t clinetlen = sizeof(clientaddr);
        int connfd = accept(_listenfd, (sockaddr *)&clientaddr, &clinetlen);
        assert(connfd >= 0);

        cout << "[+] New connection from " << inet_ntoa(clientaddr.sin_addr) 
                << ":" << ntohs(clientaddr.sin_port) << endl 
                << "[+] Accept socket fd is " << connfd <<endl;

        //non blocking
        fcntl(connfd, F_SETFL, O_NONBLOCK);

        //注册epoll事件
        Channel* pChannel = new Channel(_epollfd, connfd);
        pChannel->setCallBack(std::bind(&TcpServer::initEpoll, this, _1, _2));
        pChannel->enableReading();
    }
    else if(events & EPOLLIN){
        if( sockfd < 0){
            cout << "[+] error: EPOLLIN socket < 0" <<endl;
            return;
        } 

        bzero(line, sizeof(line));
        
        if( (n = read(sockfd, line, sizeof(line))) < 0){
            cout<< "[+] Read error" << endl;
            close(sockfd);
        }
        else if(n == 0){
            cout << "[+] Read complete, close sockfd " << sockfd << endl;
            close(sockfd);
        }
        else{
            cout << "[+] Read message:" << line;

            if( (n = write(sockfd ,line, strlen(line))) < 0){
                cout << "[+] write error" << endl;
                close(sockfd);
            }
        }
    }
    //连接断开
    if(events & (EPOLLHUP | EPOLLRDHUP)){
        struct epoll_event ev;
        ev.data.fd = sockfd;
        ev.events = events;
        cout << "[+] Sockfd "<< sockfd <<" connection closed" << endl;
        epoll_ctl(_epollfd, EPOLL_CTL_DEL, sockfd, &ev);
        close(sockfd);
        return;
    }
}

void TcpServer::start(){
    //生成用于处理accept的epoll专用的文件描述符
    _epollfd = epoll_create1(EPOLL_CLOEXEC);
    assert(_epollfd >= 0);

    _listenfd = createAndListen(_port);

    Channel* pChannel = new Channel(_epollfd, _listenfd);
    pChannel->setCallBack(std::bind(&TcpServer::initEpoll, this, _1, _2));
    pChannel->enableReading();

    while(true){
        vector<Channel*> channels;
        int fds = epoll_wait(_epollfd, _events.data(), MAX_EVENTS, -1);
        assert(fds != -1);

        for(int i=0; i<fds; i++){
            Channel *pChannel = static_cast<Channel*>(_events[i].data.ptr);
            pChannel->setRevents(_events[i].events);
            channels.push_back(pChannel);
        }

        for(auto it : channels){
            it->handleEvent();
        }
    }
}