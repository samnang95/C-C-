#ifndef UI_DISPLAY_H
#define UI_DISPLAY_H

#include <stddef.h>

#include "student/student.h"

void clear_screen(void);
void print_banner(void);
void print_main_menu(size_t student_count);
void print_student_table_header(void);
void print_student_row(const Student *student);
void print_student_table_footer(size_t shown_count);
void print_student_list(const StudentList *list);

#endif /* UI_DISPLAY_H */

