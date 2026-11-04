#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "contact/contact.h"
#include "input/input.h"
#include "ui/display.h"

static void save_changes(const ContactList *list) {
    ContactStatus status = contact_storage_save(list, CONTACT_DEFAULT_CSV_PATH);
    if (status != CONTACT_OK) printf("Warning: %s\n", contact_status_message(status));
}

static InputAction read_fields(Contact *contact) {
    InputAction action = read_nonempty_string("Enter full name: ", contact->name, sizeof(contact->name));
    if (action == INPUT_OK) action = read_nonempty_string("Enter phone: ", contact->phone, sizeof(contact->phone));
    if (action == INPUT_OK) action = read_nonempty_string("Enter email: ", contact->email, sizeof(contact->email));
    if (action == INPUT_OK) action = read_nonempty_string("Enter address: ", contact->address, sizeof(contact->address));
    return action;
}

static InputAction add_contact(ContactList *list) {
    Contact contact = {0};
    char prompt[96];
    snprintf(prompt, sizeof(prompt), "\nEnter contact ID (1-999999, e.g. %d): ", contact_list_next_id(list));
    InputAction action = read_int_in_range(prompt, 1, 999999, &contact.id);
    if (action != INPUT_OK) return action;
    if (contact_list_find_by_id(list, contact.id) != NULL) { printf("%s\n\n", contact_status_message(CONTACT_ERR_DUPLICATE_ID)); return INPUT_OK; }
    action = read_fields(&contact);
    if (action != INPUT_OK) return action;
    ContactStatus status = contact_list_add(list, &contact);
    if (status != CONTACT_OK) printf("%s\n\n", contact_status_message(status));
    else { save_changes(list); printf("Contact #%d (%s) added and saved.\n\n", contact.id, contact.name); }
    return INPUT_OK;
}

static InputAction search_contacts(const ContactList *list) {
    if (list->size == 0) { printf("\nNo contact records to search.\n\n"); return INPUT_OK; }
    char query[CONTACT_NAME_MAX];
    InputAction action = read_nonempty_string("\nEnter contact ID or name to search: ", query, sizeof(query));
    if (action != INPUT_OK) return action;
    int id = 0, has_id = parse_int(query, &id);
    size_t matches = 0;
    for (size_t i = 0; i < list->size; ++i) {
        const Contact *contact = &list->items[i];
        if ((has_id && contact->id == id) || contact_name_matches(contact, query)) {
            if (matches++ == 0) { printf("\n"); print_contact_table_header(); }
            print_contact_row(contact);
        }
    }
    if (matches == 0) printf("No contacts matched \"%s\".\n\n", query);
    else print_contact_table_footer(matches);
    return INPUT_OK;
}

static InputAction update_contact(ContactList *list) {
    int id;
    InputAction action = read_int_in_range("\nEnter contact ID to update: ", 1, 999999, &id);
    if (action != INPUT_OK) return action;
    const Contact *existing = contact_list_find_by_id(list, id);
    if (existing == NULL) { printf("%s\n\n", contact_status_message(CONTACT_ERR_NOT_FOUND)); return INPUT_OK; }
    printf("\nCurrent record:\n"); print_contact_table_header(); print_contact_row(existing); print_contact_table_footer(1);
    Contact updated = {0}; updated.id = id;
    action = read_fields(&updated);
    if (action != INPUT_OK) return action;
    ContactStatus status = contact_list_update(list, id, &updated);
    if (status != CONTACT_OK) printf("%s\n\n", contact_status_message(status));
    else { save_changes(list); printf("Contact #%d updated and saved.\n\n", id); }
    return INPUT_OK;
}

static InputAction delete_contact(ContactList *list) {
    int id;
    InputAction action = read_int_in_range("\nEnter contact ID to delete: ", 1, 999999, &id);
    if (action != INPUT_OK) return action;
    ContactStatus status = contact_list_remove_by_id(list, id);
    if (status != CONTACT_OK) printf("%s\n\n", contact_status_message(status));
    else { save_changes(list); printf("Contact #%d deleted and saved.\n\n", id); }
    return INPUT_OK;
}

int main(void) {
    ContactList list; contact_list_init(&list);
    (void)contact_storage_load(&list, CONTACT_DEFAULT_CSV_PATH);
    print_banner();
    for (;;) {
        print_main_menu(list.size);
        int choice;
        InputAction action = read_int_in_range("Select option (1-6): ", 1, 6, &choice);
        if (action == INPUT_RESET) { clear_screen(); print_banner(); continue; }
        if (action != INPUT_OK || choice == 6) break;
        if (choice == 1) action = add_contact(&list);
        else if (choice == 2) print_contact_list(&list);
        else if (choice == 3) action = search_contacts(&list);
        else if (choice == 4) action = list.size == 0 ? (printf("\nNo contact records to update.\n\n"), INPUT_OK) : update_contact(&list);
        else action = list.size == 0 ? (printf("\nNo contact records to delete.\n\n"), INPUT_OK) : delete_contact(&list);
        if (action == INPUT_RESET) { clear_screen(); print_banner(); }
        else if (action != INPUT_OK) break;
    }
    contact_list_free(&list);
    printf("Goodbye!\n");
    return EXIT_SUCCESS;
}
