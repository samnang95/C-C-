#include "ui/display.h"

#include <stdio.h>

void clear_screen(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
}

void print_banner(void) {
    printf("============================================================\n");
    printf("                 Student Management System\n");
    printf("============================================================\n");
    printf("Commands: 'c' / 'clear' to return to menu, 'q' / 'quit' to exit.\n\n");
}

void print_main_menu(size_t student_count) {
    printf("--- Main Menu (%zu %s loaded) ---\n",
           student_count, (student_count == 1) ? "student" : "students");
    printf("  [1] Add Student\n");
    printf("  [2] List All Students\n");
    printf("  [3] Search Student (by ID or Name)\n");
    printf("  [4] Update Student\n");
    printf("  [5] Delete Student\n");
    printf("  [6] Quit\n");
}

void print_student_table_header(void) {
    printf("+--------+------------------------+-----+----------------------+------+\n");
    printf("| %-6s | %-22s | %-3s | %-20s | %-4s |\n",
           "ID", "Name", "Age", "Major", "GPA");
    printf("+--------+------------------------+-----+----------------------+------+\n");
}

void print_student_row(const Student *student) {
    if (student == NULL) {
        return;
    }
    printf("| %-6d | %-22.22s | %3d | %-20.20s | %4.2f |\n",
           student->id,
           student->name,
           student->age,
           student->major,
           student->gpa);
}

void print_student_table_footer(size_t shown_count) {
    printf("+--------+------------------------+-----+----------------------+------+\n");
    printf("Total: %zu %s\n\n",
           shown_count, (shown_count == 1) ? "student" : "students");
}

void print_student_list(const StudentList *list) {
    if (list == NULL || list->count == 0) {
        printf("\nNo student records found.\n\n");
        return;
    }

    printf("\n");
    print_student_table_header();
    for (size_t i = 0; i < list->count; i++) {
        print_student_row(&list->items[i]);
    }
    print_student_table_footer(list->count);
}

