#include "TcpServer.h"

int main(int argc, char **argv){
    TcpServer tcp(argc, argv);
    tcp.start();

    return 0;
}