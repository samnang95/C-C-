#include <algorithm>
#include <cctype>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace {

enum class Priority { Low, Medium, High };

struct Task {
    int id{};
    std::string title;
    std::string description;
    Priority priority{Priority::Medium};
    std::string dueDate;
    std::vector<std::string> tags;
    bool completed{};
};

constexpr const char* kDatabaseFile = "tasks.db";

std::string trim(std::string text) {
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    const auto last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

std::string lowercase(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

std::string priorityName(Priority priority) {
    switch (priority) {
        case Priority::Low: return "low";
        case Priority::High: return "high";
        default: return "medium";
    }
}

Priority parsePriority(const std::string& text) {
    const auto value = lowercase(trim(text));
    if (value == "low") return Priority::Low;
    if (value == "high") return Priority::High;
    return Priority::Medium;
}

bool isValidDate(const std::string& date) {
    if (date.empty()) return true;
    if (date.size() != 10 || date[4] != '-' || date[7] != '-') return false;
    for (std::size_t i = 0; i < date.size(); ++i) {
        if (i != 4 && i != 7 && !std::isdigit(static_cast<unsigned char>(date[i]))) return false;
    }
    const int year = std::stoi(date.substr(0, 4));
    const int month = std::stoi(date.substr(5, 2));
    const int day = std::stoi(date.substr(8, 2));
    if (year < 1 || month < 1 || month > 12 || day < 1) return false;
    static const int daysPerMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    const bool leap = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
    const int maxDay = month == 2 && leap ? 29 : daysPerMonth[month - 1];
    return day <= maxDay;
}

std::vector<std::string> splitTags(const std::string& line) {
    std::vector<std::string> tags;
    std::stringstream stream(line);
    std::string item;
    while (std::getline(stream, item, ',')) {
        item = trim(item);
        if (!item.empty()) tags.push_back(item);
    }
    return tags;
}

std::string joinTags(const std::vector<std::string>& tags, const char* separator = ", ") {
    std::ostringstream output;
    for (std::size_t i = 0; i < tags.size(); ++i) {
        if (i) output << separator;
        output << tags[i];
    }
    return output.str();
}

// Escape field separators and newlines so the plain-text database can safely store user input.
std::string escapeField(const std::string& value) {
    std::string escaped;
    for (char c : value) {
        if (c == '\\') escaped += "\\\\";
        else if (c == '\t') escaped += "\\t";
        else if (c == '\n') escaped += "\\n";
        else escaped += c;
    }
    return escaped;
}

std::string unescapeField(const std::string& value) {
    std::string result;
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '\\' && i + 1 < value.size()) {
            const char next = value[++i];
            result += next == 't' ? '\t' : (next == 'n' ? '\n' : next);
        } else {
            result += value[i];
        }
    }
    return result;
}

std::vector<std::string> splitFields(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool escaped = false;
    for (char c : line) {
        if (escaped) { field += '\\'; field += c; escaped = false; }
        else if (c == '\\') escaped = true;
        else if (c == '\t') { fields.push_back(unescapeField(field)); field.clear(); }
        else field += c;
    }
    if (escaped) field += '\\';
    fields.push_back(unescapeField(field));
    return fields;
}

class TaskManager {
public:
    void load() {
        std::ifstream input(kDatabaseFile);
        std::string line;
        while (std::getline(input, line)) {
            const auto fields = splitFields(line);
            if (fields.size() != 7) continue;
            try {
                Task task;
                task.id = std::stoi(fields[0]);
                task.title = fields[1];
                task.description = fields[2];
                task.priority = parsePriority(fields[3]);
                task.dueDate = fields[4];
                task.tags = splitTags(fields[5]);
                task.completed = fields[6] == "1";
                tasks_.push_back(task);
                nextId_ = std::max(nextId_, task.id + 1);
            } catch (const std::exception&) {
                // Ignore malformed records while retaining all readable tasks.
            }
        }
    }

    bool save() const {
        std::ofstream output(kDatabaseFile, std::ios::trunc);
        if (!output) return false;
        for (const auto& task : tasks_) {
            output << task.id << '\t' << escapeField(task.title) << '\t'
                   << escapeField(task.description) << '\t' << priorityName(task.priority) << '\t'
                   << escapeField(task.dueDate) << '\t' << escapeField(joinTags(task.tags, ",")) << '\t'
                   << (task.completed ? "1" : "0") << '\n';
        }
        return static_cast<bool>(output);
    }

    Task& add(Task task) { task.id = nextId_++; tasks_.push_back(std::move(task)); return tasks_.back(); }
    Task* find(int id) { for (auto& task : tasks_) if (task.id == id) return &task; return nullptr; }
    bool remove(int id) {
        const auto oldSize = tasks_.size();
        tasks_.erase(std::remove_if(tasks_.begin(), tasks_.end(), [id](const Task& task) { return task.id == id; }), tasks_.end());
        return tasks_.size() != oldSize;
    }
    const std::vector<Task>& tasks() const { return tasks_; }

private:
    std::vector<Task> tasks_;
    int nextId_{1};
};

