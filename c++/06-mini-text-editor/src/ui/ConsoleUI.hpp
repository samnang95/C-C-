#pragma once

#include "editor/TextBuffer.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace textedit {

void printHelp();
void printBuffer(const TextBuffer& buffer, std::size_t startLine = 1, std::size_t endLine = 0);
void printMatches(const std::vector<SearchMatch>& matches);
void printStats(const TextBuffer& buffer);

bool parseLineNumber(std::string_view text, std::size_t& lineNumber);
std::string prompt(std::string_view label);
std::string promptRawLine(std::string_view label);

void appendInteractive(TextBuffer& buffer, std::string_view inlineText);
void insertInteractive(TextBuffer& buffer, std::string_view args);
void editInteractive(TextBuffer& buffer, std::string_view args);
void replaceInteractive(TextBuffer& buffer);

} // namespace textedit

