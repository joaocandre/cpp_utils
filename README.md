# Storage

C++ header-only library with generic extensions to STL containers.

## Classes

- `std::matrix< T >`: Container adapter providing 2D interface to a `std::vector< T >`
- `std::volume< T >`

- `std::indexer< T >`: Container adapter that deals with ID-types
- `std::sorted_indexer< T >`
- `std::tagged< T >`: Meta-type wrapping an object with and addition custom tag. Similar to `std::pair`, but implicitly convertible to and constructible to `T`, thus can be used interchangeably in most scenarios. 

## Examples

Examples are provided under `src/examples`.

## Note