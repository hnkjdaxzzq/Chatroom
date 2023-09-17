#include <Buffer.h>
#include <cstddef>
#include <cstring>
#include <string>
#include <sys/types.h>

void Buffer::Append(const std::string& str) {
    _buffer += str;
}

void Buffer::Append(const char* str) {
    _buffer.append(str);
}

void Buffer::Append(const void* data, size_t len) {
    void* tmp = new char[len];
    std::memcpy(tmp, data, len);
    const char* str = static_cast<const char*>(tmp);
    _buffer.append(str);
}

size_t Buffer::size() const {
    return _buffer.size();
}

const std::string Buffer::getBufText() const{
    return _buffer;
}

const char* Buffer::c_str() const {
    return _buffer.c_str();
}

void Buffer::clear() {
    _buffer.clear();
}