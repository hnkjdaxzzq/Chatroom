#include<cstdio>
#include<cstring>
#include <string>
#include <strings.h>
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
    clnt_addr.sin_addr.s_addr = inet_addr("183.169.76.140");
    clnt_addr.sin_port = ntohs(8888);

    errif(connect(clnt_fd, (sockaddr*)&clnt_addr, sizeof(clnt_addr)) == -1, "socket connect error");
    printf("connection successful!!!\n");
    
    // std::string httpreq = "GET /index.html HTTP/1.1\r\n"
    //                         "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7\r\n"
    //                         "Accept-Encoding: gzip, deflate, br\r\n"
    //                         "Accept-Language: zh-CN,zh;q=0.9\r\n"
    //                         "\r\n";
    // int write_byte = write(clnt_fd, httpreq.c_str(), httpreq.size());
    // if(write_byte == -1)
    //     perror("httprequest failed");
    // printf("send successful\n");
    // char buf[5555555];
    // bzero(buf, sizeof(buf));
    // int read_byte = read(clnt_fd, buf, sizeof(buf));
    // if(read_byte == -1)
    //     perror("httpresponse failed");
    // printf("buf%s", buf);

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