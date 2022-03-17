#ifndef BOOLEAN_HPP
#define BOOLEAN_HPP
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <storage/type_check.hpp>
#include <storage/comparer.hpp>
#include <storage/subset.hpp>
#include <type_traits>
#include <vector>
#include <functional>
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// templated header-only boolean operators for std::matrix, std::volume
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace std {
namespace storage {
template <typename lCT, typename rCT>
using subset_comparer = st_comparer_base<st_subset_base<lCT>, st_subset_base<rCT>>;
}  // namespace storage
}  // namespace std
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename lCT, typename rCT>
inline std::storage::subset_comparer<lCT, rCT> operator== (std::storage::st_subset_base<lCT>&& _lhs, std::storage::st_subset_base<rCT>&& _rhs) {
    return std::storage::subset_comparer<lCT, rCT> (&_lhs, &_rhs, std::equal_to<typename lCT::value_type>());
}
template <typename lCT, typename rCT>
inline std::storage::subset_comparer<lCT, rCT> operator!= (std::storage::st_subset_base<lCT>&& _lhs, std::storage::st_subset_base<rCT>&& _rhs) {
    return std::storage::subset_comparer<lCT, rCT> (&_lhs, &_rhs, std::not_equal_to<typename lCT::value_type>());
}
template <typename lCT, typename rCT>
inline std::storage::subset_comparer<lCT, rCT> operator<  (std::storage::st_subset_base<lCT>&& _lhs, std::storage::st_subset_base<rCT>&& _rhs) {
    return std::storage::subset_comparer<lCT, rCT> (&_lhs, &_rhs, std::less<typename lCT::value_type>());
}
template <typename lCT, typename rCT>
inline std::storage::subset_comparer<lCT, rCT> operator>  (std::storage::st_subset_base<lCT>&& _lhs, std::storage::st_subset_base<rCT>&& _rhs) {
    return std::storage::subset_comparer<lCT, rCT> (&_lhs, &_rhs, std::greater<typename lCT::value_type>());
    // return operator<(_rhs,_lhs); //types aren't symmetric!
}
template <typename lCT, typename rCT>
inline std::storage::subset_comparer<lCT, rCT> operator<= (std::storage::st_subset_base<lCT>&& _lhs, std::storage::st_subset_base<rCT>&& _rhs) {
    return std::storage::subset_comparer<lCT, rCT> (&_lhs, &_rhs, std::less_equal<typename lCT::value_type>());
    // return !operator>(_rhs,_lhs); //types aren't symmetric!
}
template <typename lCT, typename rCT>
inline std::storage::subset_comparer<lCT, rCT> operator>= (std::storage::st_subset_base<lCT>&& _lhs, std::storage::st_subset_base<rCT>&& _rhs) {
    return std::storage::subset_comparer<lCT, rCT> (&_lhs, &_rhs, std::greater_equal<typename lCT::value_type>());
    // return !operator<(_rhs,_lhs); //types aren't symmetric!
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace std {
namespace storage {
template <typename lCT, typename rCT>
using vector_comparer = st_comparer_base<st_subset_base<lCT>, const vector<rCT>>;
}  // namespace storage
}  // namespace std
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename lCT, typename rCT>
inline std::storage::vector_comparer<lCT, rCT> operator== (std::storage::st_subset_base<lCT>&& _lhs, const std::vector<rCT>& _rhs) {
    return std::storage::vector_comparer<lCT, rCT> (&_lhs, &_rhs, std::equal_to<typename lCT::value_type>());
}
template <typename lCT, typename rCT>
inline std::storage::vector_comparer<lCT, rCT> operator!= (std::storage::st_subset_base<lCT>&& _lhs, const std::vector<rCT>& _rhs) {
    return std::storage::vector_comparer<lCT, rCT> (&_lhs, &_rhs, std::not_equal_to<typename lCT::value_type>());
}
template <typename lCT, typename rCT>
inline std::storage::vector_comparer<lCT, rCT> operator<  (std::storage::st_subset_base<lCT>&& _lhs, const std::vector<rCT>& _rhs) {
    return std::storage::vector_comparer<lCT, rCT> (&_lhs, &_rhs, std::less<typename lCT::value_type>());
}
template <typename lCT, typename rCT>
inline std::storage::vector_comparer<lCT, rCT> operator>  (std::storage::st_subset_base<lCT>&& _lhs, const std::vector<rCT>& _rhs) {
    return std::storage::vector_comparer<lCT, rCT> (&_lhs, &_rhs, std::greater<typename lCT::value_type>());
    // return operator<(_rhs,_lhs); //types aren't symmetric!
}
template <typename lCT, typename rCT>
inline std::storage::vector_comparer<lCT, rCT> operator<= (std::storage::st_subset_base<lCT>&& _lhs, const std::vector<rCT>& _rhs) {
    return std::storage::vector_comparer<lCT, rCT> (&_lhs, &_rhs, std::less_equal<typename lCT::value_type>());
    // return !operator>(_rhs,_lhs); //types aren't symmetric!
}
template <typename lCT, typename rCT>
inline std::storage::vector_comparer<lCT, rCT> operator>= (std::storage::st_subset_base<lCT>&& _lhs, const std::vector<rCT>& _rhs) {
    return std::storage::vector_comparer<lCT, rCT> (&_lhs, &_rhs, std::greater_equal<typename lCT::value_type>());
    // return !operator<(_rhs,_lhs); //types aren't symmetric!
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace std {
namespace storage {
template <typename lCT, typename rDT>
using bulk_comparer = st_bulk_comparer_base<st_subset_base<lCT>, const rDT>;
}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename lCT, typename rDT>
inline std::storage::bulk_comparer<lCT, rDT> operator== (std::storage::st_subset_base<lCT>&& _lhs, const rDT& _rhs) {
    static_assert(std::is_convertible<typename lCT::value_type, rDT>::value, "");  // required as no explicit cast is performed in comparer class
    return std::storage::bulk_comparer<lCT, rDT> (&_lhs, &_rhs, std::equal_to<typename lCT::value_type>());
}
template <typename lCT, typename rDT>
inline std::storage::bulk_comparer<lCT, rDT> operator!= (std::storage::st_subset_base<lCT>&& _lhs, const rDT& _rhs) {
    return std::storage::bulk_comparer<lCT, rDT> (&_lhs, &_rhs, std::not_equal_to<typename lCT::value_type>());
}
template <typename lCT, typename rDT>
inline std::storage::bulk_comparer<lCT, rDT> operator<  (std::storage::st_subset_base<lCT>&& _lhs, const rDT& _rhs) {
    return std::storage::bulk_comparer<lCT, rDT> (&_lhs, &_rhs, std::less<typename lCT::value_type>());
}
template <typename lCT, typename rDT>
inline std::storage::bulk_comparer<lCT, rDT> operator>  (std::storage::st_subset_base<lCT>&& _lhs, const rDT& _rhs) {
    return std::storage::bulk_comparer<lCT, rDT> (&_lhs, &_rhs, std::greater<typename lCT::value_type>());
    // return operator<(_rhs,_lhs); //types aren't symmetric!
}
template <typename lCT, typename rDT>
inline std::storage::bulk_comparer<lCT, rDT> operator<= (std::storage::st_subset_base<lCT>&& _lhs, const rDT& _rhs) {
    return std::storage::bulk_comparer<lCT, rDT> (&_lhs, &_rhs, std::less_equal<typename lCT::value_type>());
    // return !operator>(_rhs,_lhs); //types aren't symmetric!
}
template <typename lCT, typename rDT>
inline std::storage::bulk_comparer<lCT, rDT> operator>= (std::storage::st_subset_base<lCT>&& _lhs, const rDT& _rhs) {
    return std::storage::bulk_comparer<lCT, rDT> (&_lhs, &_rhs, std::greater_equal<typename lCT::value_type>());
    // return !operator<(_rhs,_lhs); //types aren't symmetric!
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// } // boolean
// } // storage
// } // std
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif // BOOLEAN_HPP