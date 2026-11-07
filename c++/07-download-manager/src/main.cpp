#include "download/DownloadJob.hpp"
#include "download/DownloadManager.hpp"
#include "ui/ConsoleUI.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

int main() {
    using namespace dlmanager;

    DownloadManager manager;
    manager.load();
    std::cout << "Download Manager — type 'help' for commands.\n";

    std::string line;
    while (std::cout << "download> " && std::getline(std::cin, line)) {
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
        } else if (command == "add") {
            addDownloadInteractive(manager, argument);
            manager.save();
        } else if (command == "list") {
            if (argument.empty()) {
                listJobs(manager.jobs());
            } else {
                listJobs(manager.filterByStatus(argument));
            }
        } else if (command == "start" && lowercase(argument) == "all") {
            bool any = false;
            for (const auto& job : manager.jobs()) {
                if (job.status != DownloadStatus::Completed && job.status != DownloadStatus::Cancelled) {
                    runDownloadAnimation(manager, job.id);
                    any = true;
                }
            }
            if (!any) {
                std::cout << "No queued or paused downloads to start.\n";
            }
            manager.save();
        } else if (command == "view" || command == "start" || command == "step" ||
                   command == "pause" || command == "resume" || command == "cancel" ||
                   command == "retry" || command == "remove" || command == "delete") {
            int id = 0;
            std::string idText = argument;
            if (idText.empty()) {
                idText = prompt("Download ID: ");
            }
            if (!parseId(idText, id)) {
                std::cout << "Usage: " << command << " <id>\n";
                continue;
            }
            DownloadJob* job = manager.find(id);
            if (!job) {
                std::cout << "Download not found.\n";
                continue;
            }

            if (command == "view") {
                printJob(*job);
            } else if (command == "start") {
                runDownloadAnimation(manager, id);
                manager.save();
            } else if (command == "step") {
                const std::size_t chunk = std::max<std::size_t>(1, job->totalBytes / 4);
                manager.processDownload(id, chunk, false);
                manager.save();
                printJob(*job);
            } else if (command == "pause") {
                if (manager.pause(id)) {
                    manager.save();
                    std::cout << "Paused download #" << id << ".\n";
                } else {
                    std::cout << "Download #" << id << " is not currently downloading.\n";
                }
            } else if (command == "resume") {
                if (manager.resume(id)) {
                    manager.save();
                    std::cout << "Resumed download #" << id << " (status: downloading).\n";
                } else {
                    std::cout << "Download #" << id << " cannot be resumed.\n";
                }
            } else if (command == "cancel") {
                if (manager.cancel(id)) {
                    manager.save();
                    std::cout << "Cancelled download #" << id << ".\n";
                } else {
                    std::cout << "Download #" << id << " is already completed.\n";
                }
            } else if (command == "retry") {
                manager.retry(id);
                manager.save();
                std::cout << "Reset download #" << id << " back to queued.\n";
            } else {
                const auto confirm = lowercase(prompt("Remove download '" + job->fileName + "'? [y/N]: "));
                if (confirm == "y" || confirm == "yes") {
                    manager.remove(id);
                    manager.save();
                    std::cout << "Download removed.\n";
                } else {
                    std::cout << "Removal cancelled.\n";
                }
            }
        } else if (command == "search") {
            if (argument.empty()) {
                argument = prompt("Search text: ");
            }
            if (argument.empty()) {
                std::cout << "Usage: search <text>\n";
            } else {
                listJobs(manager.search(argument));
            }
        } else if (command == "stats" || command == "summary") {
            printStats(manager.computeStats());
        } else if (command == "quit" || command == "exit") {
            if (manager.save()) {
                std::cout << "Downloads saved. Goodbye.\n";
                return 0;
            }
            std::cerr << "Could not save " << manager.dbFile() << ".\n";
            return 1;
        } else {
            std::cout << "Unknown command. Type 'help' for commands.\n";
        }
    }

    if (!manager.save()) {
        std::cerr << "Could not save " << manager.dbFile() << ".\n";
        return 1;
    }
    std::cout << "\nDownloads saved. Goodbye.\n";
    return 0;
}

