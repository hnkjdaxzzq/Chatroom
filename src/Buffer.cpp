#include <Buffer.h>
#include <algorithm>
#include <bits/types/struct_iovec.h>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <string>
#include <strings.h>
#include <sys/types.h>
#include <assert.h>
#include <sys/uio.h>
#include <unistd.h>

// void Buffer::Append(const std::string& str) {
//     _buffer += str;
// }

// void Buffer::Append(const char* str) {
//     _buffer.append(str);
// }

// void Buffer::Append(const void* data, size_t len) {
//     void* tmp = new char[len];
//     std::memcpy(tmp, data, len);
//     const char* str = static_cast<const char*>(tmp);
//     _buffer.append(str);
// }

// size_t Buffer::size() const {
//     return _buffer.size();
// }

// const std::string Buffer::getBufText() const{
//     return _buffer;
// }

// const char* Buffer::c_str() const {
//     return _buffer.c_str();
// }

// void Buffer::clear() {
//     _buffer.clear();
// }

Buffer::Buffer(size_t initialSize) : buffer_(kcheapPrepend + initialSize), 
                                    readerIndex_(kcheapPrepend), 
                                    writerIndex_(kcheapPrepend)
    {}

size_t Buffer::ReadableBytes() const {
    return writerIndex_ - readerIndex_;
}

size_t Buffer::WriteableBytes() const {
    return buffer_.size() - writerIndex_;
}

size_t Buffer::PrependableBytes() const {
    return readerIndex_;
}

const char* Buffer::Peek() const {
    return BeginPtr_() + readerIndex_;
}

void Buffer::Retrieve(size_t len) {
    assert(len <= ReadableBytes());
    readerIndex_ += len;
}

void Buffer::RetrieveUntil(const char* end) {
    assert(Peek() <= end);
    Retrieve(end - Peek());
}

void Buffer::RetrieveAll() {
    bzero(&buffer_[0], buffer_.size());
    readerIndex_ = 0;
    writerIndex_ = 0;
}

std::string Buffer::RetrieveAllToStr() {
    std::string str(Peek(), ReadableBytes());
    RetrieveAll();
    return str;
}

std::string Buffer::GetBufferToStr() {
    std::string str(Peek(), ReadableBytes());
    return str;
}

const char* Buffer::BeginWriteConst() const {
    return BeginPtr_() + writerIndex_;
}

char * Buffer::BeginWrite() {
    return BeginPtr_() + writerIndex_;
}

void Buffer::HasWritten(size_t len) {
    writerIndex_ += len;
}

void Buffer::Append(const std::string& str) {
    Append(str.data(), str.length());
}

void Buffer::Append(const void* data, size_t len) {
    assert(data);
    Append(static_cast<const char*>(data), len);
}

void Buffer::Append(const char* str, size_t len) {
    assert(str);
    EnsureWriteable(len);
    std::copy(str, str + len, BeginWrite());
    HasWritten(len);
}

void Buffer::Append(const Buffer& buff) {
    Append(buff.Peek(), buff.ReadableBytes());
}

void Buffer::EnsureWriteable(size_t len) {
    if(WriteableBytes() < len) {
        MakeSpace_(len);
    }
    assert(WriteableBytes() >= len);
}

ssize_t Buffer::ReadFd(int fd, int *Errno) {
    char buff[65535];
    struct iovec iov[2];
    const size_t writable = WriteableBytes();
    /* 分别从缓冲区空间和栈空间中读，避免一开始就设置很大的缓冲区大小*/
    iov[0].iov_base = BeginWrite();
    iov[0].iov_len = writable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    const ssize_t len = readv(fd, iov, 2);
    if(len < 0) {
        *Errno = errno;
    }
    else if(static_cast<size_t>(len) <= writable) {
        writerIndex_ += len;
    }
    else {
        writerIndex_ = buffer_.size();
        Append(buff, len - writable);
    }
    return len;
}

ssize_t Buffer::WriteFd(int fd, int *Errno) {
    size_t readSize = ReadableBytes();
    if(readSize == 0)
        return 0;
    ssize_t len = write(fd, Peek(), readSize);
    if(len < 0) {
        *Errno = errno;
        return len;
    }
    readerIndex_ += len;
    return len;
}

char* Buffer::BeginPtr_() {
    return &*buffer_.begin();
}

const char* Buffer::BeginPtr_() const {
    return &*buffer_.begin();
}

void Buffer::MakeSpace_(size_t len ) {
    if(WriteableBytes() + PrependableBytes() < len) {
        buffer_.resize(writerIndex_ + len + 1);
    }
    else {
        size_t readable = ReadableBytes();
        std::copy(BeginPtr_() + readerIndex_, BeginPtr_() + writerIndex_, BeginPtr_());
        readerIndex_ = 0;
        writerIndex_ = readerIndex_ + readable;
        assert(readable == ReadableBytes());
    }
}