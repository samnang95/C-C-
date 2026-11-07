#include "ui/ConsoleUI.hpp"

#include <charconv>
#include <iomanip>
#include <iostream>

namespace taskmgr {

void printHelp() {
    std::cout << "\nCommands:\n"
              << "  add                 Create a task\n"
              << "  list [open|done]    List tasks\n"
              << "  view <id>           Show task details\n"
              << "  done <id>           Complete a task\n"
              << "  reopen <id>         Reopen a task\n"
              << "  delete <id>         Delete a task\n"
              << "  search <text>       Search titles, descriptions, and tags\n"
              << "  clear / reset       Clear the terminal screen\n"
              << "  help                Show this help\n"
              << "  quit                Save and exit\n\n";
}

void printTask(const Task& task) {
    std::cout << '\n' << (task.completed ? "[x] " : "[ ] ") << task.title << " (ID " << task.id << ")\n"
              << "Priority: " << priorityName(task.priority) << '\n'
              << "Due date: " << (task.dueDate.empty() ? "none" : task.dueDate) << '\n'
              << "Tags: " << (task.tags.empty() ? "none" : joinTags(task.tags)) << '\n'
              << "Description: " << (task.description.empty() ? "none" : task.description) << "\n";
}

void listTasks(const std::vector<const Task*>& tasks, std::string_view filter) {
    bool any = false;
    std::cout << "\n" << std::left << std::setw(5) << "ID" << std::setw(8) << "Status"
              << std::setw(9) << "Priority" << std::setw(13) << "Due" << "Title\n";
    std::cout << std::string(65, '-') << '\n';
    for (const auto* task : tasks) {
        if (!task) continue;
        if ((filter == "open" && task->completed) || (filter == "done" && !task->completed)) continue;
        any = true;
        std::cout << std::left << std::setw(5) << task->id << std::setw(8)
                  << (task->completed ? "done" : "open") << std::setw(9) << priorityName(task->priority)
                  << std::setw(13) << (task->dueDate.empty() ? "-" : task->dueDate) << task->title << '\n';
    }
    if (!any) std::cout << "No matching tasks.\n";
    std::cout << '\n';
}

void listTasks(const std::vector<Task>& tasks, std::string_view filter) {
    std::vector<const Task*> ptrs;
    ptrs.reserve(tasks.size());
    for (const auto& task : tasks) {
        ptrs.push_back(&task);
    }
    listTasks(ptrs, filter);
}

bool parseId(std::string_view value, int& id) {
    auto trimmed = trim(value);
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
    if (!std::getline(std::cin, value)) {
        return "";
    }
    return std::string(trim(value));
}

void addTask(TaskManager& manager) {
    Task task;

    while (std::cin) {
        task.title = prompt("Title: ");
        if (!std::cin) return;
        if (task.title.empty()) {
            std::cout << "A title is required.\n";
            continue;
        }
        break;
    }

    task.description = prompt("Description (optional): ");
    if (!std::cin) return;

    while (std::cin) {
        const auto value = prompt("Priority [low/medium/high] (medium): ");
        if (!std::cin) return;
        if (value.empty() || lowercase(value) == "low" || lowercase(value) == "medium" || lowercase(value) == "high") {
            task.priority = parsePriority(value);
            break;
        }
        std::cout << "Please enter low, medium, or high.\n";
    }

    while (std::cin) {
        task.dueDate = prompt("Due date YYYY-MM-DD (optional): ");
        if (!std::cin) return;
        if (isValidDate(task.dueDate)) {
            break;
        }
        std::cout << "That date is invalid.\n";
    }

    task.tags = splitTags(prompt("Tags, separated by commas (optional): "));
    if (!std::cin) return;

    const auto& added = manager.add(std::move(task));
    std::cout << "Added task " << added.id << ".\n";
}

} // namespace taskmgr
