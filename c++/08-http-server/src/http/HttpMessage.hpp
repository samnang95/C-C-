#pragma once

#include <map>
#include <string>
#include <string_view>

namespace httpserver {

struct HttpRequest {
    std::string method;
    std::string path;
    std::string query;
    std::string version{"HTTP/1.1"};
    std::map<std::string, std::string> headers;
    std::string body;
};

struct HttpResponse {
    int statusCode{200};
    std::string statusText{"OK"};
    std::string contentType{"text/plain; charset=utf-8"};
    std::map<std::string, std::string> headers;
    std::string body;

    std::string serialize() const;
};

std::string_view trim(std::string_view text);
std::string lowercase(std::string_view text);
std::string uppercase(std::string_view text);
std::string statusReasonPhrase(int statusCode);
std::string guessMimeType(std::string_view path);
bool isSafeRelativePath(std::string_view path);

bool parseHttpRequest(std::string_view raw, HttpRequest& outRequest);

} // namespace httpserver

