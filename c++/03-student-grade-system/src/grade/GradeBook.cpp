#include "grade/GradeBook.hpp"

#include <algorithm>
#include <charconv>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace gradesys {

namespace {

std::string escapeField(std::string_view value) {
    std::string result;
    for (const char c : value) {
        if (c == '\\') result += "\\\\";
        else if (c == '\t') result += "\\t";
        else if (c == '\n') result += "\\n";
        else if (c == ';') result += "\\;";
        else if (c == '=') result += "\\=";
        else result += c;
    }
    return result;
}

std::string unescapeField(std::string_view value) {
    std::string result;
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '\\' && i + 1 < value.size()) {
            const char next = value[++i];
            if (next == 't') result += '\t';
            else if (next == 'n') result += '\n';
            else result += next;
        } else {
            result += value[i];
        }
    }
    return result;
}

std::vector<std::string> splitEscaped(std::string_view text, char delimiter, bool unescapeItems) {
    std::vector<std::string> items;
    std::string current;
    bool escaped = false;
    for (const char c : text) {
        if (escaped) {
            current += '\\';
            current += c;
            escaped = false;
        } else if (c == '\\') {
            escaped = true;
        } else if (c == delimiter) {
            items.push_back(unescapeItems ? unescapeField(current) : current);
            current.clear();
        } else {
            current += c;
        }
    }
    if (escaped) current += '\\';
    items.push_back(unescapeItems ? unescapeField(current) : current);
    return items;
}

std::string serializeGrades(const std::vector<CourseGrade>& grades) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(2);
    for (std::size_t i = 0; i < grades.size(); ++i) {
        if (i > 0) out << ';';
        out << escapeField(grades[i].course) << '=' << grades[i].score;
    }
    return out.str();
}

std::vector<CourseGrade> deserializeGrades(std::string_view raw) {
    std::vector<CourseGrade> grades;
    if (raw.empty()) return grades;

    const auto entries = splitEscaped(raw, ';', false);
    for (const auto& entry : entries) {
        if (entry.empty()) continue;
        const auto kv = splitEscaped(entry, '=', true);
        if (kv.size() != 2 || kv[0].empty()) continue;

        char* endPtr = nullptr;
        const double score = std::strtod(kv[1].c_str(), &endPtr);
        if (endPtr == kv[1].c_str() || *endPtr != '\0' || score < 0.0 || score > 100.0) {
            continue;
        }
        grades.push_back(CourseGrade{kv[0], score});
    }
    return grades;
}

} // namespace

GradeBook::GradeBook(std::string dbFile)
    : dbFile_(std::move(dbFile)) {}

bool GradeBook::load() {
    std::ifstream input(dbFile_);
    if (!input) return false;

    std::string line;
    while (std::getline(input, line)) {
        const auto fields = splitEscaped(line, '\t', false);
        if (fields.size() != 4) continue;

        int id = 0;
        auto res = std::from_chars(fields[0].data(), fields[0].data() + fields[0].size(), id);
        if (res.ec != std::errc() || res.ptr != fields[0].data() + fields[0].size() || id < 1) {
            continue;
        }

        Student student;
        student.id = id;
        student.name = unescapeField(fields[1]);
        student.department = unescapeField(fields[2]);
        student.grades = deserializeGrades(fields[3]);

        if (student.name.empty()) continue;

        students_.push_back(std::move(student));
        nextId_ = std::max(nextId_, id + 1);
    }
    return true;
}

bool GradeBook::save() const {
    std::ofstream output(dbFile_, std::ios::trunc);
    if (!output) return false;

    for (const auto& student : students_) {
        output << student.id << '\t'
               << escapeField(student.name) << '\t'
               << escapeField(student.department) << '\t'
               << serializeGrades(student.grades) << '\n';
    }
    return static_cast<bool>(output);
}

Student& GradeBook::addStudent(Student student) {
    student.id = nextId_++;
    students_.push_back(std::move(student));
    return students_.back();
}

Student* GradeBook::find(int id) {
    for (auto& student : students_) {
        if (student.id == id) return &student;
    }
    return nullptr;
}

