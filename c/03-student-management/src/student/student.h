#ifndef STUDENT_H
#define STUDENT_H

#include <stddef.h>

#define STUDENT_NAME_MAX  64
#define STUDENT_MAJOR_MAX 48

#define STUDENT_MIN_AGE   1
#define STUDENT_MAX_AGE   120
#define STUDENT_MIN_GPA   0.0
#define STUDENT_MAX_GPA   4.0

#define STUDENT_DEFAULT_CSV_PATH "data/students.csv"

typedef enum {
    STUDENT_OK = 0,
    STUDENT_ERR_NULL,
    STUDENT_ERR_NOMEM,
    STUDENT_ERR_INVALID,
    STUDENT_ERR_DUPLICATE_ID,
    STUDENT_ERR_NOT_FOUND,
    STUDENT_ERR_FILE_IO
} StudentStatus;

typedef struct {
    int id;
    char name[STUDENT_NAME_MAX];
    int age;
    char major[STUDENT_MAJOR_MAX];
    double gpa;
} Student;

typedef struct {
    Student *items;
    size_t count;
    size_t capacity;
} StudentList;

/* Lifecycle & validation (student_list.c) */
void student_list_init(StudentList *list);
void student_list_free(StudentList *list);
int student_is_valid(const Student *student);
int student_list_next_id(const StudentList *list);

/* CRUD operations (student_list.c) */
StudentStatus student_list_add(StudentList *list, const Student *student);
const Student *student_list_find_by_id(const StudentList *list, int id);
StudentStatus student_list_update(StudentList *list, int id, const Student *updated);
StudentStatus student_list_remove_by_id(StudentList *list, int id);
int student_name_matches(const Student *student, const char *query);
const char *student_status_message(StudentStatus status);

/* CSV file persistence (student_storage.c) */
StudentStatus student_storage_save(const StudentList *list, const char *filepath);
StudentStatus student_storage_load(StudentList *list, const char *filepath);

#endif /* STUDENT_H */

