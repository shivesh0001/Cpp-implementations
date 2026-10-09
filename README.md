# Smart Pointers

Small implementations of C++ smart pointers:

- `unique_ptr.hpp` provides exclusive ownership and move-only semantics.
- `shared_ptr.hpp` provides reference-counted shared ownership.

Both classes support default construction, move construction and assignment,
`*`, `->`, and explicit boolean conversion. Each header currently contains a
small demo `Resource` type and `main()` function, so compile or experiment
with the headers separately.