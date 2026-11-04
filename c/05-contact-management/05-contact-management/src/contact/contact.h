
#ifndef CONTACT_H
#define CONTACT_H

#include <stddef.h>

/* Maximum field lengths */
#define CONTACT_NAME_MAX 100
#define CONTACT_PHONE_MAX 20
#define CONTACT_EMAIL_MAX 100
#define CONTACT_ADDRESS_MAX 200
#define CONTACT_DEFAULT_CSV_PATH "data/contacts.csv"

typedef enum {
    CONTACT_OK = 0,
    CONTACT_ERR_NULL,
    CONTACT_ERR_NOMEM,
    CONTACT_ERR_INVALID,
    CONTACT_ERR_DUPLICATE_ID,
    CONTACT_ERR_NOT_FOUND,
    CONTACT_ERR_FILE_IO
} ContactStatus;

/* Single contact */
typedef struct {
    int id;
    char name[CONTACT_NAME_MAX];
    char phone[CONTACT_PHONE_MAX];
    char email[CONTACT_EMAIL_MAX];
    char address[CONTACT_ADDRESS_MAX];
} Contact;

/* Dynamic contact array */
typedef struct {
    Contact *items;
    size_t size;
    size_t capacity;
} ContactList;

/* Lifecycle and validation (contact_list.c) */
void contact_list_init(ContactList *list);
void contact_list_free(ContactList *list);
int contact_is_valid(const Contact *contact);
int contact_list_next_id(const ContactList *list);

/* CRUD operations (contact_list.c) */
ContactStatus contact_list_add(ContactList *list, const Contact *contact);
const Contact *contact_list_find_by_id(const ContactList *list, int id);
ContactStatus contact_list_update(ContactList *list, int id, const Contact *updated);
ContactStatus contact_list_remove_by_id(ContactList *list, int id);
int contact_name_matches(const Contact *contact, const char *query);
const char *contact_status_message(ContactStatus status);

/* CSV file persistence (contact_storage.c) */
ContactStatus contact_storage_save(const ContactList *list, const char *filepath);
ContactStatus contact_storage_load(ContactList *list, const char *filepath);

#endif /* CONTACT_H */
