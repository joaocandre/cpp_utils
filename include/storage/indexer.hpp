//------------------------------------------------------------------------------
/// @file       indexer.hpp
/// @author     João André
///
/// @brief      Header file providing declaration & definition of std::indexer<> class template, as well as specializations std::index<> and std::series<>.
///
/// std::indexer<> is a pseudo-container class template i.e. container adapter (as per STL definition), that implements high-level indexing & sorting functionality
/// to STL's sequence containers, while also providing a pulic interface matching STL's *Container* and *SequenceContainer* named requirements. This means
/// by itself it can be treated as a container and will work with any of STL's tools (e.g. <utility>), while being versatile enough
///
/// While STL already provides associative containers in std:map<> and std::set<>, the major advantage of std::indexer<> is that it leverages the performance edge
/// of different sequence containers (std::array, std::vector, std::deque, etc), makes no assumption on the ordering/sorting of the underlying data, can be locked
/// to prevent dynamic resizing operations, maintains efficient random access to elements, is extensible, and thus is versatile enough to fit differerent usage scenarios.
///
//------------------------------------------------------------------------------

#ifndef TRIGNOCLIENT_INCLUDE_TRIGNOCLIENT_STD_INDEXER_HPP_
#define TRIGNOCLIENT_INCLUDE_TRIGNOCLIENT_STD_INDEXER_HPP_

#include <vector>
#include <type_traits>
#include <cassert>
#include <utility>
#include <exception>
#include <initializer_list>
#include <algorithm>
#include "type_check.hpp"      // std::can_apply<>
#include "tagged.hpp"          // std::tagged<>
#include "cast_iterator.hpp"   // std::cast_iterator<>
// #include "range_iterator.hpp"  // std::range_iterator<>

namespace std {

//------------------------------------------------------------------------------
/// @brief      Templated static SFINAE check for indexable types. It employs std::can_apply<> (cf. type_check.hpp) while testing specifically for the validity of a ::id member.
///
/// @note       At this stage, std::indexer<> requires:
///                 1) A Container type that meets STL's *Container* & *SequenceContainer* mandatory requirements holding objects of
///                 2) An 'indexable' type that provide a valid public non-const 'id' member.
///
/// @note       With generic use in mind, std::indexer makes minimal assumptions on data type. Any STL sequence container can be used with any class/struct that provides an 'id' member.
///             However, it was extensively tested using std::vector<> as the underlying container and std::tagged<> as the indexable type (that itself wraps around any other type).
///
template < class T >
using id_member = decltype(declval<T>().key);
template < class T >
using id_function = decltype(declval<T>().key());
template < class T >
using position_accessor = decltype(declval<T>().operator[](0));
template < class T >
using front_accessor = decltype(declval<T>().front());
template < class T >
using back_accessor = decltype(declval<T>().back());
template < class T >
using front_pusher = decltype(declval<T>().push_front());
template < class T >
using back_pusher = decltype(declval<T>().push_back());
template < class T >
using front_emplacer = decltype(declval<T>().emplace_front());
template < class T >
using back_emplacer = decltype(declval<T>().emplace_back());
template < class T >
using front_popper = decltype(declval<T>().pop_front());
template < class T >
using back_popper = decltype(declval<T>().pop_back());
template < class T >
using reserver = decltype(declval<T>().reserve(0));
///
template < class T >
using has_id = can_apply< id_member, T >;
template < class T >
using has_id_function = can_apply< id_function, T >;
template < class T >
using has_position_accessor = can_apply< position_accessor, T >;
template < class T >
using has_front_accessor = can_apply< front_accessor, T >;
template < class T >
using has_back_accessor = can_apply< back_accessor, T >;
template < class T >
using has_front_pusher = can_apply< front_pusher, T >;
template < class T >
using has_back_pusher = can_apply< back_pusher, T >;
template < class T >
using has_front_emplacer = can_apply< front_emplacer, T >;
template < class T >
using has_back_emplacer = can_apply< back_emplacer, T >;
template < class T >
using has_front_popper = can_apply< front_popper, T >;
template < class T >
using has_back_popper = can_apply< back_popper, T >;
template < class T >
using has_reserver = can_apply< reserver, T >;
///
template < typename T >
constexpr bool can_be_indexed() { return (has_id< T >() && !has_id_function< T >()); }
template < typename T, typename U >
constexpr bool can_be_replaced_by() { return (is_convertible< U, T >() && is_convertible< T, U >() /* whether T=U and U=T are valid */); }
template < typename T >
constexpr bool is_position_accessible() { return (has_position_accessor< T >()); }
template < typename T >
constexpr bool has_front() { return (has_front_accessor< T >()); }
template < typename T >
constexpr bool has_back() { return (has_back_accessor< T >()); }
template < typename T >
constexpr bool is_front_pushable() { return (has_front_pusher< T >()); }
template < typename T >
constexpr bool is_back_pushable() { return (has_back_pusher< T >()); }
template < typename T >
constexpr bool is_front_emplaceable() { return (has_front_emplacer< T >()); }
template < typename T >
constexpr bool is_back_emplaceable() { return (has_back_emplacer< T >()); }
template < typename T >
constexpr bool is_front_poppable() { return (has_front_popper< T >()); }
template < typename T >
constexpr bool is_back_poppable() { return (has_back_popper< T >()); }
template < typename T >
constexpr bool is_reservable() { return (has_reserver< T >()); }

//------------------------------------------------------------------------------
/// @brief      Simple extensible generic container adapter that provides high-level indexing/key functionality to *Container*, while maintaining a customizable public interface
///             of a container of *T*, in line with STL's *SequenceContainer* named requirements.
///             Represents a pseudo-container type for specialized "indexable" data types i.e. with a public identifier *key* member.
///             Provides associative features of std::set and std::map in sequence containers (std::array, std::vector, std::deque and std::list), leveraging perfomance advantages of these types.
///
/// @tparam     Container  Generic container of key/tagged type instances.
///                        Must fit STL's Container & SequenceContainer (partial) requirements.
/// @tparam     T          Public/interface type.
/// @tparam     Locked     Whether runtime size manipulation is allowed after initialization.
///                        Useful when size of the container does not change but elements do.
///
/// @note       Container element type must be a 'key' type that has a public 'id' member with identifier object (asserted @ compile time).
///
/// @note       Provides a minimal key/identifier interface, while exposing only a subset of container modifiers for a more generic scope.
///
/// @note       Designed to match public interface of other container adapters in the STL (std::stack, std::queue, etc),
///             while also fitting Container requirements (https://en.cppreference.com/w/cpp/named_req/Container) and partially SequenceContainer,
///             by wrapping around underlying container.
///
/// @todo       Rename to indexed_list?
///
template < typename Container, typename T = typename Container::value_type, bool Locked = false >
class indexer {
 public:
    //--------------------------------------------------------------------------
    /// @brief      Static assertions on data types/template arguments. Required in order to avoid ill-formed expressions within class definition.
    ///
    static_assert(std::is_class< Container >(),
                  "CONTAINER TYPE MUST BE A CLASS (NON-UNION)!");

