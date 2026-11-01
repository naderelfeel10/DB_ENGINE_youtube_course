# ElfeelDB

ElfeelDB is a disk-based relational database engine built from scratch in modern C++17.
The project is designed as a practical course and development journey through database internals—from raw pages and disk I/O to tables, indexes, and catalog metadata.

I’m recreating a relational database engine from scratch in modern C++17 and documenting the entire journey on YouTube.

## What has been implemented

### Part 1 — Storage foundations

- **Fields:** integer, floating-point, boolean, string, and null values.
- **Tuples:** collections of fields with serialization and deserialization.
- **Pages:** fixed-size slotted pages for storing tuples and raw data.
- **Disk manager:** page-based file creation, reading, writing, and persistence.
- **Buffer pool manager:** page fetching, dirty-page tracking, and page flushing.
- **LRU replacement:** replacement policy for managing buffer frames.

### Part 2 — Tables, indexes, and catalog

- **Columns:** column names, data types, and schema metadata.
- **RID:** record identifiers represented by page and slot locations.
- **Table heap:** table storage built on top of pages.
- **Table iterator:** sequential traversal over table records.
- **B+ tree index:** ordered keys, point lookups, range queries, leaf-page links, and persistence.
- **Catalog:** table definitions, schemas, table identifiers, and catalog persistence.

## Architecture

```text
                Catalog
                   |
        +----------+----------+
        |                     |
    TableHeap             B+ Tree Index
        |                     |
      Pages  <--------->     RID
        |
  Buffer Pool Manager
        |
    LRU Replacer
        |
    Disk Manager
        |
   Database Files
```

## Project structure

```text
src/
├── Buffer/
│   ├── BufferPoolManager.c++
│   └── LRU_replacement.c++
├── Catalog/
│   └── Catalog.c++
└── Storage/
    ├── Disk/
    │   └── DiskManager.c++
    ├── Indexing/
    │   └── BPlusTreeIndex.c++
    ├── Page/
    │   ├── Field.c++
    │   ├── page.c++
    │   └── Tuple.c++
    └── Table/
        ├── Column.c++
        ├── RID.c++
        ├── TableHeap.c++
        └── TableIterator.c++
```

## Main concepts demonstrated

- Page-oriented storage and slotted-page layouts
- Serialization of database objects into binary pages
- Buffer management and dirty-page handling
- LRU page replacement
- Record identifiers and table traversal
- B+ tree splitting, searching, range scanning, and leaf chaining
- Persistent metadata through a system catalog
- Separating in-memory objects from their on-disk representation

## Course series

This project is also being developed as a step-by-step course:

- **Part 1:** Pages, tuples, disk manager, buffer pool manager, and LRU replacement
- **Part 2:** Columns, RIDs, table heap, table iterator, B+ tree indexes, and catalog
- **Next:** SQL execution, query planning, transactions, WAL, and crash recovery

## Building

The project is currently under active development. Build commands and the test setup will be added as the build system is finalized.


## License

This project is currently intended for learning and experimentation.
