//------------------------------------------------------------------------------
/// @file       tagged.hpp
/// @author     João André
///
/// @brief      Header file providing declaration & definition of std::tagged<> class template.
///
/// std::tagged<> is a very thin wrapper around an object, that provides a public id key for indexing/identification purposes.
/// Similar to std::pair<> or std::tuple<>, but with the significant caveat that it is implicitly constructibe from and convertible to
/// the underlying object type, which means it can be used interchangeably in most situations.
///
//------------------------------------------------------------------------------

#ifndef TRIGNOCLIENT_INCLUDE_TRIGNOCLIENT_TAGGED_HPP_
#define TRIGNOCLIENT_INCLUDE_TRIGNOCLIENT_TAGGED_HPP_

#include <string>
#include <memory>
#include <iostream>
#include <utility>
#include <type_traits>

namespace std {

//------------------------------------------------------------------------------
/// @cond

//------------------------------------------------------------------------------
/// @brief      Struct providing generic implementation of static check on type *T* regarding if it is a STL smart pointer type.
///
/// @tparam     T     Type to check.
///
/// @note       A class/struct is used because functions can't be partially specialized.
///
template < typename T = void >
struct is_smart_ptr_impl {
    //--------------------------------------------------------------------------
    /// @brief      Function call operator.
    ///
    /// @return     False (placeholder implementation for non-smart pointer types)
    ///
    constexpr bool operator()() { return false; }
};

template < typename T, template < typename > typename Template >
struct is_smart_ptr_impl< Template< T > > {
    //--------------------------------------------------------------------------
    /// @brief      Function call operator.
    ///
    /// @return     True if *T* is one of std::shared_ptr, std::unique_ptr or std::weak_ptr, False otherwise
    ///
    constexpr bool operator()() {
        return (std::is_base_of< std::shared_ptr< T >, Template< T > >() ||
                std::is_base_of< std::unique_ptr< T >, Template< T > >() ||
                std::is_base_of< std::weak_ptr< T >, Template< T > >());
    }
};

//------------------------------------------------------------------------------
/// @brief      Wraps is_smart_ptr_impl<> class around a constexpr function returning a boolean.
///
/// @tparam     T     Type to check.
///
/// @return     True if *T* is one of std::shared_ptr, std::unique_ptr or std::weak_ptr, False otherwise
///
template < typename T >
constexpr bool is_smart_ptr_wrapper() {
    return is_smart_ptr_impl< T >()();
}

//------------------------------------------------------------------------------
/// @brief      Type providing STL's type_traits' interface to smart pointer check.
///             Inherits from std::true_type or std::false_type according to whether *T* is a STL smart pointer.
///
/// @tparam     T     Type to check.
///
template < typename T >
struct is_smart_ptr : std::integral_constant< bool, is_smart_ptr_wrapper< T >() > { /* ... */ };

/// @endcond
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
/// @brief      Associative type template, indented to add a key to type T, wherein a key 'tag' is attached to an object.
///
/// @tparam     T    Underlying value type.
/// @tparam     Key  Key/tag type.
///
/// @note       Similar to std::pair<>, but prioritizing T over the key: from the outside it behaves like T, is implicitly convertible to T
///             (and therefore can be used interchangeably), but provides an additional key member, publicly accessible & modifiable.
///
/// @note       Design to (loosely) match STL guidelines/conventions.
///
template < typename T, typename Key = string >
class tagged {
 public:
    //----------------------------------------------------------------------
    /// @brief      Static assertions on key type template arguments. Required in order to avoid ill-formed expressions within class definition.
    ///
    static_assert(is_default_constructible< Key >(),
                  "KEY TYPE [Key] MUST BE DEFAULT CONSTRUCTIBLE!");
    static_assert(!is_smart_ptr< T >(),
                  "TAGGED TYPE [T] CAN'T BE A STL SMART POINTER TYPE!");

    //----------------------------------------------------------------------
    /// @brief      Value type.
    ///
    using value_type = T;

    //----------------------------------------------------------------------
    /// @brief      Value pointer type.
    ///
    using pointer_type = std::shared_ptr< T >;

    //----------------------------------------------------------------------
    /// @brief      Value reference type.
    ///
    using reference = value_type&;

    //----------------------------------------------------------------------
    /// @brief      Value reference type (const).
    ///
    using const_reference = const value_type&;

    //----------------------------------------------------------------------
    /// @brief      Key type.
    ///
    using key_type = Key;