    static_assert(std::is_position_accessible< Container >(),
                  "CONTAINER TYPE MUST BE RANDOM POSITION ACCESSIBLE ([])!");

    static_assert(std::can_be_indexed< typename Container::value_type >(),
                  "UNDERLYING CONTAINER ELEMENT TYPE [Container::value_type] MUST BE INDEXABLE!");

    static_assert(std::can_be_replaced_by< typename Container::value_type, T >(),
                  "UNDERLYING CONTAINER ELEMENT TYPE [Container::value_type] MUST BE IMPLICITLY CONVERTIBLE!");

    //--------------------------------------------------------------------------
    /// @brief      Value key wrapper type (== container value type). Provided to fit with STL 'Container' requirements.
    ///
    using value_type = T;  // typename Container::value_type;

    //--------------------------------------------------------------------------
    /// @brief      Container type. Provided to fit with STL 'Container' requirements.
    ///
    using container_type = Container;

    //--------------------------------------------------------------------------
    /// @brief      Container element type. Must be an indexable type implicitly convertible to & assignable from T.
    ///
    using element_type = typename Container::value_type;

    //--------------------------------------------------------------------------
    /// @brief      Container element reference type.
    ///
    using element_reference = element_type&;

    //--------------------------------------------------------------------------
    /// @brief      Container element cons reference type.
    ///
    using element_const_reference = const element_type&;

    //--------------------------------------------------------------------------
    /// @brief      Descriptor type.
    ///
    using key_type = decltype(declval< element_type >().key);

    //--------------------------------------------------------------------------
    /// @brief      Size/index type. Provided to fit with STL 'Container' requirements.
    ///
    using size_type = typename Container::size_type;

    //--------------------------------------------------------------------------
    /// @brief      Value reference type. Provided to fit with STL 'Container' requirements.
    ///
    using reference = T&;  // typename Container::reference;

    //--------------------------------------------------------------------------
    /// @brief      Value const reference type. Provided to fit with STL 'Container' requirements.
    ///
    using const_reference = const T&;  // typename Container::const_reference;

    //--------------------------------------------------------------------------
    /// @brief      Iterator type. Provided to fit with STL 'Container' requirements.
    ///
    /// @note       cast_iterator auto-converts container elements to different data type on dereferencing operation.
    ///
    using iterator = cast_iterator< Container, T >;

    //--------------------------------------------------------------------------
    /// @brief      Const iterator type. Provided to fit with STL 'Container' requirements.
    ///
    /// @note       cast_iterator auto-converts container elements to different data type on dereferencing operation.
    ///
    using const_iterator = cast_iterator< const Container, const T >;

    //--------------------------------------------------------------------------
    /// @brief      Constructs a new instance, passing the arguments to the Container constructor (Container::Container(...))
    ///
    /// @param      args  Container constructor arguments.
    ///
    /// @tparam     Args  Variadic parameter pack describing types of constructor arguments.
    ///
    /// @note       Discards the need to explicitly declare default, copy and move constructors, required to meet STL 'Container' requirements.
    ///
    /// @note       std::indexer<> makes no assumptions about how *element_type* is constructed, as such it may not be possible to assign labels @ construction
    ///             (e.g. if *element_type* does not allow it, as is the case of std::tagged<>, or if keys are static/const values).
    ///             This is by design, as that is considered to be the responsability of the underlying indexable type, and out of the scope of the indexer class.
    ///             The constructor simply redirects its arguments to underlying container constructor, which add versatility.
    ///             Nonetheless, for convenience, std::make_indexer<>() named constructor is provided that accepts key list as arguments.
    ///
    template < typename... Args, typename = typename enable_if< is_constructible< Container, Args... >::value >::type >
    indexer(Args&&... args);

    //--------------------------------------------------------------------------
    /// @brief      Constructs a new instance, initializing *n* elements in place, forwarding *Args* to the element constructor.
    ///
    /// @param[in]  n     Number of elements.
    /// @param      args  Element constructor arguments.
    ///
    /// @tparam     Args  Variadic parameter pack describing types of constructor arguments.
    ///
    /// @note       Can be shadowed by indexer(Args&&...) if container can be constructed w/ (size_t, Args&)
    ///
    template < typename... Args, typename = typename enable_if< is_constructible< T, Args... >::value >::type,
                                 typename = typename enable_if< !is_constructible< Container, size_t, Args... >::value >::type >
    indexer(size_t n, Args&&... args);

    //--------------------------------------------------------------------------
    /// @brief      Constructs a new (copied) instance.
    ///
    /// @param[in]  other  Instance to copy/move.
    ///
    /// @note       Default copy assignment operator limits input arguments to same template specialization, but it is useful to bypass Locked value.
    ///
    template < bool oLocked >
    indexer(const indexer< Container, T, oLocked >& other);

    //--------------------------------------------------------------------------
    /// @brief      Constructs a new (moved) instance.
    ///
    /// @param[in]  other  Instance to copy/move.
    ///
    /// @note       Default copy assignment operator limits input arguments to same template specialization, but it is useful to bypass Locked value.
    ///
    template < bool oLocked >
    indexer(indexer< Container, T, oLocked >&& other);

    //--------------------------------------------------------------------------
    /// @brief      Constructs a new instance, from a conventional container of elements & respective keys
    ///             A bracket initializer list of *T* elements can be used (a temporary vector is created).
    ///
    /// @param[in]  data   List of elements to assign to underlying container.
    /// @param[in]  data   List of keys to assign to each element. Defaults to empty list i.e. keys are not assigned.
    ///
    /// @note       Useful when *T* != *Container::value_type* (public interface different from underltying element type).
    ///             Generic variadic parameter pack constructor indexer(Args&&...) captures underlaying Container initializer list constructor,
    ///             but it will notfit if a list of elements of type *T* is passed.
    ///
    /// @note       std::vector is used as the argument type as it allows passing both a lvalue/rvalue vector instance *and* bracket-enclosed lists.
    ///             May add overhead if *data* and/or *keys* are large.
    ///
    indexer(const std::vector< T >& data, const std::vector< key_type >& keys = { /* ... */ });

    //--------------------------------------------------------------------------
    /// @brief      Destroys the object.
    ///
    /// @note       Marked virtual to allow deriving from indexer class.
    ///
    /// @note       Default (implicit) destructor already meets STL 'Container' requirements.
    ///
    virtual ~indexer() = default;

    //--------------------------------------------------------------------------
    /// @brief      Copy assignment operator.
    ///
    /// @note       Default copy assignment operator limits input arguments to same template specialization, but it is useful to bypass Locked value.
    ///
    /// @note       Provided to fit with STL 'Container' requirements.
    ///
    template < bool oLocked >
    indexer< Container, T, Locked >& operator=(const indexer< Container, T, oLocked >& other);

