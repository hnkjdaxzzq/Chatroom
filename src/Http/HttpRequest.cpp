#include "HttpRequest.h"
#include <cstddef>
#include <cstdio>
#include <memory>
#include <ostream>
#include <queue>
#include <string>
#include <utility>
#include <vector>
#include <iostream>

HttpRequest::HttpRequest() {
    Requestline = std::make_shared<std::unordered_map<std::string, std::string>>();
    Header = std::make_shared<std::unordered_map<std::string, std::string>>();
    Data = std::make_shared<std::string>();
}


void HttpRequest::parse(const char* reqmesg) {
    std::string msg = reqmesg;
    parse(msg);
}

void HttpRequest::parse(const std::string& reqmesg) {
    std::vector<std::string> lines = parseLine(reqmesg);
    std::vector<std::string> reqline = parseSpace(lines[0]);
    if(reqline.size() != 3)
        throw "http request line parse failed\n";
    (*Requestline)["method"] = reqline[0];
    (*Requestline)["url"] = reqline[1];
    (*Requestline)["version"] = reqline[2];

    for(auto it = lines.begin(); it != lines.end(); ++it)
        std::printf("%s\n", it->c_str());
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
        printf("%s parse error\n", params.c_str());
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