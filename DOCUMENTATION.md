# SparseSet Library Documentation

This directory contains Doxygen configuration for generating HTML documentation of the robot project's sparse set library.

## Overview

The sparse set library provides an efficient container for managing entity identifiers with O(1) insertion, deletion, and lookup operations.

## Generating Documentation

### Prerequisites

Ensure Doxygen is installed on your system:

```bash
# On Debian/Ubuntu
sudo apt-get install doxygen

# On macOS
brew install doxygen
```

### Generate HTML Documentation

From the project root directory, run:

```bash
doxygen Doxyfile
```

This will generate HTML documentation in the `docs/html/` directory.

### View Documentation

Open the generated documentation in your browser:

```bash
# Linux
xdg-open docs/html/index.html

# macOS
open docs/html/index.html

# Or use any web browser to open the file
```

## Documentation Structure

The generated documentation includes:

- **Class Reference**: Complete API documentation for `SparseSet<EntityCount>`
- **Method Documentation**: Detailed descriptions of all public methods with:
  - Brief descriptions of functionality
  - Parameter documentation (`@param`)
  - Return value documentation (`@return`)
  - Preconditions and postconditions (`@pre`, `@post`)
  - Exception information (`@throw`)
  - Time complexity notes (`@note`)
  - Usage examples (`@code` blocks)

## Key Features Documented

### Core Operations

- **`insert(value)`** - Add an entity to the set (O(1))
- **`erase(value)`** - Remove an entity from the set (O(1))
- **`contains(value)`** - Check if an entity exists (O(1))

### Utility Methods

- **`clear()`** - Remove all entities
- **`size()`** - Get the number of stored entities
- **`empty()`** - Check if the set is empty
- **`reserve(new_cap)`** - Pre-allocate space in the dense array

### Index Mapping

- **`indexFor(entityId)`** - Get dense array index for an entity ID
- **`idFor(index)`** - Get entity ID at a dense array index

### Iterators

- **`begin()` / `end()`** - Iterate over all entities
- **`cbegin()` / `cend()`** - Const iteration

## Architecture Notes

The sparse set uses two arrays:
- **Sparse Array** (dataIndex): Maps entity IDs to their positions in the dense array
- **Dense Array** (data): Stores entity IDs in contiguous memory

This design provides:
- Constant-time lookups via the sparse array
- Efficient iteration via the dense array
- Memory-efficient storage using only necessary space

## Example Usage

See the class documentation in the generated HTML for detailed examples:

```cpp
SparseSet<5000> entities;
entities.insert(42);
entities.insert(100);

if (entities.contains(42)) {
    // Process entity 42
}

for (auto entityId : entities) {
    // Iterate over all active entities in insertion order
}
```

## Configuration

The Doxyfile is pre-configured with:
- HTML output format
- Source code highlighting
- Tree view navigation
- Search functionality
- Organized by namespaces

Edit `Doxyfile` to customize the documentation generation if needed.

## Cleaning Up

To remove generated documentation:

```bash
rm -rf docs/html/
rm -rf docs/latex/
```