    //--------------------------------------------------------------------------
    /// @brief      Move assignment operator.
    ///
    /// @note       Default move assignment operator limits input arguments to same template specialization, but it is useful to bypass Locked value.
    ///
    /// @note       Provided to fit with STL 'Container' requirements.
    ///
    template < bool oLocked >
    indexer< Container, T, Locked >& operator=(indexer< Container, T, oLocked >&& other);

    //--------------------------------------------------------------------------
    /// @brief      Access container element.
    ///
    /// @param[in]  pos   Element position in container.
    ///
    /// @tparam     Arg       Dummy template argument to force SFINAE check over key_type.
    /// @tparam     <unnamed> SFINAE check that auto-disables operator overload if key_type and size_type are implicitely convertible (e.g. numerical keys).
    ///
    /// @return     Reference to T instance.
    ///
    /// @throw      std::out_of_range if pos is invalid.
    ///
    /// @note       Only available if key_type is not implicitly convertible to size_type, in order to avoid ambiguity.
    ///
    template < typename Arg = key_type, typename = typename enable_if< !is_convertible< Arg, size_type >::value >::type >
    reference operator()(size_type pos);

    //--------------------------------------------------------------------------
    /// @brief      Access container element (const overload).
    ///
    /// @param[in]  pos   Element position in container.
    ///
    /// @tparam     Arg       Dummy template argument to force SFINAE check over key_type.
    /// @tparam     <unnamed> SFINAE check that auto-disables operator overload if key_type and size_type are implicitely convertible (e.g. numerical keys).
    ///
    /// @return     Reference to T instance.
    ///
    /// @throw      std::out_of_range if pos is invalid.
    ///
    /// @note       Only available if key_type is not implicitly convertible to size_type, in order to avoid ambiguity.
    ///
    template < typename Arg = key_type, typename = typename enable_if< !is_convertible< Arg, size_type >::value >::type >
    const_reference operator()(size_type pos) const;

    //--------------------------------------------------------------------------
    /// @brief      Access container element.
    ///
    /// @param[in]  key   Element key/identifier.
    ///
    /// @return     Reference to T instance.
    ///
    /// @throw      std::invalid_argument if identifier is not found.
    ///
    reference operator()(const key_type& key);

    //--------------------------------------------------------------------------
    /// @brief      Access container element (const overload).
    ///
    /// @param[in]  key   Element key/identifier.
    ///
    /// @return     Reference to T instance.
    ///
    /// @throw      std::invalid_argument if identifier is not found.
    ///
    const_reference operator()(const key_type& key) const;

    //--------------------------------------------------------------------------
    /// @brief      Access container element.
    ///
    /// @param[in]  pos   Element position in container.
    ///
    /// @return     Reference to T instance.
    ///
    /// @throw      std::out_of_range if pos is invalid.
    ///
    /// @note       Only available if key_type is not implicitly convertible to size_type, in order to avoid ambiguity.
    ///             In that case, only *key* overloads are accepted.
    ///
    reference operator[](size_type pos);

    //--------------------------------------------------------------------------
    /// @brief      Access container element (const overload).
    ///
    /// @param[in]  pos   Element position in container.
    ///
    /// @return     Reference to T instance.
    ///
    /// @throw      std::out_of_range if pos is invalid.
    ///
    const_reference operator[](size_type pos) const;

    //--------------------------------------------------------------------------
    /// @brief      Access container element.
    ///
    /// @param[in]  key   Element key/identifier.
    ///
    /// @tparam     Arg       Dummy template argument to force SFINAE check over key_type.
    /// @tparam     <unnamed> SFINAE check that auto-disables operator overload if key_type and size_type are implicitely convertible (e.g. numerical keys).
    ///
    /// @return     Reference to T instance.
    ///
    /// @throw      std::invalid_argument if identifier is not found.
    ///
    /// @note       Only available if key_type is not implicitly convertible to size_type, in order to avoid ambiguity.
    ///             In that case, only *position* [] operator overloads can be used (STL container interface).
    ///
    template < typename Arg = key_type, typename = typename enable_if< !is_convertible< Arg, size_type >::value >::type >
    reference operator[](const key_type& key);

    //--------------------------------------------------------------------------
    /// @brief      Access container element (const overload).
    ///
    /// @param[in]  key   Element key/identifier.
    ///
    /// @tparam     Arg       Dummy template argument to force SFINAE check over key_type.
    /// @tparam     <unnamed> SFINAE check that auto-disables operator overload if key_type and size_type are implicitely convertible (e.g. numerical keys).
    ///
    /// @return     Reference to T instance.
    ///
    /// @throw      std::invalid_argument if identifier is not found.
    ///
    /// @note       Only available if key_type is not implicitly convertible to size_type, in order to avoid ambiguity.
    ///             In that case, only *position* [] operator overloads can be used (STL container interface).
    ///
    template < typename Arg = key_type, typename = typename enable_if< !is_convertible< Arg, size_type >::value >::type >
    const_reference operator[](const key_type& key) const;

    //--------------------------------------------------------------------------
    /// @brief      Access container element.
    ///
    /// @param[in]  pos   Element position in container.
    ///
    /// @tparam     oT        Desired output type. Underlying element type *must* be convertible to this type.
    ///
    /// @return     Reference to T instance.
    ///
    /// @throw      std::out_of_range if pos is invalid.
    ///
    /// @note       Templated return type allows explictly requesting either value_type or element_type references @ *pos*.
    ///
    template < typename oT = value_type >
    oT& at(size_type pos);

    //--------------------------------------------------------------------------
    /// @brief      Access container element (const overload).
    ///
    /// @param[in]  pos   Element position in container.
    ///
    /// @tparam     oT        Desired output type. Underlying element type *must* be convertible to this type.
    ///
    /// @return     Reference to T instance.
    ///
    /// @throw      std::out_of_range if pos is invalid.
    ///
    /// @note       Templated return type allows explictly requesting either value_type or element_type references @ *pos*.
    ///
    template < typename oT = value_type >
    const oT& at(size_type pos) const;

    //--------------------------------------------------------------------------
    /// @brief      Access container element.
    ///
    /// @param[in]  key   Element key/identifier.
    ///
    /// @tparam     oT        Desired output type. Underlying element type *must* be convertible to this type.
    /// @tparam     Arg       Dummy template argument to force SFINAE check over key_type.
    /// @tparam     <unnamed> SFINAE check that auto-disables operator overload if key_type and size_type are implicitely convertible (e.g. numerical keys).
    ///
    /// @return     Reference to T instance.
    ///
    /// @throw      std::invalid_argument if identifier is not found.
    ///
    /// @note       Only available if key_type is not implicitly convertible to size_type, in order to avoid ambiguity.
    ///             In that case, only *position* [] operator overloads can be used (STL container interface).
    ///
    /// @note       Templated return type allows explictly requesting either value_type or element_type references @ *pos*.
    ///
    template < typename oT = value_type, typename Arg = key_type, typename = typename enable_if< !is_convertible< Arg, size_type >::value >::type >
    oT& at(const key_type& key);

