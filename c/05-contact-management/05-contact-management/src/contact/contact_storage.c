#include "contact/contact.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CSV_LINE_MAX 512

static void strip_newline(char *text) {
    size_t length = text == NULL ? 0 : strlen(text);
    while (length > 0 && (text[length - 1] == '\n' || text[length - 1] == '\r'))
        text[--length] = '\0';
}

static int parse_int(const char *text, int *value) {
    if (text == NULL || value == NULL || *text == '\0') return 0;
    errno = 0;
    char *end = NULL;
    long parsed = strtol(text, &end, 10);
    if (end == text || *end != '\0' || errno == ERANGE || parsed < INT_MIN || parsed > INT_MAX)
        return 0;
    *value = (int)parsed;
    return 1;
}

static int split_fields(char *line, char *fields[5]) {
    char *cursor = line;
    for (size_t i = 0; i < 5; ++i) {
        fields[i] = cursor;
        char *comma = strchr(cursor, ',');
        if (i == 4) return comma == NULL;
        if (comma == NULL) return 0;
        *comma = '\0';
        cursor = comma + 1;
    }
    return 0;
}

ContactStatus contact_storage_save(const ContactList *list, const char *filepath) {
    if (list == NULL || filepath == NULL) return CONTACT_ERR_NULL;
    FILE *file = fopen(filepath, "w");
    if (file == NULL) return CONTACT_ERR_FILE_IO;

    int failed = fprintf(file, "id,name,phone,email,address\n") < 0;
    for (size_t i = 0; !failed && i < list->size; ++i) {
        const Contact *contact = &list->items[i];
        failed = fprintf(file, "%d,%s,%s,%s,%s\n", contact->id, contact->name,
                         contact->phone, contact->email, contact->address) < 0;
    }
    if (fclose(file) != 0 || failed) return CONTACT_ERR_FILE_IO;
    return CONTACT_OK;
}

ContactStatus contact_storage_load(ContactList *list, const char *filepath) {
    if (list == NULL || filepath == NULL) return CONTACT_ERR_NULL;
    FILE *file = fopen(filepath, "r");
    if (file == NULL) return CONTACT_ERR_FILE_IO;

    char line[CSV_LINE_MAX];
    size_t line_number = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        ++line_number;
        if (strchr(line, '\n') == NULL && !feof(file)) {
            int ch;
            while ((ch = fgetc(file)) != '\n' && ch != EOF) { }
            continue;
        }
        strip_newline(line);
        if (line[0] == '\0' || (line_number == 1 && strcmp(line, "id,name,phone,email,address") == 0))
            continue;

        char *fields[5];
        Contact contact = {0};
        if (!split_fields(line, fields) || !parse_int(fields[0], &contact.id)) continue;
        snprintf(contact.name, sizeof(contact.name), "%s", fields[1]);
        snprintf(contact.phone, sizeof(contact.phone), "%s", fields[2]);
        snprintf(contact.email, sizeof(contact.email), "%s", fields[3]);
        snprintf(contact.address, sizeof(contact.address), "%s", fields[4]);
        if (contact_is_valid(&contact)) (void)contact_list_add(list, &contact);
    }
    if (ferror(file) || fclose(file) != 0) return CONTACT_ERR_FILE_IO;
    return CONTACT_OK;
}
