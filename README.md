# cpp_utils

C++ header-only library with generic extensions to STL containers.

## Classes

Complete API documentation provided at [codedocs.xyz](https://codedocs.xyz/joaocandre/cpp_utils/index.html).

![build status](https://github.com/joaocandre/cpp_utils/actions/workflows/cmake.yml/badge.svg?event=push)
[![doc status](https://codedocs.xyz/joaocandre/cpp_utils.svg)](https://codedocs.xyz/joaocandre/cpp_utils/index.html)

- `std::matrix< T >`: Container adapter providing 2D interface to a `std::vector< T >`
- `std::volume< T >`

- `std::indexer< T >`: Container adapter that deals with ID-types
- `std::sorted_indexer< T >`
- `std::tagged< T >`: Meta-type wrapping an object with and addition custom tag. Similar to `std::pair`, but implicitly convertible to and constructible to `T`, thus can be used interchangeably in most scenarios. 

## Examples

Examples are provided under `src/examples`.

## Note

...
