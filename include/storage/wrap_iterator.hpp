#ifndef CPPUTILS_INCLUDE_STORAGE_CONSTITERATORWRAPPER_HPP_
#define CPPUTILS_INCLUDE_STORAGE_CONSTITERATORWRAPPER_HPP_

#include <cassert>
#include <iterator>
#include <type_traits>


namespace std {

//------------------------------------------------------------------------------
/// @brief      This class describes a simple *input* and *random access* iterator, that unidirectioally
///             casts the iterator value type to a secondary interface type on de-referencing.
///
/// @tparam     Container  Container type (e.g. std::vector< T >)
/// @tparam     wT         Wrapper type (external interface type e.g. std::shared_ptr< const T >)
///
/// @note       Useful when using smart pointers as container elements, since const-pointer types do not propagate const to the pointed type.
///
/// @note       Alternatively, a custom smart-pointer type could be designed that handles const-corrected better.
///
template < typename Container, typename wT >
class wrap_iterator /*: std::iterator< std::random_access_iterator_tag, T > */ {
 public:
    //--------------------------------------------------------------------------
    /// @brief      Base public types (STL guidelines).
    ///
    /// @note       Required for use of STL iterator utilities e.g. std::next(), std::distance().
    ///
    /// @note       std::iterator deprecated as of C++17; public typedefs defined directly instead of inheriting from std::iterator<>
    ///
    using iterator_category = typename std::random_access_iterator_tag;  // typename std::iterator< std::random_access_iterator_tag, T >::iterator_category;
    using value_type        = wT;                                        // typename std::iterator< std::random_access_iterator_tag, T >::value_type;
    using difference_type   = size_t;                                    // typename std::iterator< std::random_access_iterator_tag, T >::difference_type;
    using pointer           = value_type*;                               // typename std::iterator< std::random_access_iterator_tag, T >::pointer;
    using reference         = value_type&;                               // typename std::iterator< std::random_access_iterator_tag, T >::reference;

    //--------------------------------------------------------------------------
    /// @brief      Extra/custom public types.
    ///
    using wrapped_type      = typename Container::value_type;
    using container_type    = Container;
    using const_reference   = const value_type&;

    //--------------------------------------------------------------------------
    /// @brief      Static assertion on data types/template arguments.
    ///             Required in order to avoid ill-formed expressions within class definition.
    ///
    /// @todo       Check if there is any need for bidirection convertible requirement, only value_type->T should be required (!)
    ///
    static_assert(is_convertible< wrapped_type, value_type >(),
                  "UNDERLYING CONTAINER ELEMENT TYPE [Container::value_type] MUST BE IMPLICITLY CONVERTIBLE!");

    //--------------------------------------------------------------------------
    /// @brief      Constructs a new instance.
    ///
    /// @param      container  Container
    /// @param[in]  pos        Initial iterator position w.r.t. container.
    ///
    explicit wrap_iterator(Container* container, unsigned int pos = 0);

    //--------------------------------------------------------------------------
    /// @brief      Increment operator (prefix).
    ///
    wrap_iterator& operator++();

    //--------------------------------------------------------------------------
    /// @brief      Increment operator (postfix).
    ///
    wrap_iterator operator++(int);

    //--------------------------------------------------------------------------
    /// @brief      Decrement operator (prefix).
    ///
    wrap_iterator& operator--();

    //--------------------------------------------------------------------------
    /// @brief      Decrement operator postfixy).
    ///
    wrap_iterator operator--(int);

    //--------------------------------------------------------------------------
    /// @brief      Compound addition operator. Increments the iterator *n* times.
    ///
    /// @param[in]  n     Number of increments.
    ///
    wrap_iterator& operator+=(const difference_type& n);

    //--------------------------------------------------------------------------
    /// @brief      Compound subtraction operator. Decrements the iterator *n* times.
    ///
    /// @param[in]  n     Number of decrements.
    ///
    wrap_iterator& operator-=(const difference_type& n);

    //--------------------------------------------------------------------------
    /// @brief      Dereference operator.
    ///
    /// @return     Reference to container value @ current iterator position.
    ///
    /// @note       Converts underlying value_type to wrapped type T
    ///
    value_type operator*();

    //--------------------------------------------------------------------------
    /// @brief      Dereference operator (*const* overload).
    ///
    /// @return     Reference to container value @ current iterator position.
    ///
    /// @note       Converts underlying value_type to wrapped type T .
    ///
    const value_type operator*() const;

    //--------------------------------------------------------------------------
    /// @brief      Conversion operator to iterator of the underlying container type.
    ///
    operator typename Container::iterator();

    //--------------------------------------------------------------------------
    /// @brief      Conversion operator to *const* iterator of the underlying container type.
    ///
    operator typename Container::const_iterator() const;

