#include "db/Database.hpp"

#include <algorithm>
#include <charconv>
#include <fstream>

namespace minidb {

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

bool parseInt(std::string_view text, int& out) {
    auto res = std::from_chars(text.data(), text.data() + text.size(), out);
    return res.ec == std::errc() && res.ptr == text.data() + text.size();
}

} // namespace

Database::Database(std::string dbFile)
    : dbFile_(std::move(dbFile)) {}

bool Database::load() {
    std::ifstream input(dbFile_);
    if (!input) return false;

    tables_.clear();
    std::string line;
    while (std::getline(input, line)) {
        if (line.empty()) continue;
        const auto fields = splitFields(line);
        if (fields.size() < 3) continue;

        if (fields[0] == "TABLE") {
            const std::string& tName = fields[1];
            int nextId = 1;
            if (!parseInt(fields[2], nextId) || nextId < 1) nextId = 1;

            std::vector<std::string> cols;
            for (std::size_t i = 3; i < fields.size(); ++i) {
                if (!fields[i].empty()) cols.push_back(fields[i]);
            }
            if (tName.empty() || cols.empty()) continue;

            Table t;
            t.name = tName;
            t.columns = std::move(cols);
            t.nextId = nextId;
            tables_.push_back(std::move(t));
        } else if (fields[0] == "ROW") {
            Table* t = findTable(fields[1]);
            if (!t) continue;

            int rowId = 0;
            if (!parseInt(fields[2], rowId) || rowId < 1) continue;

            std::vector<std::string> vals;
            for (std::size_t i = 3; i < fields.size(); ++i) {
                vals.push_back(fields[i]);
            }
            vals.resize(t->columns.size());

            t->rows.push_back(Row{rowId, std::move(vals)});
            t->nextId = std::max(t->nextId, rowId + 1);
        }
    }
    return true;
}

bool Database::save() const {
    std::ofstream output(dbFile_, std::ios::trunc);
    if (!output) return false;

    for (const auto& t : tables_) {
        output << "TABLE\t" << escapeField(t.name) << '\t' << t.nextId;
        for (const auto& col : t.columns) {
            output << '\t' << escapeField(col);
        }
        output << '\n';

        for (const auto& r : t.rows) {
            output << "ROW\t" << escapeField(t.name) << '\t' << r.id;
            for (const auto& val : r.values) {
                output << '\t' << escapeField(val);
            }
            output << '\n';
        }
    }
    return static_cast<bool>(output);
}

bool Database::createTable(std::string_view name, const std::vector<std::string>& columns) {
    const auto trimmedName = trim(name);
    if (trimmedName.empty() || columns.empty() || findTable(trimmedName) != nullptr) {
        return false;
    }

    std::vector<std::string> cleanCols;
    for (const auto& col : columns) {
        auto c = trim(col);
        if (c.empty() || lowercase(c) == "id") continue;
        cleanCols.emplace_back(c);
    }
    if (cleanCols.empty()) return false;

    Table table;
    table.name = std::string(trimmedName);
    table.columns = std::move(cleanCols);
    tables_.push_back(std::move(table));
    return true;
}

bool Database::dropTable(std::string_view name) {
    const auto target = lowercase(trim(name));
    const auto oldSize = tables_.size();
    tables_.erase(std::remove_if(tables_.begin(), tables_.end(),
                                 [&target](const Table& t) { return lowercase(t.name) == target; }),
                  tables_.end());
    return tables_.size() != oldSize;
}

Table* Database::findTable(std::string_view name) {
    const auto target = lowercase(trim(name));
    for (auto& t : tables_) {
        if (lowercase(t.name) == target) return &t;
    }
    return nullptr;
}

const Table* Database::findTable(std::string_view name) const {
    const auto target = lowercase(trim(name));
    for (const auto& t : tables_) {
        if (lowercase(t.name) == target) return &t;
    }
    return nullptr;
}

int Database::insertRow(std::string_view tableName, const std::vector<std::string>& values) {
    Table* t = findTable(tableName);
    if (!t || values.size() != t->columns.size()) return 0;

    std::vector<std::string> cleanValues;
    cleanValues.reserve(values.size());
    for (const auto& v : values) {
        cleanValues.emplace_back(trim(v));
    }

    const int id = t->nextId++;
    t->rows.push_back(Row{id, std::move(cleanValues)});
    return id;
}

QueryResult Database::selectRows(std::string_view tableName,
                                 std::string_view whereCol,
                                 std::string_view whereVal) const {
    QueryResult res;
    const Table* t = findTable(tableName);
    if (!t) {
        res.ok = false;
        res.message = "Table not found.";
        return res;
    }

    res.columns = t->columns;
    const auto filterCol = trim(whereCol);
    const auto filterVal = lowercase(trim(whereVal));

    if (filterCol.empty()) {
        res.rows = t->rows;
        return res;
    }

    if (lowercase(filterCol) == "id") {
        int targetId = 0;
        if (!parseInt(filterVal, targetId)) {
            return res;
        }
        for (const auto& r : t->rows) {
            if (r.id == targetId) res.rows.push_back(r);
        }
        return res;
    }

    const std::size_t colIdx = t->columnIndex(filterCol);
    if (colIdx >= t->columns.size()) {
        res.ok = false;
        res.message = "Unknown column '" + std::string(filterCol) + "'.";
        return res;
    }

    for (const auto& r : t->rows) {
        if (colIdx < r.values.size() && lowercase(r.values[colIdx]) == filterVal) {
            res.rows.push_back(r);
        }
    }
    return res;
}

std::size_t Database::updateRows(std::string_view tableName,
                                 int rowId,
                                 std::string_view targetCol,
                                 std::string_view newVal) {
    Table* t = findTable(tableName);
    if (!t) return 0;

    const std::size_t colIdx = t->columnIndex(targetCol);
    if (colIdx >= t->columns.size()) return 0;

    std::size_t updated = 0;
    for (auto& r : t->rows) {
        if (rowId == 0 || r.id == rowId) {
            r.values[colIdx] = std::string(trim(newVal));
            ++updated;
        }
    }
    return updated;
}

bool Database::deleteRowById(std::string_view tableName, int rowId) {
    Table* t = findTable(tableName);
    if (!t) return false;

    const auto oldSize = t->rows.size();
    t->rows.erase(std::remove_if(t->rows.begin(), t->rows.end(),
                                 [rowId](const Row& r) { return r.id == rowId; }),
                  t->rows.end());
    return t->rows.size() != oldSize;
}

std::size_t Database::deleteRowsWhere(std::string_view tableName,
                                      std::string_view whereCol,
                                      std::string_view whereVal) {
    Table* t = findTable(tableName);
    if (!t) return 0;

    const auto col = trim(whereCol);
    const auto val = lowercase(trim(whereVal));

    if (lowercase(col) == "id") {
        int id = 0;
        if (!parseInt(val, id)) return 0;
        return deleteRowById(tableName, id) ? 1 : 0;
    }

    const std::size_t colIdx = t->columnIndex(col);
    if (colIdx >= t->columns.size()) return 0;

    const auto oldSize = t->rows.size();
    t->rows.erase(std::remove_if(t->rows.begin(), t->rows.end(),
                                 [colIdx, &val](const Row& r) {
                                     return colIdx < r.values.size() && lowercase(r.values[colIdx]) == val;
                                 }),
                  t->rows.end());
    return oldSize - t->rows.size();
}

} // namespace minidb

