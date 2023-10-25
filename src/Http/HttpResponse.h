#pragma once

#include <cstddef>
#include <string>
#include <sys/stat.h>
#include <unordered_map>
#include <Buffer.h>
class HttpResponse {
public:
    HttpResponse(const std::string& srcDir , const std::string& path = "index.html", bool iskeepAlive = false, int code = 200) :
                srcDir_(srcDir), path_(path), iskeepAlive_(iskeepAlive), code_(-1), mmFile_(nullptr)
    {
        mmFileStat_ = {0};
    }

    HttpResponse() = default;

    void init(const std::string& srcDir = "webapp/default", const std::string& path = "index.html", bool iskeepAlive = false, int code = 200) {
        srcDir_ = srcDir;
        path_ = path;
        iskeepAlive_ = iskeepAlive;
        code_ = code;
        mmFile_ = nullptr;
        mmFileStat_ = {0};
    }

    char* File();
    void UnmapFile();
    size_t FileLen() const;
    void MakeResponse(Buffer &buff);

private:
    int code_;
    bool iskeepAlive_;
    
    std::string path_;
    std::string srcDir_;

    char* mmFile_;
    struct stat mmFileStat_;

    void AddStateLine_(Buffer &buff);
    void AddHeader_(Buffer &buff);
    void AddContent_(Buffer &buff);
    void ErrorContent(Buffer &buff, const std::string& errmsg);

    std::string GetFileType_();
    void ErrorHtml_();

    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<int, std::string> CODE_PATH;
    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
};