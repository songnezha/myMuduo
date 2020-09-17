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

void TcpServer::newConnection(int sockfd){
    auto connection = make_shared<TcpConnection>(TcpConnection(_epollfd, sockfd));
    _connections[sockfd] = connection;
}

void TcpServer::start(){
    //生成用于处理accept的epoll专用的文件描述符
    _epollfd = epoll_create1(EPOLL_CLOEXEC);
    assert(_epollfd >= 0);

    _pAcceptor = std::move(unique_ptr<Acceptor>(new Acceptor(_epollfd, _port)));
    _pAcceptor->setCallBack(std::bind(&TcpServer::newConnection, this, _1));
    _pAcceptor->start();

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