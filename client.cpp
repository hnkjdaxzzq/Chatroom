#include<cstdio>
#include<cstring>
#include<unistd.h>
#include<sys/socket.h>
#include"include/util.h"
#include<arpa/inet.h>

const int BUFSIZE = 2048;

int main() {
    int clnt_fd = socket(AF_INET, SOCK_STREAM, 0);
    errif(clnt_fd == -1, "socket create error");
    struct sockaddr_in clnt_addr;
    clnt_addr.sin_family = AF_INET;
    clnt_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    clnt_addr.sin_port = ntohs(8888);

    errif(connect(clnt_fd, (sockaddr*)&clnt_addr, sizeof(clnt_addr)) == -1, "socket connect error");
    printf("connection successful!!!\n");

    char buf[BUFSIZE];
    while(true) {
        bzero(buf, sizeof(buf));
        scanf("%s", buf);
        int write_byte = write(clnt_fd, buf, sizeof(buf));
        errif(write_byte == -1, "transfor message error");
        
        bzero(buf, sizeof(buf));
        int read_byte = read(clnt_fd, buf, sizeof(buf));
        errif(write_byte == -1, "read message error");
        printf("recive from [%s:%d]: %s\n", inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port), buf);
    }
    

}