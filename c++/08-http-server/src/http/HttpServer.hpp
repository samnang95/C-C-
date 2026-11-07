#pragma once

#include "http/HttpMessage.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace httpserver {

struct CustomRoute {
    int id{};
    std::string method;      // GET, POST, etc.
    std::string path;        // e.g. /api/hello
    int statusCode{200};
    std::string contentType{"application/json; charset=utf-8"};
    std::string responseBody;
};

struct AccessLogEntry {
    int id{};
    std::string method;
    std::string path;
    int statusCode{};
    std::size_t responseBytes{};
};

class HttpServer {
public:
    explicit HttpServer(std::string routesDb = "data/routes.db",
                        std::string webRoot = "data/www",
                        int port = 8080);

    bool loadRoutes();
    bool saveRoutes() const;
    void ensureDefaultRoutesAndAssets();

    CustomRoute& addRoute(CustomRoute route);
    bool removeRoute(int id);
    const CustomRoute* findRoute(int id) const;

    HttpResponse handleRequest(const HttpRequest& request);
    HttpResponse handleRawRequest(std::string_view rawRequest);

    // Runs a live POSIX TCP server for up to maxRequests (0 = until interrupted)
    bool serveBlocking(int port, std::size_t maxRequests = 0);

    const std::vector<CustomRoute>& routes() const { return routes_; }
    const std::vector<AccessLogEntry>& logs() const { return logs_; }
    void clearLogs() { logs_.clear(); }

    int port() const { return port_; }
    void setPort(int p) { port_ = p; }
    const std::string& routesDb() const { return routesDb_; }
    const std::string& webRoot() const { return webRoot_; }

private:
    void recordLog(std::string_view method, std::string_view path, int status, std::size_t bytes);

    std::string routesDb_;
    std::string webRoot_;
    int port_{8080};
    std::vector<CustomRoute> routes_;
    std::vector<AccessLogEntry> logs_;
    int nextRouteId_{1};
    int nextLogId_{1};
};

} // namespace httpserver

