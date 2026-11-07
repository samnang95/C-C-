#pragma once

#include "db/Table.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace minidb {

class Database {
public:
    explicit Database(std::string dbFile = "data/minidb.db");

    bool load();
    bool save() const;

    bool createTable(std::string_view name, const std::vector<std::string>& columns);
    bool dropTable(std::string_view name);
    Table* findTable(std::string_view name);
    const Table* findTable(std::string_view name) const;

    int insertRow(std::string_view tableName, const std::vector<std::string>& values);
    QueryResult selectRows(std::string_view tableName,
                           std::string_view whereCol = "",
                           std::string_view whereVal = "") const;
    std::size_t updateRows(std::string_view tableName,
                           int rowId,
                           std::string_view targetCol,
                           std::string_view newVal);
    bool deleteRowById(std::string_view tableName, int rowId);
    std::size_t deleteRowsWhere(std::string_view tableName,
                                std::string_view whereCol,
                                std::string_view whereVal);

    const std::vector<Table>& tables() const { return tables_; }
    const std::string& dbFile() const { return dbFile_; }

private:
    std::string dbFile_;
    std::vector<Table> tables_;
};

} // namespace minidb

