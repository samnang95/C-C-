#pragma once

#include "http/HttpMessage.hpp"
#include "http/HttpServer.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace httpserver {

void printHelp();
void listRoutes(const std::vector<CustomRoute>& routes);
void listLogs(const std::vector<AccessLogEntry>& logs);
void printResponse(const HttpResponse& response);

bool parseId(std::string_view text, int& id);
std::string prompt(std::string_view label);

void addRouteInteractive(HttpServer& server);
void handleSimulatedRequest(HttpServer& server, std::string_view args);

} // namespace httpserver

