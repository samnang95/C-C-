# Inventory Management (C++)

A modular, dependency-free C++17 command-line application for tracking product inventory, SKU codes, stock levels, restocking/selling, low-stock alerts, and warehouse valuation. Data is persisted to `data/inventory.db`.

## Project Structure

```text
05-inventory-management/
├── build/
├── data/
├── src/
│   ├── main.cpp
│   ├── inventory/
│   │   ├── Product.hpp
│   │   ├── Product.cpp
│   │   ├── Inventory.hpp
│   │   └── Inventory.cpp
│   └── ui/
│       ├── ConsoleUI.hpp
│       └── ConsoleUI.cpp
├── tests/
│   └── test_inventory.cpp
├── .gitignore
├── compile_flags.txt
├── Makefile
└── README.md
```

## Build, Run & Test

```sh
# Build executable into build/inventory_management
make

# Run interactively
make run

# Run unit tests
make test

# Clean build artifacts
make clean
```

## Commands

| Command | Description |
| --- | --- |
| `add` | Add a new product interactively (auto-generates SKU if left blank). |
| `list` | List all products with stock status (`OK`, `LOW`, `OUT`). |
| `list low` | List products at or below their reorder alert level. |
| `list <category>` | List products in a specific category. |
| `view [id]` | Display full details of a product. |
| `edit [id]` | Update name, SKU, category, unit price, or reorder level. |
| `restock [id]` | Add incoming stock quantity to a product. |
| `sell [id]` | Deduct sold quantity from a product (warns on low stock). |
| `remove [id]` | Delete a product from the catalogue. |
| `search <text>` | Search products by name, SKU, or category. |
| `summary` | Show total products, units, low-stock alerts, and inventory valuation. |
| `clear` / `reset` | Clear the terminal screen. |
| `help` | Display command help. |
| `quit` | Save and exit. |

