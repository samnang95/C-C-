#include "library/Book.hpp"
#include "library/Library.hpp"
#include "ui/ConsoleUI.hpp"

#include <cassert>
#include <cstdio>
#include <iostream>

int main() {
    using namespace libmgr;

    // 1. String helpers
    assert(trim("  \t The C++ Programming Language \n ") == "The C++ Programming Language");
    assert(trim("   ") == "");
    assert(lowercase("Effective C++") == "effective c++");
    assert(containsIgnoreCase("Bjarne Stroustrup", "stroustrup"));
    assert(!containsIgnoreCase("Bjarne Stroustrup", "meyers"));

    // 2. ID parsing
    int id = 0;
    assert(parseId("12", id) && id == 12);
    assert(!parseId("0", id));
    assert(!parseId("-5", id));
    assert(!parseId("12abc", id));
    assert(!parseId("", id));

    // 3. Library CRUD, search, and persistence
    const std::string testDb = "/tmp/test_library_mgmt.db";
    std::remove(testDb.c_str());

    {
        Library lib(testDb);
        Book b1;
        b1.title = "Effective\tModern C++";
        b1.author = "Scott Meyers";
        b1.isbn = "978-1491903995";
        b1.category = "Programming";

        const auto& added1 = lib.add(b1);
        assert(added1.id == 1);

        Book b2;
        b2.title = "Clean Code";
        b2.author = "Robert C. Martin";
        b2.isbn = "978-0132350884";
        b2.category = "Software Engineering";
        b2.borrowed = true;
        b2.borrower = "Alice";

        const auto& added2 = lib.add(b2);
        assert(added2.id == 2);

        assert(lib.books().size() == 2);
        assert(lib.find(1) != nullptr);
        assert(lib.find(99) == nullptr);

        auto results = lib.search("meyers");
        assert(results.size() == 1 && results[0]->id == 1);

        auto borrowerResults = lib.search("alice");
        assert(borrowerResults.size() == 1 && borrowerResults[0]->id == 2);

        assert(lib.save());
    }

    {
        Library loaded(testDb);
        assert(loaded.load());
        assert(loaded.books().size() == 2);

        const Book* b1 = loaded.find(1);
        assert(b1 != nullptr);
        assert(b1->title == "Effective\tModern C++");
        assert(b1->author == "Scott Meyers");
        assert(!b1->borrowed);

        const Book* b2 = loaded.find(2);
        assert(b2 != nullptr);
        assert(b2->borrowed);
        assert(b2->borrower == "Alice");

        assert(loaded.remove(1));
        assert(!loaded.remove(1));
        assert(loaded.books().size() == 1);
    }

    std::remove(testDb.c_str());
    std::cout << "All library management unit tests passed!\n";
    return 0;
}

