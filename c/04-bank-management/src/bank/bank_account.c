#include "bank/bank.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 4

static BankStatus ensure_capacity(BankList *list, size_t min_capacity) {
    if (list == NULL) {
        return BANK_ERR_NULL;
    }
    if (list->capacity >= min_capacity) {
        return BANK_OK;
    }

    size_t new_capacity = (list->capacity == 0) ? INITIAL_CAPACITY : list->capacity;
    while (new_capacity < min_capacity) {
        if (new_capacity > SIZE_MAX / 2) {
            return BANK_ERR_NOMEM;
        }
        new_capacity *= 2;
    }

    if (new_capacity > SIZE_MAX / sizeof(BankAccount)) {
        return BANK_ERR_NOMEM;
    }

    BankAccount *new_items = realloc(list->items, new_capacity * sizeof(BankAccount));
    if (new_items == NULL) {
        return BANK_ERR_NOMEM;
    }

    list->items = new_items;
    list->capacity = new_capacity;
    return BANK_OK;
}

static int find_index_by_number(const BankList *list, int account_number, size_t *out_index) {
    if (list == NULL) {
        return 0;
    }

    for (size_t i = 0; i < list->count; i++) {
        if (list->items[i].account_number == account_number) {
            if (out_index != NULL) {
                *out_index = i;
            }
            return 1;
        }
    }
    return 0;
}

