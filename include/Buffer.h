#pragma once

#include <cstddef>
#include <string>
#include <sys/types.h>

class Buffer {
public:
    Buffer() = default;
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;
    ~Buffer() = default;

    void Append(const std::string& str);
    void Append(const char* str);
    void Append(const void* data, size_t len);

    size_t size() const;
    void clear();
    const char* c_str() const;
    const std::string getBufText() const;



private:
    std::string _buffer;
};