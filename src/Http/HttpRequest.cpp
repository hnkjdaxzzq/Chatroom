#include "HttpRequest.h"
#include <cstddef>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>
#include <iostream>

HttpRequest::HttpRequest(const char* mesg) {
    std::string httpmesg = mesg;
    Requestline = std::make_shared<std::unordered_map<std::string, std::string>>();
    Header = std::make_shared<std::unordered_map<std::string, std::string>>();
    Data = std::make_shared<std::string>();
    parse(mesg);
}

HttpRequest::HttpRequest(const std::string& mesg) {
    Requestline = std::make_shared<std::unordered_map<std::string, std::string>>();
    Header = std::make_shared<std::unordered_map<std::string, std::string>>();
    Data = std::make_shared<std::string>();
    parse(mesg);
}

void HttpRequest::parse(const std::string& mesg) {
    std::vector<std::string> lines = parseLine(mesg);
    std::vector<std::string> reqline = parseSpace(lines[0]);
    if(reqline.size() != 3)
        throw "http request line parse failed\n";
    (*Requestline)["method"] = reqline[0];
    (*Requestline)["url"] = reqline[1];
    (*Requestline)["version"] = reqline[2];

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

std::pair<std::string, std::string> HttpRequest::parseColon(const std::string& params) {
    std::vector<std::string> kv = splitDelimiter(params, ":");
    if(kv.size() != 2)
        throw "parse http header failed\n";
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
    HttpRequest req(httpreq);
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