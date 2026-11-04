# 06 - Inventory Management System

A modular C11 command-line inventory manager with dynamic product storage and CSV persistence.

Each product has a SKU, name, category, price stored as exact integer cents, and current quantity. The program supports adding, listing, searching, updating, restocking, selling, and removing products. It saves data to `data/inventory.csv` after every change.

```sh
cd c/06-inventory-management
make run
make test
make clean
```
