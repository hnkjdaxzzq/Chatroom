#pragma
#include<unistd.h>

class Riobuf;
class Rio {
private:
    int fd;
    Riobuf *rp;
public:
    Rio(int);
    ~Rio();

    ssize_t rio_readn(void *usrbuf, size_t n);
    ssize_t rio_writen(void *usrbuf, size_t n);

    int rio_getFd();

    void rio_readinitb();
    ssize_t rio_read(char *usrbuf, size_t n);
    ssize_t rio_readlineb(void *usrbuf, size_t maxlen);
    ssize_t rio_readnb(void *usrbuf, size_t n);
};