#include "student/student.h"

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CSV_LINE_MAX 512

static void strip_newline(char *s) {
    if (s == NULL) {
        return;
    }
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')) {
        s[--len] = '\0';
    }
}

static int parse_csv_int(const char *str, int *out_val) {
    if (str == NULL || out_val == NULL || *str == '\0') {
        return 0;
    }
    errno = 0;
    char *end = NULL;
    long val = strtol(str, &end, 10);
    if (end == str || *end != '\0' || errno == ERANGE ||
        val < INT_MIN || val > INT_MAX) {
        return 0;
    }
    *out_val = (int)val;
    return 1;
}

static int parse_csv_double(const char *str, double *out_val) {
    if (str == NULL || out_val == NULL || *str == '\0') {
        return 0;
    }
    errno = 0;
    char *end = NULL;
    double val = strtod(str, &end);
    if (end == str || *end != '\0' || errno == ERANGE || !isfinite(val)) {
        return 0;
    }
    *out_val = val;
    return 1;
}

static int split_csv_fields(char *line, char *fields[], size_t expected_count) {
    if (line == NULL || fields == NULL || expected_count == 0) {
        return 0;
    }

    char *cursor = line;
    for (size_t i = 0; i < expected_count; i++) {
        fields[i] = cursor;
        char *comma = strchr(cursor, ',');
        if (i < expected_count - 1) {
            if (comma == NULL) {
                return 0;
            }
            *comma = '\0';
            cursor = comma + 1;
        } else {
            if (comma != NULL) {
                return 0;
            }
        }
    }
    return 1;
}

StudentStatus student_storage_save(const StudentList *list, const char *filepath) {
    if (list == NULL || filepath == NULL) {
        return STUDENT_ERR_NULL;
    }

    FILE *fp = fopen(filepath, "w");
    if (fp == NULL) {
        return STUDENT_ERR_FILE_IO;
    }

    if (fprintf(fp, "id,name,age,major,gpa\n") < 0) {
        fclose(fp);
        return STUDENT_ERR_FILE_IO;
    }

    for (size_t i = 0; i < list->count; i++) {
        const Student *s = &list->items[i];
        if (fprintf(fp, "%d,%s,%d,%s,%.2f\n",
                    s->id, s->name, s->age, s->major, s->gpa) < 0) {
            fclose(fp);
            return STUDENT_ERR_FILE_IO;
        }
    }

    if (fclose(fp) != 0) {
        return STUDENT_ERR_FILE_IO;
    }
    return STUDENT_OK;
}

StudentStatus student_storage_load(StudentList *list, const char *filepath) {
    if (list == NULL || filepath == NULL) {
        return STUDENT_ERR_NULL;
    }

    FILE *fp = fopen(filepath, "r");
    if (fp == NULL) {
        return STUDENT_ERR_FILE_IO;
    }

    char line[CSV_LINE_MAX];
    size_t line_num = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        line_num++;
        strip_newline(line);
        if (line[0] == '\0') {
            continue;
        }
        if (line_num == 1 && strcmp(line, "id,name,age,major,gpa") == 0) {
            continue;
        }

        char *fields[5];
        if (!split_csv_fields(line, fields, 5)) {
            continue;
        }

        Student s;
        memset(&s, 0, sizeof(s));

        if (!parse_csv_int(fields[0], &s.id) ||
            !parse_csv_int(fields[2], &s.age) ||
            !parse_csv_double(fields[4], &s.gpa)) {
            continue;
        }

        snprintf(s.name, sizeof(s.name), "%s", fields[1]);
        snprintf(s.major, sizeof(s.major), "%s", fields[3]);

        if (student_is_valid(&s)) {
            (void)student_list_add(list, &s);
        }
    }

    fclose(fp);
    return STUDENT_OK;
}

