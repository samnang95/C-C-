#include "http/HttpMessage.hpp"
#include "http/HttpServer.hpp"

#include <cassert>
#include <cstdio>
#include <iostream>

int main() {
    using namespace httpserver;

    // 1. HTTP Request parsing & path safety
    HttpRequest req;
    const std::string rawReq =
        "POST /api/echo?debug=true HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "Hello HTTP Server";

    assert(parseHttpRequest(rawReq, req));
    assert(req.method == "POST");
    assert(req.path == "/api/echo");
    assert(req.query == "debug=true");
    assert(req.headers["host"] == "localhost:8080");
    assert(req.body == "Hello HTTP Server");

    assert(isSafeRelativePath("/index.html"));
    assert(!isSafeRelativePath("/../etc/passwd"));
    assert(guessMimeType("/style.css") == "text/css; charset=utf-8");

    // 2. HttpServer routing, static assets, status codes, and persistence
    const std::string testRoutesDb = "/tmp/test_http_routes.db";
    const std::string testWebRoot = "/tmp/test_http_www";
    std::remove(testRoutesDb.c_str());

    {
        HttpServer srv(testRoutesDb, testWebRoot, 8080);
        srv.ensureDefaultRoutesAndAssets();

        // Built-in health route
        HttpResponse health = srv.handleRawRequest("GET /api/health HTTP/1.1\r\n\r\n");
        assert(health.statusCode == 200);
        assert(health.body.find("ok") != std::string::npos);

        // Wrong method on existing route -> 405
        HttpResponse wrongMethod = srv.handleRawRequest("DELETE /api/health HTTP/1.1\r\n\r\n");
        assert(wrongMethod.statusCode == 405);

        // Built-in echo route
        HttpResponse echoed = srv.handleRawRequest(rawReq);
        assert(echoed.statusCode == 200);
        assert(echoed.body == "Hello HTTP Server");

        // Directory traversal attempt -> 403
        HttpResponse forbidden = srv.handleRawRequest("GET /../../secret HTTP/1.1\r\n\r\n");
        assert(forbidden.statusCode == 403);

        // Static file / -> /index.html
        HttpResponse indexPage = srv.handleRawRequest("GET / HTTP/1.1\r\n\r\n");
        assert(indexPage.statusCode == 200);
        assert(indexPage.body.find("Welcome") != std::string::npos);

        // Missing route -> 404
        HttpResponse missing = srv.handleRawRequest("GET /does-not-exist HTTP/1.1\r\n\r\n");
        assert(missing.statusCode == 404);

        // Add custom route & save
        CustomRoute custom{0, "GET", "/api/custom", 201, "application/json", R"({"created":true})"};
        srv.addRoute(custom);
        assert(srv.saveRoutes());
        assert(srv.logs().size() == 6);
    }

    {
        HttpServer loaded(testRoutesDb, testWebRoot, 8080);
        assert(loaded.loadRoutes());
        assert(loaded.routes().size() == 3);

        HttpResponse customResp = loaded.handleRawRequest("GET /api/custom HTTP/1.1\r\n\r\n");
        assert(customResp.statusCode == 201);
        assert(customResp.body == R"({"created":true})");
    }

    std::remove(testRoutesDb.c_str());
    std::remove((testWebRoot + "/index.html").c_str());
    std::cout << "All HTTP server unit tests passed!\n";
    return 0;
}

