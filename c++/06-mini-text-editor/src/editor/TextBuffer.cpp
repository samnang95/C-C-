#include "editor/TextBuffer.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

namespace textedit {

std::string_view trim(std::string_view text) {
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos) return {};
    const auto last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

std::string lowercase(std::string_view text) {
    std::string result(text);
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

bool containsIgnoreCase(std::string_view value, std::string_view queryLower) {
    return lowercase(value).find(queryLower) != std::string::npos;
}

TextBuffer::TextBuffer(std::string filePath)
    : filePath_(std::move(filePath)) {}

void TextBuffer::pushUndoState() {
    undoStack_.push_back(lines_);
    redoStack_.clear();
    modified_ = true;
}

bool TextBuffer::load(std::string_view path) {
    if (!path.empty()) {
        filePath_ = std::string(trim(path));
    }
    std::ifstream input(filePath_);
    if (!input) return false;

    std::vector<std::string> loaded;
    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        loaded.push_back(std::move(line));
    }

    lines_ = std::move(loaded);
    undoStack_.clear();
    redoStack_.clear();
    modified_ = false;
    return true;
}

bool TextBuffer::save(std::string_view path) {
    if (!path.empty()) {
        filePath_ = std::string(trim(path));
    }
    if (filePath_.empty()) return false;

    std::ofstream output(filePath_, std::ios::trunc);
    if (!output) return false;

    for (const auto& line : lines_) {
        output << line << '\n';
    }
    if (output) {
        modified_ = false;
        return true;
    }
    return false;
}

void TextBuffer::clear() {
    if (lines_.empty()) return;
    pushUndoState();
    lines_.clear();
}

void TextBuffer::appendLine(std::string line) {
    pushUndoState();
    lines_.push_back(std::move(line));
}

bool TextBuffer::insertLine(std::size_t lineNumber, std::string line) {
    if (lineNumber < 1 || lineNumber > lines_.size() + 1) return false;
    pushUndoState();
    lines_.insert(lines_.begin() + static_cast<std::ptrdiff_t>(lineNumber - 1), std::move(line));
    return true;
}

bool TextBuffer::editLine(std::size_t lineNumber, std::string newText) {
    if (lineNumber < 1 || lineNumber > lines_.size()) return false;
    pushUndoState();
    lines_[lineNumber - 1] = std::move(newText);
    return true;
}

bool TextBuffer::deleteLine(std::size_t lineNumber) {
    if (lineNumber < 1 || lineNumber > lines_.size()) return false;
    pushUndoState();
    lines_.erase(lines_.begin() + static_cast<std::ptrdiff_t>(lineNumber - 1));
    return true;
}

std::size_t TextBuffer::replaceAll(std::string_view from, std::string_view to) {
    if (from.empty()) return 0;

    std::vector<std::string> updated = lines_;
    std::size_t count = 0;

    for (auto& line : updated) {
        std::size_t pos = 0;
        while ((pos = line.find(from, pos)) != std::string::npos) {
            line.replace(pos, from.size(), to);
            pos += to.size();
            ++count;
        }
    }

    if (count > 0) {
        pushUndoState();
        lines_ = std::move(updated);
    }
    return count;
}

std::vector<SearchMatch> TextBuffer::find(std::string_view query) const {
    std::vector<SearchMatch> matches;
    const auto queryLower = lowercase(query);
    if (queryLower.empty()) return matches;

    for (std::size_t i = 0; i < lines_.size(); ++i) {
        const auto lineLower = lowercase(lines_[i]);
        const auto col = lineLower.find(queryLower);
        if (col != std::string::npos) {
            matches.push_back(SearchMatch{i + 1, col + 1, lines_[i]});
        }
    }
    return matches;
}

bool TextBuffer::undo() {
    if (undoStack_.empty()) return false;
    redoStack_.push_back(lines_);
    lines_ = std::move(undoStack_.back());
    undoStack_.pop_back();
    modified_ = true;
    return true;
}

bool TextBuffer::redo() {
    if (redoStack_.empty()) return false;
    undoStack_.push_back(lines_);
    lines_ = std::move(redoStack_.back());
    redoStack_.pop_back();
    modified_ = true;
    return true;
}

DocumentStats TextBuffer::stats() const {
    DocumentStats st;
    st.lineCount = lines_.size();
    st.modified = modified_;

    for (const auto& line : lines_) {
        st.charCount += line.size();
        std::istringstream iss(line);
        std::string word;
        while (iss >> word) {
            ++st.wordCount;
        }
    }
    return st;
}

} // namespace textedit

