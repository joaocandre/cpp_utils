# cpp_utils

C++ header-only library with extensions to STL containers and other general-purpose utilities.
Complete API documentation provided at [codedocs.xyz](https://codedocs.xyz/joaocandre/cpp_utils/index.html).

![build status](https://github.com/joaocandre/cpp_utils/actions/workflows/cmake.yml/badge.svg?event=push)
![build status](https://github.com/joaocandre/cpp_utils/actions/workflows/ros2.yml/badge.svg?event=push)
[![doc status](https://codedocs.xyz/joaocandre/cpp_utils.svg)](https://codedocs.xyz/joaocandre/cpp_utils/index.html)

## Dependencies

Thread utilities require end user to provide [Boost ASIO](https://github.com/boostorg/asio) headers;
XML serialization utilities wrap around [TinyXML2](https://github.com/leethomason/tinyxml2).
Optionally, dependency sources can be pulled, built and installed alongside *cpp_utils* when CMake option `cpp_utils_FETCH_DEPENDENCIES` is enabled, but this is **heavily discouraged**, as it not the intended CMake workflow and error-prone.
Providing OS-packaged versions remains the recommended approach.
