#include "inventory/inventory.h"

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CSV_LINE_MAX 512

static int parse_int(const char *text, int *out) {
    if (text == NULL || out == NULL || *text == '\0') return 0;
    errno = 0; char *end = NULL; long value = strtol(text, &end, 10);
    if (end == text || *end != '\0' || errno == ERANGE || value < INT_MIN || value > INT_MAX) return 0;
    *out = (int)value; return 1;
}
static int parse_i64(const char *text, int64_t *out) {
    if (text == NULL || out == NULL || *text == '\0') return 0;
    errno = 0; char *end = NULL; intmax_t value = strtoimax(text, &end, 10);
    if (end == text || *end != '\0' || errno == ERANGE || value < INT64_MIN || value > INT64_MAX) return 0;
    *out = (int64_t)value; return 1;
}
static int split(char *line, char *fields[5]) {
    char *cursor = line;
    for (size_t i = 0; i < 5; ++i) {
        fields[i] = cursor; char *comma = strchr(cursor, ',');
        if (i == 4) return comma == NULL;
        if (comma == NULL) return 0;
        *comma = '\0'; cursor = comma + 1;
    }
    return 0;
}

InventoryStatus inventory_storage_save(const Inventory *inventory, const char *filepath) {
    if (inventory == NULL || filepath == NULL) return INVENTORY_ERR_NULL;
    FILE *file = fopen(filepath, "w");
    if (file == NULL) return INVENTORY_ERR_FILE_IO;
    int failed = fprintf(file, "sku,name,category,price_cents,quantity\n") < 0;
    for (size_t i = 0; !failed && i < inventory->count; ++i) {
        Product const *p = &inventory->items[i];
        failed = fprintf(file, "%d,%s,%s,%" PRId64 ",%d\n", p->sku, p->name, p->category, p->price_cents, p->quantity) < 0;
    }
    return fclose(file) != 0 || failed ? INVENTORY_ERR_FILE_IO : INVENTORY_OK;
}

InventoryStatus inventory_storage_load(Inventory *inventory, const char *filepath) {
    if (inventory == NULL || filepath == NULL) return INVENTORY_ERR_NULL;
    FILE *file = fopen(filepath, "r");
    if (file == NULL) return INVENTORY_ERR_FILE_IO;
    char line[CSV_LINE_MAX]; size_t line_number = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        ++line_number;
        if (strchr(line, '\n') == NULL && !feof(file)) { int ch; while ((ch = fgetc(file)) != '\n' && ch != EOF) {} continue; }
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0' || (line_number == 1 && strcmp(line, "sku,name,category,price_cents,quantity") == 0)) continue;
        char *fields[5]; Product p = {0};
        if (!split(line, fields) || !parse_int(fields[0], &p.sku) || !parse_i64(fields[3], &p.price_cents) || !parse_int(fields[4], &p.quantity)) continue;
        snprintf(p.name, sizeof(p.name), "%s", fields[1]);
        snprintf(p.category, sizeof(p.category), "%s", fields[2]);
        if (product_is_valid(&p)) (void)inventory_add(inventory, &p);
    }
    return ferror(file) || fclose(file) != 0 ? INVENTORY_ERR_FILE_IO : INVENTORY_OK;
}