    //--------------------------------------------------------------------------
    /// @brief      Constructs a new instance.
    ///
    /// @param[in]  value  Input value, as smart pointer type (internal representation).
    ///
    /// @note       Useful if T can be constructed
    ///
    tagged(const pointer_type& value);

    //----------------------------------------------------------------------
    /// @brief      Constructs a new instance.
    ///
    /// @param[in]  args  Constructor arguments.
    ///
    /// @tparam     Args       Variadic parameter pack (implicitely deduced) describing type list of constructor arguments.
    /// @tparam     <unnamed>  SFINAE check that shadows/disables constructor overload if *T* can't be constructed with *Args*
    ///                        i.e. T(Args..) is not declared/defined
    ///
    /// @note       Redirects all input arguments to T constructor. Captures default, copy and move constructors as well.
    ///
    template < typename... Args, typename = typename enable_if< is_constructible< T, Args... >::value >::type >
    tagged(Args&&... args);

    //--------------------------------------------------------------------------
    /// @brief      Constructs a new (copied) instance, copying from a different-type tagged object.
    ///
    /// @param[in]  other  Instance to copy.
    ///
    /// @tparam     iT     Type of input data.
    /// @tparam     iKey   Type of input key.
    ///
    template < typename iT, typename iKey >
    tagged(const tagged< iT, iKey >& other);

    //--------------------------------------------------------------------------
    /// @brief      Constructs a new (moved) instance, moving from a different-type tagged object.
    ///
    /// @param[in]  other  Instance to copy.
    ///
    /// @tparam     iT     Type of input data.
    /// @tparam     iKey   Type of input key.
    ///
    template < typename iT, typename iKey >
    tagged(tagged< iT, iKey >&& other);

    //--------------------------------------------------------------------------
    /// @brief      Constructs a new (copied) instance.
    ///
    /// @param[in]  other  Instance to copy.
    ///
    /// @note       Explicit declaration is required to avoid moving/reallocation issues in std::vector<>
    ///             cf. https://stackoverflow.com/questions/64758775/result-type-must-be-constructible-from-value-type-of-input-range-when-trying-t
    ///
    tagged(const tagged< T, Key >& other) = default;

    //--------------------------------------------------------------------------
    /// @brief      Constructs a new (copied) instance.
    ///
    /// @param[in]  other  Instance to copy.
    ///
    /// @note       Explicit declaration is required to avoid moving/reallocation issues in std::vector<>
    ///             cf. https://stackoverflow.com/questions/64758775/result-type-must-be-constructible-from-value-type-of-input-range-when-trying-t
    ///
    ///
    tagged(tagged< T, Key >& other) = default;

    //--------------------------------------------------------------------------
    /// @brief      Constructs a new (moved) instance.
    ///
    /// @param[in]  other  Instance to move.
    ///
    /// @note       Explicit declaration is required to avoid moving/reallocation issues in std::vector<>
    ///             cf. https://stackoverflow.com/questions/64758775/result-type-must-be-constructible-from-value-type-of-input-range-when-trying-t
    ///
    tagged(tagged< T, Key >&& other) = default;

    //--------------------------------------------------------------------------
    /// @brief      Constructs a new instance, instantiating value in-place.
    ///
    /// @param[in]  key_  Key to tag value with.
    /// @param[in]  args  Constructor arguments.
    ///
    /// @tparam     Args  Variadic parameter pack (implicitely deduced) describing type list of constructor arguments.
    /// @tparam     <unnamed>  SFINAE check that shadows/disables constructor overload if *T* can't be constructed with *Args*
    ///                        i.e. T(Args..) is not declared/defined
    ///
    /// @note       If first argument is of type *key_type*, assigns key value to it & redirects all remaining arguments to T constructor.
    ///
    /// @note       This overload is shadowed by tagged< Args... >(Args... args) if defined i.e. an instance of T can be constructed with (key_, args...).
    ///             In that case the non-member constructor make_tagged<> should be used.
    ///
    template < typename... Args, typename = typename enable_if< is_constructible< T, Args... >::value >::type >
    tagged(const key_type& key_, Args&&... args);

    //--------------------------------------------------------------------------
    /// @brief      Constructs a new instance, instantiating value in-place from convertible type ().
    ///
    /// @param[in]  key_    Key to tag value with.
    /// @param[in]  value_  Value to cast/convert.
    ///
    /// @tparam     iT         Variadic parameter pack (implicitely deduced) describing type list of constructor arguments.
    ///
    /// @note       This overload is required because conversion operators aren't captured by std::is_constructible<> used in tagged(const key_type&, Args&&).
    ///
    /// @note       This overload is shadowed by tagged< Args... >(Args... args) if defined i.e. an instance of T can be constructed with (const iKey&, const iT&).
    ///             In that case the non-member constructor make_tagged<> should be used.
    ///
    template < typename iT, typename iKey, typename = typename enable_if< !is_constructible< T, iT >::value >::type >
    tagged(const iKey& key_, const iT& value_);


