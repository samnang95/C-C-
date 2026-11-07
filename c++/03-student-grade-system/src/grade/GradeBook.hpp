#pragma once

#include "grade/Student.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace gradesys {

struct ClassStats {
    std::size_t totalStudents{};
    std::size_t gradedStudents{};
    double classAverage{};
    double classGpa{};
    double highestAverage{};
    double lowestAverage{};
    std::string topStudentName;
};

class GradeBook {
public:
    explicit GradeBook(std::string dbFile = "data/students.db");

    bool load();
    bool save() const;

    Student& addStudent(Student student);
    Student* find(int id);
    const Student* find(int id) const;
    bool remove(int id);

    bool setGrade(int studentId, std::string_view course, double score);
    bool removeGrade(int studentId, std::string_view course);

    std::vector<const Student*> search(std::string_view query) const;
    std::vector<const Student*> rankedStudents() const;
    ClassStats computeStats() const;

    const std::vector<Student>& students() const { return students_; }
    const std::string& dbFile() const { return dbFile_; }

private:
    std::string dbFile_;
    std::vector<Student> students_;
    int nextId_{1};
};

} // namespace gradesys

