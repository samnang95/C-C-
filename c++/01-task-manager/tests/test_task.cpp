#include "task/Task.hpp"
#include "task/TaskManager.hpp"

#include <cassert>
#include <cstdio>
#include <iostream>

void testTaskValidation() {
    using namespace taskmgr;
    assert(isValidDate("2026-11-07"));
    assert(isValidDate("2024-02-29")); // leap year
    assert(!isValidDate("2023-02-29")); // non-leap year
    assert(!isValidDate("invalid-date"));
    assert(isValidDate("")); // optional date is valid

    assert(parsePriority("low") == Priority::Low);
    assert(parsePriority("HIGH") == Priority::High);
    assert(parsePriority("other") == Priority::Medium);
}

void testTaskManagerCrud() {
    using namespace taskmgr;
    const std::string testDb = "build/test_tasks.db";
    std::remove(testDb.c_str());

    {
        TaskManager manager(testDb);
        Task t1;
        t1.title = "Buy milk";
        t1.description = "Whole milk";
        t1.priority = Priority::High;
        t1.dueDate = "2026-11-10";
        t1.tags = {"groceries", "home"};
        manager.add(std::move(t1));

        Task t2;
        t2.title = "Study C++";
        t2.priority = Priority::Medium;
        manager.add(std::move(t2));

        assert(manager.tasks().size() == 2);
        assert(manager.find(1) != nullptr);
        assert(manager.find(1)->title == "Buy milk");
        assert(manager.find(2) != nullptr);

        assert(manager.save());
    }

    {
        TaskManager manager(testDb);
        assert(manager.load());
        assert(manager.tasks().size() == 2);
        auto* loaded = manager.find(1);
        assert(loaded != nullptr);
        assert(loaded->title == "Buy milk");
        assert(loaded->tags.size() == 2);
        assert(loaded->tags[0] == "groceries");

        assert(manager.remove(1));
        assert(manager.tasks().size() == 1);
        assert(manager.find(1) == nullptr);
    }

    std::remove(testDb.c_str());
}

int main() {
    testTaskValidation();
    testTaskManagerCrud();
    std::cout << "All task manager unit tests passed!\n";
    return 0;
}

