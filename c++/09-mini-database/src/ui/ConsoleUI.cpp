#include "ui/ConsoleUI.hpp"

#include <algorithm>
#include <charconv>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace minidb {

void printHelp() {
    std::cout << "\nCommands:\n"
              << "  tables                              List all tables in the database\n"
              << "  create <table> <col1,col2,...>      Create a new table\n"
              << "  describe <table>                    Show table schema and row count\n"
              << "  drop <table>                        Drop a table and all its rows\n"
              << "  insert [table] [val1,val2,...]      Insert a row into a table\n"
              << "  select <table> [col=value]          Query rows from a table\n"
              << "  update <table> <id> <col=value>     Update a column value by row ID\n"
              << "  delete <table> <id>                 Delete a row by ID\n"
              << "  clear / reset                       Clear the terminal screen\n"
              << "  help                                Show this help\n"
              << "  quit                                Save database and exit\n\n";
}

void listTables(const Database& db) {
    if (db.tables().empty()) {
        std::cout << "\nNo tables created yet. Use 'create <table> <col1,col2,...>'.\n\n";
        return;
    }

    std::cout << "\n" << std::left << std::setw(22) << "Table"
              << std::setw(10) << "Rows"
              << "Columns\n"
              << std::string(72, '-') << '\n';

    for (const auto& t : db.tables()) {
        std::string cols = "id";
        for (const auto& c : t.columns) {
            cols += ", " + c;
        }
        std::cout << std::left << std::setw(22) << t.name
                  << std::setw(10) << t.rows.size()
                  << cols << '\n';
    }
    std::cout << '\n';
}

void describeTable(const Table& table) {
    std::cout << "\nTable: " << table.name << " (" << table.rows.size() << " rows)\n"
              << "  0: id (PRIMARY KEY, auto-increment)\n";
    for (std::size_t i = 0; i < table.columns.size(); ++i) {
        std::cout << "  " << (i + 1) << ": " << table.columns[i] << " (TEXT)\n";
    }
    std::cout << '\n';
}

