//------------------------------------------------------------------------------
/// @file       range_iterator.hpp
/// @author     João André
///
/// @brief      Header file providing declaration & definition of std::range_iterator<> class template.
///
/// std::range_iterator<> is a random access iterator class template that derives from std::iterator<> and extends basic iterator functionality to
/// 'sliding' range (element window) i.e. iterates over a container with customizable step size, and can itself be iterated with the underlying container
/// iterators. Intended to represent a contiguous range of values on a generic container, and can be useful in batch operations over input data.
///
//------------------------------------------------------------------------------

#ifndef TRIGNOCLIENT_INCLUDE_TRIGNOCLIENT_STD_RANGEITERATOR_HPP_
#define TRIGNOCLIENT_INCLUDE_TRIGNOCLIENT_STD_RANGEITERATOR_HPP_

#include <string>
#include <cassert>
#include <exception>
#include <iterator>
#include <type_traits>
#include "cast_iterator.hpp"   // std::cast_iterator<>

namespace std {

//------------------------------------------------------------------------------
/// @brief      Class that implements an *iterable* range iterator.
///             Behaves like a random access iterator while also providing a basic container-like public interface to iterate over container subset.
///
/// @tparam     Container  Container type to iterator over.
/// @tparam     T          Underlying element type. Defaults to Container::value_type.
///
/// @todo       Use cast_iterator< Container, T > as value iterator instead of iterator from underlying Container to force public T interface.
///             At the moment, range-based loops (iterator dereferencing) & direct access return different types!
///
template < typename Container, typename T = typename Container::value_type >
class range_iterator : std::iterator< std::random_access_iterator_tag, T > {
 public:
    //--------------------------------------------------------------------------
    /// @brief      Base public types.
    ///
    /// @note       Required for use of STL iterator utilities e.g. std::next(), std::distance().
    ///
    using value_type           = typename std::iterator< std::random_access_iterator_tag, T >::value_type;
    using iterator_category    = typename std::iterator< std::random_access_iterator_tag, T >::iterator_category;
    using pointer              = typename std::iterator< std::random_access_iterator_tag, T >::pointer;
    using reference            = typename std::iterator< std::random_access_iterator_tag, T >::reference;
    using const_reference      = const reference;
    using difference_type      = typename std::iterator< std::random_access_iterator_tag, T >::difference_type;
    // using value_iterator       = typename Container::iterator;
    // using value_const_iterator = typename Container::const_iterator;
    using value_iterator       = typename std::cast_iterator< Container, T >;
    using value_const_iterator = typename std::cast_iterator< const Container, const T >;

    //--------------------------------------------------------------------------
    /// @brief      Static assertion on data types/template arguments.
    ///             Required in order to avoid ill-formed expressions within class definition.
    ///
    static_assert(is_convertible< typename Container::value_type, T >() && is_convertible< T, typename Container::value_type >(),
                  "UNDERLYING CONTAINER ELEMENT TYPE [Container::value_type] MUST BE IMPLICITLY CONVERTIBLE!");

    //--------------------------------------------------------------------------
    /// @brief      Constructs a new instance.
    ///
    /// @param      container   Source container.
    /// @param[in]  pos         Initial iterator position w.r.t. container. Defaults to 0 (first element).
    /// @param[in]  range_size  Range size/width. Defaults to 1 (single element iteration).
    /// @param[in]  overlap     Overlap size (number of elements) between consecutive ranges. Defaults to 0 (no overlap).
    ///
    /// @todo       Allow negative values for *pos*, place iterator @ end of input container
    ///
    explicit range_iterator(Container* container, unsigned int pos     = 0,
                                                  unsigned int width   = 1,
                                                  unsigned int overlap = 0);

    //--------------------------------------------------------------------------
    /// @brief      Increment operator (prefix).
    ///
    range_iterator& operator++();

    //--------------------------------------------------------------------------
    /// @brief      Increment operator (postfix).
    ///
    range_iterator operator++(int);

