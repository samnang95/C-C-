#include "ui/ConsoleUI.hpp"

#include <charconv>
#include <iomanip>
#include <iostream>

namespace libmgr {

void printHelp() {
    std::cout << "\nCommands:\n"
              << "  add                         Add a book\n"
              << "  list [available|borrowed]   List books\n"
              << "  view <id>                   Show book details\n"
              << "  search <text>               Search the catalogue\n"
              << "  borrow <id>                 Borrow a book\n"
              << "  return <id>                 Return a book\n"
              << "  remove <id>                 Remove a book\n"
              << "  clear / reset               Clear the terminal screen\n"
              << "  help                        Show this help\n"
              << "  quit                        Save and exit\n\n";
}

void printBook(const Book& book) {
    std::cout << "\n" << book.title << " (ID " << book.id << ")\n"
              << "Author: " << book.author << '\n'
              << "ISBN: " << (book.isbn.empty() ? "none" : book.isbn) << '\n'
              << "Category: " << (book.category.empty() ? "none" : book.category) << '\n'
              << "Status: " << (book.borrowed ? "borrowed" : "available") << '\n';
    if (book.borrowed) {
        std::cout << "Borrower: " << book.borrower << '\n';
    }
}

void listBooks(const std::vector<const Book*>& books, std::string_view filter) {
    bool found = false;
    std::cout << "\n" << std::left << std::setw(5) << "ID" << std::setw(12) << "Status"
              << std::setw(28) << "Title" << std::setw(23) << "Author" << "Category\n"
              << std::string(85, '-') << '\n';
    for (const auto* book : books) {
        if (!book) continue;
        if ((filter == "available" && book->borrowed) || (filter == "borrowed" && !book->borrowed)) {
            continue;
        }
        found = true;
        std::cout << std::left << std::setw(5) << book->id << std::setw(12)
                  << (book->borrowed ? "borrowed" : "available") << std::setw(28)
                  << book->title.substr(0, 27) << std::setw(23) << book->author.substr(0, 22)
                  << book->category << '\n';
    }
    if (!found) std::cout << "No matching books.\n";
    std::cout << '\n';
}

void listBooks(const std::vector<Book>& books, std::string_view filter) {
    std::vector<const Book*> ptrs;
    ptrs.reserve(books.size());
    for (const auto& book : books) {
        ptrs.push_back(&book);
    }
    listBooks(ptrs, filter);
}

bool parseId(std::string_view text, int& id) {
    auto trimmed = trim(text);
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

void addBook(Library& library) {
    Book book;

    while (std::cin) {
        book.title = prompt("Title: ");
        if (!std::cin) return;
        if (book.title.empty()) {
            std::cout << "A title is required.\n";
            continue;
        }
        break;
    }

    while (std::cin) {
        book.author = prompt("Author: ");
        if (!std::cin) return;
        if (book.author.empty()) {
            std::cout << "An author is required.\n";
            continue;
        }
        break;
    }

    book.isbn = prompt("ISBN (optional): ");
    if (!std::cin) return;

    book.category = prompt("Category (optional): ");
    if (!std::cin) return;

    const auto& added = library.add(std::move(book));
    std::cout << "Added book " << added.id << ".\n";
}

} // namespace libmgr

