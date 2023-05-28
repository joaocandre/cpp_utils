//------------------------------------------------------------------------------
/// @file       check.hpp
/// @author     João André
///
/// @brief      Header file providing generic alias template over class members.
///
/// Intended to be used w/ std::details::can_apply<> to implement custom/advanced SFINAE approaches
/// for static programming assertions/checks.
///
//------------------------------------------------------------------------------

#ifndef CPPUTILS_INCLUDE_TYPE_CHECK_HPP_
#define CPPUTILS_INCLUDE_TYPE_CHECK_HPP_

#include <type_traits>
#include <iostream>
#include "type/details.hpp"  // std::can_apply< >

namespace std {
namespace traits {
//------------------------------------------------------------------------------
/// @defgroup   Convetional STL container members (cf. STL containers)
///
/// @{

template < class T, class Index>
using subscript_operator = decltype(std::declval< T >()[std::declval< Index >()]);

template < class T >
using size = decltype(std::declval< T >().size());

template < class T >
using begin = decltype(std::declval< T >().begin());

template < class T >
using end = decltype(std::declval< T >().end());

// template < class T >
// using pointer_accessor = decltype(std::declval< T >().operator*());

template < class T >
using value_type = typename T::value_type;

template < class T >
using iterator_type = typename T::interator;

template < class T >
using const_iterator_type = typename T::const_interator;

template < class T >
using shape_member = decltype(std::declval< T >().shape());   // not STL default!

// @todo    add other members listed in container named requirements https://en.cppreference.com/w/cpp/named_req/Container
// ...

// @}


//------------------------------------------------------------------------------
/// @defgroup   Comparison operators
///
/// @{

template < class lT, class rT >
using equal_operator = decltype(std::declval< lT >() == std::declval<rT >());

template < class lT, class rT >
using unequal_operator = decltype(std::declval< lT >() != std::declval<rT >());

template < class lT, class rT >
using greater_operator = decltype(std::declval< lT >() > std::declval<rT >());

template < class lT, class rT >
using less_operator = decltype(std::declval< lT >() < std::declval<rT >());

template < class lT, class rT >
using greater_or_equal_operator = decltype(std::declval< lT >() >= std::declval<rT >());

template < class lT, class rT >
using less_or_equal_operator = decltype(std::declval< lT >() <= std::declval<rT >());

// @todo    add remaining member operator overloads && rename
// @todo    check if member/non-member distinction i required!
// ...

// @}

//------------------------------------------------------------------------------
/// @defgroup   Bitshift/stream operators.
///
/// @{

template < class T, class IStream = istream >
using istream_operator = decltype(declval< IStream >() >> declval< T >());

template < class T, class OStream = ostream >
using ostream_operator = decltype(declval< OStream >() << declval< T >());

// @}

//------------------------------------------------------------------------------
/// @defgroup   Conventional STL Iterator members
///
/// @{

template < class T >
using dereference_operator = decltype(*(declval< T >()));  // decltype(std::declval< T >().operator*());

template < class T >
using prefix_increment_operator = decltype(declval< T >().operator++());

template < class T >
using postfix_increment_operator = decltype((declval< T >()).operator++(declval< int >()));

template < class T >
using prefix_decrement_operator = decltype(declval< T >().operator--());

template < class T >
using postfix_decrement_operator = decltype((declval< T >()).operator--(declval< int >()));

// @todo    add remaining members (cf. https://cplusplus.com/reference/iterator/)
// ...

}  // namespace traits

//------------------------------------------------------------------------------
/// @defgroup   Specializations of std::can_apply< > to member/operator alias
///
/// @{

template < class T, class Index = int >
using is_subscriptable = can_apply< traits::subscript_operator, T, Index>;

template < class T >
using has_size = can_apply< traits::size, T >;

template < class T >
using has_begin = can_apply< traits::begin, T >;

template < class T >
using has_end = can_apply< traits::end, T >;

// template < class T >
// using has_pointer_accessor = can_apply< pointer_accessor, T >;

template < class T >
using has_value_type = can_apply< traits::value_type, T >;

template < class T >
using has_iterator_type = can_apply< traits::iterator_type, T >;

template < class T >
using has_const_iterator_type = can_apply< traits::const_iterator_type, T >;

template < class T >
using has_shape = can_apply< traits::shape_member, T >;

// ...

template < class lT, class rT >
using are_equal_comparable = can_apply< traits::equal_operator, lT, rT >;

template < class lT, class rT >
using are_unequal_comparable = can_apply< traits::unequal_operator, lT, rT >;

// ...

template < class T >
using is_dereferenceable = can_apply< traits::dereference_operator, T >;

template < class T >
using is_prefix_incrementable = can_apply< traits::prefix_increment_operator, T >;

template < class T >
using is_prefix_decrementable = can_apply< traits::prefix_decrement_operator, T >;

template < class T >
using is_postfix_incrementable = can_apply< traits::postfix_increment_operator, T >;

template < class T >
using is_postfix_decrementable = can_apply< traits::postfix_decrement_operator, T >;

// ...


//------------------------------------------------------------------------------
/// @defgroup   Generic static checks for SFINAE implementations
///
/// @{

//------------------------------------------------------------------------------
/// @brief      Check if *I* provides a minimal forward iterator-like interface
///
/// @tparam     I     Iterator/class type.
///
/// @return     True if iteratable on a ranged loop, False otherwise.
///
/// @note       STL iterator convention requires at least the increment (++) and dereference (*) operators.
///             (cf. https://cplusplus.com/reference/iterator/).
///
/// @todo       Add similar checks for bidirectional (++ and --) and random-access iterators.
///
template < typename I >
constexpr bool is_forward_iterator_like() {
    return (is_prefix_incrementable< I >() &&
            is_postfix_incrementable< I >() &&
            is_dereferenceable< I >());
}


//------------------------------------------------------------------------------
/// @brief      Check if *C* has a STL-like iterator.
///
/// @tparam     C     Container/class type.
///
/// @return     True if *C* provides an iterator type, false otherwise.
///
template < typename C >
constexpr bool has_iterator() {
    if constexpr (has_iterator_type< C >()) {
        return is_forward_iterator_like< typename C::iterator_type >();
    }
    return false;
}

//------------------------------------------------------------------------------
/// @brief      Check if *C* is range iteratable i.e. can be swept on a a range loop.
///
/// @tparam     C     Container/class type.
///
/// @return     True if *C* is iteratable on a ranged loop, False otherwise.
///
template < typename C >
constexpr bool is_range_iteratable() {
    if constexpr (has_iterator_type< C >()) {
        return (has_begin< C >()  &&
                has_end< C >()    &&
                is_forward_iterator_like< typename C::iterator_type >() &&                 // @note is_forward_iterator_like only chekcs for increment and deference operators
                are_unequal_comparable< typename C::iterator_type, typename C::iterator_type >());  // @note ranged loops require unequal comparison between iterator types
    }
    return false;
}

/// @}

//------------------------------------------------------------------------------

}  // namespace std

#endif  // CPPUTILS_INCLUDE_TYPE_CHECK_HPP_
