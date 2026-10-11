# C++ Implementations

implementations of common C++ data structures and ownership utilities.

## Files

- `unique_ptr.hpp` implements an exclusive-ownership smart pointer with move-only semantics.
- `shared_ptr.hpp` implements a reference-counted shared-ownership smart pointer.
- `hashmap.hpp` implements a templated hash map using separate chaining with `std::list` buckets.

## Smart pointers

Both smart pointer classes support:

- Default construction
- Move construction and move assignment
- Dereference operators: `*` and `->`
- Explicit boolean conversion

`shared_ptr.hpp` also supports copy construction and copy assignment by updating an internal reference count.

## Hash map

`hashmap.hpp` supports:

- `insert(key, value)` to add or update values
- `get(key)` to retrieve a value
- `containskey(key)` to check whether a key exists
- `erase(key)` to remove a key-value pair
- `getsize()` and `isempty()` for basic state checks
- Automatic rehashing when the load factor exceeds `0.75`

The hash map uses `std::hash` for bucket selection and doubles its bucket capacity during rehashing.

## Notes

These files are intended for learning and experimentation. Each header currently contains a small `main()` function, so compile or experiment with the headers separately.