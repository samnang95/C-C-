#include "grade/GradeBook.hpp"
#include "grade/Student.hpp"
#include "ui/ConsoleUI.hpp"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <iostream>

int main() {
    using namespace gradesys;

    // 1. Helpers and letter/GPA conversion
    assert(trim("  Alice Smith \t\n") == "Alice Smith");
    assert(lowercase("Computer Science") == "computer science");
    assert(scoreToLetter(95.0) == "A");
    assert(scoreToLetter(85.0) == "B");
    assert(scoreToLetter(75.0) == "C");
    assert(scoreToLetter(65.0) == "D");
    assert(scoreToLetter(50.0) == "F");

    int id = 0;
    assert(parseId("7", id) && id == 7);
    assert(!parseId("0", id));
    assert(!parseId("-2", id));
    assert(!parseId("7a", id));

    double score = 0.0;
    assert(parseScore("92.5", score) && std::abs(score - 92.5) < 1e-6);
    assert(!parseScore("-1", score));
    assert(!parseScore("100.5", score));
    assert(!parseScore("abc", score));

    // 2. GradeBook CRUD, grades, ranking, stats, and persistence
    const std::string testDb = "/tmp/test_student_grade_system.db";
    std::remove(testDb.c_str());

    {
        GradeBook book(testDb);

        Student s1;
        s1.name = "Alice\tJohnson";
        s1.department = "Computer Science";
        const auto& added1 = book.addStudent(s1);
        assert(added1.id == 1);

        Student s2;
        s2.name = "Bob Martin";
        s2.department = "Mathematics";
        const auto& added2 = book.addStudent(s2);
        assert(added2.id == 2);

        assert(book.setGrade(1, "Algorithms=1", 95.0));
        assert(book.setGrade(1, "Operating;Systems", 85.0));
        // Update existing course grade case-insensitively
        assert(book.setGrade(1, "algorithms=1", 96.0));

        assert(book.setGrade(2, "Calculus", 78.0));

        const Student* p1 = book.find(1);
        assert(p1 != nullptr);
        assert(p1->grades.size() == 2);
        assert(std::abs(p1->average() - 90.5) < 1e-6);
        assert(std::abs(p1->gpa() - 3.5) < 1e-6);
        assert(p1->letterGrade() == "A");

        const auto ranked = book.rankedStudents();
        assert(ranked.size() == 2);
        assert(ranked[0]->id == 1);
        assert(ranked[1]->id == 2);

        const auto stats = book.computeStats();
        assert(stats.totalStudents == 2);
        assert(stats.gradedStudents == 2);
        assert(std::abs(stats.highestAverage - 90.5) < 1e-6);
        assert(std::abs(stats.lowestAverage - 78.0) < 1e-6);

        assert(book.save());
    }

    {
        GradeBook loaded(testDb);
        assert(loaded.load());
        assert(loaded.students().size() == 2);

        const Student* p1 = loaded.find(1);
        assert(p1 != nullptr);
        assert(p1->name == "Alice\tJohnson");
        assert(p1->grades.size() == 2);
        assert(p1->grades[0].course == "algorithms=1");
        assert(std::abs(p1->grades[0].score - 96.0) < 1e-6);
        assert(p1->grades[1].course == "Operating;Systems");

        assert(loaded.removeGrade(1, "Operating;Systems"));
        assert(p1->grades.size() == 1);

        const auto matches = loaded.search("calculus");
        assert(matches.size() == 1 && matches[0]->id == 2);

        assert(loaded.remove(2));
        assert(!loaded.remove(2));
        assert(loaded.students().size() == 1);
    }

    std::remove(testDb.c_str());
    std::cout << "All student grade system unit tests passed!\n";
    return 0;
}

