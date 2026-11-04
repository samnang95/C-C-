#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "contact/contact.h"
#include "input/input.h"

#define TEST_CSV_PATH "build/test_contacts.csv"

static Contact make_contact(int id, const char *name, const char *phone,
                            const char *email, const char *address) {
    Contact contact = {0};
    contact.id = id;
    snprintf(contact.name, sizeof(contact.name), "%s", name);
    snprintf(contact.phone, sizeof(contact.phone), "%s", phone);
    snprintf(contact.email, sizeof(contact.email), "%s", email);
    snprintf(contact.address, sizeof(contact.address), "%s", address);
    return contact;
}

static void test_crud(void) {
    ContactList list;
    contact_list_init(&list);
    assert(list.items == NULL && list.size == 0 && list.capacity == 0);
    assert(contact_list_next_id(&list) == 1001);
    for (int i = 1; i <= 6; ++i) {
        char name[32]; snprintf(name, sizeof(name), "Contact %d", i);
        Contact contact = make_contact(1000 + i, name, "012345678", "a@example.com", "Phnom Penh");
        assert(contact_list_add(&list, &contact) == CONTACT_OK);
    }
    assert(list.size == 6 && list.capacity >= 8);
    assert(contact_list_add(&list, &list.items[0]) == CONTACT_ERR_DUPLICATE_ID);
    const Contact *found = contact_list_find_by_id(&list, 1003);
    assert(found != NULL && contact_name_matches(found, "TACT 3"));
    Contact updated = make_contact(1003, "Alice Example", "099999999", "alice@example.com", "Siem Reap");
    assert(contact_list_update(&list, 1003, &updated) == CONTACT_OK);
    assert(strcmp(contact_list_find_by_id(&list, 1003)->name, "Alice Example") == 0);
    assert(contact_list_remove_by_id(&list, 1003) == CONTACT_OK);
    assert(list.size == 5 && contact_list_find_by_id(&list, 1003) == NULL);
    assert(contact_list_remove_by_id(&list, 9999) == CONTACT_ERR_NOT_FOUND);
    contact_list_free(&list);
    assert(list.items == NULL && list.size == 0 && list.capacity == 0);
}

static void test_validation_and_storage(void) {
    ContactList out; contact_list_init(&out);
    Contact bad = make_contact(0, "Name", "1", "a@b.com", "Address");
    assert(contact_list_add(&out, &bad) == CONTACT_ERR_INVALID);
    assert(contact_list_add(NULL, &bad) == CONTACT_ERR_NULL);
    Contact first = make_contact(1001, "Alice Smith", "012-345-678", "alice@example.com", "Phnom Penh");
    Contact second = make_contact(1002, "Bob Tran", "098-765-432", "bob@example.com", "Battambang");
    assert(contact_list_add(&out, &first) == CONTACT_OK);
    assert(contact_list_add(&out, &second) == CONTACT_OK);
    assert(contact_storage_save(&out, TEST_CSV_PATH) == CONTACT_OK);
    contact_list_free(&out);
    ContactList in; contact_list_init(&in);
    assert(contact_storage_load(&in, TEST_CSV_PATH) == CONTACT_OK);
    assert(in.size == 2 && strcmp(contact_list_find_by_id(&in, 1001)->email, "alice@example.com") == 0);
    contact_list_free(&in);
    remove(TEST_CSV_PATH);
}

int main(void) {
    test_crud();
    test_validation_and_storage();
    int value = 0;
    assert(parse_int("42", &value) && value == 42);
    assert(!parse_int("42x", &value));
    puts("All contact-management unit tests passed!");
    return 0;
}