const Student* GradeBook::find(int id) const {
    for (const auto& student : students_) {
        if (student.id == id) return &student;
    }
    return nullptr;
}

bool GradeBook::remove(int id) {
    const auto oldSize = students_.size();
    students_.erase(std::remove_if(students_.begin(), students_.end(),
                                   [id](const Student& s) { return s.id == id; }),
                    students_.end());
    return students_.size() != oldSize;
}

bool GradeBook::setGrade(int studentId, std::string_view course, double score) {
    auto trimmedCourse = trim(course);
    if (trimmedCourse.empty() || score < 0.0 || score > 100.0) return false;

    Student* student = find(studentId);
    if (!student) return false;

    const auto targetLower = lowercase(trimmedCourse);
    for (auto& g : student->grades) {
        if (lowercase(g.course) == targetLower) {
            g.course = std::string(trimmedCourse);
            g.score = score;
            return true;
        }
    }

    student->grades.push_back(CourseGrade{std::string(trimmedCourse), score});
    return true;
}

bool GradeBook::removeGrade(int studentId, std::string_view course) {
    auto trimmedCourse = trim(course);
    if (trimmedCourse.empty()) return false;

    Student* student = find(studentId);
    if (!student) return false;

    const auto targetLower = lowercase(trimmedCourse);
    const auto oldSize = student->grades.size();
    student->grades.erase(
        std::remove_if(student->grades.begin(), student->grades.end(),
                       [&targetLower](const CourseGrade& g) { return lowercase(g.course) == targetLower; }),
        student->grades.end());
    return student->grades.size() != oldSize;
}

std::vector<const Student*> GradeBook::search(std::string_view query) const {
    const auto queryLower = lowercase(trim(query));
    std::vector<const Student*> matches;
    if (queryLower.empty()) return matches;

    for (const auto& student : students_) {
        bool matched = containsIgnoreCase(student.name, queryLower) ||
                       containsIgnoreCase(student.department, queryLower);
        if (!matched) {
            for (const auto& g : student.grades) {
                if (containsIgnoreCase(g.course, queryLower)) {
                    matched = true;
                    break;
                }
            }
        }
        if (matched) matches.push_back(&student);
    }
    return matches;
}

std::vector<const Student*> GradeBook::rankedStudents() const {
    std::vector<const Student*> ranked;
    ranked.reserve(students_.size());
    for (const auto& student : students_) {
        ranked.push_back(&student);
    }

    std::sort(ranked.begin(), ranked.end(), [](const Student* a, const Student* b) {
        const bool aHasGrades = !a->grades.empty();
        const bool bHasGrades = !b->grades.empty();
        if (aHasGrades != bHasGrades) return aHasGrades > bHasGrades;
        const double avgA = a->average();
        const double avgB = b->average();
        if (avgA != avgB) return avgA > avgB;
        return a->id < b->id;
    });
    return ranked;
}

ClassStats GradeBook::computeStats() const {
    ClassStats stats;
    stats.totalStudents = students_.size();

    double sumAvg = 0.0;
    double sumGpa = 0.0;

    for (const auto& student : students_) {
        if (student.grades.empty()) continue;
        const double avg = student.average();
        const double gpa = student.gpa();

        if (stats.gradedStudents == 0) {
            stats.highestAverage = avg;
            stats.lowestAverage = avg;
            stats.topStudentName = student.name;
        } else {
            if (avg > stats.highestAverage) {
                stats.highestAverage = avg;
                stats.topStudentName = student.name;
            }
            if (avg < stats.lowestAverage) {
                stats.lowestAverage = avg;
            }
        }

        sumAvg += avg;
        sumGpa += gpa;
        ++stats.gradedStudents;
    }

    if (stats.gradedStudents > 0) {
        stats.classAverage = sumAvg / static_cast<double>(stats.gradedStudents);
        stats.classGpa = sumGpa / static_cast<double>(stats.gradedStudents);
    }
    return stats;
}

} // namespace gradesys

