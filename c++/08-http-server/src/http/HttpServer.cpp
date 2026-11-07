#include "http/HttpServer.hpp"

#include <algorithm>
#include <charconv>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

namespace httpserver {

namespace {

std::string escapeField(std::string_view value) {
    std::string result;
    for (const char c : value) {
        if (c == '\\') result += "\\\\";
        else if (c == '\t') result += "\\t";
        else if (c == '\n') result += "\\n";
        else result += c;
    }
    return result;
}

std::string unescapeField(std::string_view value) {
    std::string result;
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '\\' && i + 1 < value.size()) {
            const char next = value[++i];
            result += (next == 't') ? '\t' : ((next == 'n') ? '\n' : next);
        } else {
            result += value[i];
        }
    }
    return result;
}

std::vector<std::string> splitFields(std::string_view line) {
    std::vector<std::string> fields;
    std::string field;
    bool escaped = false;
    for (const char c : line) {
        if (escaped) {
            field += '\\';
            field += c;
            escaped = false;
        } else if (c == '\\') {
            escaped = true;
        } else if (c == '\t') {
            fields.push_back(unescapeField(field));
            field.clear();
        } else {
            field += c;
        }
    }
    if (escaped) field += '\\';
    fields.push_back(unescapeField(field));
    return fields;
}

bool parseInt(std::string_view text, int& out) {
    auto res = std::from_chars(text.data(), text.data() + text.size(), out);
    return res.ec == std::errc() && res.ptr == text.data() + text.size();
}

} // namespace

HttpServer::HttpServer(std::string routesDb, std::string webRoot, int port)
    : routesDb_(std::move(routesDb)), webRoot_(std::move(webRoot)), port_(port) {}

bool HttpServer::loadRoutes() {
    std::ifstream input(routesDb_);
    if (!input) return false;

    routes_.clear();
    std::string line;
    while (std::getline(input, line)) {
        const auto fields = splitFields(line);
        if (fields.size() != 6) continue;

        int id = 0;
        int status = 200;
        if (!parseInt(fields[0], id) || id < 1) continue;
        if (!parseInt(fields[3], status) || status < 100 || status > 599) continue;

        if (fields[1].empty() || fields[2].empty()) continue;

        CustomRoute route{
            id,
            uppercase(fields[1]),
            fields[2],
            status,
            fields[4].empty() ? "text/plain; charset=utf-8" : fields[4],
            fields[5]
        };

        routes_.push_back(std::move(route));
        nextRouteId_ = std::max(nextRouteId_, id + 1);
    }
    return true;
}

bool HttpServer::saveRoutes() const {
    std::ofstream output(routesDb_, std::ios::trunc);
    if (!output) return false;

    for (const auto& r : routes_) {
        output << r.id << '\t'
               << escapeField(r.method) << '\t'
               << escapeField(r.path) << '\t'
               << r.statusCode << '\t'
               << escapeField(r.contentType) << '\t'
               << escapeField(r.responseBody) << '\n';
    }
    return static_cast<bool>(output);
}

void HttpServer::ensureDefaultRoutesAndAssets() {
    if (!webRoot_.empty()) {
        ::mkdir(webRoot_.c_str(), 0755);
        const std::string indexPath = webRoot_ + "/index.html";
        std::ifstream existing(indexPath);
        if (!existing) {
            std::ofstream out(indexPath, std::ios::trunc);
            out << "<!DOCTYPE html>\n"
                << "<html><head><title>C++17 Mini HTTP Server</title></head>\n"
                << "<body><h1>Welcome to C++17 Mini HTTP Server</h1>\n"
                << "<p>Serving static files and dynamic routes.</p></body></html>\n";
        }
    }

    if (routes_.empty()) {
        addRoute(CustomRoute{
            0,
            "GET",
            "/api/health",
            200,
            "application/json; charset=utf-8",
            R"({"status":"ok","service":"08-http-server"})"
        });
        addRoute(CustomRoute{
            0,
            "GET",
            "/api/version",
            200,
            "application/json; charset=utf-8",
            R"({"version":"1.0.0","std":"c++17"})"
        });
        saveRoutes();
    }
}

CustomRoute& HttpServer::addRoute(CustomRoute route) {
    route.id = nextRouteId_++;
    route.method = uppercase(trim(route.method));
    if (route.path.empty() || route.path.front() != '/') {
        route.path = "/" + route.path;
    }
    if (route.contentType.empty()) {
        route.contentType = "text/plain; charset=utf-8";
    }
    routes_.push_back(std::move(route));
    return routes_.back();
}

bool HttpServer::removeRoute(int id) {
    const auto oldSize = routes_.size();
    routes_.erase(std::remove_if(routes_.begin(), routes_.end(),
                                 [id](const CustomRoute& r) { return r.id == id; }),
                  routes_.end());
    return routes_.size() != oldSize;
}

const CustomRoute* HttpServer::findRoute(int id) const {
    for (const auto& r : routes_) {
        if (r.id == id) return &r;
    }
    return nullptr;
}

