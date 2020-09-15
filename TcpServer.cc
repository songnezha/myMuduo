#include "TcpServer.h"

TcpServer::TcpServer(int argc, char **argv) : _argc(argc), _argv(argv+1, argv+argc) {
    if( 2 == argc ){
        port = atoi(argv[1]);
        cout << "[+] Listen from localhost:" << port <<endl;
    }
    else{
        cout << "[+] Usage:" << argv[0] << " port" <<endl;
        exit(1);
    }
}

TcpServer::~TcpServer(){

}

void TcpServer::modify_event(int epollfd, int sockfd, int state){
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = sockfd;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, sockfd, &ev);
}

void TcpServer::start(){
    int listenfd, connfd, sockfd, epollfd;
    int n;
    char line[MAX_LINE];

    //声明epoll_event结构体的变量,ev用于注册事件,数组用于回传要处理的事件
    struct epoll_event ev;
    EventList events(MAX_EVENTS);

    struct sockaddr_in clientaddr;
    struct sockaddr_in serveraddr;

    socklen_t clinetlen;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    //non blocking
    fcntl(listenfd, F_SETFL, O_NONBLOCK);

    //打开地址复用
    int on = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    //生成用于处理accept的epoll专用的文件描述符
    epollfd = epoll_create1(EPOLL_CLOEXEC);
    assert(epollfd >= 0);

    //设置与要处理的事件相关的文件描述符
    ev.data.fd = listenfd;
    //设置要处理的事件类型(LT)
    ev.events = EPOLLIN;

    //注册epoll事件
    epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev);
    
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(port);

    int ret = bind(listenfd, (sockaddr *)&serveraddr, sizeof(serveraddr));
    assert(ret != -1);
    ret = listen(listenfd, MAX_LISTENFD);
    assert(ret != -1);

    while(true){
        int fds = epoll_wait(epollfd, events.data(), MAX_EVENTS, -1);
        assert(fds != -1);

        for(int i=0; i<fds; i++){
            //如果新监测到一个SOCKET用户连接到了绑定的SOCKET端口，建立新的连接。
            if(events[i].data.fd == listenfd){
                connfd = accept(listenfd, (sockaddr *)&clientaddr, &clinetlen);
                assert(connfd >= 0);

                cout << "[+] New connection from " << inet_ntoa(clientaddr.sin_addr) 
                        << ":" << ntohs(clientaddr.sin_port) << endl 
                        << "[+] Accept socket fd is " << connfd <<endl;

                //non blocking
                fcntl(connfd, F_SETFL, O_NONBLOCK);

                //注册epoll事件
                ev.data.fd = connfd;
                ev.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP;
                epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &ev);
            }
            //如果是已经连接的用户，并且收到数据，进行读入。
            else if(events[i].events & EPOLLIN){
                if( (sockfd = events[i].data.fd) < 0){
                    cout << "[+] error: EPOLLIN socket < 0" <<endl;
                    continue;
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
                    
                    //modify event
                    modify_event(epollfd, sockfd, EPOLLOUT);
                }
            }
            else if(events[i].data.fd & EPOLLOUT){
                if( (n = write(sockfd ,line, strlen(line))) < 0){
                    cout << "[+] write error" << endl;
                    close(sockfd);
                }
                else modify_event(epollfd, sockfd, EPOLLIN);
            }
            //连接断开
            if(events[i].events & (EPOLLHUP | EPOLLRDHUP)){
                sockfd =  events[i].data.fd;
                ev = events[i];
                cout << "[+] Sockfd "<< sockfd <<" connection closed" << endl;
                epoll_ctl(epollfd, EPOLL_CTL_DEL, sockfd, &ev);
                close(sockfd);
                continue;
            }
        }
    }
}