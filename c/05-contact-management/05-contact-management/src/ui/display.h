#ifndef UI_DISPLAY_H
#define UI_DISPLAY_H

#include "contact/contact.h"

void clear_screen(void);
void print_banner(void);
void print_main_menu(size_t contact_count);
void print_contact_table_header(void);
void print_contact_row(const Contact *contact);
void print_contact_table_footer(size_t shown_count);
void print_contact_list(const ContactList *list);

#endif /* UI_DISPLAY_H */
