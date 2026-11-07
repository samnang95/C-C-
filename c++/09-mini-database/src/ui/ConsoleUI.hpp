#pragma once

#include "db/Database.hpp"
#include "db/Table.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace minidb {

void printHelp();
void listTables(const Database& db);
void describeTable(const Table& table);
void printQueryResult(const QueryResult& result);

bool parseId(std::string_view text, int& id);
std::vector<std::string> splitCsv(std::string_view line);
std::string prompt(std::string_view label);

void createTableInteractive(Database& db, std::string_view args);
void insertRowInteractive(Database& db, std::string_view args);
void selectInteractive(const Database& db, std::string_view args);
void updateInteractive(Database& db, std::string_view args);
void deleteInteractive(Database& db, std::string_view args);

} // namespace minidb

