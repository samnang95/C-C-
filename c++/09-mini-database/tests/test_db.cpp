#include "db/Database.hpp"
#include "db/Table.hpp"
#include "ui/ConsoleUI.hpp"

#include <cassert>
#include <cstdio>
#include <iostream>

int main() {
    using namespace minidb;

    // 1. Helpers
    assert(trim("  users \n") == "users");
    const auto cols = splitCsv("name, email , role");
    assert(cols.size() == 3 && cols[1] == "email");

    // 2. Database DDL/DML, filtering, updates, deletes, and persistence
    const std::string testDb = "/tmp/test_mini_database.db";
    std::remove(testDb.c_str());

    {
        Database db(testDb);
        assert(db.createTable("users", {"name", "email", "role"}));
        assert(!db.createTable("USERS", {"name"})); // duplicate table rejected

        assert(db.insertRow("users", {"Alice\tSmith", "alice@example.com", "admin"}) == 1);
        assert(db.insertRow("users", {"Bob Jones", "bob@example.com", "member"}) == 2);
        assert(db.insertRow("users", {"Charlie", "charlie@example.com", "member"}) == 3);

        const auto allRows = db.selectRows("users");
        assert(allRows.ok && allRows.rows.size() == 3);

        const auto members = db.selectRows("users", "role", "MEMBER");
        assert(members.ok && members.rows.size() == 2);

        const auto byId = db.selectRows("users", "id", "1");
        assert(byId.ok && byId.rows.size() == 1 && byId.rows[0].values[0] == "Alice\tSmith");

        assert(db.updateRows("users", 2, "role", "moderator") == 1);
        const auto updatedRow = db.selectRows("users", "id", "2");
        assert(updatedRow.rows[0].values[2] == "moderator");

        assert(db.deleteRowById("users", 3));
        assert(db.selectRows("users").rows.size() == 2);

        assert(db.save());
    }

    {
        Database loaded(testDb);
        assert(loaded.load());
        assert(loaded.tables().size() == 1);

        const auto rows = loaded.selectRows("users");
        assert(rows.ok && rows.rows.size() == 2);
        assert(rows.rows[0].values[0] == "Alice\tSmith");
        assert(rows.rows[1].values[2] == "moderator");

        assert(loaded.dropTable("users"));
        assert(loaded.tables().empty());
    }

    std::remove(testDb.c_str());
    std::cout << "All mini database unit tests passed!\n";
    return 0;
}

