#pragma once

#include "grade/GradeBook.hpp"
#include "grade/Student.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace gradesys {

void printHelp();
void printStudent(const Student& student);
void listStudents(const std::vector<const Student*>& students);
void listStudents(const std::vector<Student>& students);
void printStats(const ClassStats& stats);

bool parseId(std::string_view text, int& id);
bool parseScore(std::string_view text, double& score);
std::string prompt(std::string_view label);

void addStudentInteractive(GradeBook& gradeBook);
void recordGradeInteractive(GradeBook& gradeBook, int studentId);
void removeGradeInteractive(GradeBook& gradeBook, int studentId);

} // namespace gradesys