    //--------------------------------------------------------------------------
    /// @brief      Access container element (const overload).
    ///
    /// @param[in]  key   Element key/identifier.
    ///
    /// @tparam     oT        Desired output type. Underlying element type *must* be convertible to this type.
    /// @tparam     Arg       Dummy template argument to force SFINAE check over key_type.
    /// @tparam     <unnamed> SFINAE check that auto-disables operator overload if key_type and size_type are implicitely convertible (e.g. numerical keys).
    ///
    /// @return     Reference to T instance.
    ///
    /// @throw      std::invalid_argument if identifier is not found.
    ///
    /// @note       Only available if key_type is not implicitly convertible to size_type, in order to avoid ambiguity.
    ///             In that case, only *position* [] operator overloads can be used (STL container interface).
    ///
    /// @note       Templated return type allows explictly requesting either value_type or element_type references @ *pos*.
    ///
    template < typename oT = value_type, typename Arg = key_type, typename = typename enable_if< !is_convertible< Arg, size_type >::value >::type >
    const oT& at(const key_type& key) const;

    //--------------------------------------------------------------------------
    /// @brief      Get element position from its identifier.
    ///
    /// @param[in]  id  Element key/identifier.
    ///
    /// @return     Position in container.
    ///
    /// @throw      std::invalid_argument if key is not found.
    ///
    virtual size_type find(const key_type& key) const;

    //--------------------------------------------------------------------------
    /// @brief      Access element key/identifier.
    ///
    /// @param[in]  pos   Element position in container.
    ///
    /// @return     Reference to key object.
    ///
    /// @throw      std::out_of_range if pos is invalid.
    ///
    /// @note       Usdeful to set element key/identifiers.
    ///
    key_type& key(size_type pos);

    //--------------------------------------------------------------------------
    /// @brief      Access element key/identifier (const overload).
    ///
    /// @param[in]  pos   Element position in container.
    ///
    /// @return     Reference to key object.
    ///
    /// @throw      std::out_of_range if pos is invalid.
    ///
    const key_type& key(size_type pos) const;

    //--------------------------------------------------------------------------
    /// @brief      Gets a list of the ids describing the content of the underlying container.
    ///
    /// @return     Container with id values.
    ///
    /// @tparam     OutputContainer  Container type. Must be a SequenceContainer with ::push_back() defined (e.g. std::basic_string, std::deque, std::vector)
    ///                              of type compatible with key_type .
    ///
    template < typename OutputContainer >
    OutputContainer get_keys() const;

    //--------------------------------------------------------------------------
    /// @brief      Set key/identifiers into container elements.
    ///
    /// @param[in]  ids  Element keys/identifiers
    ///
    /// @tparam     InputContainer  Container type. Must be compatible with range-based loops (e.g. STL containers), and be of a type compatible
    ///
    template < typename InputContainer >
    void set_keys(const InputContainer& id);

    //--------------------------------------------------------------------------
    /// @brief      Checks if container is empty.
    ///
    /// @return     True if undelying container is empty, false otherwise.
    ///
    /// @note       Provided to fit with STL 'Container' requirements.
    ///
    bool empty() const noexcept;

    //--------------------------------------------------------------------------
    /// @brief      Number of elements in the underlying container.
    ///
    /// @note       Provided to fit with STL 'Container' requirements.
    ///
    size_type size() const noexcept;

    //--------------------------------------------------------------------------
    /// @brief      Maximum number of elements in the underlying container.
    ///
    /// @note       Provided to fit with STL 'Container' requirements.
    ///
    size_type max_size() const noexcept;

    //--------------------------------------------------------------------------
    /// @brief      Clears the contents of the underlying container.
    ///
    /// @note       Only available if Locked is false, otherwise it is not compilable.
    ///
    /// @note       Provided to fit with STL 'Container' requirements.
    ///
    void clear() noexcept;

    //--------------------------------------------------------------------------
    /// @brief      Get an iterator to the beginning.
    ///
    /// @note       Provided to fit with STL 'Container' requirements.
    ///
    iterator begin() noexcept;

    //--------------------------------------------------------------------------
    /// @brief      Get an iterator to the end.
    ///
    /// @note       Provided to fit with STL 'Container' requirements.
    ///
    iterator end() noexcept;

    //--------------------------------------------------------------------------
    /// @brief      Get an iterator to the beginning (const overload).
    ///
    /// @note       Provided to fit with STL 'Container' requirements.
    ///
    const_iterator begin() const noexcept;

    //--------------------------------------------------------------------------
    /// @brief      Get an iterator to the end (const overload).
    ///
    /// @note       Provided to fit with STL 'Container' requirements.
    ///
    const_iterator end() const noexcept;

    //--------------------------------------------------------------------------
    /// @brief      Get an const iterator to the beginning.
    ///
    /// @note       Provided to fit with STL 'Container' requirements.
    ///
    const_iterator cbegin() const noexcept;

    //--------------------------------------------------------------------------
    /// @brief      Get an const iterator to the end.
    ///
    /// @note       Provided to fit with STL 'Container' requirements.
    ///
    const_iterator cend() const noexcept;

    //--------------------------------------------------------------------------
    /// @brief      Reference to the underlying container.
    ///
    /// @note       Useful to manage objects directly, in cases where T != Container::value_type (i.e. iterators return different types when dereferencing).
    ///
    Container& elements() noexcept;

    //--------------------------------------------------------------------------
    /// @brief      Reference to the underlying container (const overload).
    ///
    /// @note       Useful to manage objects directly, in cases where T != Container::value_type (i.e. iterators return different types when dereferencing).
    ///
    const Container& elements() const noexcept;

    //--------------------------------------------------------------------------
    /// @brief      Get a reference to element at given position. Bypasses public type interface to access underlying indexable/descriptor type.
    ///
    /// @param[in]  pos Element position in underlying container.
    ///
    element_reference element(size_type pos);

    //--------------------------------------------------------------------------
    /// @brief      Get a const reference to element at given position. Bypasses public type interface to access underlying indexable/descriptor type.
    ///
    /// @param[in]  pos Element position in underlying container.
    ///
    element_const_reference element(size_type pos) const;

    //--------------------------------------------------------------------------
    /// @brief      Swaps the contents of the underlying container.
    ///
    /// @param[in]  other    Other locked/unlocked indexer instance.
    ///
    /// @tparam     oLocked  Lock status of the input indexer.
    ///
    /// @note       Provided to fit with STL 'Container' requirements.
    ///
    template < bool oLocked >
    void swap(const indexer< Container, T, oLocked >& other) noexcept;

    void assign(size_type count, const T& value);

    template< class InputIt >
    void assign(InputIt first, InputIt last);

    void assign(std::initializer_list< T > ilist);

    template< typename InputIt, typename = typename enable_if< is_convertible< InputIt, typename container_type::iterator >::value >::type,
              typename... Args, typename = typename enable_if< is_constructible< element_type, Args... >::value >::type >
    void emplace(InputIt pos, Args... args);

    template< class InputIt, typename = typename enable_if< is_convertible< InputIt, typename container_type::iterator >::value >::type >
    void insert(InputIt pos, const T& value);

