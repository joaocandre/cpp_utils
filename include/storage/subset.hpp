//------------------------------------------------------------------------------
/// @file       subset.hpp
/// @author     João André
///
/// @brief      Definition of a container-like subset type template, for use with 2D or 3D containers and
///             to encode different groupings (row, col, diagonal, etc)
///
/// @copyright  Copyright (c) 2019 João André.
///             Subject to the tms of the MIT No Attribution License.
///             All other rights reserved.
///
//------------------------------------------------------------------------------

#ifndef STORAGE_INCLUDE_STORAGE_SUBSET_HPP
#define STORAGE_INCLUDE_STORAGE_SUBSET_HPP

#include <vector>
#include <functional>
#include <cassert>
#include "storage/iterator.hpp"
#include "storage/type_check.hpp"

namespace std {
namespace storage {

//------------------------------------------------------------------------------
/// @brief      Container subset class template
///
/// @note       Iteratable container-like type compatible with the rest of storage types and definitions
///
/// @note       Can also be used generic containers (e.g. std::vector<> or std::array<>), as long as:
///             1) size_t [container]::size() is defined and
///             2) _CT& [container]::operator[](size_t) is overloaded
///
/// @note       By design, subset holds no shape info from  original object (e.g. std::matrix<> or std::volume<>)
///
/// @todo       Replace assert() calls with exception throws.
///
/// @tparam     _CT   Container type
///
template < typename _CT >
class st_subset_base {
    _CT* _container;
    vector< size_t > idx;

 public:
    //--------------------------------------------------------------------------
    /// @brief      Static assertion
    ///
    static_assert(is_generic_container< _CT >(), "_CT IS NOT A GENERIC ITERATABLE CONTAINER");

    //--------------------------------------------------------------------------
    /// @brief      Container element value type
    ///
    typedef typename _CT::value_type value_type;

    //--------------------------------------------------------------------------
    /// @brief      Constructs a new instance.
    ///
    /// @param      container    Source container
    /// @param[in]  idx          Index/positions of elements in subset.
    ///
    st_subset_base(_CT* container, const vector< size_t >& idx);

    //--------------------------------------------------------------------------
    /// @brief      Bulk copy assignment operator.
    ///             All subset elements set to given *in* value.
    ///
    /// @param[in]  in   Input value to assign.
    ///
    /// @return     Modified subset instance.
    ///
    st_subset_base& operator=(const value_type& in);

    //--------------------------------------------------------------------------
    /// @brief      Copy assignment operator (container subsets).
    ///
    /// @note       DEPRECATED. Assignment template overload should be called implicitly.
    ///
    /// @param[in]  sbst  Subset to assign. Dimensions must match.
    ///
    /// @return     Modified subset instance.
    ///
    st_subset_base& operator=(const st_subset_base< _CT >& other);

    //--------------------------------------------------------------------------
    /// @brief      Copy assignment operator (container subsets).
    ///
    /// @note       DEPRECATED. Assignment template overload should be called implicitly.
    ///
    /// @param[in]  in   Input vector to assign.
    ///
    /// @return     Modified subset instance.
    ///
    st_subset_base& operator=(const std::vector< value_type >& in);

    //--------------------------------------------------------------------------
    /// @brief      Copy assignment operator (generic container objects).
    ///
    /// @param[in]  in         Input container to assign.
    ///
    /// @tparam     iT         Input container type.
    /// @tparam     <unnamed>  SFINAE check to ensure template is only valid for generic container types.
    ///                        cf. type_check.hpp
    ///
    /// @return     Modified subset instance.
    ///
    template < typename iT, typename = typename enable_if< is_generic_container< iT >()>::type >
    st_subset_base& operator=(const iT& in);

    //--------------------------------------------------------------------------
    /// @brief      Vector conversion operator template.
    ///
    /// @tparam     oT      Output value type.
    ///
    /// @return     Vector instance with elements converted to oT type.
    ///
    template < typename oT > operator vector< oT >() const;
    // template <typename _oT> operator matrix<_oT>() const;  // DONT DO THIS, it will create ambiguity with function overloads for vector or matrix

    //--------------------------------------------------------------------------
    /// @brief      Positional element acessor.
    ///
    /// @param[in]  pos   Index/position of element (relative to subset elements).
    ///
    /// @return     Reference to element @ given pos.
    ///
    value_type& operator[](size_t pos);
    // auto        operator[](size_t _id)       ->  decltype(std::declval< _CT >()[0]);

