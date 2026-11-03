#ifndef UI_DISPLAY_H
#define UI_DISPLAY_H

#include <stddef.h>
#include <stdint.h>

#include "bank/bank.h"

void format_money(int64_t cents, char *buf, size_t buf_size);
void clear_screen(void);
void print_banner(void);
void print_main_menu(size_t account_count);
void print_account_table_header(void);
void print_account_row(const BankAccount *account);
void print_account_table_footer(size_t shown_count);
void print_account_list(const BankList *list);

#endif /* UI_DISPLAY_H */