    //--------------------------------------------------------------------------
    /// @brief      Decrement operator (prefix).
    ///
    range_iterator& operator--();

    //--------------------------------------------------------------------------
    /// @brief      Decrement operator (postfix).
    ///
    range_iterator operator--(int);

    //--------------------------------------------------------------------------
    /// @brief      Range size/width.
    ///
    typename Container::size_type size() const noexcept;

    //--------------------------------------------------------------------------
    /// @brief      Conpound addition operator. Increments the iterator *n* times (skips *n* ranges).
    ///
    /// @param[in]  n     Number of increments.
    ///
    range_iterator< Container, T >& operator+=(const difference_type& n);

    //--------------------------------------------------------------------------
    /// @brief      Conpound subtraction operator. Decrements the iterator *n* times (skips *n* ranges).
    ///
    /// @param[in]  n     Number of decrements.
    ///
    range_iterator< Container, T >& operator-=(const difference_type& n);

    //--------------------------------------------------------------------------
    /// @brief      Dereference operator.
    ///
    /// @return     Reference to container value @ pos.
    ///
    reference operator*();

    //--------------------------------------------------------------------------
    /// @brief      Dereference operator.
    ///
    /// @return     Reference to container value @ pos.
    ///
    const T& operator*() const;

    //--------------------------------------------------------------------------
    /// @brief      Dereference operator.
    ///
    /// @return     Reference to container value @ pos.
    ///
    T& operator[](const typename Container::size_type pos);

    //--------------------------------------------------------------------------
    /// @brief      Dereference operator.
    ///
    /// @return     Reference to container value @ pos.
    ///
    const T& operator[](const typename Container::size_type pos) const;

    //--------------------------------------------------------------------------
    /// @brief      Iterator to range start.
    ///
    value_iterator begin();

    //--------------------------------------------------------------------------
    /// @brief      Iterator to range end.
    ///
    value_iterator end();

    //--------------------------------------------------------------------------
    /// @brief      Iterator (const overload) to range start.
    ///
    value_const_iterator begin() const;

    //--------------------------------------------------------------------------
    /// @brief      Iterator (const overload) to range end.
    ///
    value_const_iterator end() const;

    //--------------------------------------------------------------------------
    /// @brief      Checks if first possible range in input container.
    ///
    /// @return     True if iterator can be decremented, false otherwise.
    ///
    bool first() const;

    //--------------------------------------------------------------------------
    /// @brief      Checks if last possible range in input container.
    ///
    /// @return     True if iterator can be incremented, false otherwise.
    ///
    bool last() const;

    //--------------------------------------------------------------------------
    /// @brief      Conversion operator for the underlying value iterator type (begin()).
    ///
    /// @note       Redundant, same functionality of beign(), only provides implicit conversion!
    ///
    operator value_iterator();

    //--------------------------------------------------------------------------
    /// @brief      Conversion operator for the underlying value iterator type (const overload) (begin())
    ///
    operator value_const_iterator() const;

    //--------------------------------------------------------------------------
    /// @brief      Conversion operator for a range of a different value type
    ///
    // template < typename oT, typename = typename enable_if< is_convertible< T, oT >::value >::type >
    // operator range_iterator< Container, oT >();

    //--------------------------------------------------------------------------
    /// @brief      Conversion operator for a range of a different container/value type.
    ///
    /// @note       Also captures conversions to *const* range iterators i.e. range_iterator< const Container, const T >
    ///
    template < typename oContainer, typename oT, typename = typename enable_if< is_convertible< T, oT >::value >::type >
    operator range_iterator< oContainer, oT >() const;

    //--------------------------------------------------------------------------
    /// @brief      Equality operator.
    ///
    /// @param[in]  lhs  Left operand (iterator to compare).
    /// @param[in]  rhs  Right operand (iterator to compare).
    ///
    /// @return     True if iterators @ same position w.r.t container AND range width are the same, false othewise.
    ///
    template < typename oContainer, typename oT >
    friend bool operator==(const range_iterator< oContainer, oT >& lhs, const range_iterator< oContainer, oT >& rhs);