    //--------------------------------------------------------------------------
    /// @brief      Positional element acessor (const overload).
    ///
    /// @param[in]  pos   Index/position of element (relative to subset elements).
    ///
    /// @return     Reference to element @ given pos.
    ///
    const value_type& operator[](size_t pos) const;
    // const auto  operator[](size_t _id) const ->  decltype(std::declval< _CT >()[0]);

    //--------------------------------------------------------------------------
    /// @brief      Get source container.
    ///
    /// @return     Pointer to source container
    ///
    /// @todo       Should return a const pointer instead
    ///
    _CT* source() const;

    //--------------------------------------------------------------------------
    /// @brief      Get length of subset.
    ///
    /// @return     Number of elements in subset.
    ///
    size_t size() const;

    //--------------------------------------------------------------------------
    /// @brief      Get length/dimensions of source container
    ///
    /// @return     Number of total elements in source container
    ///
    size_t source_size() const;
    // vector< size_t > shape() const;

    //--------------------------------------------------------------------------
    /// @brief      Get positions of subset elements.
    ///
    /// @return     Index values of each element, relative to source container.
    ///
    const std::vector< size_t >& index() const;

    //--------------------------------------------------------------------------
    /// @brief      Get values of subset elements.
    ///
    /// @return     Values of each element.
    ///
    std::vector< value_type > data() const;

    //--------------------------------------------------------------------------
    /// @brief      Instantiate a new subset instance from elements from *start* to *stop*.
    ///
    /// @note       Redundant, equivalent to segment() member.
    ///
    /// @param[in]  start  Index of first element.
    /// @param[in]  stop   Index of last element.
    ///
    /// @return     Subset instance.
    ///
    st_subset_base range(size_t start, size_t stop = 0) const;

    //--------------------------------------------------------------------------
    /// @brief      Instantiate a new subset instance from elements from *start* to *stop*.
    ///
    /// @note       Redundant, equivalent to range() member.
    ///
    /// @param[in]  start  Index of first element.
    /// @param[in]  stop   Index of last element.
    ///
    /// @return     Subset instance.
    ///
    st_subset_base< _CT > segment(size_t start, size_t stop) const;

    //--------------------------------------------------------------------------
    /// @brief      Get iterator to the beginning of the subset.
    ///
    /// @return     Iterator instance (cf. std::st_pseudo_iterator)
    ///
    st_pseudo_iterator< st_subset_base > begin();

    //--------------------------------------------------------------------------
    /// @brief      Get iterator to the end of the subset.
    ///
    /// @return     Iterator instance (cf. std::st_pseudo_iterator)
    ///
    st_pseudo_iterator< st_subset_base > end();

    //--------------------------------------------------------------------------
    /// @brief      Get iterator to the beginning of the subset (const overload).
    ///
    /// @return     Iterator instance (cf. std::st_pseudo_iterator)
    ///
    st_pseudo_iterator< const st_subset_base > begin() const;