    //--------------------------------------------------------------------------
    /// @brief      Conversion operator to different wrapped type.
    ///
    /// @note       Also captures conversions to *const* cast iterators i.e. wrap_iterator< const Container, const T >
    ///
    template < typename owT, typename = typename enable_if< is_convertible< typename Container::value_type, owT >::value >::type >
    operator wrap_iterator< Container, owT >();

    //--------------------------------------------------------------------------
    /// @brief      Conversion operator to different wrapped type (*const* overload).
    ///
    /// @note       Also captures conversions to *const* cast iterators i.e. wrap_iterator< const Container, const T >
    ///
    template < typename owT, typename = typename enable_if< is_convertible< typename Container::value_type, owT >::value >::type >
    operator wrap_iterator< Container, owT >() const;

    //--------------------------------------------------------------------------
    /// @brief      Equality operator.
    ///
    /// @param[in]  lhs  Left operand (iterator to compare).
    /// @param[in]  rhs  Right operand (iterator to compare).
    ///
    /// @return     True if iterators @ same position w.r.t container, false othewise.
    ///
    template < typename oContainer, typename lwT, typename rwT >
    friend bool operator==(const wrap_iterator< oContainer, lwT >& lhs, const wrap_iterator< oContainer, rwT >& rhs);

    //------------------------------------------------------------------------------
    /// @brief      'Less than' comparison operator.
    ///
    /// @param[in]  lhs  Left operand (iterator to compare).
    /// @param[in]  rhs  Right operand (iterator to compare).
    ///
    /// @return     True if *lhs* @ previous position w.r.t container than *rhs*, false othewise.
    ///
    template < typename lwT, typename rwT >
    friend bool operator<(const wrap_iterator< Container, lwT >& lhs, const wrap_iterator< Container, rwT >& rhs);

    //--------------------------------------------------------------------------
    /// @brief      Subtraction operator.
    ///
    /// @param[in]  lhs  Left operand (iterator to subtract to).
    /// @param[in]  rhs  Right operand (iterator to subtract).
    ///
    /// @return     Difference between iterator positions.
    ///
    /// @note       Required as part of STL *RandomAccessIterator* requirements, allows use of STL iterator utilities with wrap_iterator.
    ///
    template < typename lwT, typename rwT >
    friend typename wrap_iterator< Container, lwT >::difference_type operator-(const wrap_iterator< Container, lwT >& lhs, const wrap_iterator< Container, rwT >& rhs);

 protected:
    //--------------------------------------------------------------------------
    /// @brief      Iterator position.
    ///
    unsigned int _pos;

