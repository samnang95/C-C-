#include "grade/Student.hpp"

#include <algorithm>
#include <cctype>

namespace gradesys {

std::string_view trim(std::string_view text) {
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos) return {};
    const auto last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

std::string lowercase(std::string_view text) {
    std::string result(text);
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

bool containsIgnoreCase(std::string_view value, std::string_view queryLower) {
    return lowercase(value).find(queryLower) != std::string::npos;
}

std::string scoreToLetter(double score) {
    if (score >= 90.0) return "A";
    if (score >= 80.0) return "B";
    if (score >= 70.0) return "C";
    if (score >= 60.0) return "D";
    return "F";
}

double scoreToPoints(double score) {
    if (score >= 90.0) return 4.0;
    if (score >= 80.0) return 3.0;
    if (score >= 70.0) return 2.0;
    if (score >= 60.0) return 1.0;
    return 0.0;
}

double Student::average() const {
    if (grades.empty()) return 0.0;
    double sum = 0.0;
    for (const auto& g : grades) {
        sum += g.score;
    }
    return sum / static_cast<double>(grades.size());
}

double Student::gpa() const {
    if (grades.empty()) return 0.0;
    double totalPoints = 0.0;
    for (const auto& g : grades) {
        totalPoints += scoreToPoints(g.score);
    }
    return totalPoints / static_cast<double>(grades.size());
}

std::string Student::letterGrade() const {
    if (grades.empty()) return "N/A";
    return scoreToLetter(average());
}

} // namespace gradesys