    //--------------------------------------------------------------------------
    /// @brief      Get iterator to the end of the subset (const overload).
    ///
    /// @return     Iterator instance (cf. std::st_pseudo_iterator)
    ///
    st_pseudo_iterator< const st_subset_base > end()   const;
};


//------------------------------------------------------------------------------
/// @cond

template < typename _CT >
st_subset_base< _CT >::st_subset_base(_CT* container, const std::vector< size_t >& idx):  _container(container), idx(idx) {
    for (size_t i : idx) {
        assert(i <= source_size());
    }
}


template < typename _CT >
st_subset_base< _CT >& st_subset_base< _CT >::operator=(const value_type& in) {
    for (auto i : idx) {
        (*_container)[i] = (in);
    }
    return *this;
}


template < typename _CT >
st_subset_base< _CT >& st_subset_base< _CT >::operator=(const st_subset_base< _CT >& other) {
    return operator=<st_subset_base< _CT >>(other);
}


template < typename _CT >
st_subset_base< _CT >& st_subset_base< _CT >::operator=(const std::vector< typename st_subset_base< _CT >::value_type >& in) {
    assert(in.size() >= idx.size());
    size_t ii = 0;
    for (auto i : idx) {
        (*_container)[i] = (in[ii++]);
    }
    return *this;
}


template < typename _CT >
template <typename _iT, typename>
st_subset_base< _CT >& st_subset_base< _CT >::operator=(const _iT& in) {  // container input (elementwise assignment)
    static_assert(is_generic_container<_iT>(), "_iT IS NOT A GENERIC ITERATABLE CONTAINER");
    assert(in.size() >= idx.size());
    size_t ii = 0;
    for (auto i : idx) {
        (*_container)[i] = (in[ii++]);
    }
    return *this;
}


template < typename _CT >
template <typename _oT>
st_subset_base< _CT >::operator std::vector<_oT>() const {
    std::vector<_oT> ss(0);
    for (auto id : idx) ss.push_back((*_container)[id]);
    return ss;
}


// template < typename _CT >
// template <typename _oT>
// st_subset_base< _CT >::operator std::matrix<_oT>() const {
//     // get rows & col indexes of each block element
//     std::vector< std::vector<size_t> > pos;
//     for (auto id : idx) {
//         pos.push_back(container->position(idx));
//     }
//     auto min = pos.back();
//     auto max = pos[0];
//     for (const auto& p : pos) {
//         if (p[0] < min[0]) {
//             min[0] = p[0];
//         }
//         if (p[0] > max[0]) {
//             max[0] = p[0];
//         }
//         if (p[1] < min[1]) {
//             min[1] = p[1];
//         }
//         if (p[1] > max[1]) {
//             max[1] = p[1];
//         }
//     }
//     std::matrix<_oT> out(/* n_rows */ 1 + max[0] - min[0], /* n_cols */ 1 + max[1] - min[1], /* value */ 0.0);
//     for (int i = 0; i < size(); i++) {
//         out(pos[i][0], pos[i][1]) = (*container)[idx[i]];
//     }
//     return out;
// }


template < typename _CT >
typename st_subset_base< _CT >::value_type& st_subset_base< _CT >::operator[](size_t pos) {
    return (*_container)[idx[pos]];
}


// template < typename _CT >
// auto       st_subset_base< _CT >::operator[](size_t _id) ->  decltype(std::declval<_CT>()[0]) {
//     return (*_container)[idx[_id]];
// }


template < typename _CT >
const typename st_subset_base< _CT >::value_type& st_subset_base< _CT >::operator[](size_t pos)  const{
    return (*_container)[idx[pos]];
}


// template < typename _CT >
// const auto st_subset_base< _CT >::operator[](size_t _id)  const ->  decltype(std::declval<_CT>()[0]) {
//     return (*_container)[idx[_id]];
// }


template < typename _CT >
_CT* st_subset_base< _CT >::source()  const {
    return _container;
}


template < typename _CT >
size_t  st_subset_base< _CT >::size() const {
    return idx.size();
}


template < typename _CT >
size_t  st_subset_base< _CT >::source_size() const {
    return (*_container).size();
}


template < typename _CT >
const std::vector<size_t>& st_subset_base< _CT >::index() const {
    return idx;
}


template < typename _CT >
vector<typename st_subset_base< _CT >::value_type> st_subset_base< _CT >::data() const {
    vector<value_type> data(0);
    for (auto& val : (*this)) {
        data.push_back(val);
    }
    return data;
}


template < typename _CT >
st_subset_base< _CT > st_subset_base< _CT >::range(size_t start, size_t stop) const {
    assert(start <= idx.size());
    auto last = stop > start ? stop : idx.size();
    std::vector< size_t > idxs;
    idxs.reserve(last - start);
    for (size_t i = start; i < last; i++) {
        idxs.emplace_back(idx[i]);
    }
    return st_subset_base< _CT >(_container, idxs);
}


template < typename _CT >
st_subset_base< _CT > st_subset_base< _CT >::segment(size_t _f, size_t _l) const {
    assert(_f< _l && _l < size());
    return st_subset_base(_container, std::vector<size_t> (idx.begin()+_f, idx.begin()+_l));
}


template < typename _CT >
st_pseudo_iterator<st_subset_base< _CT >> st_subset_base< _CT >::begin() {
    return st_pseudo_iterator<st_subset_base>(this, 0);
}


template < typename _CT >
st_pseudo_iterator<st_subset_base< _CT >> st_subset_base< _CT >::end() {
    return st_pseudo_iterator<st_subset_base>(this, size());
}


template < typename _CT >
st_pseudo_iterator<const st_subset_base< _CT >> st_subset_base< _CT >::begin() const {
    return st_pseudo_iterator<const st_subset_base>(this, 0);
}


template < typename _CT >
st_pseudo_iterator<const st_subset_base< _CT >> st_subset_base< _CT >::end() const {
    return st_pseudo_iterator<const st_subset_base>(this, size());
}

/// @endcond
//------------------------------------------------------------------------------

}  // namespace storage
}  // namespace std

#endif  // STORAGE_INCLUDE_STORAGE_SUBSET_HPP
