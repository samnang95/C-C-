#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bank/bank.h"
#include "input/input.h"
#include "ui/display.h"

typedef enum {
    MENU_CREATE = 1,
    MENU_LIST = 2,
    MENU_SEARCH = 3,
    MENU_DEPOSIT = 4,
    MENU_WITHDRAW = 5,
    MENU_TRANSFER = 6,
    MENU_CLOSE = 7,
    MENU_QUIT = 8
} MenuOption;

static void save_changes(const BankList *list) {
    BankStatus status = bank_storage_save(list, BANK_DEFAULT_DATA_PATH);
    if (status != BANK_OK) {
        printf("Warning: %s\n", bank_status_message(status));
    }
}

static InputAction handle_create_account(BankList *list) {
    BankAccount acc;
    memset(&acc, 0, sizeof(acc));

    char prompt[128];
    snprintf(prompt, sizeof(prompt),
             "\nEnter account number (1-999999, e.g. %d): ",
             bank_list_next_account_number(list));

    InputAction action = read_int_in_range(prompt, 1, 999999, &acc.account_number);
    if (action != INPUT_OK) {
        return action;
    }

    if (bank_list_find(list, acc.account_number) != NULL) {
        printf("%s\n\n", bank_status_message(BANK_ERR_DUPLICATE_ID));
        return INPUT_OK;
    }

    action = read_nonempty_string("Enter owner full name: ",
                                  acc.owner_name, sizeof(acc.owner_name));
    if (action != INPUT_OK) {
        return action;
    }

    int type_choice = 0;
    action = read_int_in_range("Account type ([1] Checking, [2] Savings): ",
                               ACCOUNT_CHECKING, ACCOUNT_SAVINGS, &type_choice);
    if (action != INPUT_OK) {
        return action;
    }
    acc.type = (AccountType)type_choice;

    action = read_money_cents("Enter initial deposit ($0.00 or more): ",
                              0, &acc.balance_cents);
    if (action != INPUT_OK) {
        return action;
    }

    BankStatus status = bank_list_create(list, &acc);
    if (status != BANK_OK) {
        printf("%s\n\n", bank_status_message(status));
        return INPUT_OK;
    }

    save_changes(list);

    char money_buf[32];
    format_money(acc.balance_cents, money_buf, sizeof(money_buf));
    printf("Account #%d (%s, %s) created with balance %s.\n\n",
           acc.account_number, acc.owner_name,
           bank_account_type_name(acc.type), money_buf);
    return INPUT_OK;
}

static InputAction handle_search_account(const BankList *list) {
    if (list->count == 0) {
        printf("\nNo accounts to search.\n\n");
        return INPUT_OK;
    }

    char query[BANK_OWNER_MAX];
    InputAction action = read_nonempty_string("\nEnter account number or owner name to search: ",
                                              query, sizeof(query));
    if (action != INPUT_OK) {
        return action;
    }

    int target_num = 0;
    int has_num = parse_int(query, &target_num);
    size_t matches = 0;

    printf("\n");
    for (size_t i = 0; i < list->count; i++) {
        const BankAccount *acc = &list->items[i];
        if ((has_num && acc->account_number == target_num) ||
            bank_owner_matches(acc, query)) {
            if (matches == 0) {
                print_account_table_header();
            }
            print_account_row(acc);
            matches++;
        }
    }

    if (matches == 0) {
        printf("No accounts matched \"%s\".\n\n", query);
    } else {
        print_account_table_footer(matches);
    }
    return INPUT_OK;
}

static InputAction handle_deposit(BankList *list) {
    if (list->count == 0) {
        printf("\nNo accounts available.\n\n");
        return INPUT_OK;
    }

    int acct_num = 0;
    InputAction action = read_int_in_range("\nEnter account number for deposit: ",
                                           1, 999999, &acct_num);
    if (action != INPUT_OK) {
        return action;
    }

    if (bank_list_find(list, acct_num) == NULL) {
        printf("%s\n\n", bank_status_message(BANK_ERR_NOT_FOUND));
        return INPUT_OK;
    }

    int64_t amount_cents = 0;
    action = read_money_cents("Enter deposit amount ($0.01 or more): ", 1, &amount_cents);
    if (action != INPUT_OK) {
        return action;
    }

    BankStatus status = bank_deposit(list, acct_num, amount_cents);
    if (status != BANK_OK) {
        printf("%s\n\n", bank_status_message(status));
        return INPUT_OK;
    }

    save_changes(list);

    const BankAccount *updated = bank_list_find(list, acct_num);
    char money_buf[32];
    format_money(updated->balance_cents, money_buf, sizeof(money_buf));
    printf("Deposit successful. New balance for #%d: %s\n\n", acct_num, money_buf);
    return INPUT_OK;
}

