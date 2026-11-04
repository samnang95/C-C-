#ifndef INVENTORY_H
#define INVENTORY_H

#include <stddef.h>
#include <stdint.h>

#define PRODUCT_NAME_MAX 100
#define PRODUCT_CATEGORY_MAX 50
#define INVENTORY_DEFAULT_CSV_PATH "data/inventory.csv"

typedef enum {
    INVENTORY_OK = 0, INVENTORY_ERR_NULL, INVENTORY_ERR_NOMEM,
    INVENTORY_ERR_INVALID, INVENTORY_ERR_DUPLICATE_SKU,
    INVENTORY_ERR_NOT_FOUND, INVENTORY_ERR_INSUFFICIENT_STOCK,
    INVENTORY_ERR_OVERFLOW, INVENTORY_ERR_FILE_IO
} InventoryStatus;

typedef struct {
    int sku;
    char name[PRODUCT_NAME_MAX];
    char category[PRODUCT_CATEGORY_MAX];
    int64_t price_cents;
    int quantity;
} Product;

typedef struct {
    Product *items;
    size_t count;
    size_t capacity;
} Inventory;

void inventory_init(Inventory *inventory);
void inventory_free(Inventory *inventory);
int product_is_valid(const Product *product);
int inventory_next_sku(const Inventory *inventory);
InventoryStatus inventory_add(Inventory *inventory, const Product *product);
const Product *inventory_find_by_sku(const Inventory *inventory, int sku);
InventoryStatus inventory_update(Inventory *inventory, int sku, const Product *updated);
InventoryStatus inventory_remove(Inventory *inventory, int sku);
InventoryStatus inventory_restock(Inventory *inventory, int sku, int quantity);
InventoryStatus inventory_sell(Inventory *inventory, int sku, int quantity);
int product_matches(const Product *product, const char *query);
const char *inventory_status_message(InventoryStatus status);

InventoryStatus inventory_storage_save(const Inventory *inventory, const char *filepath);
InventoryStatus inventory_storage_load(Inventory *inventory, const char *filepath);

#endif /* INVENTORY_H */
