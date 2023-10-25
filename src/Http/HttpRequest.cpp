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

bool HttpRequest::parse(Buffer &readbuf) {
    // 获取HttpRequest报文数据
    std::string reqData = readbuf.GetBufferToStr();
    ParseState state = ParseState::REQUEST_LINE;
    size_t pos = 0;
    while(true) {
        switch (state) {
        case ParseState::REQUEST_LINE:
            parseRequestLine(reqData, pos, state);
            break;
        case ParseState::HEADERS:
            parseHeaders(reqData, pos, state);
            break;
        case ParseState::REQUEST_DATA:
            parseRequestData(reqData, pos, state);
            break;
        case ParseState::FINISH:
            readbuf.HasWritten(pos);
            return true;
        case ParseState::ERROR:
            return false;
        default: 
            return false;
        }
    }
}

void HttpRequest::parseRequestLine(const std::string &str, size_t &pos, ParseState &state) {
    state = ParseState::METHOD;
    while(true) {
        switch (state) {
            case ParseState::METHOD:
                method_ = parseMethod(str, pos, state);
                break;
            case ParseState::URL:
                uri_ = parseUri(str, pos, state);
                break;
            case ParseState::VERSION:
                version_ = parseVersion(str, pos, state);
                break;
            case ParseState::CRLF:
                state = ParseState::HEADERS;
                return;
            case ParseState::ERROR:
                return;
            default:
                state = ParseState::ERROR;
                return;
        }
    }
}

void HttpRequest::parseHeaders(const std::string& str, size_t &pos, ParseState &state) {
    state = ParseState::HEADER_NAME;
    std::string key, value;
    while (true) {
        switch (state) {
            case ParseState::HEADER_NAME:
                key = parseHeaderName(str, pos, state);
                break;
            case ParseState::HEADER_VALUE:
                value = parseHeaderValue(str, pos, state);
                (*Header)[key] = value;
                break;
            case ParseState::CRLF:
                state = ParseState::REQUEST_DATA;
                return;
            case ParseState::ERROR:
                return;
            default:
                state = ParseState::ERROR;
                return;
        }
    }
}

void HttpRequest::parseRequestData(const std::string& str, size_t &pos, ParseState &state) {
    if(pos >= str.size()) {
        state = ParseState::ERROR;
        return;
    }
    Data = std::make_shared<std::string>(str.substr(pos));
    state = ParseState::FINISH;
}

std::string HttpRequest::parseHeaderName(const std::string& str, size_t &pos, ParseState &state) {
    std::string name;
    size_t len = 0;
    do {
        if(pos + len >= str.size()) {
            state = ParseState::ERROR;
            return name;
        }
        ++len;
    } while (str[pos+len] != ':') ;
    
    name = str.substr(pos, len);
    state = ParseState::HEADER_VALUE;
    pos += len + 1;
    return name;
}

std::string HttpRequest::parseHeaderValue(const std::string& str, size_t &pos, ParseState &state) {
    std::string value;
    size_t len = 0;
    do {
        if(pos + len >= str.size()) {
            state = ParseState::ERROR;
            return value;
        }
        ++len;
    } while(str[pos + len] != '\r');

    value = str.substr(pos, len);
    ++len;

    if(!(pos + len < str.size() && str[pos + len] == '\n')) {
        state = ParseState::ERROR;
        return value;
    }

    state = ParseState::HEADER_NAME;
    pos += len + 1;

    if( pos + 1 < str.size() && str[pos] == '\r' && str[pos + 1] == '\n') {
        state = ParseState::CRLF;
        pos += 2;
    }

    return value;
}

std::string HttpRequest::parseMethod(const std::string& str, size_t &pos, ParseState &state) {
    std::string method;
    size_t len = 0;
    do {
        if(pos + len >= str.size()) {
            state = ParseState::ERROR;
            return method;
        }
        ++len;
    } while (str[pos+len] != ' ') ;
    
    method = str.substr(pos, len);
    state = ParseState::URL;
    pos += len + 1;
    return method;
}

std::string HttpRequest::parseUri(const std::string& str, size_t &pos, ParseState &state) {
    std::string uri;
    size_t len = 0; 
    do {
        if(pos + len >= str.size()) {
            state = ParseState::ERROR;
            return uri;
        }
        ++len;
    } while (str[pos+len] != ' ') ;

    uri = str.substr(pos, len);
    uri = urlDecode(uri);
    state = ParseState::VERSION;
    pos += len + 1;
    return uri;
}

std::string HttpRequest::parseVersion(const std::string& str, size_t &pos, ParseState &state) {
    std::string version;
    size_t len = 0;
    do {
        if(pos + len >= str.size()) {
            state = ParseState::ERROR;
            return version;
        }
        ++len;
    } while(str[pos + len] != '\r');

    version = str.substr(pos, len);
    ++len;
    if(pos + len < str.size() && str[pos + len] == '\n') {
        state = ParseState::CRLF;
        pos += len + 1;
    } else {
        state = ParseState::ERROR;
    }

    return version;
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
    Buffer buf;
    buf.Append(httpreq);
    req.parse(buf);
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