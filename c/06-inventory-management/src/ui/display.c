#include "ui/display.h"
#include "inventory/inventory.h"
#include <inttypes.h>
#include <stdio.h>
void clear_screen(void){printf("\033[2J\033[H");fflush(stdout);} 
void print_banner(void){puts("============================================================");puts("                 Inventory Management System");puts("============================================================");puts("Commands: 'c' / 'clear' for menu, 'q' / 'quit' to exit.\n");}
void print_main_menu(size_t n){printf("--- Main Menu (%zu %s loaded) ---\n",n,n==1?"product":"products");puts("  [1] Add Product\n  [2] List Products\n  [3] Search Product\n  [4] Update Product\n  [5] Restock Product\n  [6] Sell Product\n  [7] Remove Product\n  [8] Quit");}
void format_money(int64_t cents,char *buffer,size_t size){if(!buffer||!size)return;snprintf(buffer,size,"$%" PRId64 ".%02" PRId64,cents/100,cents%100);}
void print_product_header(void){puts("+--------+----------------------+----------------+------------+----------+");puts("| SKU    | Name                 | Category       | Price      | Quantity |");puts("+--------+----------------------+----------------+------------+----------+");}
void print_product_row(const Product *p){if(!p)return;char price[32];format_money(p->price_cents,price,sizeof(price));printf("| %-6d | %-20.20s | %-14.14s | %10s | %8d |\n",p->sku,p->name,p->category,price,p->quantity);}
void print_product_footer(size_t n){puts("+--------+----------------------+----------------+------------+----------+");printf("Total: %zu %s\n\n",n,n==1?"product":"products");}
void print_inventory(const Inventory *inventory){if(!inventory||!inventory->count){puts("\nNo products found.\n");return;}puts("");print_product_header();for(size_t i=0;i<inventory->count;++i)print_product_row(&inventory->items[i]);print_product_footer(inventory->count);}
