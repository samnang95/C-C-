#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr const char* kDatabaseFile = "books.db";

struct Book {
    int id{};
    std::string title;
    std::string author;
    std::string isbn;
    std::string category;
    bool borrowed{};
    std::string borrower;
};

std::string trim(std::string text) {
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    return text.substr(first, text.find_last_not_of(" \t\r\n") - first + 1);
}

std::string lowercase(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

std::string escapeField(const std::string& value) {
    std::string result;
    for (const char character : value) {
        if (character == '\\') result += "\\\\";
        else if (character == '\t') result += "\\t";
        else if (character == '\n') result += "\\n";
        else result += character;
    }
    return result;
}

std::string unescapeField(const std::string& value) {
    std::string result;
    for (std::size_t index = 0; index < value.size(); ++index) {
        if (value[index] == '\\' && index + 1 < value.size()) {
            const char next = value[++index];
            result += next == 't' ? '\t' : (next == 'n' ? '\n' : next);
        } else {
            result += value[index];
        }
    }
    return result;
}

std::vector<std::string> splitFields(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool escaped = false;
    for (const char character : line) {
        if (escaped) { field += '\\'; field += character; escaped = false; }
        else if (character == '\\') escaped = true;
        else if (character == '\t') { fields.push_back(unescapeField(field)); field.clear(); }
        else field += character;
    }
    if (escaped) field += '\\';
    fields.push_back(unescapeField(field));
    return fields;
}

class Library {
public:
    void load() {
        std::ifstream input(kDatabaseFile);
        std::string line;
        while (std::getline(input, line)) {
            const auto fields = splitFields(line);
            if (fields.size() != 7) continue;
            try {
                Book book{std::stoi(fields[0]), fields[1], fields[2], fields[3], fields[4], fields[5] == "1", fields[6]};
                if (book.id < 1 || book.title.empty() || book.author.empty()) continue;
                if (!book.borrowed) book.borrower.clear();
                books_.push_back(book);
                nextId_ = std::max(nextId_, book.id + 1);
            } catch (const std::exception&) {
                // Keep readable records when a database line is malformed.
            }
        }
    }

    bool save() const {
        std::ofstream output(kDatabaseFile, std::ios::trunc);
        if (!output) return false;
        for (const auto& book : books_) {
            output << book.id << '\t' << escapeField(book.title) << '\t' << escapeField(book.author) << '\t'
                   << escapeField(book.isbn) << '\t' << escapeField(book.category) << '\t'
                   << (book.borrowed ? "1" : "0") << '\t' << escapeField(book.borrower) << '\n';
        }
        return static_cast<bool>(output);
    }

    Book& add(Book book) { book.id = nextId_++; books_.push_back(std::move(book)); return books_.back(); }
    Book* find(int id) { for (auto& book : books_) if (book.id == id) return &book; return nullptr; }
    bool remove(int id) {
        const auto originalSize = books_.size();
        books_.erase(std::remove_if(books_.begin(), books_.end(), [id](const Book& book) { return book.id == id; }), books_.end());
        return books_.size() != originalSize;
    }
    const std::vector<Book>& books() const { return books_; }

private:
    std::vector<Book> books_;
    int nextId_{1};
};

bool parseId(const std::string& text, int& id) {
    try {
        std::size_t position = 0;
        id = std::stoi(text, &position);
        return position == text.size() && id > 0;
    } catch (const std::exception&) { return false; }
}

std::string prompt(const std::string& label) {
    std::cout << label;
    std::string value;
    std::getline(std::cin, value);
    return trim(value);
}

void printHelp() {
    std::cout << "\nCommands:\n"
              << "  add                         Add a book\n"
              << "  list [available|borrowed]   List books\n"
              << "  view <id>                   Show book details\n"
              << "  search <text>               Search the catalogue\n"
              << "  borrow <id>                 Borrow a book\n"
              << "  return <id>                 Return a book\n"
              << "  remove <id>                 Remove a book\n"
              << "  help                        Show this help\n"
              << "  quit                        Save and exit\n\n";
}

void printBook(const Book& book) {
    std::cout << "\n" << book.title << " (ID " << book.id << ")\n"
              << "Author: " << book.author << '\n'
              << "ISBN: " << (book.isbn.empty() ? "none" : book.isbn) << '\n'
              << "Category: " << (book.category.empty() ? "none" : book.category) << '\n'
              << "Status: " << (book.borrowed ? "borrowed" : "available") << '\n';
    if (book.borrowed) std::cout << "Borrower: " << book.borrower << '\n';
}

void listBooks(const std::vector<Book>& books, const std::string& filter) {
    bool found = false;
    std::cout << "\n" << std::left << std::setw(5) << "ID" << std::setw(12) << "Status"
              << std::setw(28) << "Title" << std::setw(23) << "Author" << "Category\n"
              << std::string(85, '-') << '\n';
    for (const auto& book : books) {
        if ((filter == "available" && book.borrowed) || (filter == "borrowed" && !book.borrowed)) continue;
        found = true;
        std::cout << std::left << std::setw(5) << book.id << std::setw(12)
                  << (book.borrowed ? "borrowed" : "available") << std::setw(28) << book.title.substr(0, 27)
                  << std::setw(23) << book.author.substr(0, 22) << book.category << '\n';
    }
    if (!found) std::cout << "No matching books.\n";
    std::cout << '\n';
}

void addBook(Library& library) {
    Book book;
    do { book.title = prompt("Title: "); if (book.title.empty()) std::cout << "A title is required.\n"; } while (book.title.empty() && std::cin);
    do { book.author = prompt("Author: "); if (book.author.empty()) std::cout << "An author is required.\n"; } while (book.author.empty() && std::cin);
    book.isbn = prompt("ISBN (optional): ");
    book.category = prompt("Category (optional): ");
    const auto& added = library.add(std::move(book));
    std::cout << "Added book " << added.id << ".\n";
}

bool contains(const std::string& value, const std::string& query) { return lowercase(value).find(query) != std::string::npos; }

void searchBooks(const Library& library, const std::string& query) {
    std::vector<Book> matches;
    for (const auto& book : library.books()) {
        if (contains(book.title, query) || contains(book.author, query) || contains(book.isbn, query) ||
            contains(book.category, query) || contains(book.borrower, query)) matches.push_back(book);
    }
    listBooks(matches, "");
}

}  // namespace

