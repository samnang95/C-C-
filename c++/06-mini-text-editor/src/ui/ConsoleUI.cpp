#include "ui/ConsoleUI.hpp"

#include <charconv>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace textedit {

void printHelp() {
    std::cout << "\nCommands:\n"
              << "  open <file>             Open a text file into the buffer\n"
              << "  save [file]             Save buffer to current or new file\n"
              << "  print / list            Display all lines with line numbers\n"
              << "  append [text]           Append a line (or enter multi-line mode with '.')\n"
              << "  insert <line> [text]    Insert a line at position <line>\n"
              << "  edit <line> [text]      Replace the text at <line>\n"
              << "  delete <line>           Delete line <line>\n"
              << "  find <text>             Search for text across lines\n"
              << "  replace                 Find and replace text across the document\n"
              << "  undo / redo             Undo or redo the last edit\n"
              << "  stats                   Show line, word, and character counts\n"
              << "  new                     Clear the current buffer\n"
              << "  clear / reset           Clear the terminal screen\n"
              << "  help                    Show this help\n"
              << "  quit                    Save and exit\n\n";
}

void printBuffer(const TextBuffer& buffer, std::size_t startLine, std::size_t endLine) {
    const auto& lines = buffer.lines();
    std::cout << "\n--- " << buffer.filePath() << (buffer.isModified() ? " [modified]" : "") << " ---\n";
    if (lines.empty()) {
        std::cout << "(empty buffer)\n\n";
        return;
    }

    const std::size_t first = (startLine < 1) ? 1 : startLine;
    const std::size_t last = (endLine == 0 || endLine > lines.size()) ? lines.size() : endLine;

    for (std::size_t i = first; i <= last; ++i) {
        std::cout << std::right << std::setw(4) << i << " | " << lines[i - 1] << '\n';
    }
    std::cout << '\n';
}

void printMatches(const std::vector<SearchMatch>& matches) {
    if (matches.empty()) {
        std::cout << "No matches found.\n";
        return;
    }
    std::cout << "\nFound " << matches.size() << " match(es):\n";
    for (const auto& m : matches) {
        std::cout << "  Line " << m.lineNumber << ", Col " << m.column << ": " << m.lineContent << '\n';
    }
    std::cout << '\n';
}

void printStats(const TextBuffer& buffer) {
    const auto st = buffer.stats();
    std::cout << "\nDocument Statistics (" << buffer.filePath() << "):\n"
              << "  Lines:      " << st.lineCount << '\n'
              << "  Words:      " << st.wordCount << '\n'
              << "  Characters: " << st.charCount << '\n'
              << "  Status:     " << (st.modified ? "modified (unsaved)" : "saved") << "\n\n";
}

bool parseLineNumber(std::string_view text, std::size_t& lineNumber) {
    auto trimmed = trim(text);
    if (trimmed.empty()) return false;

    std::size_t parsed = 0;
    auto res = std::from_chars(trimmed.data(), trimmed.data() + trimmed.size(), parsed);
    if (res.ec != std::errc() || res.ptr != trimmed.data() + trimmed.size() || parsed < 1) {
        return false;
    }
    lineNumber = parsed;
    return true;
}

std::string prompt(std::string_view label) {
    std::cout << label;
    std::string value;
    if (!std::getline(std::cin, value)) return "";
    return std::string(trim(value));
}

std::string promptRawLine(std::string_view label) {
    std::cout << label;
    std::string value;
    if (!std::getline(std::cin, value)) return "";
    if (!value.empty() && value.back() == '\r') value.pop_back();
    return value;
}

void appendInteractive(TextBuffer& buffer, std::string_view inlineText) {
    if (!inlineText.empty()) {
        buffer.appendLine(std::string(inlineText));
        std::cout << "Appended line " << buffer.lines().size() << ".\n";
        return;
    }

    std::cout << "Enter lines to append (type '.' on a single line to finish):\n";
    std::string line;
    std::size_t added = 0;
    while (std::cout << "  + " && std::getline(std::cin, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line == ".") break;
        buffer.appendLine(std::move(line));
        ++added;
    }
    std::cout << "Appended " << added << " line(s).\n";
}

void insertInteractive(TextBuffer& buffer, std::string_view args) {
    std::istringstream iss{std::string(args)};
    std::string lineToken;
    iss >> lineToken;

    std::string rest;
    std::getline(iss, rest);
    if (!rest.empty() && rest.front() == ' ') rest.erase(0, 1);

    if (lineToken.empty()) {
        lineToken = prompt("Line number (1.." + std::to_string(buffer.lines().size() + 1) + "): ");
    }

    std::size_t lineNum = 0;
    if (!parseLineNumber(lineToken, lineNum) || lineNum > buffer.lines().size() + 1) {
        std::cout << "Invalid line number.\n";
        return;
    }

    if (rest.empty()) {
        rest = promptRawLine("Text: ");
        if (!std::cin) return;
    }

    buffer.insertLine(lineNum, std::move(rest));
    std::cout << "Inserted at line " << lineNum << ".\n";
}

void editInteractive(TextBuffer& buffer, std::string_view args) {
    if (buffer.lines().empty()) {
        std::cout << "Buffer is empty.\n";
        return;
    }

    std::istringstream iss{std::string(args)};
    std::string lineToken;
    iss >> lineToken;

    std::string rest;
    std::getline(iss, rest);
    if (!rest.empty() && rest.front() == ' ') rest.erase(0, 1);

    if (lineToken.empty()) {
        lineToken = prompt("Line number (1.." + std::to_string(buffer.lines().size()) + "): ");
    }

    std::size_t lineNum = 0;
    if (!parseLineNumber(lineToken, lineNum) || lineNum > buffer.lines().size()) {
        std::cout << "Invalid line number.\n";
        return;
    }

    if (rest.empty()) {
        std::cout << "Current: " << buffer.lines()[lineNum - 1] << '\n';
        rest = promptRawLine("New text: ");
        if (!std::cin) return;
    }

    buffer.editLine(lineNum, std::move(rest));
    std::cout << "Updated line " << lineNum << ".\n";
}

void replaceInteractive(TextBuffer& buffer) {
    const auto from = promptRawLine("Find text: ");
    if (!std::cin || from.empty()) return;
    const auto to = promptRawLine("Replace with: ");
    if (!std::cin) return;

    const auto count = buffer.replaceAll(from, to);
    std::cout << "Replaced " << count << " occurrence(s).\n";
}

} // namespace textedit

