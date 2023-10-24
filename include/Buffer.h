#pragma once

#include <cstddef>
#include <string>
#include <sys/types.h>
#include <vector>

// class Buffer {
// public:
//     Buffer() = default;
//     Buffer(const Buffer&) = delete;
//     Buffer& operator=(const Buffer&) = delete;
//     ~Buffer() = default;

//     void Append(const std::string& str);
//     void Append(const char* str);
//     void Append(const void* data, size_t len);

//     size_t size() const;
//     void clear();
//     const char* c_str() const;
//     const std::string getBufText() const;



// private:
//     std::string _buffer;
// };

class Buffer {
public:
    static const size_t kcheapPrepend = 8; // 预留8个字节
    static const size_t kInitialSize = 1024;  // 初始化缓冲区长度为1k
    explicit Buffer(size_t initialSize = kInitialSize);

    size_t ReadableBytes() const;
    size_t WriteableBytes() const;
    size_t PrependableBytes() const;

    const char* Peek() const;
    void EnsureWriteable(size_t len);
    void HasWritten(size_t len);

    void Retrieve(size_t len);
    void RetrieveUntil(const char* end);

    void RetrieveAll();
    std::string RetrieveAllToStr();
    std::string GetBufferToStr();

    const char* BeginWriteConst() const;
    char* BeginWrite();

    void Append(const std::string& str);
    void Append(const char* str, size_t len);
    void Append(const void* data, size_t len);
    void Append(const Buffer& buff);

    ssize_t ReadFd(int fd, int* Errno);
    ssize_t WriteFd(int fd, int* Errno);

private:
    char* BeginPtr_();
    const char* BeginPtr_() const;
    void MakeSpace_(size_t len);

    size_t readerIndex_;
    size_t writerIndex_;
    std::vector<char> buffer_;
};