#include "contact/contact.h"

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 4

static ContactStatus ensure_capacity(ContactList *list, size_t minimum) {
    if (list == NULL) return CONTACT_ERR_NULL;
    if (list->capacity >= minimum) return CONTACT_OK;

    size_t capacity = list->capacity == 0 ? INITIAL_CAPACITY : list->capacity;
    while (capacity < minimum) {
        if (capacity > SIZE_MAX / 2) return CONTACT_ERR_NOMEM;
        capacity *= 2;
    }
    if (capacity > SIZE_MAX / sizeof(*list->items)) return CONTACT_ERR_NOMEM;

    Contact *items = realloc(list->items, capacity * sizeof(*items));
    if (items == NULL) return CONTACT_ERR_NOMEM;
    list->items = items;
    list->capacity = capacity;
    return CONTACT_OK;
}

static int find_index(const ContactList *list, int id, size_t *index) {
    if (list == NULL) return 0;
    for (size_t i = 0; i < list->size; ++i) {
        if (list->items[i].id == id) {
            if (index != NULL) *index = i;
            return 1;
        }
    }
    return 0;
}

void contact_list_init(ContactList *list) {
    if (list == NULL) return;
    list->items = NULL;
    list->size = 0;
    list->capacity = 0;
}

void contact_list_free(ContactList *list) {
    if (list == NULL) return;
    free(list->items);
    contact_list_init(list);
}

int contact_is_valid(const Contact *contact) {
    return contact != NULL && contact->id > 0 && contact->name[0] != '\0' &&
           contact->phone[0] != '\0' && contact->email[0] != '\0' &&
           contact->address[0] != '\0';
}

int contact_list_next_id(const ContactList *list) {
    int max_id = 1000;
    if (list != NULL) {
        for (size_t i = 0; i < list->size; ++i)
            if (list->items[i].id > max_id) max_id = list->items[i].id;
    }
    return max_id == INT32_MAX ? INT32_MAX : max_id + 1;
}

ContactStatus contact_list_add(ContactList *list, const Contact *contact) {
    if (list == NULL || contact == NULL) return CONTACT_ERR_NULL;
    if (!contact_is_valid(contact)) return CONTACT_ERR_INVALID;
    if (find_index(list, contact->id, NULL)) return CONTACT_ERR_DUPLICATE_ID;

    ContactStatus status = ensure_capacity(list, list->size + 1);
    if (status != CONTACT_OK) return status;
    list->items[list->size++] = *contact;
    return CONTACT_OK;
}

const Contact *contact_list_find_by_id(const ContactList *list, int id) {
    size_t index = 0;
    return find_index(list, id, &index) ? &list->items[index] : NULL;
}

ContactStatus contact_list_update(ContactList *list, int id, const Contact *updated) {
    if (list == NULL || updated == NULL) return CONTACT_ERR_NULL;
    if (!contact_is_valid(updated) || updated->id != id) return CONTACT_ERR_INVALID;
    size_t index = 0;
    if (!find_index(list, id, &index)) return CONTACT_ERR_NOT_FOUND;
    list->items[index] = *updated;
    return CONTACT_OK;
}

ContactStatus contact_list_remove_by_id(ContactList *list, int id) {
    if (list == NULL) return CONTACT_ERR_NULL;
    size_t index = 0;
    if (!find_index(list, id, &index)) return CONTACT_ERR_NOT_FOUND;
    if (index + 1 < list->size)
        memmove(&list->items[index], &list->items[index + 1],
                (list->size - index - 1) * sizeof(*list->items));
    --list->size;
    return CONTACT_OK;
}

int contact_name_matches(const Contact *contact, const char *query) {
    if (contact == NULL || query == NULL || *query == '\0') return 0;
    size_t query_len = strlen(query);
    size_t name_len = strlen(contact->name);
    if (query_len > name_len) return 0;
    for (size_t i = 0; i <= name_len - query_len; ++i) {
        size_t j = 0;
        while (j < query_len && tolower((unsigned char)contact->name[i + j]) ==
                                tolower((unsigned char)query[j])) ++j;
        if (j == query_len) return 1;
    }
    return 0;
}

const char *contact_status_message(ContactStatus status) {
    switch (status) {
        case CONTACT_OK: return "OK";
        case CONTACT_ERR_NULL: return "Error: internal null pointer.";
        case CONTACT_ERR_NOMEM: return "Error: out of memory.";
        case CONTACT_ERR_INVALID: return "Error: invalid contact record.";
        case CONTACT_ERR_DUPLICATE_ID: return "Error: a contact with that ID already exists.";
        case CONTACT_ERR_NOT_FOUND: return "Error: contact ID not found.";
        case CONTACT_ERR_FILE_IO: return "Error: could not read or write contact data file.";
        default: return "Error: unknown contact error.";
    }
}
