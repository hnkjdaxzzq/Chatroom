#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>


class HttpRequest{
public:
    HttpRequest(const char*);

    HttpRequest(const std::string&);

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
        return (*Requestline)["method"];
    }

    std::string getUrl() const {
        return (*Requestline)["url"];
    }

    std::string getVersion() const {
        return (*Requestline)["version"]; 
    }

private:
    std::shared_ptr<std::unordered_map<std::string, std::string>> Requestline;
    std::shared_ptr<std::unordered_map<std::string, std::string>> Header;
    std::shared_ptr<std::string> Data;

    void parse(const std::string&);

    std::vector<std::string> splitDelimiter(const std::string& mesg, const std::string& delimiter);
    std::vector<std::string> parseLine(const std::string&);
    std::vector<std::string> parseSpace(const std::string&);
    std::pair<std::string, std::string> parseColon(const std::string&);
};