    tagged(std::shared_ptr< T > other);
    tagged(const key_type& key_, std::shared_ptr< T > other);

    // //--------------------------------------------------------------------------
    // /// @brief      Constructs a new instance, copying/moving from a value-key pair.
    // ///
    // /// @param[in]  value  Value to tag
    // /// @param[in]  key    Key
    // ///
    // /// @note       If *T* can be constructed w/ (T&, key_type) arguments, variadic template constructor is used
    // ///             and this overload is shadowed/ignored.
    // ///
    // /// @note       This overload is shadowed by previous constructor templates if *T* is constructible with (const T&, const key_type&) arguments.
    // ///             Otherwise it ensures that std::tagged< T, key_type > can be constructed with tagged(T&, key&) and tagged(key&, T&)
    // ///
    // /// @todo       Remove this constructor overload, there is no need for it.
    // ///
    // tagged(const T& value_, const key_type& key_);

    //--------------------------------------------------------------------------
    /// @brief      Assignment operator, *copying* from another tagged instance.
    ///
    /// @param[in]  other  Input instance.
    ///
    /// @return     Reference to modified instance.
    ///
    tagged& operator=(const tagged< T, Key >& other);

    //--------------------------------------------------------------------------
    /// @brief      Assignment operator, *moving* from another tagged instance.
    ///
    /// @param[in]  other  Input instance, consumed after operation.
    ///
    /// @return     Reference to modified instance.
    ///
    tagged& operator=(tagged< T, Key >&& other);

    //----------------------------------------------------------------------
    /// @brief      Assignment operator, assiging argument directly to value member.
    ///
    /// @note       Allows modifying value directly from an element instance.
    ///
    /// @note       Assignment operator of the value type [T::operator=(const iT&)] is called when assigning the value.
    ///
    /// @param[in]  other   Input value.
    ///
    /// @tparam     iT    Type of input argument.
    ///
    template < typename iT >
    tagged< T, Key >& operator=(const iT& other);

    //--------------------------------------------------------------------------
    /// @brief      Function call operator.
    ///
    /// @return     Reference to value member.
    ///
    /// @note       Acts as explicit conversion operator. Provided for convenience.
    ///
    reference get();

    //--------------------------------------------------------------------------
    /// @brief      Function call operator (const overload).
    ///
    /// @return     Const reference to value member.
    ///
    /// @note       Acts as explicit conversion operator. Provided for convenience.
    ///
    const_reference get() const;

    //--------------------------------------------------------------------------
    /// @brief      Function call operator.
    ///
    /// @return     Reference to value member.
    ///
    /// @note       Acts as explicit conversion operator. Provided for convenience.
    ///
    reference operator*();

    //--------------------------------------------------------------------------
    /// @brief      Function call operator (const overload).
    ///
    /// @return     Const reference to value member.
    ///
    /// @note       Acts as explicit conversion operator. Provided for convenience.
    ///
    const_reference operator*() const;

    //----------------------------------------------------------------------
    /// @brief      Implicit conversion operator overload between element and T.
    ///
    /// @note       Allows fetching value directly from an element instance.
    ///
    /// @note       Templating conversion in reference to output type creates ambiguity in operator calls.
    ///
    operator reference();

    //----------------------------------------------------------------------
    /// @brief      Implicit conversion operator overload between element and T (const overload).
    ///
    /// @note       Allows fetching value directly from an element instance.
    ///
    /// @note       Templating conversion in reference to output type creates ambiguity in operator calls.
    ///
    operator const_reference() const;

    //----------------------------------------------------------------------
    /// @brief      Object key e.g. text label.
    ///
    key_type key;