    template< class InputIt, typename = typename enable_if< is_convertible< InputIt, typename container_type::iterator >::value >::type >
    void insert(InputIt pos, T&& value);

    template< class InputIt, typename = typename enable_if< is_convertible< InputIt, typename container_type::iterator >::value >::type >
    void insert(InputIt pos, size_type count, const T& value);

    template< typename InputIt, typename SourceIt, typename = typename enable_if< is_convertible< InputIt, typename container_type::iterator >::value >::type >
    void insert(InputIt pos, SourceIt first, SourceIt last);

    template< class InputIt, typename = typename enable_if< is_convertible< InputIt, typename container_type::iterator >::value >::type >
    void insert(InputIt pos, std::initializer_list< T > ilist);

    //--------------------------------------------------------------------------
    /// @brief      Erases element @ given *pos*.
    ///
    /// @param[in]  pos   Iterator to element to erase.
    ///
    template< class InputIt, typename = typename enable_if< is_convertible< InputIt, typename container_type::iterator >::value >::type >
    void erase(InputIt pos);

    //--------------------------------------------------------------------------
    /// @brief      Erases elements @ range between *first* and *last*.
    ///
    /// @param[in]  pos   Iterator to element to erase.
    ///
    /// @return     Iterator to first element after *pos*.
    ///
    template< class InputIt, typename = typename enable_if< is_convertible< InputIt, typename container_type::iterator >::value >::type >
    void erase(InputIt first, InputIt last);

    //--------------------------------------------------------------------------
    /// @brief      Access first element in container.
    ///
    /// @return     Reference to first element.
    ///
    template < typename oT = value_type, typename C = Container, typename = typename enable_if< has_front< C >() >::type >
    oT& front();

    //--------------------------------------------------------------------------
    /// @brief      Access first element in container.
    ///
    /// @return     *const* reference to first element.
    ///
    template < typename oT = value_type, typename C = Container, typename = typename enable_if< has_front< C >() >::type >
    const oT& front() const;

    //--------------------------------------------------------------------------
    /// @brief      Access last element in container.
    ///
    /// @return     Reference to last element.
    ///
    template < typename oT = value_type, typename C = Container, typename = typename enable_if< has_back< C >() >::type >
    oT& back();

    //--------------------------------------------------------------------------
    /// @brief      Access last element in container.
    ///
    /// @return     *const* reference to last element.
    ///
    template < typename oT = value_type, typename C = Container, typename = typename enable_if< has_back< C >() >::type >
    const oT& back() const;

    //--------------------------------------------------------------------------
    /// @brief      Creates a new element in-place @ front of container.
    ///
    /// @param      args       Arguments passed to the element constructor.
    ///
    /// @tparam     Args  Variadic parameter pack (implicitely deduced) describing type list of constructor arguments.
    /// @tparam     <unnamed>  SFINAE check that shadows/disables constructor overload if *T* can't be constructed with *Args*
    ///                        i.e. T(Args..) is not declared/defined
    ///
    template < typename C = Container, typename = typename enable_if< is_front_emplaceable< C >() >::type,
               typename... Args, typename = typename enable_if< is_constructible< element_type, Args... >::value >::type >
    void emplace_front(Args&&... args);

    //--------------------------------------------------------------------------
    /// @brief      Creates a new element in-place @ front of container, w/ indexer key.
    ///
    /// @param      key        Key to assign to new element.
    /// @param      args       Arguments passed to the element constructor.
    ///
    /// @tparam     Args  Variadic parameter pack (implicitely deduced) describing type list of constructor arguments.
    /// @tparam     <unnamed>  SFINAE check that shadows/disables constructor overload if *T* can't be constructed with *Args*
    ///                        i.e. T(Args..) is not declared/defined
    ///
    template < typename C = Container, typename = typename enable_if< is_front_emplaceable< C >() >::type,
               typename... Args, typename = typename enable_if< is_constructible< element_type, Args... >::value >::type >
    void emplace_front(const key_type& key, Args&&... args);

    //--------------------------------------------------------------------------
    /// @brief      Creates a new element in-place @ back of container.
    ///
    /// @param      args       Arguments passed to the element constructor.
    ///
    /// @tparam     Args  Variadic parameter pack (implicitely deduced) describing type list of constructor arguments.
    /// @tparam     <unnamed>  SFINAE check that shadows/disables constructor overload if *T* can't be constructed with *Args*
    ///                        i.e. T(Args..) is not declared/defined
    ///
    template < typename C = Container, typename = typename enable_if< is_back_emplaceable< C >() >::type,
               typename... Args, typename = typename enable_if< is_constructible< element_type, Args... >::value >::type >
    void emplace_back(Args&&... args);

    //--------------------------------------------------------------------------
    /// @brief      Creates a new element in-place @ back of container, w/ indexer key.
    ///
    /// @param      key        Key to assign to new element.
    /// @param      args       Arguments passed to the element constructor.
    ///
    /// @tparam     Args  Variadic parameter pack (implicitely deduced) describing type list of constructor arguments.
    /// @tparam     <unnamed>  SFINAE check that shadows/disables constructor overload if *T* can't be constructed with *Args*
    ///                        i.e. T(Args..) is not declared/defined
    ///
    template < typename C = Container, typename = typename enable_if< is_back_emplaceable< C >() >::type,
               typename... Args, typename = typename enable_if< is_constructible< element_type, Args... >::value >::type >
    void emplace_back(const key_type& key, Args&&... args);

    //--------------------------------------------------------------------------
    /// @brief      Copies given *value* to @ front of container.
    ///
    /// @param      value      Value to add to the underlying container.
    ///
    template < typename C = Container, typename = typename enable_if< is_front_pushable< C >() >::type >
    void push_front(value_type&& value);

    //--------------------------------------------------------------------------
    /// @brief      Copies given *value* to @ front of container, w/ indexer key.
    ///
    /// @param      key        Key to assign to new element.
    /// @param      value      Value to add to the underlying container.
    ///
    template < typename C = Container, typename = typename enable_if< is_front_pushable< C >() >::type >
    void push_front(const key_type& key, value_type&& value);

    //--------------------------------------------------------------------------
    /// @brief      Copies given *value* to @ back of container.
    ///
    /// @param      value      Value to add to the underlying container.
    ///
    template < typename C = Container, typename = typename enable_if< is_back_pushable< C >() >::type >
    void push_back(value_type&& value);

    //--------------------------------------------------------------------------
    /// @brief      Copies given *value* to @ back of container, w/ indexer key.
    ///
    /// @param      key        Key to assign to new element.
    /// @param      value      Value to add to the underlying container.
    ///
    template < typename C = Container, typename = typename enable_if< is_back_pushable< C >() >::type >
    void push_back(const key_type& key, value_type&& value);

    //--------------------------------------------------------------------------
    /// @brief      Deletes/destroys element @ front of container
    ///
    template < typename C = Container, typename = typename enable_if< is_front_poppable< C >() >::type >
    void pop_front();

