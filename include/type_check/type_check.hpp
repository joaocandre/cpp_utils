//------------------------------------------------------------------------------
/// @file       type_check.hpp
/// @author     João André
///
/// @brief      Header file declaring utility implementations over C++ 'type traits' interface (<type_traits>).
///
/// Defines std::can_apply<> class template, that leverages SFINAE static assertions/checks in <type_traits> header to provide a generic
/// attribute/property check over custom types.
///
//------------------------------------------------------------------------------

#ifndef TRIGNOCLIENT_INCLUDE_TRIGNOCLIENT_TYPECHECK_HPP_
#define TRIGNOCLIENT_INCLUDE_TRIGNOCLIENT_TYPECHECK_HPP_

#include <type_traits>

namespace std {

namespace details {

// custom class template converting one or more types to void
// i.e. voider< T >::type is always void
template < class... >
struct voider{
    using type = void;
};

// alias adding verbosity
template < class... Ts >
using void_t = typename voider< Ts... >::type;


// class templates inherting from std::false_type/true_type (static/compile time representation of boolean false/true)
// only 1 of these will be defined at any time
template < template < class... > class Z, class, class... Ts >
struct can_apply: std::false_type {
    /* ... */
};

template < template < class... > class Z, class... Ts >
struct can_apply< Z, void_t< Z< Ts... > >, Ts... > : std::true_type {
    /* ... */
};

}  // namespace details



// alias adding verbosity for outside details namespace
template < template < class... > class Z, class...Ts >
using can_apply = details::can_apply< Z, void ,Ts... >;

}  // namespace std

#endif  // TRIGNOCLIENT_INCLUDE_TRIGNOCLIENT_TYPECHECK_HPP_
