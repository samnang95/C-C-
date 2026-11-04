#include "ui/display.h"

#include <stdio.h>

void clear_screen(void) { printf("\033[2J\033[H"); fflush(stdout); }

void print_banner(void) {
    printf("===============================================================\n");
    printf("                  Contact Management System\n");
    printf("===============================================================\n");
    printf("Commands: 'c' / 'clear' for menu, 'q' / 'quit' to exit.\n\n");
}

void print_main_menu(size_t count) {
    printf("--- Main Menu (%zu %s loaded) ---\n", count, count == 1 ? "contact" : "contacts");
    printf("  [1] Add Contact\n  [2] List All Contacts\n  [3] Search Contact\n");
    printf("  [4] Update Contact\n  [5] Delete Contact\n  [6] Quit\n");
}

void print_contact_table_header(void) {
    printf("+--------+----------------------+------------------+------------------------+------------------------------+\n");
    printf("| %-6s | %-20s | %-16s | %-22s | %-28s |\n", "ID", "Name", "Phone", "Email", "Address");
    printf("+--------+----------------------+------------------+------------------------+------------------------------+\n");
}

void print_contact_row(const Contact *contact) {
    if (contact == NULL) return;
    printf("| %-6d | %-20.20s | %-16.16s | %-22.22s | %-28.28s |\n", contact->id,
           contact->name, contact->phone, contact->email, contact->address);
}

void print_contact_table_footer(size_t shown_count) {
    printf("+--------+----------------------+------------------+------------------------+------------------------------+\n");
    printf("Total: %zu %s\n\n", shown_count, shown_count == 1 ? "contact" : "contacts");
}

void print_contact_list(const ContactList *list) {
    if (list == NULL || list->size == 0) { printf("\nNo contact records found.\n\n"); return; }
    printf("\n");
    print_contact_table_header();
    for (size_t i = 0; i < list->size; ++i) print_contact_row(&list->items[i]);
    print_contact_table_footer(list->size);
}
