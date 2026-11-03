#include "student/student.h"

#include <ctype.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 4

static StudentStatus ensure_capacity(StudentList *list, size_t min_capacity) {
    if (list == NULL) {
        return STUDENT_ERR_NULL;
    }
    if (list->capacity >= min_capacity) {
        return STUDENT_OK;
    }

    size_t new_capacity = (list->capacity == 0) ? INITIAL_CAPACITY : list->capacity;
    while (new_capacity < min_capacity) {
        if (new_capacity > SIZE_MAX / 2) {
            return STUDENT_ERR_NOMEM;
        }
        new_capacity *= 2;
    }

    if (new_capacity > SIZE_MAX / sizeof(Student)) {
        return STUDENT_ERR_NOMEM;
    }

    /* Assign realloc result to a temporary pointer so original block is not leaked on failure */
    Student *new_items = realloc(list->items, new_capacity * sizeof(Student));
    if (new_items == NULL) {
        return STUDENT_ERR_NOMEM;
    }

    list->items = new_items;
    list->capacity = new_capacity;
    return STUDENT_OK;
}

static int find_index_by_id(const StudentList *list, int id, size_t *out_index) {
    if (list == NULL) {
        return 0;
    }

    for (size_t i = 0; i < list->count; i++) {
        if (list->items[i].id == id) {
            if (out_index != NULL) {
                *out_index = i;
            }
            return 1;
        }
    }
    return 0;
}

void student_list_init(StudentList *list) {
    if (list == NULL) {
        return;
    }
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

void student_list_free(StudentList *list) {
    if (list == NULL) {
        return;
    }
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

int student_is_valid(const Student *student) {
    if (student == NULL) {
        return 0;
    }
    if (student->id <= 0) {
        return 0;
    }
    if (student->name[0] == '\0' || student->major[0] == '\0') {
        return 0;
    }
    if (student->age < STUDENT_MIN_AGE || student->age > STUDENT_MAX_AGE) {
        return 0;
    }
    if (!isfinite(student->gpa) ||
        student->gpa < STUDENT_MIN_GPA || student->gpa > STUDENT_MAX_GPA) {
        return 0;
    }
    return 1;
}

int student_list_next_id(const StudentList *list) {
    int max_id = 1000;
    if (list != NULL) {
        for (size_t i = 0; i < list->count; i++) {
            if (list->items[i].id > max_id) {
                max_id = list->items[i].id;
            }
        }
    }
    return max_id + 1;
}

StudentStatus student_list_add(StudentList *list, const Student *student) {
    if (list == NULL || student == NULL) {
        return STUDENT_ERR_NULL;
    }
    if (!student_is_valid(student)) {
        return STUDENT_ERR_INVALID;
    }
    if (find_index_by_id(list, student->id, NULL)) {
        return STUDENT_ERR_DUPLICATE_ID;
    }

    StudentStatus status = ensure_capacity(list, list->count + 1);
    if (status != STUDENT_OK) {
        return status;
    }

    list->items[list->count] = *student;
    list->count++;
    return STUDENT_OK;
}

const Student *student_list_find_by_id(const StudentList *list, int id) {
    size_t idx = 0;
    if (!find_index_by_id(list, id, &idx)) {
        return NULL;
    }
    return &list->items[idx];
}

StudentStatus student_list_update(StudentList *list, int id, const Student *updated) {
    if (list == NULL || updated == NULL) {
        return STUDENT_ERR_NULL;
    }
    if (!student_is_valid(updated) || updated->id != id) {
        return STUDENT_ERR_INVALID;
    }

    size_t idx = 0;
    if (!find_index_by_id(list, id, &idx)) {
        return STUDENT_ERR_NOT_FOUND;
    }

    list->items[idx] = *updated;
    return STUDENT_OK;
}

StudentStatus student_list_remove_by_id(StudentList *list, int id) {
    if (list == NULL) {
        return STUDENT_ERR_NULL;
    }

    size_t idx = 0;
    if (!find_index_by_id(list, id, &idx)) {
        return STUDENT_ERR_NOT_FOUND;
    }

    size_t trailing = list->count - idx - 1;
    if (trailing > 0) {
        memmove(&list->items[idx],
                &list->items[idx + 1],
                trailing * sizeof(Student));
    }

    list->count--;
    return STUDENT_OK;
}

int student_name_matches(const Student *student, const char *query) {
    if (student == NULL || query == NULL || *query == '\0') {
        return 0;
    }

    size_t qlen = strlen(query);
    size_t nlen = strlen(student->name);
    if (qlen > nlen) {
        return 0;
    }

    for (size_t i = 0; i <= nlen - qlen; i++) {
        size_t j = 0;
        while (j < qlen &&
               tolower((unsigned char)student->name[i + j]) ==
               tolower((unsigned char)query[j])) {
            j++;
        }
        if (j == qlen) {
            return 1;
        }
    }
    return 0;
}

const char *student_status_message(StudentStatus status) {
    switch (status) {
        case STUDENT_OK:
            return "OK";
        case STUDENT_ERR_NULL:
            return "Error: internal null pointer.";
        case STUDENT_ERR_NOMEM:
            return "Error: out of memory.";
        case STUDENT_ERR_INVALID:
            return "Error: invalid student record.";
        case STUDENT_ERR_DUPLICATE_ID:
            return "Error: a student with that ID already exists.";
        case STUDENT_ERR_NOT_FOUND:
            return "Error: student ID not found.";
        case STUDENT_ERR_FILE_IO:
            return "Error: could not read or write student data file.";
        default:
            return "Error: unknown student error.";
    }
}