void HttpServer::recordLog(std::string_view method, std::string_view path, int status, std::size_t bytes) {
    logs_.push_back(AccessLogEntry{
        nextLogId_++,
        std::string(method),
        std::string(path),
        status,
        bytes
    });
}

HttpResponse HttpServer::handleRequest(const HttpRequest& request) {
    HttpResponse resp;

    if (!isSafeRelativePath(request.path)) {
        resp.statusCode = 403;
        resp.statusText = statusReasonPhrase(403);
        resp.body = "403 Forbidden: unsafe path\n";
        recordLog(request.method, request.path, resp.statusCode, resp.body.size());
        return resp;
    }

    // 1. Check custom configured routes (exact method + path match)
    bool pathMatchedDifferentMethod = false;
    for (const auto& route : routes_) {
        if (route.path == request.path) {
            if (route.method == request.method) {
                resp.statusCode = route.statusCode;
                resp.statusText = statusReasonPhrase(route.statusCode);
                resp.contentType = route.contentType;
                resp.body = route.responseBody;
                recordLog(request.method, request.path, resp.statusCode, resp.body.size());
                return resp;
            }
            pathMatchedDifferentMethod = true;
        }
    }

    // 2. Built-in echo endpoint for POST /api/echo
    if (request.path == "/api/echo" && request.method == "POST") {
        resp.statusCode = 200;
        resp.statusText = "OK";
        resp.contentType = "text/plain; charset=utf-8";
        resp.body = request.body;
        recordLog(request.method, request.path, resp.statusCode, resp.body.size());
        return resp;
    }

    if (pathMatchedDifferentMethod) {
        resp.statusCode = 405;
        resp.statusText = statusReasonPhrase(405);
        resp.body = "405 Method Not Allowed\n";
        recordLog(request.method, request.path, resp.statusCode, resp.body.size());
        return resp;
    }

    // 3. Static file lookup under webRoot_
    if (request.method == "GET" && !webRoot_.empty()) {
        std::string relPath = (request.path == "/") ? "/index.html" : request.path;
        std::string fullPath = webRoot_ + relPath;
        std::ifstream file(fullPath, std::ios::binary);
        if (file) {
            std::ostringstream buffer;
            buffer << file.rdbuf();
            resp.statusCode = 200;
            resp.statusText = "OK";
            resp.contentType = guessMimeType(relPath);
            resp.body = buffer.str();
            recordLog(request.method, request.path, resp.statusCode, resp.body.size());
            return resp;
        }
    }

    resp.statusCode = 404;
    resp.statusText = statusReasonPhrase(404);
    resp.body = "404 Not Found: " + request.path + "\n";
    recordLog(request.method, request.path, resp.statusCode, resp.body.size());
    return resp;
}

HttpResponse HttpServer::handleRawRequest(std::string_view rawRequest) {
    HttpRequest req;
    if (!parseHttpRequest(rawRequest, req)) {
        HttpResponse bad;
        bad.statusCode = 400;
        bad.statusText = statusReasonPhrase(400);
        bad.body = "400 Bad Request\n";
        recordLog("INVALID", "/", 400, bad.body.size());
        return bad;
    }
    return handleRequest(req);
}

bool HttpServer::serveBlocking(int listenPort, std::size_t maxRequests) {
    const int serverFd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
        std::cerr << "Failed to create socket.\n";
        return false;
    }

    int opt = 1;
    ::setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(static_cast<uint16_t>(listenPort));

    if (::bind(serverFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "Could not bind to 127.0.0.1:" << listenPort << ".\n";
        ::close(serverFd);
        return false;
    }

    if (::listen(serverFd, 16) < 0) {
        std::cerr << "Could not listen on port " << listenPort << ".\n";
        ::close(serverFd);
        return false;
    }

    std::cout << "Listening on http://127.0.0.1:" << listenPort
              << " (serving " << (maxRequests == 0 ? "until Ctrl+C" : std::to_string(maxRequests) + " request(s)")
              << ")...\n";

    std::size_t handled = 0;
    while (maxRequests == 0 || handled < maxRequests) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        const int clientFd = ::accept(serverFd, reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);
        if (clientFd < 0) break;

        char buffer[8192];
        const ssize_t received = ::recv(clientFd, buffer, sizeof(buffer) - 1, 0);
        if (received > 0) {
            buffer[received] = '\0';
            const HttpResponse resp = handleRawRequest(std::string_view(buffer, static_cast<std::size_t>(received)));
            const std::string wire = resp.serialize();
            ::send(clientFd, wire.data(), wire.size(), 0);
            if (!logs_.empty()) {
                const auto& last = logs_.back();
                std::cout << "  [" << last.statusCode << "] " << last.method << ' ' << last.path
                          << " (" << last.responseBytes << " bytes)\n";
            }
        }
        ::close(clientFd);
        ++handled;
    }

    ::close(serverFd);
    return true;
}

} // namespace httpserver