int main() {
    Library library;
    library.load();
    std::cout << "Library Management — type 'help' for commands.\n";

    std::string line;
    while (std::cout << "library> " && std::getline(std::cin, line)) {
        std::istringstream input(trim(line));
        std::string command;
        input >> command;
        command = lowercase(command);
        std::string argument;
        std::getline(input, argument);
        argument = trim(argument);

        if (command.empty()) continue;
        if (command == "help") printHelp();
        else if (command == "add") addBook(library);
        else if (command == "list") {
            const auto filter = lowercase(argument);
            if (filter.empty() || filter == "available" || filter == "borrowed") listBooks(library.books(), filter);
            else std::cout << "Usage: list [available|borrowed]\n";
        } else if (command == "view") {
            int id{};
            if (!parseId(argument, id)) { std::cout << "Usage: view <id>\n"; continue; }
            const Book* book = library.find(id);
            if (book) printBook(*book); else std::cout << "Book not found.\n";
        } else if (command == "search") {
            if (argument.empty()) std::cout << "Usage: search <text>\n";
            else searchBooks(library, lowercase(argument));
        } else if (command == "borrow" || command == "return" || command == "remove") {
            int id{};
            if (!parseId(argument, id)) { std::cout << "Usage: " << command << " <id>\n"; continue; }
            Book* book = library.find(id);
            if (!book) { std::cout << "Book not found.\n"; continue; }
            if (command == "borrow") {
                if (book->borrowed) { std::cout << "That book is already borrowed by " << book->borrower << ".\n"; continue; }
                const auto borrower = prompt("Borrower name: ");
                if (borrower.empty()) { std::cout << "A borrower name is required.\n"; continue; }
                book->borrowed = true;
                book->borrower = borrower;
                std::cout << "Book borrowed.\n";
            } else if (command == "return") {
                if (!book->borrowed) { std::cout << "That book is already available.\n"; continue; }
                book->borrowed = false;
                book->borrower.clear();
                std::cout << "Book returned.\n";
            } else {
                const auto confirmation = lowercase(prompt("Remove '" + book->title + "'? [y/N]: "));
                if (confirmation == "y" || confirmation == "yes") { library.remove(id); std::cout << "Book removed.\n"; }
                else std::cout << "Removal cancelled.\n";
            }
        } else if (command == "quit" || command == "exit") {
            if (library.save()) std::cout << "Library saved. Goodbye.\n";
            else std::cerr << "Could not save " << kDatabaseFile << ".\n";
            return 0;
        } else {
            std::cout << "Unknown command. Type 'help' for commands.\n";
        }
    }

    if (!library.save()) std::cerr << "Could not save " << kDatabaseFile << ".\n";
}
