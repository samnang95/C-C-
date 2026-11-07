#pragma once

#include "library/Book.hpp"
#include "library/Library.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace libmgr {

void printHelp();
void printBook(const Book& book);
void listBooks(const std::vector<const Book*>& books, std::string_view filter = "");
void listBooks(const std::vector<Book>& books, std::string_view filter = "");
bool parseId(std::string_view text, int& id);
std::string prompt(std::string_view label);
void addBook(Library& library);

} // namespace libmgr

