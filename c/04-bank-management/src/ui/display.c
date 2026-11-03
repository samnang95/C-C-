#include "ui/display.h"

#include <inttypes.h>
#include <stdio.h>

void format_money(int64_t cents, char *buf, size_t buf_size) {
    if (buf == NULL || buf_size == 0) {
        return;
    }
    if (cents < 0) {
        int64_t abs_cents = -cents;
        snprintf(buf, buf_size, "-$%" PRId64 ".%02" PRId64,
                 abs_cents / 100, abs_cents % 100);
    } else {
        snprintf(buf, buf_size, "$%" PRId64 ".%02" PRId64,
                 cents / 100, cents % 100);
    }
}

void clear_screen(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
}

void print_banner(void) {
    printf("============================================================\n");
    printf("                   Bank Management System\n");
    printf("============================================================\n");
    printf("Commands: 'c' / 'clear' to return to menu, 'q' / 'quit' to exit.\n\n");
}

void print_main_menu(size_t account_count) {
    printf("--- Main Menu (%zu %s loaded) ---\n",
           account_count, (account_count == 1) ? "account" : "accounts");
    printf("  [1] Create Account\n");
    printf("  [2] List All Accounts\n");
    printf("  [3] Search Account (by Number or Owner)\n");
    printf("  [4] Deposit\n");
    printf("  [5] Withdraw\n");
    printf("  [6] Transfer\n");
    printf("  [7] Close Account\n");
    printf("  [8] Quit\n");
}

void print_account_table_header(void) {
    printf("+----------+--------------------------+------------+----------------+\n");
    printf("| %-8s | %-24s | %-10s | %14s |\n",
           "Acct #", "Owner Name", "Type", "Balance");
    printf("+----------+--------------------------+------------+----------------+\n");
}

void print_account_row(const BankAccount *account) {
    if (account == NULL) {
        return;
    }

    char money_buf[32];
    format_money(account->balance_cents, money_buf, sizeof(money_buf));

    printf("| %-8d | %-24.24s | %-10s | %14s |\n",
           account->account_number,
           account->owner_name,
           bank_account_type_name(account->type),
           money_buf);
}

void print_account_table_footer(size_t shown_count) {
    printf("+----------+--------------------------+------------+----------------+\n");
    printf("Total: %zu %s\n\n",
           shown_count, (shown_count == 1) ? "account" : "accounts");
}

void print_account_list(const BankList *list) {
    if (list == NULL || list->count == 0) {
        printf("\nNo bank accounts found.\n\n");
        return;
    }

    printf("\n");
    print_account_table_header();
    for (size_t i = 0; i < list->count; i++) {
        print_account_row(&list->items[i]);
    }
    print_account_table_footer(list->count);
}

