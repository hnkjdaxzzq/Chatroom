#pragma once

const int RIO_BUFSIZE = 8192;

class Riobuf{
public:
    int rio_fd;
    int rio_cnt;  // Unread bytes in internal buf
    char *rio_bufptr;
    char rio_buf[RIO_BUFSIZE];
};