    //--------------------------------------------------------------------------
    /// @brief      Deletes/destroys element @ back of container.
    ///
    template < typename C = Container, typename = typename enable_if< is_back_poppable< C >() >::type >
    void pop_back();

    //--------------------------------------------------------------------------
    /// @brief      Reserve storage.
    ///
    /// @param[in]  new_capacity  New capacity (maximum number of elements) for container.
    ///
    template < typename C = Container, typename = typename enable_if< is_reservable< C >() >::type >
    void reserve(size_t new_capacity);

 protected:
    //--------------------------------------------------------------------------
    /// @brief      Underlying data container.
    ///
    Container _data;
};



//--------------------------------------------------------------------------
/// @cond

template < typename Container, typename T, bool Locked >
template < typename... Args, typename >
indexer< Container, T, Locked >::indexer(Args&&... args) : _data(std::forward< Args >(args)...) {
    /* ... */
    assert(_data.size() || !Locked && ("LOCKED CONTAINER CAN'T BE DEFAULT/EMPTY-INITIALIZED!"));
}



template < typename Container, typename T, bool Locked >
template < typename... Args, typename, typename >
indexer< Container, T, Locked >::indexer(size_t n, Args&&... args) {
    /* ... */
    assert(n || !Locked && ("LOCKED CONTAINER CAN'T BE DEFAULT/EMPTY-INITIALIZED!"));
    // add elements iteratively
    for (size_t idx = 0; idx < n; idx++) {
        this->emplace_back(std::forward< Args >(args)...);
    }
}



template < typename Container, typename T, bool Locked >
template < bool oLocked >
indexer< Container, T, Locked >::indexer(const indexer< Container, T, oLocked >& other) {
    /* ... */
    assert(other.size() || !Locked && ("LOCKED CONTAINER CAN'T BE DEFAULT/EMPTY-INITIALIZED!"));
    _data = other._data;
}



template < typename Container, typename T, bool Locked >
template < bool oLocked >
indexer< Container, T, Locked >::indexer(indexer< Container, T, oLocked >&& other) {
    /* ... */
    assert(other.size() || !Locked && ("LOCKED CONTAINER CAN'T BE DEFAULT/EMPTY-INITIALIZED!"));
    _data = std::move(other._data);
}



template < typename Container, typename T, bool Locked >
indexer< Container, T, Locked >::indexer(const std::vector< T >& data, const std::vector< typename indexer< Container, T, Locked >::key_type >& keys) {
    /* ... */
    // _data.reserve(data.size());  // requires Container to be std::vector!
    if (keys.size()) {
        assert(keys.size() == data.size() && ("INSUFFICIENT NUMBER OF KEYS!"));
    }

    for (size_t idx = 0; idx < data.size(); idx++) {
        if (!keys.empty()) {
            // since element_type and T are implicitely convertible/constructible
            // calls element_type(const T&) copy constructor overload
            this->emplace_back(keys[idx], std::move(data[idx]));
        } else {
            this->emplace_back(std::move(data[idx]));
        }
    }
}


// template < typename Container, typename T, bool Locked >
// indexer< Container, T, Locked >::indexer(std::initializer_list< T > data, std::initializer_list< typename indexer< Container, T, Locked >::key_type > keys) {
//     /* ... */
//     // _data.reserve(data.size());  // requires Container to be std::vector!
//     if (keys.size()) {
//         assert(keys.size() == data.size() && ("INSUFFICIENT NUMBER OF KEYS!"));
//     }

//     for (auto& element : data) {
//         // since element_type and T are implicitely convertible/constructible
//         // calls element_type(const T&) copy constructor overload
//         _data.emplace_back(std::move(element));
//     }
// }



// template < typename Container, typename T, bool Locked >
// indexer< Container, T, Locked >::~indexer() {
//     /* ... */
// }


template < typename Container, typename T, bool Locked >
template < bool oLocked >
indexer< Container, T, Locked >& indexer< Container, T, Locked >::operator=(const indexer< Container, T, oLocked >& other) {
    /* ... */
    assert(_data.size() == other.size() || !Locked && ("LOCKED CONTAINER CAN'T BE RESIZED!"));
    _data = other._data;
    /* ... */
    return (*this);
}



template < typename Container, typename T, bool Locked >
template < bool oLocked >
indexer< Container, T, Locked >& indexer< Container, T, Locked >::operator=(indexer< Container, T, oLocked >&& other) {
    /* ... */
    assert(_data.size() == other.size() || !Locked && ("LOCKED CONTAINER CAN'T BE RESIZED!"));
    _data = other._data;
    /* ... */
    return (*this);
}



template < typename Container, typename T, bool Locked >
template < typename, typename >
typename indexer< Container, T, Locked >::reference indexer< Container, T, Locked >::operator()(typename indexer< Container, T, Locked >::size_type pos) {
    // if (pos >= 0 && pos < _data.size()) {
    //     return _data[pos];
    // }
    // throw std::out_of_range("[" + std::string(__func__) + "] Invalid position!");
    return at(pos);
}



template < typename Container, typename T, bool Locked >
template < typename, typename >
typename indexer< Container, T, Locked >::const_reference indexer< Container, T, Locked >::operator()(typename indexer< Container, T, Locked >::size_type pos) const {
    // if (pos >= 0 && pos < _data.size()) {
    //     return _data[pos];
    // }
    // throw std::out_of_range("[" + std::string(__func__) + "] Invalid position!");
    return at(pos);
}



template < typename Container, typename T, bool Locked >
typename indexer< Container, T, Locked >::reference indexer< Container, T, Locked >::operator()(const typename indexer< Container, T, Locked >::key_type& key) {
    return at(key);
}



template < typename Container, typename T, bool Locked >
typename indexer< Container, T, Locked >::const_reference indexer< Container, T, Locked >::operator()(const typename indexer< Container, T, Locked >::key_type& key) const {
    return at(key);
}



template < typename Container, typename T, bool Locked >
typename indexer< Container, T, Locked >::reference indexer< Container, T, Locked >::operator[](typename indexer< Container, T, Locked >::size_type pos) {
    return at(pos);
}



template < typename Container, typename T, bool Locked >
typename indexer< Container, T, Locked >::const_reference indexer< Container, T, Locked >::operator[](typename indexer< Container, T, Locked >::size_type pos) const {
    return at(pos);
}



template < typename Container, typename T, bool Locked >
template < typename, typename >
typename indexer< Container, T, Locked >::reference indexer< Container, T, Locked >::operator[](const typename indexer< Container, T, Locked >::key_type& key) {
    return at(key);
}



template < typename Container, typename T, bool Locked >
template < typename, typename >
typename indexer< Container, T, Locked >::const_reference indexer< Container, T, Locked >::operator[](const typename indexer< Container, T, Locked >::key_type& key) const {
    return at(key);
}



template < typename Container, typename T, bool Locked >
template < typename oT >
oT& indexer< Container, T, Locked >::at(typename indexer< Container, T, Locked >::size_type pos) {
    static_assert(is_convertible< element_type, oT >(),
                  "[element_type] MUST BE CONVERTIBLE TO OUTPUT TYPE [oT]");
    //
    if (pos >= 0 && pos < _data.size()) {
        return _data[pos];
    }
    throw std::out_of_range("[" + std::string(__func__) + "] Invalid position!");
}



template < typename Container, typename T, bool Locked >
template < typename oT >
const oT& indexer< Container, T, Locked >::at(typename indexer< Container, T, Locked >::size_type pos) const {
    static_assert(is_convertible< element_type, oT >(),
                  "[element_type] MUST BE CONVERTIBLE TO OUTPUT TYPE [oT]");
    //
    if (pos >= 0 && pos < _data.size()) {
        return _data[pos];
    }
    throw std::out_of_range("[" + std::string(__func__) + "] Invalid position!");
}



template < typename Container, typename T, bool Locked >
template < typename oT, typename, typename >
oT& indexer< Container, T, Locked >::at(const typename indexer< Container, T, Locked >::key_type& key) {
    auto idx = find(key);
    if (idx >= 0) {
        return _data[find(key)];
    }
    throw std::invalid_argument("[" + std::string(__func__) + "] Invalid key!");
}



template < typename Container, typename T, bool Locked >
template < typename oT, typename, typename >
const oT& indexer< Container, T, Locked >::at(const typename indexer< Container, T, Locked >::key_type& key) const {
    auto idx = find(key);
    if (idx >= 0) {
        return _data[find(key)];
    }
    throw std::invalid_argument("[" + std::string(__func__) + "] Invalid key!");
}



template < typename Container, typename T, bool Locked >
typename indexer< Container, T, Locked >::size_type indexer< Container, T, Locked >::find(const indexer< Container, T, Locked >::key_type& key) const {
    for (int idx = 0; idx < _data.size(); idx++) {
        if (_data[idx].key == key) {
            return idx;
        }
    }
    throw std::invalid_argument("[" + std::string(__func__) + "] Key not found!");
}



template < typename Container, typename T, bool Locked >
typename indexer< Container, T, Locked >::key_type& indexer< Container, T, Locked >::key(indexer< Container, T, Locked >::size_type pos) {
    if (pos >= 0 && pos < _data.size()) {
        return _data[pos].key;
    }
    throw std::out_of_range("[" + std::string(__func__) + "] Invalid position!");
}



template < typename Container, typename T, bool Locked >
const typename indexer< Container, T, Locked >::key_type& indexer< Container, T, Locked >::key(indexer< Container, T, Locked >::size_type pos) const {
    if (pos >= 0 && pos < _data.size()) {
        return _data[pos].key;
    }
    throw std::out_of_range("[" + std::string(__func__) + "] Invalid position!");
}



template < typename Container, typename T, bool Locked >
template < typename OutputContainer >
OutputContainer indexer< Container, T, Locked >::get_keys() const {
    OutputContainer ids;
    for (const auto& element : _data) {
        ids.push_back(element.key);
    }
    return ids;
}



template < typename Container, typename T, bool Locked >
template < typename InputContainer >
void indexer< Container, T, Locked >::set_keys(const InputContainer& identifiers) {
    int i = 0;
    for (const auto& element : _data) {
        element.key = identifiers[i++];
    }
}



template < typename Container, typename T, bool Locked >
bool indexer< Container, T, Locked >::empty() const noexcept {
    return _data.empty();
}



template < typename Container, typename T, bool Locked >
typename indexer< Container, T, Locked >::size_type indexer< Container, T, Locked >::size() const noexcept {
    return _data.size();
}



template < typename Container, typename T, bool Locked >
typename indexer< Container, T, Locked >::size_type indexer< Container, T, Locked >::max_size() const noexcept {
    return _data.max_size();
}



template < typename Container, typename T, bool Locked >
void indexer< Container, T, Locked >::clear() noexcept {
    static_assert(!Locked, "LOCKED CONTAINER; RESIZE OPERATIONS FORBIDDEN");
    _data.clear();
}



template < typename Container, typename T, bool Locked >
typename indexer< Container, T, Locked >::iterator indexer< Container, T, Locked >::begin() noexcept {
    return iterator(&_data, 0);
}



template < typename Container, typename T, bool Locked >
typename indexer< Container, T, Locked >::iterator indexer< Container, T, Locked >::end() noexcept {
    return iterator(&_data, _data.size());
}



template < typename Container, typename T, bool Locked >
typename indexer< Container, T, Locked >::const_iterator indexer< Container, T, Locked >::begin() const noexcept {
    return const_iterator(&_data, 0);
}



template < typename Container, typename T, bool Locked >
typename indexer< Container, T, Locked >::const_iterator indexer< Container, T, Locked >::end() const noexcept {
    return const_iterator(&_data, _data.size());
}



template < typename Container, typename T, bool Locked >
typename indexer< Container, T, Locked >::const_iterator indexer< Container, T, Locked >::cbegin() const noexcept {
    return const_iterator(&_data, 0);
}



template < typename Container, typename T, bool Locked >
typename indexer< Container, T, Locked >::const_iterator indexer< Container, T, Locked >::cend() const noexcept {
    return const_iterator(&_data, _data.size());
}



template < typename Container, typename T, bool Locked >
Container& indexer< Container, T, Locked >::elements() noexcept {
    return _data;
}



template < typename Container, typename T, bool Locked >
const Container& indexer< Container, T, Locked >::elements() const noexcept {
    return _data;
}



template < typename Container, typename T, bool Locked >
typename indexer< Container, T, Locked >::element_reference indexer< Container, T, Locked >::element(indexer< Container, T, Locked >::size_type pos) {
    return _data[pos];  // return _data.at< element_reference >(pos);
}



template < typename Container, typename T, bool Locked >
typename indexer< Container, T, Locked >::element_const_reference indexer< Container, T, Locked >::element(indexer< Container, T, Locked >::size_type pos) const {
    return _data[pos];  // return _data.at< element_reference >(pos);
}



template < typename Container, typename T, bool Locked >
template < bool oLocked >
void indexer< Container, T, Locked >::swap(const indexer< Container, T, oLocked >& other) noexcept {
    _data.swap(other._data);
}



template < typename Container, typename T, bool Locked >
void indexer< Container, T, Locked >::assign(indexer< Container, T, Locked >::size_type count, const T& value) {
    _data.assign(count, value);
}



template < typename Container, typename T, bool Locked >
template< class InputIt >
void indexer< Container, T, Locked >::assign(InputIt first, InputIt last) {
    _data.assign(first, last);
}



template < typename Container, typename T, bool Locked >
void indexer< Container, T, Locked >::assign(std::initializer_list< T > ilist) {
    _data.assign(ilist);
}



template < typename Container, typename T, bool Locked >
template < typename InputIt, typename, typename... Args, typename >
void indexer< Container, T, Locked >::emplace(InputIt pos, Args... args) {
    return _data.emplace(pos, std::forward< Args >(args)...);
}



template < typename Container, typename T, bool Locked >
template < typename InputIt, typename >
void indexer< Container, T, Locked >::insert(InputIt pos, const T& value) {
    // parse input iterator
    _data.insert(pos, value);
}



template < typename Container, typename T, bool Locked >
template < typename InputIt, typename >
void indexer< Container, T, Locked >::insert(InputIt pos, T&& value) {
    _data.insert(pos, value);
}



template < typename Container, typename T, bool Locked >
template < typename InputIt, typename >
void indexer< Container, T, Locked >::insert(InputIt pos, typename indexer< Container, T, Locked >::size_type count, const T& value) {
    _data.insert(pos, count, value);
}



template < typename Container, typename T, bool Locked >
template< class InputIt, typename SourceIt, typename >
void indexer< Container, T, Locked >::insert(InputIt pos, SourceIt first, SourceIt last) {
    _data.insert(pos, first, last);
}



template < typename Container, typename T, bool Locked >
template < typename InputIt, typename >
void indexer< Container, T, Locked >::insert(InputIt pos, std::initializer_list< T > ilist) {
    _data.insert(pos, ilist);
}



template < typename Container, typename T, bool Locked >
template< class InputIt, typename >
void indexer< Container, T, Locked >::erase(InputIt pos) {
    // @note  cast_iterator< container_type > is implicitly convertible to container_type::iterator, we can just call underlying erase() overload
    _data.erase(pos);
}



template < typename Container, typename T, bool Locked >
template< class InputIt, typename >
void indexer< Container, T, Locked >::erase(InputIt first, InputIt last) {
    // @note  cast_iterator< container_type > is implicitly convertible to container_type::iterator, we can just call underlying erase() overload
    _data.erase(first, last);
}



template < typename Container, typename T, bool Locked >
template < typename oT, typename, typename >
oT& indexer< Container, T, Locked >::front() {
    return _data.front();
}



template < typename Container, typename T, bool Locked >
template < typename oT, typename, typename >
const oT& indexer< Container, T, Locked >::front() const {
    return _data.front();
}



template < typename Container, typename T, bool Locked >
template < typename oT, typename, typename >
oT& indexer< Container, T, Locked >::back() {
    return _data.back();
}



template < typename Container, typename T, bool Locked >
template < typename oT, typename, typename >
const oT& indexer< Container, T, Locked >::back() const {
    return _data.back();
}



template < typename Container, typename T, bool Locked >
template < typename, typename, typename... Args, typename >
void indexer< Container, T, Locked >::emplace_front(Args&&... args) {
    _data.emplace_front(std::forward< Args >(args)...);  // for return type, compile with C++17!!
}



template < typename Container, typename T, bool Locked >
template < typename, typename, typename... Args, typename >
void indexer< Container, T, Locked >::emplace_front(const typename indexer< Container, T, Locked >::key_type& key, Args&&... args) {
    _data.emplace_front(key, std::forward< Args >(args)...);  // for return type, compile with C++17!!
}



template < typename Container, typename T, bool Locked >
template < typename, typename, typename... Args, typename >
void indexer< Container, T, Locked >::emplace_back(Args&&... args) {
    _data.emplace_back(std::forward< Args >(args)...);  // for return type, compile with C++17!!
}



template < typename Container, typename T, bool Locked >
template < typename, typename, typename... Args, typename >
void indexer< Container, T, Locked >::emplace_back(const typename indexer< Container, T, Locked >::key_type& key, Args&&... args) {
    _data.emplace_back(key, std::forward< Args >(args)...);  // for return type, compile with C++17!!
}



template < typename Container, typename T, bool Locked >
template < typename, typename >
void indexer< Container, T, Locked >::push_front(typename indexer< Container, T, Locked >::value_type&& value) {
    _data.push_front(std::forward< value_type >(value));
}



template < typename Container, typename T, bool Locked >
template < typename, typename >
void indexer< Container, T, Locked >::push_front(const indexer< Container, T, Locked >::key_type& key, typename indexer< Container, T, Locked >::value_type&& value) {
    _data.push_front(std::forward< value_type >(value));
    _data.front().key = key;
}



template < typename Container, typename T, bool Locked >
template < typename, typename >
void indexer< Container, T, Locked >::push_back(typename indexer< Container, T, Locked >::value_type&& value) {
    _data.push_back(std::forward< value_type >(value));
}



template < typename Container, typename T, bool Locked >
template < typename, typename >
void indexer< Container, T, Locked >::push_back(const indexer< Container, T, Locked >::key_type& key, typename indexer< Container, T, Locked >::value_type&& value) {
    _data.push_back(std::forward< value_type >(value));
    _data.back().key = key;
}



template < typename Container, typename T, bool Locked >
template < typename, typename >
void indexer< Container, T, Locked >::pop_front() {
    _data.pop_front();
}



template < typename Container, typename T, bool Locked >
template < typename, typename >
void indexer< Container, T, Locked >::pop_back() {
    _data.pop_back();
}


template < typename Container, typename T, bool Locked >
template < typename, typename >
void indexer< Container, T, Locked >::reserve(size_t new_capacity) {
    _data.reserve(new_capacity);
}

/// @endcond


//------------------------------------------------------------------------------
/// @brief      Utility constructor that allows inline initialization of an indexer object and its key set.
///
/// @param[in]  keys       Key values to be assigned to elements of resulting indexer<>.
/// @param[in]  args       Argument list to be passed to the constructor.
///
/// @tparam     Container  Generic container of key/tagged type instances.
///                        Must fit STL's Container & SequenceContainer (partial) requirements.
/// @tparam     T          Public/interface type.
/// @tparam     Locked     Whether runtime size manipulation is allowed after initialization.
///                        Useful when size of the container does not change but elements do.
/// @tparam     Args       Variadic parameter pack describing types of arguments passed to constructor.
///
/// @return     indexer<> instance with given *keys*.
///
template <typename Container, typename T = typename Container::value_type, bool Locked = false, typename... Args >
indexer< Container, T, Locked > make_indexer(const initializer_list< typename indexer< Container, T, Locked >::key_type > keys, Args... args) {
    auto obj = indexer< Container, T, Locked >(args...);
    obj.set_keys(keys);
    return obj;
}



//------------------------------------------------------------------------------
/// @brief      Public std::indexer specialization to std::vector with text keys, and generalizing over given type T through std::tagged<> indexable wrapper.
///
/// @note       Practical/intuitive specialization, exploiting std::indexer interface to provide a pseudo-container with associative properties.
///
/// @tparam     T       Data type (*not* the container data type!)
/// @tparam     Locked  Whether container is resizeable after initialization. Defaults to false (expandable container).
/// @tparam     Id      Type of key/identifier. Defautls to string.
///
template < typename T, typename Key = string, bool Locked = false>
using index = indexer< std::vector< std::tagged< T, Key > >, T, Locked >;

}  // namespace std

#endif  // TRIGNOCLIENT_INCLUDE_TRIGNOCLIENT_STD_REGISTER_HPP_