    //--------------------------------------------------------------------------
    /// @brief      'Less than' comparison operator.
    ///
    /// @param[in]  lhs  Left operand (iterator to compare).
    /// @param[in]  rhs  Right operand (iterator to compare).
    ///
    /// @return     True if *lhs* @ previous position w.r.t container than *rhs*, false othewise (width is ignored).
    ///
    template < typename oContainer, typename oT >
    friend bool operator<(const range_iterator< oContainer, oT >& lhs, const range_iterator< oContainer, oT >& rhs);

    //--------------------------------------------------------------------------
    /// @brief      Subtraction operator.
    ///
    /// @param[in]  lhs  Left operand (iterator to subtract to).
    /// @param[in]  rhs  Right operand (iterator to subtract).
    ///
    /// @return     Difference between iterator positions.
    ///
    /// @note       Required as part of STL *RandomAccessIterator* requirements, allows use of STL iterator utilities with range_iterator<>.
    ///
    template < typename oContainer, typename oT >
    friend typename range_iterator< oContainer, oT >::difference_type operator-(const range_iterator< oContainer, oT >& lhs, const range_iterator< oContainer, oT >& rhs);

 protected:
    //--------------------------------------------------------------------------
    /// @brief      Iterator position (@ begin of range).
    ///
    unsigned int _pos;

    //--------------------------------------------------------------------------
    /// @brief      Range width.
    ///
    unsigned int _width;

    //--------------------------------------------------------------------------
    /// @brief      Increment step.
    ///
    unsigned int _step;