void printQueryResult(const QueryResult& result) {
    if (!result.ok) {
        std::cout << result.message << '\n';
        return;
    }
    if (result.rows.empty()) {
        std::cout << "\n(0 rows)\n\n";
        return;
    }

    std::vector<std::size_t> widths(result.columns.size() + 1, 4);
    widths[0] = 4; // "id"
    for (std::size_t i = 0; i < result.columns.size(); ++i) {
        widths[i + 1] = std::max<std::size_t>(result.columns[i].size(), 8);
    }
    for (const auto& r : result.rows) {
        widths[0] = std::max(widths[0], std::to_string(r.id).size());
        for (std::size_t i = 0; i < r.values.size() && i < result.columns.size(); ++i) {
            widths[i + 1] = std::max(widths[i + 1], r.values[i].size());
        }
    }

    std::size_t totalWidth = 0;
    std::cout << "\n" << std::left << std::setw(static_cast<int>(widths[0] + 3)) << "id";
    totalWidth += widths[0] + 3;
    for (std::size_t i = 0; i < result.columns.size(); ++i) {
        std::cout << std::left << std::setw(static_cast<int>(widths[i + 1] + 3)) << result.columns[i];
        totalWidth += widths[i + 1] + 3;
    }
    std::cout << '\n' << std::string(totalWidth, '-') << '\n';

    for (const auto& r : result.rows) {
        std::cout << std::left << std::setw(static_cast<int>(widths[0] + 3)) << r.id;
        for (std::size_t i = 0; i < result.columns.size(); ++i) {
            const std::string& val = (i < r.values.size()) ? r.values[i] : "";
            std::cout << std::left << std::setw(static_cast<int>(widths[i + 1] + 3)) << val;
        }
        std::cout << '\n';
    }
    std::cout << "(" << result.rows.size() << " row" << (result.rows.size() == 1 ? "" : "s") << ")\n\n";
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

std::vector<std::string> splitCsv(std::string_view line) {
    std::vector<std::string> items;
    std::stringstream ss{std::string(line)};
    std::string item;
    while (std::getline(ss, item, ',')) {
        items.emplace_back(trim(item));
    }
    return items;
}

std::string prompt(std::string_view label) {
    std::cout << label;
    std::string value;
    if (!std::getline(std::cin, value)) return "";
    return std::string(trim(value));
}

void createTableInteractive(Database& db, std::string_view args) {
    std::istringstream iss{std::string(args)};
    std::string tableName;
    iss >> tableName;

    std::string colStr;
    std::getline(iss, colStr);
    colStr = std::string(trim(colStr));

    if (tableName.empty()) {
        tableName = prompt("Table name: ");
    }
    if (colStr.empty()) {
        colStr = prompt("Columns (comma-separated, e.g. name,email,role): ");
    }

    const auto cols = splitCsv(colStr);
    if (db.createTable(tableName, cols)) {
        std::cout << "Created table '" << tableName << "'.\n";
    } else {
        std::cout << "Could not create table (check name or duplicate table).\n";
    }
}

void insertRowInteractive(Database& db, std::string_view args) {
    std::istringstream iss{std::string(args)};
    std::string tableName;
    iss >> tableName;

    std::string valStr;
    std::getline(iss, valStr);
    valStr = std::string(trim(valStr));

    if (tableName.empty()) {
        tableName = prompt("Table name: ");
    }

    const Table* table = db.findTable(tableName);
    if (!table) {
        std::cout << "Table '" << tableName << "' not found.\n";
        return;
    }

    std::vector<std::string> values;
    if (!valStr.empty()) {
        values = splitCsv(valStr);
    } else {
        for (const auto& col : table->columns) {
            values.push_back(prompt("  " + col + ": "));
            if (!std::cin) return;
        }
    }

    if (values.size() != table->columns.size()) {
        std::cout << "Expected " << table->columns.size() << " value(s), got " << values.size() << ".\n";
        return;
    }

    const int id = db.insertRow(tableName, values);
    std::cout << "Inserted row with id=" << id << " into '" << table->name << "'.\n";
}

void selectInteractive(const Database& db, std::string_view args) {
    std::istringstream iss{std::string(args)};
    std::string tableName;
    iss >> tableName;

    std::string filterExpr;
    std::getline(iss, filterExpr);
    filterExpr = std::string(trim(filterExpr));

    if (tableName.empty()) {
        tableName = prompt("Table name: ");
    }

    std::string whereCol;
    std::string whereVal;
    if (!filterExpr.empty()) {
        const auto eqPos = filterExpr.find('=');
        if (eqPos == std::string::npos) {
            std::cout << "Usage: select <table> [column=value]\n";
            return;
        }
        whereCol = std::string(trim(std::string_view(filterExpr).substr(0, eqPos)));
        whereVal = std::string(trim(std::string_view(filterExpr).substr(eqPos + 1)));
    }

    printQueryResult(db.selectRows(tableName, whereCol, whereVal));
}

void updateInteractive(Database& db, std::string_view args) {
    std::istringstream iss{std::string(args)};
    std::string tableName;
    std::string idToken;
    iss >> tableName >> idToken;

    std::string assignExpr;
    std::getline(iss, assignExpr);
    assignExpr = std::string(trim(assignExpr));

    if (tableName.empty()) tableName = prompt("Table name: ");
    if (idToken.empty()) idToken = prompt("Row ID: ");
    if (assignExpr.empty()) assignExpr = prompt("Assignment (column=new_value): ");

    int id = 0;
    const auto eqPos = assignExpr.find('=');
    if (!parseId(idToken, id) || eqPos == std::string::npos) {
        std::cout << "Usage: update <table> <id> <column=value>\n";
        return;
    }

    const auto col = trim(std::string_view(assignExpr).substr(0, eqPos));
    const auto val = trim(std::string_view(assignExpr).substr(eqPos + 1));

    const auto updated = db.updateRows(tableName, id, col, val);
    if (updated > 0) {
        std::cout << "Updated " << updated << " row(s).\n";
    } else {
        std::cout << "No matching row or column found.\n";
    }
}

void deleteInteractive(Database& db, std::string_view args) {
    std::istringstream iss{std::string(args)};
    std::string tableName;
    std::string idToken;
    iss >> tableName >> idToken;

    if (tableName.empty()) tableName = prompt("Table name: ");
    if (idToken.empty()) idToken = prompt("Row ID to delete: ");

    int id = 0;
    if (!parseId(idToken, id)) {
        std::cout << "Usage: delete <table> <id>\n";
        return;
    }

    if (db.deleteRowById(tableName, id)) {
        std::cout << "Deleted row " << id << " from '" << tableName << "'.\n";
    } else {
        std::cout << "Row or table not found.\n";
    }
}

} // namespace minidb

