#include "ui/ConsoleUI.hpp"

#include <charconv>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace httpserver {

void printHelp() {
    std::cout << "\nCommands:\n"
              << "  routes                  List configured HTTP endpoints\n"
              << "  add                     Register a new custom HTTP endpoint\n"
              << "  remove [id]             Delete a custom HTTP endpoint\n"
              << "  request <METHOD> <path> Simulate an HTTP request (e.g. request GET /api/health)\n"
              << "  serve [port] [count]    Start live TCP server on 127.0.0.1 (e.g. serve 8080 1)\n"
              << "  logs                    Display HTTP access logs\n"
              << "  clear / reset           Clear the terminal screen\n"
              << "  help                    Show this help\n"
              << "  quit                    Save routes and exit\n\n";
}

void listRoutes(const std::vector<CustomRoute>& routes) {
    if (routes.empty()) {
        std::cout << "\nNo custom routes registered.\n\n";
        return;
    }

    std::cout << "\n" << std::left << std::setw(4) << "ID"
              << std::setw(8) << "Method"
              << std::setw(22) << "Path"
              << std::setw(8) << "Status"
              << std::setw(32) << "Content-Type"
              << "Body Preview\n"
              << std::string(96, '-') << '\n';

    for (const auto& r : routes) {
        std::cout << std::left << std::setw(4) << r.id
                  << std::setw(8) << r.method
                  << std::setw(22) << r.path.substr(0, 21)
                  << std::setw(8) << r.statusCode
                  << std::setw(32) << r.contentType.substr(0, 31)
                  << r.responseBody.substr(0, 24) << '\n';
    }
    std::cout << '\n';
}

void listLogs(const std::vector<AccessLogEntry>& logs) {
    if (logs.empty()) {
        std::cout << "\nNo requests logged yet.\n\n";
        return;
    }

    std::cout << "\n" << std::left << std::setw(5) << "#"
              << std::setw(10) << "Method"
              << std::setw(28) << "Path"
              << std::setw(10) << "Status"
              << "Bytes\n"
              << std::string(62, '-') << '\n';

    for (const auto& entry : logs) {
        std::cout << std::left << std::setw(5) << entry.id
                  << std::setw(10) << entry.method
                  << std::setw(28) << entry.path.substr(0, 27)
                  << std::setw(10) << entry.statusCode
                  << entry.responseBytes << '\n';
    }
    std::cout << '\n';
}

void printResponse(const HttpResponse& response) {
    std::cout << "\n--- HTTP Response ---\n"
              << "HTTP/1.1 " << response.statusCode << ' ' << response.statusText << '\n'
              << "Content-Type: " << response.contentType << '\n'
              << "Content-Length: " << response.body.size() << "\n\n"
              << response.body << "\n\n";
}

bool parseId(std::string_view text, int& id) {
    auto trimmed = trim(text);
    if (trimmed.empty()) return false;

    int parsed = 0;
    auto res = std::from_chars(trimmed.data(), trimmed.data() + trimmed.size(), parsed);
    if (res.ec != std::errc() || res.ptr != trimmed.data() + trimmed.size() || parsed <= 0) {
        return false;
    }
    id = parsed;
    return true;
}

std::string prompt(std::string_view label) {
    std::cout << label;
    std::string value;
    if (!std::getline(std::cin, value)) return "";
    return std::string(trim(value));
}

void addRouteInteractive(HttpServer& server) {
    CustomRoute route;

    const auto rawMethod = prompt("HTTP Method [GET/POST/PUT/DELETE] (GET): ");
    if (!std::cin) return;
    route.method = rawMethod.empty() ? "GET" : uppercase(rawMethod);

    while (std::cin) {
        route.path = prompt("URL Path (e.g. /api/items): ");
        if (!std::cin) return;
        if (!route.path.empty()) break;
        std::cout << "A path is required.\n";
    }

    const auto rawStatus = prompt("HTTP Status Code (200): ");
    if (!std::cin) return;
    if (!rawStatus.empty()) {
        int code = 200;
        if (parseId(rawStatus, code) && code >= 100 && code <= 599) {
            route.statusCode = code;
        }
    }

    const auto rawType = prompt("Content-Type (application/json; charset=utf-8): ");
    if (!std::cin) return;
    if (!rawType.empty()) {
        route.contentType = rawType;
    }

    route.responseBody = prompt("Response Body: ");
    if (!std::cin) return;

    const auto& added = server.addRoute(std::move(route));
    std::cout << "Added route #" << added.id << " (" << added.method << " " << added.path << ").\n";
}

void handleSimulatedRequest(HttpServer& server, std::string_view args) {
    std::istringstream iss{std::string(args)};
    std::string method;
    std::string path;
    iss >> method >> path;

    std::string body;
    std::getline(iss, body);
    body = std::string(trim(body));

    if (method.empty()) {
        method = prompt("Method (GET): ");
        if (method.empty()) method = "GET";
    }
    if (path.empty()) {
        path = prompt("Path (/api/health): ");
        if (path.empty()) path = "/api/health";
    }

    HttpRequest req;
    req.method = uppercase(method);
    req.path = path;
    req.body = body;

    const auto resp = server.handleRequest(req);
    printResponse(resp);
}

} // namespace httpserver

