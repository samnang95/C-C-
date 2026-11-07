#include "ui/ConsoleUI.hpp"

#include <charconv>
#include <cstdlib>
#include <iomanip>
#include <iostream>

namespace gradesys {

void printHelp() {
    std::cout << "\nCommands:\n"
              << "  add                 Enroll a new student\n"
              << "  list [top]          List students (or rank by average)\n"
              << "  view <id>           Show full student transcript\n"
              << "  grade <id>          Add or update a course grade for a student\n"
              << "  ungrade <id>        Remove a course grade from a student\n"
              << "  search <text>       Search by name, department, or course\n"
              << "  stats               Show overall class grade statistics\n"
              << "  remove <id>         Remove a student record\n"
              << "  clear / reset       Clear the terminal screen\n"
              << "  help                Show this help\n"
              << "  quit                Save and exit\n\n";
}

void printStudent(const Student& student) {
    std::cout << "\n" << student.name << " (ID " << student.id << ")\n"
              << "Department: " << (student.department.empty() ? "undeclared" : student.department) << '\n';

    if (student.grades.empty()) {
        std::cout << "Grades: none recorded\n";
        return;
    }

    std::cout << std::fixed << std::setprecision(2)
              << "Average: " << student.average() << "% (" << student.letterGrade()
              << ", GPA " << student.gpa() << ")\n\n"
              << std::left << std::setw(30) << "Course" << std::setw(10) << "Score" << "Letter\n"
              << std::string(48, '-') << '\n';

    for (const auto& g : student.grades) {
        std::cout << std::left << std::setw(30) << g.course.substr(0, 29)
                  << std::setw(10) << g.score << scoreToLetter(g.score) << '\n';
    }
}

void listStudents(const std::vector<const Student*>& students) {
    if (students.empty()) {
        std::cout << "\nNo matching students.\n\n";
        return;
    }

    std::cout << "\n" << std::left << std::setw(5) << "ID" << std::setw(24) << "Name"
              << std::setw(18) << "Department" << std::setw(9) << "Courses"
              << std::setw(10) << "Average" << std::setw(6) << "GPA" << "Grade\n"
              << std::string(78, '-') << '\n';

    std::cout << std::fixed << std::setprecision(2);
    for (const auto* s : students) {
        if (!s) continue;
        std::cout << std::left << std::setw(5) << s->id
                  << std::setw(24) << s->name.substr(0, 23)
                  << std::setw(18) << (s->department.empty() ? "-" : s->department.substr(0, 17))
                  << std::setw(9) << s->grades.size();
        if (s->grades.empty()) {
            std::cout << std::setw(10) << "-" << std::setw(6) << "-" << "N/A\n";
        } else {
            std::cout << std::setw(10) << s->average()
                      << std::setw(6) << s->gpa()
                      << s->letterGrade() << '\n';
        }
    }
    std::cout << '\n';
}

void listStudents(const std::vector<Student>& students) {
    std::vector<const Student*> ptrs;
    ptrs.reserve(students.size());
    for (const auto& s : students) {
        ptrs.push_back(&s);
    }
    listStudents(ptrs);
}

void printStats(const ClassStats& stats) {
    std::cout << "\nClass Statistics:\n"
              << "  Total students:  " << stats.totalStudents << '\n'
              << "  Graded students: " << stats.gradedStudents << '\n';

    if (stats.gradedStudents == 0) {
        std::cout << "  No course grades recorded yet.\n\n";
        return;
    }

    std::cout << std::fixed << std::setprecision(2)
              << "  Class average:   " << stats.classAverage << "% ("
              << scoreToLetter(stats.classAverage) << ")\n"
              << "  Class GPA:       " << stats.classGpa << '\n'
              << "  Highest average: " << stats.highestAverage << "% (" << stats.topStudentName << ")\n"
              << "  Lowest average:  " << stats.lowestAverage << "%\n\n";
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

bool parseScore(std::string_view text, double& score) {
    std::string trimmed(trim(text));
    if (trimmed.empty()) return false;

    char* endPtr = nullptr;
    const double parsed = std::strtod(trimmed.c_str(), &endPtr);
    if (endPtr == trimmed.c_str() || *endPtr != '\0' || parsed < 0.0 || parsed > 100.0) {
        return false;
    }
    score = parsed;
    return true;
}

std::string prompt(std::string_view label) {
    std::cout << label;
    std::string value;
    if (!std::getline(std::cin, value)) {
        return "";
    }
    return std::string(trim(value));
}

void addStudentInteractive(GradeBook& gradeBook) {
    Student student;

    while (std::cin) {
        student.name = prompt("Student name: ");
        if (!std::cin) return;
        if (student.name.empty()) {
            std::cout << "A student name is required.\n";
            continue;
        }
        break;
    }

    student.department = prompt("Department / Major (optional): ");
    if (!std::cin) return;

    const auto& added = gradeBook.addStudent(std::move(student));
    std::cout << "Added student " << added.id << ".\n";
}

void recordGradeInteractive(GradeBook& gradeBook, int studentId) {
    Student* student = gradeBook.find(studentId);
    if (!student) {
        std::cout << "Student not found.\n";
        return;
    }

    std::string course;
    while (std::cin) {
        course = prompt("Course name: ");
        if (!std::cin) return;
        if (course.empty()) {
            std::cout << "A course name is required.\n";
            continue;
        }
        break;
    }

    double score = 0.0;
    while (std::cin) {
        const auto raw = prompt("Score (0 - 100): ");
        if (!std::cin) return;
        if (parseScore(raw, score)) {
            break;
        }
        std::cout << "Please enter a valid score between 0 and 100.\n";
    }

    gradeBook.setGrade(studentId, course, score);
    std::cout << "Recorded grade for " << student->name << " in " << course << ".\n";
}

void removeGradeInteractive(GradeBook& gradeBook, int studentId) {
    Student* student = gradeBook.find(studentId);
    if (!student) {
        std::cout << "Student not found.\n";
        return;
    }
    if (student->grades.empty()) {
        std::cout << "This student has no recorded grades.\n";
        return;
    }

    const auto course = prompt("Course name to remove: ");
    if (!std::cin || course.empty()) return;

    if (gradeBook.removeGrade(studentId, course)) {
        std::cout << "Course grade removed.\n";
    } else {
        std::cout << "Course '" << course << "' not found for this student.\n";
    }
}

} // namespace gradesys

