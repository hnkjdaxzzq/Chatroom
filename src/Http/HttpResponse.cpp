#include "HttpResponse.h"
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <ios>
#include <streambuf>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_map>
#include <sys/mman.h>
#include <Log.h>

using std::string;
using std::unordered_map;

const unordered_map<string, string> HttpResponse::SUFFIX_TYPE {
    { ".html",  "text/html" },
    { ".xml",   "text/xml" },
    { ".xhtml", "application/xhtml+xml" },
    { ".txt",   "text/plain" },
    { ".rtf",   "application/rtf" },
    { ".pdf",   "application/pdf" },
    { ".word",  "application/nsword" },
    { ".png",   "image/png" },
    { ".gif",   "image/gif" },
    { ".jpg",   "image/jpeg" },
    { ".jpeg",  "image/jpeg" },
    { ".au",    "audio/basic" },
    { ".mpeg",  "video/mpeg" },
    { ".mpg",   "video/mpeg" },
    { ".avi",   "video/x-msvideo" },
    { ".gz",    "application/x-gzip" },
    { ".tar",   "application/x-tar" },
    { ".css",   "text/css "},
    { ".js",    "text/javascript "},
};

const unordered_map<int, string> HttpResponse::CODE_STATUS {
    { 200, "ok" },
    { 400, "Bad Request" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
};

const unordered_map<int, string> HttpResponse::CODE_PATH {
    { 400, "/400.html" },
    { 403, "/403.html" },
    { 404, "/404.html" },
};


void HttpResponse::MakeResponse(Buffer &buff) {
    buff.RetrieveAll();
    // 如果文件不存在，或者请求的是一个目录
    // std::fprintf(stderr, "MakeResponse() path: %s\n", (srcDir_ + path_).c_str());
    if(stat((srcDir_ + path_).c_str(), &mmFileStat_) < 0 || S_ISDIR(mmFileStat_.st_mode)) {
        code_ = 404;
        LOG_DEBUG("request %s 404\n", (srcDir_ + path_).c_str());
    }
    else if(!(mmFileStat_.st_mode & S_IROTH)) { // 文件没有访问权限
        code_ = 403;
    }
    else if(code_ == -1) {
        code_ = 200;
    }
    LOG_DEBUG("MakeResponse() path: %s  code: %d\n", (srcDir_ + path_).c_str(), code_);
    ErrorHtml_();
    AddStateLine_(buff);
    AddHeader_(buff);
    AddContent_(buff);
}

char* HttpResponse::File() {
    return mmFile_;
}

size_t HttpResponse::FileLen()const {
    return mmFileStat_.st_size;
}

void HttpResponse::ErrorHtml_() {
    if(CODE_PATH.count(code_) == 1) {
        path_ = CODE_PATH.find(code_)->second;
        stat((srcDir_ + path_).data(), &mmFileStat_);
    }
}

void HttpResponse::AddStateLine_(Buffer &buff) {
    string status;
    if(CODE_STATUS.find(code_) != CODE_STATUS.end()) {
        status = CODE_STATUS.find(code_)->second;
    }
    else {
        code_ = 400;
        status = CODE_STATUS.find(code_)->second;
    }
    buff.Append("HTTP/1.1 " + std::to_string(code_) + " " + status + "\r\n");
}

void HttpResponse::AddHeader_(Buffer &buff) {
    buff.Append("Connection: ");
    if(iskeepAlive_) {
        buff.Append("keep-alive\r\n");
        buff.Append("keep-alive: max=6, timeout=120\r\n");
    } else {
        buff.Append("close\r\n");
    }
    buff.Append("Content-type: " + GetFileType_() + "\r\n");
}

string HttpResponse::GetFileType_() {
    string::size_type idx = path_.find_last_of('.');
    if(idx == string::npos) {
        return "text/plain";
    }
    string suffix = path_.substr(idx);
    if(SUFFIX_TYPE.find(suffix) != SUFFIX_TYPE.end()) {
        return SUFFIX_TYPE.find(suffix)->second;
    }
    return "text/plain";
}

void HttpResponse::AddContent_(Buffer &buff) {
    LOG_DEBUG("AddContent() file path: %s\n", (srcDir_ + path_).c_str());
    int srcfd = open((srcDir_ + path_).c_str(), O_RDONLY);
    if(srcfd < 0) {
        LOG_ERROR("%s: open failed %s", (srcDir_ + path_).c_str(), strerror(errno));
        buff.RetrieveAll();
        code_ = 400;
        AddStateLine_(buff);
        ErrorContent(buff, "File not found, open failed");
        return;
    }

    auto mfile = mmap(0, mmFileStat_.st_size, PROT_READ, MAP_PRIVATE, srcfd, 0);
    if(mfile == MAP_FAILED) {
        LOG_ERROR("%s: mmap failed", (srcDir_ + path_).c_str());
        close(srcfd);

        std::ifstream file_stream(srcDir_ + path_, std::ios::in);
        if(!file_stream.is_open()) {
            LOG_ERROR("%s: ifstream open failed", (srcDir_ + path_).c_str());
            ErrorContent(buff, "File not Found, mmap failed");
            return;
        }

        file_stream.seekg(0, std::ios::end);
        std::streampos file_size = file_stream.tellg();
        file_stream.seekg(0, std::ios::beg);

        string fileContent;
        fileContent.resize(file_size);

        file_stream.read(&fileContent[0], file_size);
        buff.Append("Content-length: " + std::to_string(file_size) + "\r\n\r\n");
        buff.Append(fileContent);

        file_stream.close();
        return;
    }
    mmFile_ = (char*) mfile;
    close(srcfd);
    buff.Append("Content-length: " + std::to_string(mmFileStat_.st_size) +"\r\n\r\n");
}

void HttpResponse::UnmapFile() {
    if(mmFile_) {
        munmap(mmFile_, mmFileStat_.st_size);
        mmFile_ = nullptr;
    }
}



void HttpResponse::ErrorContent(Buffer &buff, const string& errmsg) {
    string body;
    string status;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    if(CODE_STATUS.count(code_) == 1) {
        status = CODE_STATUS.find(code_)->second;
    } else {
        status = "Bad Request";
    }
    body += std::to_string(code_) + " : " + status + "\n";
    body += "<p>" + errmsg + "</p>";
    body += "<hr><em>TinyWebServer</em></body></html>";

    buff.Append("Cotent-length: " + std::to_string(body.size()) + "\r\n\r\n");
    buff.Append(body);
}