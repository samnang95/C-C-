#include "http/HttpMessage.hpp"
#include "http/HttpServer.hpp"
#include "ui/ConsoleUI.hpp"

#include <iostream>
#include <sstream>
#include <string>

int main() {
    using namespace httpserver;

    HttpServer server;
    server.loadRoutes();
    server.ensureDefaultRoutesAndAssets();

    std::cout << "Mini HTTP Server — type 'help' for commands.\n";

    std::string line;
    while (std::cout << "http> " && std::getline(std::cin, line)) {
        std::istringstream input(std::string(trim(line)));
        std::string command;
        input >> command;
        command = lowercase(command);

        std::string argument;
        std::getline(input, argument);
        argument = std::string(trim(argument));

        if (command.empty()) continue;

        if (command == "help") {
            printHelp();
        } else if (command == "clear" || command == "reset" || command == "cls") {
            std::cout << "\033[2J\033[H" << std::flush;
        } else if (command == "routes" || command == "list") {
            listRoutes(server.routes());
        } else if (command == "add") {
            addRouteInteractive(server);
            server.saveRoutes();
        } else if (command == "remove" || command == "delete") {
            std::string idText = argument;
            if (idText.empty()) idText = prompt("Route ID to remove: ");
            int id = 0;
            if (!parseId(idText, id) || !server.removeRoute(id)) {
                std::cout << "Route not found.\n";
            } else {
                server.saveRoutes();
                std::cout << "Route #" << id << " removed.\n";
            }
        } else if (command == "request" || command == "get" || command == "post") {
            if (command == "get" || command == "post") {
                handleSimulatedRequest(server, uppercase(command) + " " + argument);
            } else {
                handleSimulatedRequest(server, argument);
            }
        } else if (command == "serve") {
            std::istringstream sargs(argument);
            int port = server.port();
            int count = 1;
            sargs >> port;
            if (!(sargs >> count) || count < 0) count = 1;
            server.serveBlocking(port, static_cast<std::size_t>(count));
        } else if (command == "logs") {
            listLogs(server.logs());
        } else if (command == "quit" || command == "exit") {
            server.saveRoutes();
            std::cout << "Routes saved. Goodbye.\n";
            return 0;
        } else {
            std::cout << "Unknown command. Type 'help' for commands.\n";
        }
    }

    server.saveRoutes();
    std::cout << "\nRoutes saved. Goodbye.\n";
    return 0;
}

