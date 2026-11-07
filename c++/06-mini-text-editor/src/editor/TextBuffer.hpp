#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace textedit {

struct SearchMatch {
    std::size_t lineNumber{}; // 1-based
    std::size_t column{};     // 1-based
    std::string lineContent;
};

struct DocumentStats {
    std::size_t lineCount{};
    std::size_t wordCount{};
    std::size_t charCount{};
    bool modified{};
};

std::string_view trim(std::string_view text);
std::string lowercase(std::string_view text);
bool containsIgnoreCase(std::string_view value, std::string_view queryLower);

class TextBuffer {
public:
    explicit TextBuffer(std::string filePath = "data/notes.txt");

    bool load(std::string_view path = "");
    bool save(std::string_view path = "");
    void clear();

    void appendLine(std::string line);
    bool insertLine(std::size_t lineNumber, std::string line);
    bool editLine(std::size_t lineNumber, std::string newText);
    bool deleteLine(std::size_t lineNumber);

    std::size_t replaceAll(std::string_view from, std::string_view to);
    std::vector<SearchMatch> find(std::string_view query) const;

    bool undo();
    bool redo();

    DocumentStats stats() const;

    const std::vector<std::string>& lines() const { return lines_; }
    const std::string& filePath() const { return filePath_; }
    bool isModified() const { return modified_; }

private:
    void pushUndoState();

    std::string filePath_;
    std::vector<std::string> lines_;
    std::vector<std::vector<std::string>> undoStack_;
    std::vector<std::vector<std::string>> redoStack_;
    bool modified_{false};
};

} // namespace textedit

