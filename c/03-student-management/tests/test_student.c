#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "input/input.h"
#include "student/student.h"

#define TEST_CSV_PATH "build/test_students.csv"
#define EPSILON 1e-6

static int approx_equal(double a, double b) {
    return fabs(a - b) < EPSILON;
}

static Student make_student(int id, const char *name, int age, const char *major, double gpa) {
    Student s;
    memset(&s, 0, sizeof(s));
    s.id = id;
    snprintf(s.name, sizeof(s.name), "%s", name);
    s.age = age;
    snprintf(s.major, sizeof(s.major), "%s", major);
    s.gpa = gpa;
    return s;
}

static void test_dynamic_array_crud_and_growth(void) {
    StudentList list;
    student_list_init(&list);
    assert(list.count == 0 && list.capacity == 0 && list.items == NULL);
    assert(student_list_next_id(&list) == 1001);

    /* Insert 6 students to trigger initial allocation (4) and realloc doubling (8) */
    for (int i = 1; i <= 6; i++) {
        char name[32];
        snprintf(name, sizeof(name), "Student %d", i);
        Student s = make_student(1000 + i, name, 18 + i, "CS", 3.0 + (i * 0.1));
        assert(student_list_add(&list, &s) == STUDENT_OK);
    }
    assert(list.count == 6);
    assert(list.capacity >= 8);
    assert(student_list_next_id(&list) == 1007);

    /* Reject duplicate ID */
    Student dup = make_student(1003, "Duplicate", 20, "Math", 3.5);
    assert(student_list_add(&list, &dup) == STUDENT_ERR_DUPLICATE_ID);
    assert(list.count == 6);

    /* Find by ID & case-insensitive name search */
    const Student *found = student_list_find_by_id(&list, 1003);
    assert(found != NULL && strcmp(found->name, "Student 3") == 0);
    assert(student_name_matches(found, "student 3") == 1);
    assert(student_name_matches(found, "DENT 3") == 1);
    assert(student_name_matches(found, "Physics") == 0);

    /* Update existing student */
    Student updated = make_student(1003, "Alice Nguyen", 21, "Software Eng", 3.95);
    assert(student_list_update(&list, 1003, &updated) == STUDENT_OK);
    found = student_list_find_by_id(&list, 1003);
    assert(found != NULL && strcmp(found->name, "Alice Nguyen") == 0);
    assert(approx_equal(found->gpa, 3.95));

    /* Delete middle and last elements and verify shifting */
    assert(student_list_remove_by_id(&list, 1003) == STUDENT_OK);
    assert(list.count == 5);
    assert(student_list_find_by_id(&list, 1003) == NULL);
    assert(student_list_find_by_id(&list, 1004) != NULL);

    assert(student_list_remove_by_id(&list, 9999) == STUDENT_ERR_NOT_FOUND);

    student_list_free(&list);
    assert(list.items == NULL && list.count == 0 && list.capacity == 0);
}

static void test_validation_and_null_guards(void) {
    StudentList list;
    student_list_init(&list);

    Student bad_id = make_student(0, "Bob", 20, "CS", 3.0);
    Student bad_age = make_student(1, "Bob", 150, "CS", 3.0);
    Student bad_gpa = make_student(1, "Bob", 20, "CS", 4.5);
    Student empty_name = make_student(1, "", 20, "CS", 3.0);

    assert(student_list_add(&list, &bad_id) == STUDENT_ERR_INVALID);
    assert(student_list_add(&list, &bad_age) == STUDENT_ERR_INVALID);
    assert(student_list_add(&list, &bad_gpa) == STUDENT_ERR_INVALID);
    assert(student_list_add(&list, &empty_name) == STUDENT_ERR_INVALID);

    assert(student_list_add(NULL, &bad_id) == STUDENT_ERR_NULL);
    assert(student_list_add(&list, NULL) == STUDENT_ERR_NULL);
    assert(student_list_update(NULL, 1, &bad_id) == STUDENT_ERR_NULL);
    assert(student_list_remove_by_id(NULL, 1) == STUDENT_ERR_NULL);

    student_list_free(&list);
}

static void test_csv_save_and_load(void) {
    StudentList out_list;
    student_list_init(&out_list);

    Student s1 = make_student(1001, "Alice Smith", 20, "Computer Science", 3.85);
    Student s2 = make_student(1002, "Bob Tran", 22, "Mathematics", 3.60);
    assert(student_list_add(&out_list, &s1) == STUDENT_OK);
    assert(student_list_add(&out_list, &s2) == STUDENT_OK);

    assert(student_storage_save(&out_list, TEST_CSV_PATH) == STUDENT_OK);
    student_list_free(&out_list);

    StudentList in_list;
    student_list_init(&in_list);
    assert(student_storage_load(&in_list, TEST_CSV_PATH) == STUDENT_OK);
    assert(in_list.count == 2);

    const Student *loaded = student_list_find_by_id(&in_list, 1001);
    assert(loaded != NULL);
    assert(strcmp(loaded->name, "Alice Smith") == 0);
    assert(loaded->age == 20);
    assert(strcmp(loaded->major, "Computer Science") == 0);
    assert(approx_equal(loaded->gpa, 3.85));

    student_list_free(&in_list);
    remove(TEST_CSV_PATH);
}

static void test_input_parsers(void) {
    int ival = 0;
    double dval = 0.0;

    assert(parse_int("1001", &ival) == 1 && ival == 1001);
    assert(parse_int("12abc", &ival) == 0);
    assert(parse_double("3.75", &dval) == 1 && approx_equal(dval, 3.75));
    assert(parse_double("nan", &dval) == 0);
    assert(parse_double("inf", &dval) == 0);
}

int main(void) {
    test_dynamic_array_crud_and_growth();
    test_validation_and_null_guards();
    test_csv_save_and_load();
    test_input_parsers();

    printf("All student-management unit tests passed!\n");
    return 0;
}

