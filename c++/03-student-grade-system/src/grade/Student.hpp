#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace gradesys {

struct CourseGrade {
    std::string course;
    double score{};
};

struct Student {
    int id{};
    std::string name;
    std::string department;
    std::vector<CourseGrade> grades;

    double average() const;
    double gpa() const;
    std::string letterGrade() const;
};

std::string_view trim(std::string_view text);
std::string lowercase(std::string_view text);
bool containsIgnoreCase(std::string_view value, std::string_view queryLower);
std::string scoreToLetter(double score);
double scoreToPoints(double score);

} // namespace gradesys