 public:
    //--------------------------------------------------------------------------
    /// @brief      Associated container instance.
    ///
    Container* _container;
};



//--------------------------------------------------------------------------
/// @cond

template < typename Container, typename T >
range_iterator< Container, T >::range_iterator(Container* container, unsigned int pos, unsigned int width, unsigned int overlap) :
    _container(container),
    _pos(pos),
    _width(width) {
        if (_container != nullptr) {
            // runtime assertions, required for proper function of range_iterator
            assert(pos >= 0 && pos <= _container->size());
            assert(width > 0 && width > overlap);
            // compute increment/decrement step
            _step = width - overlap;
            // printf("Iterator [%lu, %lu, %lu] -> src %p\n", _pos, _width, _step, _container);
        } else {
            // printf("Placeholder/dummy iterator! [%lu, %lu, %lu]\n", _pos, _width, _step);
        }
}



template < typename Container, typename T >
range_iterator< Container, T >& range_iterator< Container, T >::operator++() {
    if (_container == nullptr) {
        throw std::runtime_error("[" + std::string(__func__) + "] Dummy/placeholder range_iterators can't be incremented!");
    }
    if ((_pos + _step) <= _container->size()) {
        _pos += _step;
    }
    return *this;
}



template < typename Container, typename T >
range_iterator< Container, T > range_iterator< Container, T >::operator++(int) {
    range_iterator< Container, T > retval = *this;
    ++(*this);
    return retval;
}



template < typename Container, typename T >
range_iterator< Container, T >& range_iterator< Container, T >::operator--() {
    if (static_cast< int >(_pos - _step) > 0) {
        _pos -= _step;
    }
    return *this;
}



template < typename Container, typename T >
range_iterator< Container, T > range_iterator< Container, T >::operator--(int) {
    range_iterator< Container, T > retval = *this;
    --(*this);
    return retval;
}



template < typename Container, typename T >
range_iterator< Container, T >& range_iterator< Container, T >::operator+=(const typename range_iterator< Container, T >::difference_type& n) {
    if (static_cast< int >(_pos + (n * _step)) <= _container->size()) {
        _pos += (n * _step);
    }
    return *this;
}



template < typename Container, typename T >
range_iterator< Container, T >& range_iterator< Container, T >::operator-=(const typename range_iterator< Container, T >::difference_type& n) {
    if (static_cast< int >(_pos - (n * _step)) > 0) {
        _pos -= (n * _step);
    }
    return *this;
}



template < typename Container, typename T >
typename Container::size_type range_iterator< Container, T >::size() const noexcept {
    return std::distance(this->begin(), this->end());
}



template < typename Container, typename T >
typename range_iterator< Container, T >::reference range_iterator< Container, T >::operator*() {
    return _container->template at< reference >(_pos);
}



template < typename Container, typename T >
const T& range_iterator< Container, T >::operator*() const {
    return _container->at(_pos);
}



template < typename Container, typename T >
T& range_iterator< Container, T >::operator[](const typename Container::size_type pos) {
    if ((pos + _pos) < _container->size()) {
        return (_container->at(_pos + pos));
    }
    throw std::out_of_range("[" + std::string(__func__) + "] Invalid position!");
}



template < typename Container, typename T >
const T& range_iterator< Container, T >::operator[](const typename Container::size_type pos) const {
    if ((pos + _pos) < _container->size()) {
        return (_container->at(_pos + pos));
    }
    throw std::out_of_range("[" + std::string(__func__) + "] Invalid position!");
}



template < typename Container, typename T >
typename range_iterator< Container, T >::value_iterator range_iterator< Container, T >::begin() {
    return value_iterator(_container, _pos);
}



template < typename Container, typename T >
typename range_iterator< Container, T >::value_iterator range_iterator< Container, T >::end() {
    if (_pos + _width > _container->size()) {
        return value_iterator(_container, _container->size());
    }
    return value_iterator(_container, _pos + _width);
}



template < typename Container, typename T >
typename range_iterator< Container, T >::value_const_iterator range_iterator< Container, T >::begin() const {
    // @note       here, _container is not const (?), cbegin() needs to be called explicitely!
    return value_const_iterator(_container, _pos);
    // return (_container->cbegin() + _pos);
}



template < typename Container, typename T >
typename range_iterator< Container, T >::value_const_iterator range_iterator< Container, T >::end() const {
    // @note       here, _countainer is not const (?), cbegin() needs to be called explicitely!
    if (_pos + _width > _container->size()) {
        return value_const_iterator(_container, _container->size());
        // return _container->cend();
    }
    return value_const_iterator(_container, _pos + _width);
    // return (_container->cbegin() + _pos + _width);
}



template < typename Container, typename T >
bool range_iterator< Container, T >::first() const {
    return ((*this) == (*this - 1));
}



template < typename Container, typename T >
bool range_iterator< Container, T >::last() const {
    return ((*this) == (*this + 1));
}



template < typename Container, typename T >
range_iterator< Container, T >::operator range_iterator< Container, T >::value_iterator() {
    return begin();
}



template < typename Container, typename T >
range_iterator< Container, T >::operator range_iterator< Container, T >::value_const_iterator() const {
    return begin();
}



// template < typename Container, typename T >
// template < typename oT, typename >
// range_iterator< Container, T >::operator range_iterator< Container, oT >() {
//     return range_iterator< Container, oT >(_container, _pos, _width,  _width - _step);
// }



template < typename Container, typename T >
template < typename oContainer, typename oT, typename >
range_iterator< Container, T >::operator range_iterator< oContainer, oT >() const {
    return range_iterator< oContainer, oT >(_container, _pos, _width,  _width - _step);
}

///  @endcond


//------------------------------------------------------------------------------
/// @brief      Equality operator.
///
/// @param[in]  lhs  Left operand (iterator to compare).
/// @param[in]  rhs  Right operand (iterator to compare).
///
/// @return     True if iterators @ same position w.r.t container AND range width are the same, false othewise.
///
template < typename oContainer, typename oT >
bool operator==(const range_iterator< oContainer, oT >& lhs, const range_iterator< oContainer, oT >& rhs) {
    return  ((lhs._container == rhs._container) && (lhs._pos == rhs._pos) && (lhs._width == rhs._width));
}



//------------------------------------------------------------------------------
/// @brief      Inequality operator.
///
/// @param[in]  lhs  Left operand (iterator to compare).
/// @param[in]  rhs  Right operand (iterator to compare).
///
/// @return     True if iterators *not* @ same position w.r.t container, or size/widrth are different, false othewise.
///
/// @note       Wraps around equality operator, thus does not need to be friend to range_iterator<>.
///
template < typename Container, typename T >
bool operator!=(const range_iterator< Container, T >& lhs, const range_iterator< Container, T >& rhs) {
    return !(lhs == rhs);
}



//------------------------------------------------------------------------------
/// @brief      'Less than' comparison operator.
///
/// @param[in]  lhs  Left operand (iterator to compare).
/// @param[in]  rhs  Right operand (iterator to compare).
///
/// @return     True if *lhs* @ previous position w.r.t container than *rhs*, false othewise (width is ignored).
///
template < typename oContainer, typename oT >
bool operator< (const range_iterator< oContainer, oT >& lhs, const range_iterator< oContainer, oT >& rhs) {
    return ((lhs._container == rhs._container) && (lhs._pos < rhs._pos));
}



//------------------------------------------------------------------------------
/// @brief      'Greater than' comparison operator.
///
/// @param[in]  lhs  Left operand (iterator to compare).
/// @param[in]  rhs  Right operand (iterator to compare).
///
/// @return     True if *lhs* @ later position w.r.t container than *rhs*, false othewise.
///
/// @note       Wraps around '<' operator, thus does not need to be friend to range_iterator<>.
///
template < typename Container, typename T >
bool operator> (const range_iterator< Container, T >& lhs, const range_iterator< Container, T >& rhs) {
    return operator< (rhs, lhs);
}



//------------------------------------------------------------------------------
/// @brief      'Greater or equal than' comparison operator.
///
/// @param[in]  lhs  Left operand (iterator to compare).
/// @param[in]  rhs  Right operand (iterator to compare).
///
/// @return     True if *lhs* @ previous or equal position w.r.t container than *rhs*, false othewise.
///
/// @note       Wraps around '>' operator, thus does not need to be friend to range_iterator<>.
///
template < typename Container, typename T >
inline bool operator<=(const range_iterator< Container, T >& lhs, const range_iterator< Container, T >& rhs) {
    return !operator> (lhs, rhs);
}



//------------------------------------------------------------------------------
/// @brief      'Greater or equal than' comparison operator.
///
/// @param[in]  lhs  Left operand (iterator to compare).
/// @param[in]  rhs  Right operand (iterator to compare).
///
/// @return     True if *lhs* @ later or equal position w.r.t container than *rhs*, false othewise.
///
/// @note       Wraps around '<' operator, thus does not need to be friend to range_iterator<>.
///
template < typename Container, typename T >
inline bool operator>=(const range_iterator< Container, T >& lhs, const range_iterator< Container, T >& rhs) {
    return !operator< (lhs, rhs);
}



template < typename oContainer, typename oT >
typename range_iterator< oContainer, oT >::difference_type operator-(const range_iterator< oContainer, oT >& lhs, const range_iterator< oContainer, oT >& rhs) {
    return (lhs._pos - rhs._pos);
}



template < typename Container, typename T >
range_iterator< Container, T > operator+(range_iterator< Container, T > lhs, const typename range_iterator< Container, T >::difference_type& n) {
    // delegate to member compound addition
    lhs += n;
    return lhs;
}



template < typename Container, typename T >
range_iterator< Container, T > operator-(range_iterator< Container, T > lhs, const typename range_iterator< Container, T >::difference_type& n) {
    // delegate to member compound subtraction
    lhs -= n;
    return lhs;
}

}  // namespace std

#endif  // TRIGNOCLIENT_INCLUDE_TRIGNOCLIENT_CASTITERATOR_HPP_
