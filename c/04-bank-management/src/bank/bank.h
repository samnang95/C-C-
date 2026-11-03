#ifndef BANK_H
#define BANK_H

#include <stddef.h>
#include <stdint.h>

#define BANK_OWNER_MAX 64
#define BANK_DEFAULT_DATA_PATH "data/accounts.dat"

typedef enum {
    ACCOUNT_CHECKING = 1,
    ACCOUNT_SAVINGS = 2
} AccountType;

typedef enum {
    BANK_OK = 0,
    BANK_ERR_NULL,
    BANK_ERR_NOMEM,
    BANK_ERR_INVALID,
    BANK_ERR_DUPLICATE_ID,
    BANK_ERR_NOT_FOUND,
    BANK_ERR_INSUFFICIENT_FUNDS,
    BANK_ERR_SAME_ACCOUNT,
    BANK_ERR_OVERFLOW,
    BANK_ERR_FILE_IO
} BankStatus;

typedef struct {
    int account_number;
    char owner_name[BANK_OWNER_MAX];
    AccountType type;
    int64_t balance_cents;
} BankAccount;

typedef struct {
    BankAccount *items;
    size_t count;
    size_t capacity;
} BankList;

/* Lifecycle & helpers (bank_account.c) */
void bank_list_init(BankList *list);
void bank_list_free(BankList *list);
int bank_account_is_valid(const BankAccount *account);
const char *bank_account_type_name(AccountType type);
int bank_list_next_account_number(const BankList *list);

/* Account CRUD & transactions (bank_account.c) */
BankStatus bank_list_create(BankList *list, const BankAccount *account);
const BankAccount *bank_list_find(const BankList *list, int account_number);
int bank_owner_matches(const BankAccount *account, const char *query);
BankStatus bank_deposit(BankList *list, int account_number, int64_t amount_cents);
BankStatus bank_withdraw(BankList *list, int account_number, int64_t amount_cents);
BankStatus bank_transfer(BankList *list, int from_account, int to_account, int64_t amount_cents);
BankStatus bank_list_close(BankList *list, int account_number);
const char *bank_status_message(BankStatus status);

/* Binary file persistence (bank_storage.c) */
BankStatus bank_storage_save(const BankList *list, const char *filepath);
BankStatus bank_storage_load(BankList *list, const char *filepath);

#endif /* BANK_H */

