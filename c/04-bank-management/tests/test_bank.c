#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bank/bank.h"
#include "input/input.h"
#include "ui/display.h"

#define TEST_DATA_PATH "build/test_accounts.dat"

static BankAccount make_account(int num, const char *owner, AccountType type, int64_t cents) {
    BankAccount acc;
    memset(&acc, 0, sizeof(acc));
    acc.account_number = num;
    snprintf(acc.owner_name, sizeof(acc.owner_name), "%s", owner);
    acc.type = type;
    acc.balance_cents = cents;
    return acc;
}

static void test_account_crud_and_transactions(void) {
    BankList list;
    bank_list_init(&list);
    assert(bank_list_next_account_number(&list) == 10001);

    BankAccount a1 = make_account(10001, "Alice Smith", ACCOUNT_CHECKING, 50000); /* $500.00 */
    BankAccount a2 = make_account(10002, "Bob Tran", ACCOUNT_SAVINGS, 20000);     /* $200.00 */

    assert(bank_list_create(&list, &a1) == BANK_OK);
    assert(bank_list_create(&list, &a2) == BANK_OK);
    assert(bank_list_create(&list, &a1) == BANK_ERR_DUPLICATE_ID);
    assert(list.count == 2);

    /* Deposit */
    assert(bank_deposit(&list, 10001, 12550) == BANK_OK); /* +$125.50 -> $625.50 */
    assert(bank_list_find(&list, 10001)->balance_cents == 62550);
    assert(bank_deposit(&list, 10001, 0) == BANK_ERR_INVALID);
    assert(bank_deposit(&list, 99999, 100) == BANK_ERR_NOT_FOUND);

    /* Withdraw & insufficient funds */
    assert(bank_withdraw(&list, 10001, 2550) == BANK_OK); /* -$25.50 -> $600.00 */
    assert(bank_list_find(&list, 10001)->balance_cents == 60000);
    assert(bank_withdraw(&list, 10001, 90000) == BANK_ERR_INSUFFICIENT_FUNDS);
    assert(bank_list_find(&list, 10001)->balance_cents == 60000);

    /* Atomic transfer */
    assert(bank_transfer(&list, 10001, 10002, 15025) == BANK_OK); /* $150.25 */
    assert(bank_list_find(&list, 10001)->balance_cents == 44975);
    assert(bank_list_find(&list, 10002)->balance_cents == 35025);

    /* Reject self-transfer and insufficient transfer without mutating either account */
    assert(bank_transfer(&list, 10001, 10001, 100) == BANK_ERR_SAME_ACCOUNT);
    assert(bank_transfer(&list, 10001, 10002, 999999) == BANK_ERR_INSUFFICIENT_FUNDS);
    assert(bank_list_find(&list, 10001)->balance_cents == 44975);
    assert(bank_list_find(&list, 10002)->balance_cents == 35025);

    /* Close account */
    assert(bank_list_close(&list, 10001) == BANK_OK);
    assert(list.count == 1);
    assert(bank_list_find(&list, 10001) == NULL);
    assert(bank_list_find(&list, 10002) != NULL);

    bank_list_free(&list);
}

static void test_binary_storage_round_trip(void) {
    BankList out_list;
    bank_list_init(&out_list);

    BankAccount a1 = make_account(10001, "Alice Smith", ACCOUNT_CHECKING, 123456);
    BankAccount a2 = make_account(10002, "Bob Tran", ACCOUNT_SAVINGS, 78900);
    assert(bank_list_create(&out_list, &a1) == BANK_OK);
    assert(bank_list_create(&out_list, &a2) == BANK_OK);

    assert(bank_storage_save(&out_list, TEST_DATA_PATH) == BANK_OK);
    bank_list_free(&out_list);

    BankList in_list;
    bank_list_init(&in_list);
    assert(bank_storage_load(&in_list, TEST_DATA_PATH) == BANK_OK);
    assert(in_list.count == 2);

    const BankAccount *loaded = bank_list_find(&in_list, 10001);
    assert(loaded != NULL);
    assert(strcmp(loaded->owner_name, "Alice Smith") == 0);
    assert(loaded->type == ACCOUNT_CHECKING);
    assert(loaded->balance_cents == 123456);

    bank_list_free(&in_list);
    remove(TEST_DATA_PATH);
}

static void test_money_parser_and_formatting(void) {
    int64_t cents = 0;
    char buf[32];

    assert(parse_money_cents("100", &cents) == 1 && cents == 10000);
    assert(parse_money_cents("100.5", &cents) == 1 && cents == 10050);
    assert(parse_money_cents("$100.05", &cents) == 1 && cents == 10005);
    assert(parse_money_cents("  0.00  ", &cents) == 1 && cents == 0);

    /* Exact cent arithmetic without floating-point drift (0.10 + 0.20 == 0.30) */
    int64_t c1 = 0;
    int64_t c2 = 0;
    assert(parse_money_cents("0.10", &c1) == 1);
    assert(parse_money_cents("0.20", &c2) == 1);
    assert(c1 + c2 == 30);

    assert(parse_money_cents(NULL, &cents) == 0);
    assert(parse_money_cents("10", NULL) == 0);
    assert(parse_money_cents("", &cents) == 0);
    assert(parse_money_cents("-50.00", &cents) == 0);
    assert(parse_money_cents("10.", &cents) == 0);
    assert(parse_money_cents(".50", &cents) == 0);
    assert(parse_money_cents("10.123", &cents) == 0);
    assert(parse_money_cents("99999999999999999999999", &cents) == 0);

    format_money(123456, buf, sizeof(buf));
    assert(strcmp(buf, "$1234.56") == 0);
}

int main(void) {
    test_account_crud_and_transactions();
    test_binary_storage_round_trip();
    test_money_parser_and_formatting();

    printf("All bank-management unit tests passed!\n");
    return 0;
}

