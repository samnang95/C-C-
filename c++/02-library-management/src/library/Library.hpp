#pragma once

#include "library/Book.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace libmgr {

class Library {
public:
    explicit Library(std::string dbFile = "data/books.db");

    bool load();
    bool save() const;

    Book& add(Book book);
    Book* find(int id);
    const Book* find(int id) const;
    bool remove(int id);
    std::vector<const Book*> search(std::string_view query) const;

    const std::vector<Book>& books() const { return books_; }
    const std::string& dbFile() const { return dbFile_; }

private:
    std::string dbFile_;
    std::vector<Book> books_;
    int nextId_{1};
};

} // namespace libmgr