static InputAction handle_withdraw(BankList *list) {
    if (list->count == 0) {
        printf("\nNo accounts available.\n\n");
        return INPUT_OK;
    }

    int acct_num = 0;
    InputAction action = read_int_in_range("\nEnter account number for withdrawal: ",
                                           1, 999999, &acct_num);
    if (action != INPUT_OK) {
        return action;
    }

    if (bank_list_find(list, acct_num) == NULL) {
        printf("%s\n\n", bank_status_message(BANK_ERR_NOT_FOUND));
        return INPUT_OK;
    }

    int64_t amount_cents = 0;
    action = read_money_cents("Enter withdrawal amount ($0.01 or more): ", 1, &amount_cents);
    if (action != INPUT_OK) {
        return action;
    }

    BankStatus status = bank_withdraw(list, acct_num, amount_cents);
    if (status != BANK_OK) {
        printf("%s\n\n", bank_status_message(status));
        return INPUT_OK;
    }

    save_changes(list);

    const BankAccount *updated = bank_list_find(list, acct_num);
    char money_buf[32];
    format_money(updated->balance_cents, money_buf, sizeof(money_buf));
    printf("Withdrawal successful. New balance for #%d: %s\n\n", acct_num, money_buf);
    return INPUT_OK;
}

static InputAction handle_transfer(BankList *list) {
    if (list->count < 2) {
        printf("\nAt least 2 accounts are required for a transfer.\n\n");
        return INPUT_OK;
    }

    int from_acct = 0;
    InputAction action = read_int_in_range("\nEnter source account number: ",
                                           1, 999999, &from_acct);
    if (action != INPUT_OK) {
        return action;
    }
    if (bank_list_find(list, from_acct) == NULL) {
        printf("%s\n\n", bank_status_message(BANK_ERR_NOT_FOUND));
        return INPUT_OK;
    }

    int to_acct = 0;
    action = read_int_in_range("Enter destination account number: ",
                               1, 999999, &to_acct);
    if (action != INPUT_OK) {
        return action;
    }
    if (bank_list_find(list, to_acct) == NULL) {
        printf("%s\n\n", bank_status_message(BANK_ERR_NOT_FOUND));
        return INPUT_OK;
    }

    int64_t amount_cents = 0;
    action = read_money_cents("Enter transfer amount ($0.01 or more): ", 1, &amount_cents);
    if (action != INPUT_OK) {
        return action;
    }

    BankStatus status = bank_transfer(list, from_acct, to_acct, amount_cents);
    if (status != BANK_OK) {
        printf("%s\n\n", bank_status_message(status));
        return INPUT_OK;
    }

    save_changes(list);

    char amount_buf[32];
    format_money(amount_cents, amount_buf, sizeof(amount_buf));
    printf("Transferred %s from #%d to #%d.\n\n", amount_buf, from_acct, to_acct);
    return INPUT_OK;
}

static InputAction handle_close_account(BankList *list) {
    if (list->count == 0) {
        printf("\nNo accounts to close.\n\n");
        return INPUT_OK;
    }

    int acct_num = 0;
    InputAction action = read_int_in_range("\nEnter account number to close: ",
                                           1, 999999, &acct_num);
    if (action != INPUT_OK) {
        return action;
    }

    BankStatus status = bank_list_close(list, acct_num);
    if (status != BANK_OK) {
        printf("%s\n\n", bank_status_message(status));
        return INPUT_OK;
    }

    save_changes(list);
    printf("Account #%d closed and saved.\n\n", acct_num);
    return INPUT_OK;
}

int main(void) {
    BankList list;
    bank_list_init(&list);
    (void)bank_storage_load(&list, BANK_DEFAULT_DATA_PATH);

    print_banner();

    while (1) {
        print_main_menu(list.count);

        int choice = 0;
        InputAction action = read_int_in_range("Select option (1-8): ",
                                               MENU_CREATE, MENU_QUIT, &choice);
        if (action == INPUT_RESET) {
            clear_screen();
            print_banner();
            continue;
        }
        if (action != INPUT_OK || choice == MENU_QUIT) {
            break;
        }

        switch ((MenuOption)choice) {
            case MENU_CREATE:
                action = handle_create_account(&list);
                break;
            case MENU_LIST:
                print_account_list(&list);
                break;
            case MENU_SEARCH:
                action = handle_search_account(&list);
                break;
            case MENU_DEPOSIT:
                action = handle_deposit(&list);
                break;
            case MENU_WITHDRAW:
                action = handle_withdraw(&list);
                break;
            case MENU_TRANSFER:
                action = handle_transfer(&list);
                break;
            case MENU_CLOSE:
                action = handle_close_account(&list);
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

    bank_list_free(&list);
    printf("Goodbye!\n");
    return EXIT_SUCCESS;
}

