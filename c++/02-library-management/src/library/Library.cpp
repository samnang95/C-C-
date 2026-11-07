#include "library/Library.hpp"

#include <algorithm>
#include <charconv>
#include <fstream>

namespace libmgr {

namespace {

std::string escapeField(std::string_view value) {
    std::string result;
    for (const char c : value) {
        if (c == '\\') result += "\\\\";
        else if (c == '\t') result += "\\t";
        else if (c == '\n') result += "\\n";
        else result += c;
    }
    return result;
}

std::string unescapeField(std::string_view value) {
    std::string result;
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '\\' && i + 1 < value.size()) {
            const char next = value[++i];
            result += (next == 't') ? '\t' : ((next == 'n') ? '\n' : next);
        } else {
            result += value[i];
        }
    }
    return result;
}

std::vector<std::string> splitFields(std::string_view line) {
    std::vector<std::string> fields;
    std::string field;
    bool escaped = false;
    for (const char c : line) {
        if (escaped) {
            field += '\\';
            field += c;
            escaped = false;
        } else if (c == '\\') {
            escaped = true;
        } else if (c == '\t') {
            fields.push_back(unescapeField(field));
            field.clear();
        } else {
            field += c;
        }
    }
    if (escaped) field += '\\';
    fields.push_back(unescapeField(field));
    return fields;
}

} // namespace

Library::Library(std::string dbFile)
    : dbFile_(std::move(dbFile)) {}

bool Library::load() {
    std::ifstream input(dbFile_);
    if (!input) return false;

    std::string line;
    while (std::getline(input, line)) {
        const auto fields = splitFields(line);
        if (fields.size() != 7) continue;

        int id = 0;
        auto res = std::from_chars(fields[0].data(), fields[0].data() + fields[0].size(), id);
        if (res.ec != std::errc() || res.ptr != fields[0].data() + fields[0].size() || id < 1) {
            continue;
        }

        Book book{
            id,
            fields[1],
            fields[2],
            fields[3],
            fields[4],
            fields[5] == "1",
            fields[6]
        };

        if (book.title.empty() || book.author.empty()) continue;
        if (!book.borrowed || book.borrower.empty()) {
            book.borrowed = false;
            book.borrower.clear();
        }

        books_.push_back(std::move(book));
        nextId_ = std::max(nextId_, id + 1);
    }
    return true;
}

bool Library::save() const {
    std::ofstream output(dbFile_, std::ios::trunc);
    if (!output) return false;

    for (const auto& book : books_) {
        output << book.id << '\t'
               << escapeField(book.title) << '\t'
               << escapeField(book.author) << '\t'
               << escapeField(book.isbn) << '\t'
               << escapeField(book.category) << '\t'
               << (book.borrowed ? "1" : "0") << '\t'
               << escapeField(book.borrower) << '\n';
    }
    return static_cast<bool>(output);
}

Book& Library::add(Book book) {
    book.id = nextId_++;
    books_.push_back(std::move(book));
    return books_.back();
}

Book* Library::find(int id) {
    for (auto& book : books_) {
        if (book.id == id) return &book;
    }
    return nullptr;
}

const Book* Library::find(int id) const {
    for (const auto& book : books_) {
        if (book.id == id) return &book;
    }
    return nullptr;
}

bool Library::remove(int id) {
    const auto originalSize = books_.size();
    books_.erase(std::remove_if(books_.begin(), books_.end(),
                                [id](const Book& book) { return book.id == id; }),
                 books_.end());
    return books_.size() != originalSize;
}

std::vector<const Book*> Library::search(std::string_view query) const {
    const auto queryLower = lowercase(trim(query));
    std::vector<const Book*> matches;
    if (queryLower.empty()) return matches;

    for (const auto& book : books_) {
        if (containsIgnoreCase(book.title, queryLower) ||
            containsIgnoreCase(book.author, queryLower) ||
            containsIgnoreCase(book.isbn, queryLower) ||
            containsIgnoreCase(book.category, queryLower) ||
            containsIgnoreCase(book.borrower, queryLower)) {
            matches.push_back(&book);
        }
    }
    return matches;
}

} // namespace libmgr

