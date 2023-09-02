#include<cstdio>
#include"include/Socket.h"
#include"include/InetAddress.h"
#include<unistd.h>

const int BUF_SIZE = 1024;

int main() {
    Socket* serv_sock = new Socket();
    InetAddress serv_addr = InetAddress("127.0.0.1", 8888);
    serv_sock->bind(serv_addr);
    printf("Server start at %s:%4d\n", inet_ntoa(serv_addr.getAddr().sin_addr), ntohs(serv_addr.getAddr().sin_port));
    serv_sock->listen();
    while(true) {
        InetAddress clnt_addr;
        int clnt_fd = serv_sock->accept(clnt_addr);
        errif(clnt_fd == -1, "client connect error");
        char buf[BUF_SIZE];
        bzero(buf, sizeof(buf));
        int read_byte = read(clnt_fd, buf, sizeof(buf));
        if(read_byte > 0) {
            printf("Client[%s:%d] say: %s\n", inet_ntoa(clnt_addr.getAddr().sin_addr), ntohs(clnt_addr.getAddr().sin_port),buf);
        }
        write(clnt_fd, buf, sizeof(buf));
    }

}