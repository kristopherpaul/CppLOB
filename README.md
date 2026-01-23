# C++ Limit Order Book Matching Engine

Two implementations over one semantic contract:
- **Runtime LOB**: Single-threaded, preallocated, hierarchical bitmap
- **Compile-Time LOB**: Type-level state, template metaprogramming, constexpr evaluation

## Building

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Running Tests

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```