    //--------------------------------------------------------------------------
    /// @brief      Associated container instance.
    ///
    Container* _container;
};




//--------------------------------------------------------------------------
/// @cond

template < typename Container, typename wT >
wrap_iterator< Container, wT >::wrap_iterator(Container* container, unsigned int pos) :
    _container(container),
    _pos(pos) {
        /* ... */
        assert(pos <= _container->size());
}



template < typename Container, typename wT >
wrap_iterator< Container, wT >& wrap_iterator< Container, wT >::operator++() {
    if (_pos < _container->size()) { _pos++; } return *this;
}



template < typename Container, typename wT >
wrap_iterator< Container, wT > wrap_iterator< Container, wT >::operator++(int) {
    wrap_iterator< Container, wT > retval = *this;
    ++(*this);
    return retval;
}



template < typename Container, typename wT >
wrap_iterator< Container, wT >& wrap_iterator< Container, wT >::operator--() {
    if (_pos > 0) {
        _pos--;
    }
    return *this;
}



template < typename Container, typename wT >
wrap_iterator< Container, wT > wrap_iterator< Container, wT >::operator--(int) {
    wrap_iterator< Container, wT > retval = *this;
    --(*this);
    return retval;
}



template < typename Container, typename wT >
wrap_iterator< Container, wT >& wrap_iterator< Container, wT >::operator+=(const typename wrap_iterator< Container, wT >::difference_type& n) {
    if (static_cast< int >(_pos + n) > static_cast<int>(_container->size()) ) {
        _pos = _container->size();
    } else if (static_cast< int >(_pos + n) < 0) {
        _pos = 0;
    } else {
        _pos += n;
    }
    return *this;
}



template < typename Container, typename wT >
wrap_iterator< Container, wT >& wrap_iterator< Container, wT >::operator-=(const typename wrap_iterator< Container, wT >::difference_type& n) {
    if (static_cast< int >(_pos - n) > static_cast<int>(_container->size()) ) {
        _pos = _container->size();
    } else if (static_cast< int >(_pos - n) < 0) {
        _pos = 0;
    } else {
        _pos -= n;
    }
    return *this;
}



template < typename Container, typename wT >
typename wrap_iterator< Container, wT >::value_type wrap_iterator< Container, wT >::operator*() {
    // return static_cast< reference >(_container->at(_pos));
    return _container->at(_pos);  // cast must be implicit (!)
}



template < typename Container, typename wT >
const typename wrap_iterator< Container, wT >::value_type wrap_iterator< Container, wT >::operator*() const {
    // return static_cast< const_reference >(_container->at(_pos));
    return _container->at(_pos);   // cast must be implicit (!)
}



template < typename Container, typename wT >
wrap_iterator< Container, wT >::operator typename Container::iterator() {
    return (_container->begin() + _pos);
}



template < typename Container, typename wT >
wrap_iterator< Container, wT >::operator typename Container::const_iterator() const {
    return (_container->cbegin() + _pos);
}



template < typename Container, typename wT >
template < typename owT, typename >
wrap_iterator< Container, wT >::operator wrap_iterator< Container, owT >() {
    return wrap_iterator< Container, owT >(_container, _pos);
}



template < typename Container, typename wT >
template < typename owT, typename >
wrap_iterator< Container, wT >::operator wrap_iterator< Container, owT >() const {
    // static_assert(is_const< oContainer >(), "CONVERSION FROM CV-QUALIFIED OBJECTS TO NON-CONST TYPES NOT ALLOWED!");
    static_assert(is_const< owT >(), "CONVERSION FROM CV-QUALIFIED OBJECTS TO NON-CONST TYPES NOT ALLOWED!");
    return wrap_iterator< Container, owT >(_container, _pos);
}



template < typename Container, typename lwT, typename rwT >
bool operator==(const wrap_iterator< Container, lwT >& lhs, const wrap_iterator< Container, rwT >& rhs) {
    return  ((lhs._container == rhs._container) && (lhs._pos == rhs._pos));
}



template < typename Container, typename lwT, typename rwT >
bool operator< (const wrap_iterator< Container, lwT >& lhs, const wrap_iterator< Container, rwT >& rhs) {
    return ((lhs._container == rhs._container) && (lhs._pos < rhs._pos));
}



template < typename Container, typename lwT, typename rwT >
typename wrap_iterator< Container, lwT >::difference_type operator-(const wrap_iterator< Container, lwT >& lhs, const wrap_iterator< Container, rwT >& rhs) {
    return (lhs._pos - rhs._pos);
}

/// @endcond
//--------------------------------------------------------------------------


//------------------------------------------------------------------------------
/// @brief      Inequality operator.
///
/// @param[in]  lhs  Left operand (iterator to compare).
/// @param[in]  rhs  Right operand (iterator to compare).
///
/// @return     True if iterators *not* @ same position w.r.t container, false othewise.
///
/// @note       Wraps around equality operator, thus does not need to be friend to cast_iterator<>.
///
template < typename Container, typename T >
bool operator!=(const wrap_iterator< Container, T >& lhs, const wrap_iterator< Container, T >& rhs) {
    return !(lhs == rhs);
}



//------------------------------------------------------------------------------
/// @brief      'Greater than' comparison operator.
///
/// @param[in]  lhs  Left operand (iterator to compare).
/// @param[in]  rhs  Right operand (iterator to compare).
///
/// @return     True if *lhs* @ later position w.r.t container than *rhs*, false othewise.
///
/// @note       Wraps around '<' operator, thus does not need to be friend to cast_iterator<>.
///
template < typename Container, typename T >
bool operator> (const wrap_iterator< Container, T >& lhs, const wrap_iterator< Container, T >& rhs) {
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
/// @note       Wraps around '>' operator, thus does not need to be friend to cast_iterator<>.
///
template < typename Container, typename T >
inline bool operator<=(const wrap_iterator< Container, T >& lhs, const wrap_iterator< Container, T >& rhs) {
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
/// @note       Wraps around '<' operator, thus does not need to be friend to cast_iterator<>.
///
template < typename Container, typename T >
inline bool operator>=(const wrap_iterator< Container, T >& lhs, const wrap_iterator< Container, T >& rhs) {
    return !operator< (lhs, rhs);
}



//------------------------------------------------------------------------------
/// @brief      Addition operator, between an iterator and difference_type (number of positions).
///
/// @param[in]  lhs  Left operand (iterator).
/// @param[in]  n    Number of position increments.
///
/// @return     Copy of lhs with given position increments.
///
/// @note       Wraps around member compound addition operator overload.
///
template < typename Container, typename T >
wrap_iterator< Container, T > operator+(wrap_iterator< Container, T > lhs, const typename wrap_iterator< Container, T >::difference_type& n) {
    lhs += n;
    return lhs;
}



//------------------------------------------------------------------------------
/// @brief      Subtraction operator, between an iterator and difference_type (number of positions).
///
/// @param[in]  lhs  Left operand (iterator).
/// @param[in]  n    Number of position drecrements.
///
/// @return     Copy of lhs with given position decrements.
///
/// @note       Wraps around member compound addition operator overload.
///
template < typename Container, typename T >
wrap_iterator< Container, T > operator-(wrap_iterator< Container, T > lhs, const typename wrap_iterator< Container, T >::difference_type& n) {
    lhs += n;
    return lhs;
}


}  // namespace std

#endif   // CPPUTILS_INCLUDE_STORAGE_CONSTITERATORWRAPPER_HPP_
