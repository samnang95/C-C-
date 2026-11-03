#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "input/input.h"
#include "student/student.h"
#include "ui/display.h"

typedef enum {
    MENU_ADD = 1,
    MENU_LIST = 2,
    MENU_SEARCH = 3,
    MENU_UPDATE = 4,
    MENU_DELETE = 5,
    MENU_QUIT = 6
} MenuOption;

static void save_changes(const StudentList *list) {
    StudentStatus status = student_storage_save(list, STUDENT_DEFAULT_CSV_PATH);
    if (status != STUDENT_OK) {
        printf("Warning: %s\n", student_status_message(status));
    }
}

static InputAction read_student_fields(Student *out_student) {
    InputAction action = read_nonempty_string("Enter full name: ",
                                              out_student->name,
                                              sizeof(out_student->name));
    if (action != INPUT_OK) {
        return action;
    }

    action = read_int_in_range("Enter age (1-120): ",
                               STUDENT_MIN_AGE, STUDENT_MAX_AGE,
                               &out_student->age);
    if (action != INPUT_OK) {
        return action;
    }

    action = read_nonempty_string("Enter major: ",
                                  out_student->major,
                                  sizeof(out_student->major));
    if (action != INPUT_OK) {
        return action;
    }

    action = read_double_in_range("Enter GPA (0.00-4.00): ",
                                  STUDENT_MIN_GPA, STUDENT_MAX_GPA,
                                  &out_student->gpa);
    return action;
}

static InputAction handle_add_student(StudentList *list) {
    Student s;
    memset(&s, 0, sizeof(s));

    char prompt[128];
    snprintf(prompt, sizeof(prompt),
             "\nEnter student ID (1-999999, e.g. %d): ",
             student_list_next_id(list));

    InputAction action = read_int_in_range(prompt, 1, 999999, &s.id);
    if (action != INPUT_OK) {
        return action;
    }

    if (student_list_find_by_id(list, s.id) != NULL) {
        printf("%s\n\n", student_status_message(STUDENT_ERR_DUPLICATE_ID));
        return INPUT_OK;
    }

    action = read_student_fields(&s);
    if (action != INPUT_OK) {
        return action;
    }

    StudentStatus status = student_list_add(list, &s);
    if (status != STUDENT_OK) {
        printf("%s\n\n", student_status_message(status));
        return INPUT_OK;
    }

    save_changes(list);
    printf("Student #%d (%s) added and saved.\n\n", s.id, s.name);
    return INPUT_OK;
}

static InputAction handle_search_student(const StudentList *list) {
    if (list->count == 0) {
        printf("\nNo student records to search.\n\n");
        return INPUT_OK;
    }

    char query[STUDENT_NAME_MAX];
    InputAction action = read_nonempty_string("\nEnter student ID or name to search: ",
                                              query, sizeof(query));
    if (action != INPUT_OK) {
        return action;
    }

    int target_id = 0;
    int has_id = parse_int(query, &target_id);
    size_t matches = 0;

    printf("\n");
    for (size_t i = 0; i < list->count; i++) {
        const Student *s = &list->items[i];
        if ((has_id && s->id == target_id) || student_name_matches(s, query)) {
            if (matches == 0) {
                print_student_table_header();
            }
            print_student_row(s);
            matches++;
        }
    }

    if (matches == 0) {
        printf("No students matched \"%s\".\n\n", query);
    } else {
        print_student_table_footer(matches);
    }
    return INPUT_OK;
}

static InputAction handle_update_student(StudentList *list) {
    if (list->count == 0) {
        printf("\nNo student records to update.\n\n");
        return INPUT_OK;
    }

    int id = 0;
    InputAction action = read_int_in_range("\nEnter student ID to update: ",
                                           1, 999999, &id);
    if (action != INPUT_OK) {
        return action;
    }

    const Student *existing = student_list_find_by_id(list, id);
    if (existing == NULL) {
        printf("%s\n\n", student_status_message(STUDENT_ERR_NOT_FOUND));
        return INPUT_OK;
    }

    printf("\nCurrent record:\n");
    print_student_table_header();
    print_student_row(existing);
    print_student_table_footer(1);

    Student updated;
    memset(&updated, 0, sizeof(updated));
    updated.id = id;

    action = read_student_fields(&updated);
    if (action != INPUT_OK) {
        return action;
    }

    StudentStatus status = student_list_update(list, id, &updated);
    if (status != STUDENT_OK) {
        printf("%s\n\n", student_status_message(status));
        return INPUT_OK;
    }

    save_changes(list);
    printf("Student #%d updated and saved.\n\n", id);
    return INPUT_OK;
}

static InputAction handle_delete_student(StudentList *list) {
    if (list->count == 0) {
        printf("\nNo student records to delete.\n\n");
        return INPUT_OK;
    }

    int id = 0;
    InputAction action = read_int_in_range("\nEnter student ID to delete: ",
                                           1, 999999, &id);
    if (action != INPUT_OK) {
        return action;
    }

    StudentStatus status = student_list_remove_by_id(list, id);
    if (status != STUDENT_OK) {
        printf("%s\n\n", student_status_message(status));
        return INPUT_OK;
    }

    save_changes(list);
    printf("Student #%d deleted and saved.\n\n", id);
    return INPUT_OK;
}

int main(void) {
    StudentList list;
    student_list_init(&list);
    (void)student_storage_load(&list, STUDENT_DEFAULT_CSV_PATH);

    print_banner();

    while (1) {
        print_main_menu(list.count);

        int choice = 0;
        InputAction action = read_int_in_range("Select option (1-6): ",
                                               MENU_ADD, MENU_QUIT, &choice);
        if (action == INPUT_RESET) {
            clear_screen();
            print_banner();
            continue;
        }
        if (action != INPUT_OK || choice == MENU_QUIT) {
            break;
        }

        switch ((MenuOption)choice) {
            case MENU_ADD:
                action = handle_add_student(&list);
                break;
            case MENU_LIST:
                print_student_list(&list);
                break;
            case MENU_SEARCH:
                action = handle_search_student(&list);
                break;
            case MENU_UPDATE:
                action = handle_update_student(&list);
                break;
            case MENU_DELETE:
                action = handle_delete_student(&list);
                break;
            case MENU_QUIT:
            default:
                action = INPUT_QUIT;
                break;
        }

        if (action == INPUT_RESET) {
            clear_screen();
            print_banner();
            continue;
        }
        if (action == INPUT_QUIT || action == INPUT_ERR_NULL) {
            break;
        }
    }

    student_list_free(&list);
    printf("Goodbye!\n");
    return EXIT_SUCCESS;
}

