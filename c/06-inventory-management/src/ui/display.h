#ifndef DISPLAY_H
#define DISPLAY_H
#include "inventory/inventory.h"
void clear_screen(void); void print_banner(void); void print_main_menu(size_t count);
void format_money(int64_t cents, char *buffer, size_t size);
void print_product_header(void); void print_product_row(const Product *product);
void print_product_footer(size_t count); void print_inventory(const Inventory *inventory);
#endif
