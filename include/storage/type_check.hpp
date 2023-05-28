#ifndef ST_TYPE_CHECK_HPP
#define ST_TYPE_CHECK_HPP
///////////////////////////////////
#include <type_traits>
#include "type/check.hpp"   // std::can_apply<>
// SFINAE (Substitution Failure Is Not An Error) approach
// reference: https://stackoverflow.com/a/31306194

///////////////////////////////////
namespace std {
// namespace storage{
///////////////////////////////////
///////////////////////////////////
// namespace details {
//   template<class...>struct voider{using type=void;};
//   template<class...Ts>using void_t=typename voider<Ts...>::type;

//   template<template<class...>class Z, class, class...Ts>
//   struct can_apply:
//     std::false_type
//   {};
//   template<template<class...>class Z, class...Ts>
//   struct can_apply<Z, void_t<Z<Ts...>>, Ts...>:
//     std::true_type
//   {};
// }
// template<template<class...>class Z, class...Ts>
// using can_apply=details::can_apply<Z,void,Ts...>;
// ///////////////////////////////////
// // member checks
// template <class T, class Index>
// using subscript_operator= decltype(std::declval<T>()[std::declval<Index>()]);
// template <class T>
// using size_member       = decltype(std::declval<T>().size());
// template <class T>
// using begin_iterator    = decltype(std::declval<T>().begin());
// template <class T>
// using end_iterator      = decltype(std::declval<T>().end());
// template <class T>
// using incrementer       = decltype(std::declval<T>().operator++());
// template <class lT, class rT>
// using equal_operator    = decltype(std::declval<lT>().operator==(std::declval<lT>()));
// template <class lT, class rT>
// using unequal_operator  = decltype(std::declval<lT>().operator!=(std::declval<lT>()));
// template <class T>
// using pointer_accessor  = decltype(std::declval<T>().operator*());
// template <class T>
// using value_type_member = typename T::value_type;
// template <class T>
// using shape_member      = decltype(std::declval<T>().shape());
// ///////////////////////////////////
// // non-member checks
// template <class lT, class rT>
// using nm_comparison_equal     = decltype(std::declval<lT>() == std::declval<rT>());
// template <class lT, class rT>
// using nm_comparison_not_equal = decltype(std::declval<lT>() != std::declval<rT>());
// template <class lT, class rT>
// using nm_comparison_greater   = decltype(std::declval<lT>() > std::declval<rT>());
// template <class lT, class rT>
// using nm_comparison_less      = decltype(std::declval<lT>() < std::declval<rT>());
// template <class lT, class rT>
// using nm_comparison_greater_equal = decltype(std::declval<lT>() >= std::declval<rT>());
// template <class lT, class rT>
// using nm_comparison_less_equal    = decltype(std::declval<lT>() <= std::declval<rT>());
// ///////////////////////////////////
// // boolean calls
// template <class T, class Index = int>
// using has_subscript         = can_apply<subscript_operator, T, Index>;
// template <class T>
// using has_size              = can_apply<size_member, T>;
// template <class T>
// using has_begin             = can_apply<begin_iterator, T>;
// template <class T>
// using has_end               = can_apply<end_iterator, T>;
// template <class T>
// using has_incrementer       = can_apply<incrementer, T>;
// template <class T>
// using has_equal_operator    = can_apply<equal_operator, T>;
// template <class T>
// using has_unequal_operator  = can_apply<unequal_operator, T>;
// template <class T>
// using has_pointer_accessor  = can_apply<pointer_accessor, T>;
// template <class T>
// using has_value_type        = can_apply<value_type_member, T>;
// template <class T>
// using has_shape             = can_apply<shape_member, T>;
// ///////////////////////////////////
// // check functions
// template <typename C>
// constexpr bool is_generic_container() {
//     return (has_subscript<C>() && has_size<C>() && has_value_type<C>());
// }
// template <typename C>
// constexpr bool is_iteratable() { //asserts if typename C can be used in a ranged loop
//     return (has_begin<C>()  &&
//             has_end<C>()    &&
//             has_incrementer<C>() &&
//             has_equal_operator<C>() &&
//             has_unequal_operator<C>() &&
//             has_pointer_accessor<C>() );
// }
// template <typename C>
// constexpr bool is_nd_container() { //asserts if typename C can be used in a ranged loop
//   return (is_generic_container<C>() && has_shape<C>() );
// }
//

//------------------------------------------------------------------------------
/// @brief      Check if *C* provides a generic STL container (e.g. std::vector<>) public interface.
///
/// @tparam     C     Container/class type.
///
/// @return     True if *C* behaves like a generic STL container (e.g. std::vector<>), false otherwise.
///
/// @todo       Add more detail to checks, tehre are more requirements to STL container interface.
///             cf. https://en.cppreference.com/w/cpp/named_req/Container
///
template < typename C >
constexpr bool is_generic_container() {
    return (is_subscriptable< C >() && has_size< C >() && has_value_type< C >());
}


//------------------------------------------------------------------------------
/// @brief      Check if *C* provides a generic multi-dimensional container (e.g. std::matrix<>) public interface.
///
/// @tparam     C     Container/class type.
///
/// @return     True if *C* behaves like a generic multi-dimensional container (e.g. std::matrix<>), false otherwise.
///
template < typename C >
constexpr bool is_nd_container() { //asserts if typename C can be used in a ranged loop
  return (is_generic_container< C >() && has_shape< C >() );
}

///////////////////////////////////
// } //storage
} //std
#endif //TYPE_CHECK_HPP