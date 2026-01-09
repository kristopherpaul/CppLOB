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
cmake -B build -DBUILD_TESTS=ON
cmake --build build
./build/conformance_tests
```