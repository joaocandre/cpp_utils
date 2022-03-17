#ifndef SUBSET_HPP
#define SUBSET_HPP
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <vector>
#include <functional>
#include <cassert>
// #include "storage/matrix.hpp"  // creates a dependency cycle
#include "storage/iterator.hpp"
#include "storage/type_check.hpp"
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace std {
namespace storage {
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// container subset class, templated, iteratable
// can be used generic containers, as long as 1) size_t [container]::size() is defined and 2) _DT& [container]::operator[](size_t) is overloaded
// by design, subset holds no shape info from  original object (e.g. std::matrix<> or std::volume<>)
// TODO: use smart pointers?
template < typename _CT >
class st_subset_base {
    _CT* container;
    vector< size_t > idx;

 public:
    // type member redirection
    typedef typename _CT::value_type value_type;

    // default constructor
    st_subset_base(_CT* _c, const vector<size_t>& _i);
    
    // input assignment operator overload (only data is copied -> for data/member reassignment, call default or copy constructor explicitely)
    st_subset_base& operator=(const value_type& _in);           // single variable (bulk assignment), implicit cast
    
    st_subset_base& operator=(const st_subset_base<_CT>& _sbst);    // copy/move assignment overload is required otherwise a default copy assignment overload is called and members are copied/data is no reassigned

    st_subset_base& operator=(const std::vector< value_type >& _in);  // general container (different container subsets/vectors, etc)
    
    template <typename _iT, typename = typename enable_if<is_generic_container<_iT>()>::type>
    st_subset_base& operator=(const _iT& _in);  // general container (different container subsets/vectors, etc)
    
    st_subset_base range(size_t start, size_t stop = 0) const;

    // output cast operator
    // operator vector<typename _CT::value_type>()    const; //check if it helps
    template <typename _oT> operator vector<_oT>() const;
    // template <typename _oT> operator matrix<_oT>() const;  // DONT DO THIS, it will create ambiguity with function overloads for vector or matrix
    // direct access to data
    auto        operator[](size_t _id)       ->  decltype(std::declval<_CT>()[0]);
    const auto  operator[](size_t _id) const ->  decltype(std::declval<_CT>()[0]);
    // container pointer return
    _CT*        source()            const;
    // index hard limits (const)
    size_t      size()              const;
    size_t      source_size()       const;
    vector<size_t> shape()          const;
    // index accessor
    const vector<size_t>& index()       const;
    vector<value_type> data()       const;  // -> vector<std::remove_reference<decltype(std::declval<_CT>()[0])>::type>;
    // segment subset
    st_subset_base<_CT> segment(size_t _f, size_t _l) const;
    // pseudo-iterators (non-const/const)
    st_pseudo_iterator<st_subset_base> begin();
    st_pseudo_iterator<st_subset_base> end();
    st_pseudo_iterator< const st_subset_base > begin() const;
    st_pseudo_iterator< const st_subset_base > end()   const;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONSTRUCTOR
template <typename _CT>
st_subset_base<_CT>::st_subset_base(_CT* _c, const std::vector<size_t>& _i):  container(_c), idx(_i) {
    static_assert(is_generic_container<_CT>(), "");
    for (size_t i : idx) assert(i <= source_size());
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ASSIGNMENT (CONTAINER MODIFIERS)
template <typename _CT>
st_subset_base<_CT>& st_subset_base<_CT>::operator=(const value_type& _in) {  // single object input (bulk assignment)
    for (auto index : idx) { (*container)[index] = (_in); }
    return *this;
}
template <typename _CT>
st_subset_base<_CT>& st_subset_base<_CT>::operator=(const st_subset_base<_CT>& _sbst) {
    return operator=<st_subset_base<_CT>>(_sbst);
}
template <typename _CT>
st_subset_base<_CT>& st_subset_base<_CT>::operator=(const vector< typename st_subset_base<_CT>::value_type >& _in) {
    assert(_in.size() >= idx.size());
    size_t ind = 0;
    for (auto index : idx) { (*container)[index] = (_in[ind++]); }
    return *this;
}
template <typename _CT>
template <typename _iT, typename>
st_subset_base<_CT>& st_subset_base<_CT>::operator=(const _iT& _in) {  // container input (elementwise assignment)
    static_assert(is_generic_container<_iT>(), "");
    assert(_in.size() >= idx.size());
    size_t ind = 0;
    for (auto index : idx) { (*container)[index] = (_in[ind++]); }
    return *this;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename _CT>
st_subset_base<_CT> st_subset_base<_CT>::range(size_t start, size_t stop) const {
    assert(start <= idx.size());
    auto last = stop > start ? stop : idx.size();
    std::vector< size_t > idxs;
    idxs.reserve(last - start);
    for (size_t i = start; i < last; i++) {
        idxs.emplace_back(idx[i]);
    }
    return st_subset_base<_CT>(container, idxs);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CAST (BULK COPY)
template <typename _CT>
template <typename _oT>
st_subset_base<_CT>::operator std::vector<_oT>() const {
    std::vector<_oT> ss(0);
    for (auto id : idx) ss.push_back((*container)[id]);
    return ss;
}
// template <typename _CT>
// template <typename _oT>
// st_subset_base<_CT>::operator std::matrix<_oT>() const {
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
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DATA ACCESS
template <typename _CT>
auto       st_subset_base<_CT>::operator[](size_t _id) ->  decltype(std::declval<_CT>()[0]) {
    return (*container)[idx[_id]];
};
template <typename _CT>
const auto st_subset_base<_CT>::operator[](size_t _id)  const ->  decltype(std::declval<_CT>()[0]) {
    return (*container)[idx[_id]];
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename _CT>
_CT* st_subset_base<_CT>::source()  const {
    return container;
}
template <typename _CT>
size_t  st_subset_base<_CT>::size() const {
    return idx.size();
}
template <typename _CT>
size_t  st_subset_base<_CT>::source_size() const {
    return (*container).size();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// INDEX POSITIONS
template <typename _CT>
const std::vector<size_t>& st_subset_base<_CT>::index() const {
    return idx;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONTAINER DATA
template <typename _CT>
vector<typename st_subset_base<_CT>::value_type> st_subset_base<_CT>::data() const {
    vector<value_type> data(0);
    for (auto& val : (*this)) data.push_back(val);
    return data;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SEGMENT
template <typename _CT>
st_subset_base<_CT> st_subset_base<_CT>::segment(size_t _f, size_t _l) const {
    assert(_f< _l && _l < size());
    return st_subset_base(container, std::vector<size_t> (idx.begin()+_f, idx.begin()+_l));
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ITERATORS
template <typename _CT>
st_pseudo_iterator<st_subset_base<_CT>> st_subset_base<_CT>::begin() {
    return st_pseudo_iterator<st_subset_base>(this, 0);
}
template <typename _CT>
st_pseudo_iterator<st_subset_base<_CT>> st_subset_base<_CT>::end() {
    return st_pseudo_iterator<st_subset_base>(this, size());
}
template <typename _CT>
st_pseudo_iterator<const st_subset_base<_CT>> st_subset_base<_CT>::begin() const {
    return st_pseudo_iterator<const st_subset_base>(this, 0);
}
template <typename _CT>
st_pseudo_iterator<const st_subset_base<_CT>> st_subset_base<_CT>::end() const {
    return st_pseudo_iterator<const st_subset_base>(this, size());
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}  // namespace storage
}  // namespace std
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif