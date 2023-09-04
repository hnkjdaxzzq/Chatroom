#include "../include/Rio.h"
#include "../include/Riobuf.h"
#include<errno.h>
#include<cstring>
#include<unistd.h>


Rio::Rio(int _fd) : fd(_fd) {
    rp = new Riobuf;
    rio_readinitb();
}

Rio::~Rio() {
    delete rp;
    if(fd != -1) {
        close(fd);
        fd = -1;
    }
}

ssize_t Rio::rio_readn(void *usrbuf, size_t n) {
    size_t nleft = n;
    ssize_t nread;
    char *bufp = (char *)usrbuf;

    while(nleft > 0) {
        if((nread = read(fd, bufp, nleft)) < 0) {
            if(errno = EINTR) /* Interrupted by sig handler return */
                nread = 0;    /* and call read() again*/
            else 
                return -1;    /* errno set by read */
        } else if(nread == 0) 
            break;   // EOF
        nleft -= nread;
        bufp += nread;
    }    
    return (n - nleft);
}

ssize_t Rio::rio_writen(void *usrbuf, size_t n) {
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = (char*) usrbuf;

    while(nleft > 0) {
        if((nwritten = write(fd, bufp, nleft)) <= 0) {
            if(errno == EINTR)
                nwritten = 0;
            else
                return -1;
        }
        nleft -= nwritten;
        bufp += nwritten;
    }
    return n;
}

void Rio::rio_readinitb() {
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

ssize_t Rio::rio_read(char *usrbuf, size_t n) {
    int cnt;

    while(rp->rio_cnt <=0 ) {  // 如果缓冲区为空就得往缓冲区添加东西
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));

        if(rp->rio_cnt < 0) {
            if(errno != EINTR)
                return -1;
        }
        else if(rp->rio_cnt == 0) // EOF
            return 0;
        else
            rp->rio_bufptr = rp->rio_buf;
    }

    cnt = n;
    if(rp->rio_cnt < n)
        cnt = rp->rio_cnt;
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;
}

int Rio::rio_getFd() {
    return fd;
}

ssize_t Rio::rio_readlineb(void *usrbuf, size_t maxlen) {
    int n, rc;
    char c, *bufp = (char*)usrbuf;

    for(n = 1; n < maxlen; n++) {
        if((rc = rio_read(&c, 1)) == 1) {
            *bufp++ = c;
            if(c == '\n') {
                n++;
                break;
            }
        } else if( rc == 0) {
            if(n == 1)
                return 0; // EOF, no data read
            else
                break;    // EOF, some data was read
        } else
            return -1;    // Error
    }
    *bufp = 0;
    return n - 1;
}

ssize_t Rio::rio_readnb(void *usrbuf, size_t n) {
    size_t nleft = n;
    size_t nread;
    char *bufp = (char*)usrbuf;

    while( nleft > 0) {
        if((nread = rio_read(bufp, nleft)) < 0 )
            return -1;
        else if (nread == 0)
            break;
        nleft -= nread;
        bufp += nread;
    }
    return (n - nleft);
}