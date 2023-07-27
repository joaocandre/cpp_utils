//------------------------------------------------------------------------------
/// @file       stream.hpp
/// @author     João André
///
/// @brief      Header file providing declaration and definition of free generic global functions
///             that provide higher-level functionality when working with input/ouput streams.
///
//------------------------------------------------------------------------------

#ifndef CPPUTILS_INCLUDE_EXTRA_STREAM_HPP_
#define CPPUTILS_INCLUDE_EXTRA_STREAM_HPP_

#include <string>
#include <sstream>
#include <vector>
#include <limits>
#include <iostream>
#include <utility>
#include "type/check.hpp"  // std::is_iteratable<>

namespace std {

//------------------------------------------------------------------------------
/// @brief      Split text string into multiple sub-strings according to a given delimiter character.
///
/// @param[in]  data       Input text string.
/// @param[in]  separator  Delimiter/separator character.
///
/// @return     Vector of words/tokens matching substrings delimited by separator char. Empty vector if separator is not found.
///
template < typename C = vector< string > >
inline C tokenize(const string& data, char separator = ',', bool single_split = false) {
    C words;
    size_t start;
    size_t end = 0;

    while ((start = data.find_first_not_of(separator, end)) != std::string::npos) {
        end = data.find(separator, start);
        words.push_back(data.substr(start, end - start));
        if (single_split && end != string::npos) {
            words.push_back(data.substr(end + 1));
            break;
        }
    }

    return words;
}

//------------------------------------------------------------------------------
/// @brief      Replaces occurrences of "old_text" with "new_text" in given text string
///
/// @param      source    The source
/// @param[in]  old_text  The old text
/// @param[in]  new_text  The new text
///
/// @return     { description_of_the_return_value }
///
inline size_t replace(string* source, const string& old_text, const string& new_text) {
    size_t count = 0;
    auto pos = source->find(old_text);

    while (pos != std::string::npos) {
        source->replace(pos, old_text.size(), new_text);
        count++;
        pos = source->find(old_text);
    }

    return count;  // += replace(source.substr(pos), old_text, new_text);
}


//------------------------------------------------------------------------------
/// @brief      Converts a numeric value to text string with variable precision.
///             Overloads std::string with additional precision argument.
///
/// @param[in]  value      Value to convert
/// @param[in]  precision  Pecision setting i.e. number of decimals.
///
/// @tparam     T          Type of given *value*.
///
/// @return     Given *value* serialized as text string.
///
/// @note       cf. https://stackoverflow.com/questions/16605967/set-precision-of-stdto-string-when-converting-floating-point-values
///
/// @todo       Move to io.hpp ?
///
template< typename T, typename = typename std::enable_if< std::is_arithmetic< T >::value >::type >
std::string to_string(const T& value, size_t precision = 2) {
    std::ostringstream out;
    out.precision(precision);
    out << std::fixed << value;
    return std::move(out).str();
}


//------------------------------------------------------------------------------
/// @brief      Generates a string from *args* w/ given *format*.
///
/// @param[in]  format     Format to apply to output string.
/// @param      args       Arguments of given *format*, equivalent to standard format specifiers (e.g. printf).
///
/// @tparam     MaxLength  Buffer size i.e. maximum length of output string. Defaults to 50.
/// @tparam     Args       Variadic parameter pack encoding the types of *args*.
///
/// @return     Text string with formated *args* according to *format*.
///
/// @note       Wraps around snprintf and std::string constructor, simplifying usage wo/ the need to instantiate a temporary buffer.
///
/// @todo       Purpose may be similar to std::format<> (cf. https://en.cppreference.com/w/cpp/utility/format/format), rename function?
///
/// @todo       Move to io.hpp?
///
template < size_t MaxLength = 50, typename... Args >
std::string format(const std::string& format, Args&&... args) {
    // char buffer[3 * format.length()];
    char buffer[MaxLength];
    snprintf(buffer, sizeof(buffer), format.data(), std::forward< Args >(args)...);

    return std::string(buffer);
}


//------------------------------------------------------------------------------
/// @brief      Skips/consumes input stream until character
///
/// @param      istream  Input stream.
/// @param[in]  target   Stop character.
///
/// @return     Modified input stream.
///
/// @note       Wrapper function provided for convenience/verbosity/cleaner syntax.
///
inline istream& ignore_until(istream& istream, char target) {
    // if (istream.peek() != target) {
    istream.ignore(std::numeric_limits< std::streamsize >::max(), target);
    // }
    return istream;
}


//------------------------------------------------------------------------------
/// @brief      Skips/consumes input stream until any of given 'delimiters' is found.
///
/// @param      istream  Input stream.
/// @param[in]  targets  List of stop characters.
///
/// @return     Modified input stream.
///
inline istream& ignore_until(istream& istream, const std::initializer_list< char >& targets) {
    char t = istream.peek();
    while (istream.good()) {
        for (auto target : targets) {
            if (t == target) {
                return istream;
            }
        }
        istream >> t;
    }
    // istream.ignore(std::numeric_limits< std::streamsize >::max(), target);
    return istream;
}


//------------------------------------------------------------------------------
/// @brief      Skips input stream until next line.
///
/// @param      istream  Input stream.
///
/// @return     Modified input stream.
///
/// @note       Wrapper function provided for convenience/verbosity/cleaner syntax.
///
inline istream& skipline(istream& istream) {
    return ignore_until(istream, '\n');
}


//------------------------------------------------------------------------------
/// @brief      Generic formatted printer function, for range/container-like types.
///
/// @param[out] ostream    The stream to write data to
/// @param[in]  range      Input range/container. Left shift (<<) stream operator is called for each element.
/// @param[in]  delimiter  Delimiter text/character. Defaults to ','.
///
/// @tparam     T          Range/container type. Must be iteratable (provide begin & end dereferenceable iterators)
///
/// @return     Modified output stream.
///
/// @note       Offers performance advatanges over std::formatted<>, as it writes directly into the output stream,
///             and does not instantiate a new object.
///
template < typename T >
inline ostream& print_into(ostream& ostream, const T& input, char delimiter = ',') {
    if constexpr(!std::is_range_iteratable< T >()) {
        // for non-iteratable types
        ostream << input;
    } else {
        // for iteratable types (e.g. containers), recursive call
        auto begin = input.begin();
        auto end = input.end();

        if (begin != end) {
            print_into(ostream, *begin, delimiter);
            while (++begin != end) {
                ostream << delimiter;
                print_into(ostream, *begin, delimiter);
            }
        }
    }
    return ostream;
}


//------------------------------------------------------------------------------
/// @brief      Loads values from an input stream into a range/container.
///
/// @param[out] istream       The stream to read data from.
/// @param[in]  range         Output range. Right shift (>>) stream operator is called for each element.
/// @param[in]  delimiter     Delimiter text/character. Defaults to ','.
/// @param[in]  ignore_break  Ignore break flag. If true (default), assumes new line character as a delimiter and will load continuously.
///                           Otherwise will stop @ end of line.
///
/// @tparam     Range      Range/container type. Must be iteratable (provide begin & end dereferenceable iterators)
///
/// @return     Modified (consumed) output stream.
///
template < typename T >
inline istream& load_from(istream& istream, T& input, char delimiter = ',', bool ignore_break = true) {
    if constexpr(!std::is_range_iteratable< T >()) {
        // if *ignore_new_line* is true, return @ '\n' character
        // in top-most iteratable types, will only use a single line to load data
        if (istream.peek() == '\n' && ignore_break) return istream;  // skipline(istream);
        // in case first value is a delimiter char (bad data -> should account for that case here?)
        if (istream.peek() == delimiter) ignore_until(istream, delimiter);
        // for non-iteratable types, use shift operator (must be defined!)
        // if constexpr (is_loadable< T >()) istream >> input;
        istream >> input;
        // ignore streeam until next delimiter *unless* at end of line
        if (istream.peek() != '\n') ignore_until(istream, delimiter);
    } else {
        // iterate through source, call load_from() recursively
        auto begin = input.begin();
        auto end = input.end();

        size_t valid_pos = 0;
        while (begin != end && istream.good()) {
            // recursive call
            load_from(istream, *begin, delimiter);
            // increment iterator
            begin++;
        }
    }
    return istream;
}


//------------------------------------------------------------------------------
/// @brief      Formatted pseudo-type holding delimiter char and reference/pointer to source range/container.
///
/// @tparam     T     Source data type i.e. container/range-like type.
///
/// @note       Resulting syntax is simple enough, but implementation of stream IO is defined outside of class,
///             directly into operator overloads, which may not be a particularly good practice.
///
/// @note       Common usage would be `std::cout << formatted(obj, ",") << std::endl`
///
template < typename T >
struct formatted {
    //--------------------------------------------------------------------------
    /// @brief      Constructs a new instance.
    ///
    /// @param[in]  data       Input data to print.
    /// @param[in]  delimiter  Delimiter character. defaults to ','
    ///
    explicit formatted(T* data, char delimiter = ',') :
        src(data), del(delimiter) { /* ... */ }

