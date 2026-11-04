#include "inventory/inventory.h"

#include <ctype.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 8

static InventoryStatus ensure_capacity(Inventory *inventory, size_t needed) {
    if (inventory == NULL) return INVENTORY_ERR_NULL;
    if (inventory->capacity >= needed) return INVENTORY_OK;
    size_t capacity = inventory->capacity == 0 ? INITIAL_CAPACITY : inventory->capacity;
    while (capacity < needed) {
        if (capacity > SIZE_MAX / 2) return INVENTORY_ERR_NOMEM;
        capacity *= 2;
    }
    if (capacity > SIZE_MAX / sizeof(*inventory->items)) return INVENTORY_ERR_NOMEM;
    Product *items = realloc(inventory->items, capacity * sizeof(*items));
    if (items == NULL) return INVENTORY_ERR_NOMEM;
    inventory->items = items;
    inventory->capacity = capacity;
    return INVENTORY_OK;
}

static int find_index(const Inventory *inventory, int sku, size_t *out_index) {
    if (inventory == NULL) return 0;
    for (size_t i = 0; i < inventory->count; ++i) {
        if (inventory->items[i].sku == sku) {
            if (out_index != NULL) *out_index = i;
            return 1;
        }
    }
    return 0;
}

void inventory_init(Inventory *inventory) {
    if (inventory == NULL) return;
    inventory->items = NULL; inventory->count = 0; inventory->capacity = 0;
}

void inventory_free(Inventory *inventory) {
    if (inventory == NULL) return;
    free(inventory->items);
    inventory_init(inventory);
}

int product_is_valid(const Product *product) {
    return product != NULL && product->sku > 0 && product->name[0] != '\0' &&
           product->category[0] != '\0' && product->price_cents >= 0 && product->quantity >= 0;
}

int inventory_next_sku(const Inventory *inventory) {
    int maximum = 1000;
    if (inventory != NULL) for (size_t i = 0; i < inventory->count; ++i)
        if (inventory->items[i].sku > maximum) maximum = inventory->items[i].sku;
    return maximum == INT_MAX ? INT_MAX : maximum + 1;
}

InventoryStatus inventory_add(Inventory *inventory, const Product *product) {
    if (inventory == NULL || product == NULL) return INVENTORY_ERR_NULL;
    if (!product_is_valid(product)) return INVENTORY_ERR_INVALID;
    if (find_index(inventory, product->sku, NULL)) return INVENTORY_ERR_DUPLICATE_SKU;
    InventoryStatus status = ensure_capacity(inventory, inventory->count + 1);
    if (status != INVENTORY_OK) return status;
    inventory->items[inventory->count++] = *product;
    return INVENTORY_OK;
}

const Product *inventory_find_by_sku(const Inventory *inventory, int sku) {
    size_t index = 0;
    return find_index(inventory, sku, &index) ? &inventory->items[index] : NULL;
}

InventoryStatus inventory_update(Inventory *inventory, int sku, const Product *updated) {
    if (inventory == NULL || updated == NULL) return INVENTORY_ERR_NULL;
    if (!product_is_valid(updated) || updated->sku != sku) return INVENTORY_ERR_INVALID;
    size_t index = 0;
    if (!find_index(inventory, sku, &index)) return INVENTORY_ERR_NOT_FOUND;
    inventory->items[index] = *updated;
    return INVENTORY_OK;
}

InventoryStatus inventory_remove(Inventory *inventory, int sku) {
    if (inventory == NULL) return INVENTORY_ERR_NULL;
    size_t index = 0;
    if (!find_index(inventory, sku, &index)) return INVENTORY_ERR_NOT_FOUND;
    if (index + 1 < inventory->count) memmove(&inventory->items[index], &inventory->items[index + 1],
        (inventory->count - index - 1) * sizeof(*inventory->items));
    --inventory->count;
    return INVENTORY_OK;
}

InventoryStatus inventory_restock(Inventory *inventory, int sku, int quantity) {
    if (inventory == NULL) return INVENTORY_ERR_NULL;
    if (quantity <= 0) return INVENTORY_ERR_INVALID;
    size_t index = 0;
    if (!find_index(inventory, sku, &index)) return INVENTORY_ERR_NOT_FOUND;
    if (inventory->items[index].quantity > INT_MAX - quantity) return INVENTORY_ERR_OVERFLOW;
    inventory->items[index].quantity += quantity;
    return INVENTORY_OK;
}

InventoryStatus inventory_sell(Inventory *inventory, int sku, int quantity) {
    if (inventory == NULL) return INVENTORY_ERR_NULL;
    if (quantity <= 0) return INVENTORY_ERR_INVALID;
    size_t index = 0;
    if (!find_index(inventory, sku, &index)) return INVENTORY_ERR_NOT_FOUND;
    if (inventory->items[index].quantity < quantity) return INVENTORY_ERR_INSUFFICIENT_STOCK;
    inventory->items[index].quantity -= quantity;
    return INVENTORY_OK;
}

int product_matches(const Product *product, const char *query) {
    if (product == NULL || query == NULL || *query == '\0') return 0;
    const char *fields[] = { product->name, product->category };
    size_t qlen = strlen(query);
    for (size_t f = 0; f < 2; ++f) {
        size_t length = strlen(fields[f]);
        for (size_t i = 0; i + qlen <= length; ++i) {
            size_t j = 0;
            while (j < qlen && tolower((unsigned char)fields[f][i + j]) == tolower((unsigned char)query[j])) ++j;
            if (j == qlen) return 1;
        }
    }
    return 0;
}

const char *inventory_status_message(InventoryStatus status) {
    switch (status) {
        case INVENTORY_OK: return "OK";
        case INVENTORY_ERR_NULL: return "Error: internal null pointer.";
        case INVENTORY_ERR_NOMEM: return "Error: out of memory.";
        case INVENTORY_ERR_INVALID: return "Error: invalid product or quantity.";
        case INVENTORY_ERR_DUPLICATE_SKU: return "Error: a product with that SKU already exists.";
        case INVENTORY_ERR_NOT_FOUND: return "Error: product SKU not found.";
        case INVENTORY_ERR_INSUFFICIENT_STOCK: return "Error: insufficient stock.";
        case INVENTORY_ERR_OVERFLOW: return "Error: quantity would overflow.";
        case INVENTORY_ERR_FILE_IO: return "Error: could not read or write inventory data file.";
        default: return "Error: unknown inventory error.";
    }
}