 protected:
    //----------------------------------------------------------------------
    /// @brief      Tagged object value.
    ///
    /// @note       Using a smart pointer makes std::tagged more portable.
    ///
    std::shared_ptr< T > _value;
};



template < typename T, typename Key >
template < typename... Args, typename    >
tagged< T, Key >::tagged(Args&&... args) :
    _value(new T(std::forward< Args >(args)...)) /* @note std::forward preseves lvalue/rvalue references! */ {
        /* ... */
}



template < typename T, typename Key >
template < typename iT, typename iKey >
tagged< T, Key >::tagged(const tagged< iT, iKey >& other) :
    tagged(other.key, other.get()) /* delegate to conversion overload */ {
        /* ... */
}



template < typename T, typename Key >
template < typename iT, typename iKey >
tagged< T, Key >::tagged(tagged< iT, iKey >&& other) :
    key(std::move(other.key)),
    _value(new T(std::move(other.get()))) /* move into data members */ {
        /* ... */
        static_assert(is_convertible< iT, T >(),
                      "INPUT VALUE TYPE IS NOT CONVERTIBLE TO T!");
        static_assert(is_convertible< iKey, Key >(),
                      "INPUT KEY TYPE IS NOT CONVERTIBLE TO Key!");
}



template < typename T, typename Key >
template < typename... Args, typename >
tagged< T, Key >::tagged(const Key& key_, Args&&... args) :
    key(key_),
    _value(new T(std::forward< Args >(args)...)) /* @note std::forward preseves lvalue/rvalue references! */ {
        /* ... */
}



template < typename T, typename Key >
template < typename iT, typename iKey, typename >
tagged< T, Key >::tagged(const iKey& key_, const iT& in) :
    key(key_),
    _value(new T(in)) {
        /* ... */
        static_assert(is_convertible< iT, T >(),
                      "INPUT VALUE TYPE IS NOT CONVERTIBLE TO T!");
        static_assert(is_convertible< iKey, Key >(),
                      "INPUT KEY TYPE IS NOT CONVERTIBLE TO Key!");
}



template < typename T, typename Key >
tagged< T, Key >::tagged(std::shared_ptr< T > other) :
    _value(other) {
        /* ... */
}



template < typename T, typename Key >
tagged< T, Key >::tagged(const Key& key_, std::shared_ptr< T > other) :
    key(key_),
    _value(other) {
        /* ... */
}



template < typename T, typename Key >
tagged< T, Key >& tagged< T, Key >::operator=(const tagged< T, Key >& other) {
    key = other.key;
    _value = std::shared_ptr< T >(new T(other.get()));
    return *this;
}



template < typename T, typename Key >
tagged< T, Key >& tagged< T, Key >::operator=(tagged< T, Key >&& other) {
    key = other.key;
    _value = std::shared_ptr< T >(new T(std::move(other.get())));
    return *this;
}



template < typename T, typename Key >
template < typename iT >
tagged< T, Key >& tagged< T, Key >::operator=(const iT& other) {
    *_value = other;  // calls T::operator(const iT&)
    return *this;
}



template < typename T, typename Key >
typename tagged< T, Key >::reference tagged< T, Key >::get() {
    return *_value;
}



template < typename T, typename Key >
typename tagged< T, Key >::const_reference tagged< T, Key >::get() const {
    return *_value;
}



template < typename T, typename Key >
typename tagged< T, Key >::reference tagged< T, Key >::operator*() {
    return *_value;
}



template < typename T, typename Key >
typename tagged< T, Key >::const_reference tagged< T, Key >::operator*() const {
    return *_value;
}



template < typename T, typename Key >
tagged< T, Key >::operator tagged< T, Key >::reference() {
    return *_value;
}



template < typename T, typename Key >
tagged< T, Key >::operator tagged< T, Key >::const_reference() const {
    return *_value;
}



//------------------------------------------------------------------------------
/// @brief      Static named 'in-place' constructor. Creates a std::tagged<> instance, from a key value and arguments passed.
///             Way to initialize object with both value and key, as std::tagged<> constructors redirect arguments to value constructors.
///
/// @param[in]  key  Key instance.
/// @param[in]  args Value constructor (T::T()) arguments.
///
/// @tparam     T    Value type.
/// @tparam     Key  Key type.
/// @tparam     Args Variadic parameter pack (implicitely deduced) describing type list of constructor arguments.
///
/// @return     std::tagged<> instance.
///
template < typename T, typename Key = string, typename... Args >
tagged< T, Key > make_tagged(const typename tagged< T, Key >::key_type& key, Args... args) {
    static_assert(std::is_constructible< T, Args... >(),
                  "VALUE TYPE IS NOT CONSTRUCTIBLE WITH GIVEN ARGUMENTS");
    tagged< T, Key > obj(args...);
    obj.key = key;
    return obj;  // no need to use std::move, RVO takes care of it
}


}  // namespace std

#endif  // TRIGNOCLIENT_INCLUDE_TRIGNOCLIENT_TAGGED_HPP_