    operator std::string() const {
        std::stringstream ss;
        ss << (*this);
        return ss.str();  // should be std::move(ss).str()?
    }

    template < typename U >
    friend std::ostream& operator<<(std::ostream& ostream, const std::formatted< U >& f);

    template < typename U >
    friend std::istream& operator>>(std::istream& istream, std::formatted< U >&& f);

 private:
    T* src;
    char del;
};


//------------------------------------------------------------------------------
/// @brief      Left shift / output stream operator overload for std::formatted.
///
/// @param      ostream  The stream to write data to.
/// @param[in]  f        Instance of std::formatted to print.
///
/// @tparam     U        Underlying range/container type.
///
/// @return     Modified output stream.
///
template < typename U >
inline std::ostream& operator<<(std::ostream& ostream, const std::formatted< U >& f) {
    return std::print_into(ostream, *(f.src), f.del);
}


//------------------------------------------------------------------------------
/// @brief      Right shift / input stream operator overload for std::formatted.
///
/// @param      istream  The stream to load data from.
/// @param[in]  f        Instance of std::formatted to print.
///
/// @tparam     U        Underlying range/container type.
///
/// @return     Modified input stream.
///
template < typename U >
inline std::istream& operator>>(std::istream& istream, std::formatted< U >&& f) {
    return std::load_from(istream, *(f.src), f.del);
}

//------------------------------------------------------------------------------
/// @brief      Output stream modifier turning text output to boldface.
///
/// @param      os    Output stream
///
/// @return     Modified output stream - further stream operations *with* bold text.
///
inline std::ostream& bold_on(std::ostream& os) {
    return os << "\e[1m";
}


//------------------------------------------------------------------------------
/// @brief      Output stream modifier disabling boldface output.
///
/// @param      os    Output stream
///
/// @return     Modified output stream - further stream operations *without* bold text.
///
inline std::ostream& bold_off(std::ostream& os) {
    return os << "\e[0m";
}


}  // namespace std

#endif  // TRIGNOCLIENT_INCLUDE_TRIGNOCLIENT_STD_UTILS_HPP_
