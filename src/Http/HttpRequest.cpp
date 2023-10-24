#include "HttpRequest.h"
#include <Buffer.h>
#include <cstddef>
#include <cstdio>
#include <memory>
#include <ostream>
#include <queue>
#include <string>
#include <utility>
#include <sstream>
#include <iomanip>
#include <vector>
#include <iostream>
#include <Log.h>        

HttpRequest::HttpRequest() {
    Requestline = std::make_shared<std::unordered_map<std::string, std::string>>();
    Header = std::make_shared<std::unordered_map<std::string, std::string>>();
    Data = std::make_shared<std::string>();
}


void HttpRequest::parse(const char* reqmesg) {
    std::string msg = reqmesg;
    parse(msg);
}

void HttpRequest::parse(Buffer &readbuf) {
    
    
}

std::string HttpRequest::urlDecode(const std::string &url) {
    /* chatgpt 生成的url解码代码 */
    std::stringstream decoded;
    for (size_t i = 0; i < url.length(); ++i) {
        if (url[i] == '%' && i + 2 < url.length()) {
            int hex1 = url[i + 1];
            int hex2 = url[i + 2];
            if (isxdigit(hex1) && isxdigit(hex2)) {
                int decodedChar = (hex1 >= 'A' ? hex1 - 'A' + 10 : hex1 - '0') * 16
                                + (hex2 >= 'A' ? hex2 - 'A' + 10 : hex2 - '0');
                decoded << static_cast<char>(decodedChar);
                i += 2;
                continue;
            }
        }
        decoded << url[i];
    }
    return decoded.str();
}

void HttpRequest::parse(const std::string& reqmesg) {
    std::vector<std::string> lines = parseLine(reqmesg);
    std::vector<std::string> reqline = parseSpace(lines[0]);
    if(reqline.size() != 3) {
        LOG_ERROR("header line:%s\n", lines[0].c_str());
        throw "http request line parse failed\n";
    }
    (*Requestline)["method"] = reqline[0];
    auto url = urlDecode(reqline[1]);
    (*Requestline)["url"] = url == "/" ? "/index.html" : url;
    (*Requestline)["version"] = reqline[2];

    // for(auto it = lines.begin(); it != lines.end(); ++it)
        // std::printf("%s\n", it->c_str());
    // throw "test";
    (*Data) = lines.back();

    for(auto it = lines.begin() + 1; it != lines.end() - 2; ++it) {
        auto [k, v] = parseColon(*it);
        (*Header)[k] = v;
    }

}

std::vector<std::string> HttpRequest::splitDelimiter(const std::string& mesg, const std::string& delimiter) {
    std::vector<std::string> tokens;

    size_t start = 0, end;
    while((end = mesg.find(delimiter, start)) != std::string::npos) {
        std::string token = mesg.substr(start, end - start);
        tokens.emplace_back(token);
        start = end + delimiter.length();
    }

    std::string lastToken = mesg.substr(start);
    tokens.emplace_back(lastToken);
    return tokens;
}

std::vector<std::string> HttpRequest::parseLine(const std::string& mesg) {
    return splitDelimiter(mesg, "\r\n");
}

std::vector<std::string> HttpRequest::parseSpace(const std::string& reqline) {
    return splitDelimiter(reqline, " ");
}

bool HttpRequest::IskeepAlive() const {
    if(Header->count("Connection") == 1 ) {
        return Header->find("Connection")->second == "keep-alive" && getVersion() == "HTTP/1.1";
    }
    return false;
}

std::pair<std::string, std::string> HttpRequest::parseColon(const std::string& params) {
    std::vector<std::string> kv = splitDelimiter(params, ":");
    if(kv.size() < 2) {
        LOG_ERROR("%s parse error\n", params.c_str());
        throw "parse http header failed\n";
    }
    for(auto it = kv.begin() + 2; it != kv.end(); ++it)
        kv[1] += *it;
    return std::make_pair(kv[0], kv[1]);
}

#ifdef TEST
int main() {
    const char* httpreq = "GET /hello.txt HTTP/1.1\r\n"
                            "User-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\n"
                            "Host: www.example.com\r\n"
                            "Accept-Language: en, mi\r\n"
                            "\r\n"
                            "";
    HttpRequest req;
    req.parse(httpreq);
    std::cout << req.getMethod() << std::endl;
    std::cout << req.getUrl() << std::endl;
    std::cout << req.getVersion() << std::endl;

    auto header = req.getHeader();
    for(auto it = header->begin(); it != header->end(); ++it) {
        std::cout << it->first << ":" << it->second << std::endl;
    }

    std::cout << *req.getData() << std::endl;


}
#endif