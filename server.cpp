#include<cstdio>
#include <functional>
#include <strings.h>
#include<unistd.h>
#include"include/EventLoop.h"
#include"include/Server.h"
#include <Connection.h>
#include <util.h>
#include <Http.h>

const int BUF_SIZE = 1024;

int main() {
    EventLoop *loop = new EventLoop();
    // auto task = [](Connection* con) {
    //     char buf[1024];
    //     while(true) {    //由于使用非阻塞IO，读取客户端buffer，一次读取buf大小数据，直到全部读取完毕
    //         bzero(&buf, sizeof(buf));
    //         ssize_t bytes_read = con->creadn(buf, sizeof(buf));
    //         if(bytes_read > 0){
    //             con->readBuffer.Append(buf);
    //         } else if(bytes_read == -1 && errno == EINTR){  //客户端正常中断、继续读取
    //             printf("continue reading");
    //             continue;
    //         } else if(bytes_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))){//非阻塞IO，这个条件表示数据全部读取完毕
    //             printf("message from client fd %d: %s\n", con->getFd(), con->readBuffer.c_str());
    //             errif(con->cwriten(con->readBuffer.c_str(), sizeof(buf)) == -1, "socket write error");
    //             con->readBuffer.clear();
    //             break;
    //         } else if(bytes_read == 0){  //EOF，客户端断开连接
    //             printf("EOF, client fd %d disconnected\n", con->getFd());
    //             // close(sockfd);   //关闭socket会自动将文件描述符从epoll树上移除
    //             con->deleteConnectionCallback(con->getSocket());
    //             break;
    //         }
    //     } 

    // };
    Http httpserv("./webapp");
    auto task = std::bind(&Http::process, &httpserv, std::placeholders::_1);
    Server *server = new Server(loop, task);    
    loop->loop();
    return 0;
    // Socket* serv_sock = new Socket();
    // InetAddress serv_addr = InetAddress("127.0.0.1", 8888);
    // serv_sock->bind(serv_addr);
    // printf("Server start at %s:%4d\n", inet_ntoa(serv_addr.getAddr().sin_addr), ntohs(serv_addr.getAddr().sin_port));
    // serv_sock->listen();
    // while(true) {
    //     InetAddress clnt_addr;
    //     int clnt_fd = serv_sock->accept(clnt_addr);
    //     errif(clnt_fd == -1, "client connect error");
    //     char buf[BUF_SIZE];
    //     bzero(buf, sizeof(buf));
    //     int read_byte = read(clnt_fd, buf, sizeof(buf));
    //     if(read_byte > 0) {
    //         printf("Client[%s:%d] say: %s\n", inet_ntoa(clnt_addr.getAddr().sin_addr), ntohs(clnt_addr.getAddr().sin_port),buf);
    //     }
    //     write(clnt_fd, buf, sizeof(buf));
    // }

}