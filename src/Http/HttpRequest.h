#pragma once

#include <Buffer.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>


class HttpRequest{
public:
    HttpRequest();

    enum class ParseState {
        REQUEST_LINE,
        METHOD,
        URL,
        VERSION,
        HEADERS,
        HEADER_NAME,
        HEADER_VALUE,
        ERROR
    };

    std::shared_ptr<std::unordered_map<std::string, std::string>> getRequestline() const {
        return Requestline;
    }

    std::shared_ptr<std::unordered_map<std::string, std::string>> getHeader() const {
        return Header;
    }

    std::shared_ptr<std::string> getData() const {
        return Data;
    }

    std::string getMethod() const {
        // return method_;
        return (*Requestline)["method"];
    }

    std::string getUri() const {
        return uri_;
    }

    std::string getUrl() const {
        return (*Requestline)["url"];
    }

    std::string getVersion() const {
        // return version_;
        return (*Requestline)["version"]; 
    }

    void parse(Buffer &readbuf);
    void parse(const char* reqmesg);
    void parse(const std::string& reqmesg);
    bool IskeepAlive() const ;

private:
    std::shared_ptr<std::unordered_map<std::string, std::string>> Requestline;
    std::string method_;
    std::string uri_;
    std::string version_;
    std::shared_ptr<std::unordered_map<std::string, std::string>> Header;
    std::shared_ptr<std::string> Data;


    std::vector<std::string> splitDelimiter(const std::string& mesg, const std::string& delimiter);
    std::vector<std::string> parseLine(const std::string&);
    std::vector<std::string> parseSpace(const std::string&);
    std::pair<std::string, std::string> parseColon(const std::string&);
    std::string urlDecode(const std::string &url);
};