void printHelp() {
    std::cout << "\nCommands:\n"
              << "  add                 Create a task\n"
              << "  list [open|done]    List tasks\n"
              << "  view <id>           Show task details\n"
              << "  done <id>           Complete a task\n"
              << "  reopen <id>         Reopen a task\n"
              << "  delete <id>         Delete a task\n"
              << "  search <text>       Search titles, descriptions, and tags\n"
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

void listTasks(const std::vector<Task>& tasks, const std::string& filter) {
    bool any = false;
    std::cout << "\n" << std::left << std::setw(5) << "ID" << std::setw(8) << "Status"
              << std::setw(9) << "Priority" << std::setw(13) << "Due" << "Title\n";
    std::cout << std::string(65, '-') << '\n';
    for (const auto& task : tasks) {
        if ((filter == "open" && task.completed) || (filter == "done" && !task.completed)) continue;
        any = true;
        std::cout << std::left << std::setw(5) << task.id << std::setw(8)
                  << (task.completed ? "done" : "open") << std::setw(9) << priorityName(task.priority)
                  << std::setw(13) << (task.dueDate.empty() ? "-" : task.dueDate) << task.title << '\n';
    }
    if (!any) std::cout << "No matching tasks.\n";
    std::cout << '\n';
}

bool parseId(const std::string& value, int& id) {
    try {
        std::size_t position = 0;
        id = std::stoi(value, &position);
        return position == value.size() && id > 0;
    } catch (const std::exception&) { return false; }
}

std::string prompt(const std::string& label) { std::cout << label; std::string value; std::getline(std::cin, value); return trim(value); }

void addTask(TaskManager& manager) {
    Task task;
    do { task.title = prompt("Title: "); if (task.title.empty()) std::cout << "A title is required.\n"; } while (task.title.empty() && std::cin);
    task.description = prompt("Description (optional): ");
    while (true) {
        const auto value = prompt("Priority [low/medium/high] (medium): ");
        if (value.empty() || lowercase(value) == "low" || lowercase(value) == "medium" || lowercase(value) == "high") { task.priority = parsePriority(value); break; }
        std::cout << "Please enter low, medium, or high.\n";
    }
    do { task.dueDate = prompt("Due date YYYY-MM-DD (optional): "); if (!isValidDate(task.dueDate)) std::cout << "That date is invalid.\n"; } while (!isValidDate(task.dueDate));
    task.tags = splitTags(prompt("Tags, separated by commas (optional): "));
    const auto& added = manager.add(std::move(task));
    std::cout << "Added task " << added.id << ".\n";
}

} // namespace

int main() {
    TaskManager manager;
    manager.load();
    std::cout << "Task Manager — type 'help' for commands.\n";

    std::string line;
    while (std::cout << "task> " && std::getline(std::cin, line)) {
        std::istringstream input(trim(line));
        std::string command;
        input >> command;
        command = lowercase(command);
        std::string argument;
        std::getline(input, argument);
        argument = trim(argument);
        if (command.empty()) continue;
        if (command == "help") printHelp();
        else if (command == "add") addTask(manager);
        else if (command == "list") {
            const auto filter = lowercase(argument);
            if (filter.empty() || filter == "open" || filter == "done") listTasks(manager.tasks(), filter);
            else std::cout << "Usage: list [open|done]\n";
        } else if (command == "view" || command == "done" || command == "reopen" || command == "delete") {
            int id;
            if (!parseId(argument, id)) { std::cout << "Please provide a valid task ID.\n"; continue; }
            Task* task = manager.find(id);
            if (!task) { std::cout << "Task " << id << " was not found.\n"; continue; }
            if (command == "view") printTask(*task);
            else if (command == "done") { task->completed = true; std::cout << "Task " << id << " completed.\n"; }
            else if (command == "reopen") { task->completed = false; std::cout << "Task " << id << " reopened.\n"; }
            else {
                const auto answer = lowercase(prompt("Delete task " + std::to_string(id) + " permanently? [y/N]: "));
                if (answer == "y" || answer == "yes") { manager.remove(id); std::cout << "Task deleted.\n"; }
                else std::cout << "Deletion cancelled.\n";
            }
        } else if (command == "search") {
            if (argument.empty()) { std::cout << "Usage: search <text>\n"; continue; }
            const auto query = lowercase(argument);
            std::vector<Task> matches;
            for (const auto& task : manager.tasks()) {
                const auto haystack = lowercase(task.title + " " + task.description + " " + joinTags(task.tags));
                if (haystack.find(query) != std::string::npos) matches.push_back(task);
            }
            listTasks(matches, "");
        } else if (command == "quit" || command == "exit") {
            if (!manager.save()) { std::cerr << "Could not save " << kDatabaseFile << ".\n"; return 1; }
            std::cout << "Tasks saved. Goodbye.\n";
            return 0;
        } else std::cout << "Unknown command. Type 'help' for commands.\n";
    }
    if (!manager.save()) { std::cerr << "Could not save " << kDatabaseFile << ".\n"; return 1; }
    std::cout << "\nTasks saved. Goodbye.\n";
}