void bank_list_init(BankList *list) {
    if (list == NULL) {
        return;
    }
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

void bank_list_free(BankList *list) {
    if (list == NULL) {
        return;
    }
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

int bank_account_is_valid(const BankAccount *account) {
    if (account == NULL) {
        return 0;
    }
    if (account->account_number <= 0) {
        return 0;
    }
    if (account->owner_name[0] == '\0') {
        return 0;
    }
    if (account->type != ACCOUNT_CHECKING && account->type != ACCOUNT_SAVINGS) {
        return 0;
    }
    if (account->balance_cents < 0) {
        return 0;
    }
    return 1;
}

const char *bank_account_type_name(AccountType type) {
    switch (type) {
        case ACCOUNT_CHECKING:
            return "Checking";
        case ACCOUNT_SAVINGS:
            return "Savings";
        default:
            return "Unknown";
    }
}

int bank_list_next_account_number(const BankList *list) {
    int max_num = 10000;
    if (list != NULL) {
        for (size_t i = 0; i < list->count; i++) {
            if (list->items[i].account_number > max_num) {
                max_num = list->items[i].account_number;
            }
        }
    }
    return max_num + 1;
}

BankStatus bank_list_create(BankList *list, const BankAccount *account) {
    if (list == NULL || account == NULL) {
        return BANK_ERR_NULL;
    }
    if (!bank_account_is_valid(account)) {
        return BANK_ERR_INVALID;
    }
    if (find_index_by_number(list, account->account_number, NULL)) {
        return BANK_ERR_DUPLICATE_ID;
    }

    BankStatus status = ensure_capacity(list, list->count + 1);
    if (status != BANK_OK) {
        return status;
    }

    list->items[list->count] = *account;
    list->count++;
    return BANK_OK;
}

const BankAccount *bank_list_find(const BankList *list, int account_number) {
    size_t idx = 0;
    if (!find_index_by_number(list, account_number, &idx)) {
        return NULL;
    }
    return &list->items[idx];
}

int bank_owner_matches(const BankAccount *account, const char *query) {
    if (account == NULL || query == NULL || *query == '\0') {
        return 0;
    }

    size_t qlen = strlen(query);
    size_t nlen = strlen(account->owner_name);
    if (qlen > nlen) {
        return 0;
    }

    for (size_t i = 0; i <= nlen - qlen; i++) {
        size_t j = 0;
        while (j < qlen &&
               tolower((unsigned char)account->owner_name[i + j]) ==
               tolower((unsigned char)query[j])) {
            j++;
        }
        if (j == qlen) {
            return 1;
        }
    }
    return 0;
}

BankStatus bank_deposit(BankList *list, int account_number, int64_t amount_cents) {
    if (list == NULL) {
        return BANK_ERR_NULL;
    }
    if (amount_cents <= 0) {
        return BANK_ERR_INVALID;
    }

    size_t idx = 0;
    if (!find_index_by_number(list, account_number, &idx)) {
        return BANK_ERR_NOT_FOUND;
    }

    if (list->items[idx].balance_cents > INT64_MAX - amount_cents) {
        return BANK_ERR_OVERFLOW;
    }

    list->items[idx].balance_cents += amount_cents;
    return BANK_OK;
}

BankStatus bank_withdraw(BankList *list, int account_number, int64_t amount_cents) {
    if (list == NULL) {
        return BANK_ERR_NULL;
    }
    if (amount_cents <= 0) {
        return BANK_ERR_INVALID;
    }

    size_t idx = 0;
    if (!find_index_by_number(list, account_number, &idx)) {
        return BANK_ERR_NOT_FOUND;
    }

    if (list->items[idx].balance_cents < amount_cents) {
        return BANK_ERR_INSUFFICIENT_FUNDS;
    }

    list->items[idx].balance_cents -= amount_cents;
    return BANK_OK;
}

BankStatus bank_transfer(BankList *list, int from_account, int to_account, int64_t amount_cents) {
    if (list == NULL) {
        return BANK_ERR_NULL;
    }
    if (amount_cents <= 0) {
        return BANK_ERR_INVALID;
    }
    if (from_account == to_account) {
        return BANK_ERR_SAME_ACCOUNT;
    }

    size_t from_idx = 0;
    size_t to_idx = 0;
    if (!find_index_by_number(list, from_account, &from_idx) ||
        !find_index_by_number(list, to_account, &to_idx)) {
        return BANK_ERR_NOT_FOUND;
    }

    /* Validate both balance and overflow BEFORE mutating either account */
    if (list->items[from_idx].balance_cents < amount_cents) {
        return BANK_ERR_INSUFFICIENT_FUNDS;
    }
    if (list->items[to_idx].balance_cents > INT64_MAX - amount_cents) {
        return BANK_ERR_OVERFLOW;
    }

    list->items[from_idx].balance_cents -= amount_cents;
    list->items[to_idx].balance_cents += amount_cents;
    return BANK_OK;
}

BankStatus bank_list_close(BankList *list, int account_number) {
    if (list == NULL) {
        return BANK_ERR_NULL;
    }

    size_t idx = 0;
    if (!find_index_by_number(list, account_number, &idx)) {
        return BANK_ERR_NOT_FOUND;
    }

    size_t trailing = list->count - idx - 1;
    if (trailing > 0) {
        memmove(&list->items[idx],
                &list->items[idx + 1],
                trailing * sizeof(BankAccount));
    }

    list->count--;
    return BANK_OK;
}

const char *bank_status_message(BankStatus status) {
    switch (status) {
        case BANK_OK:
            return "OK";
        case BANK_ERR_NULL:
            return "Error: internal null pointer.";
        case BANK_ERR_NOMEM:
            return "Error: out of memory.";
        case BANK_ERR_INVALID:
            return "Error: invalid account data or amount.";
        case BANK_ERR_DUPLICATE_ID:
            return "Error: an account with that number already exists.";
        case BANK_ERR_NOT_FOUND:
            return "Error: account number not found.";
        case BANK_ERR_INSUFFICIENT_FUNDS:
            return "Error: insufficient funds for this transaction.";
        case BANK_ERR_SAME_ACCOUNT:
            return "Error: source and destination accounts must be different.";
        case BANK_ERR_OVERFLOW:
            return "Error: balance would exceed maximum representable amount.";
        case BANK_ERR_FILE_IO:
            return "Error: could not read or write bank data file.";
        default:
            return "Error: unknown banking error.";
    }
}

