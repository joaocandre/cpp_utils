//------------------------------------------------------------------------------
/// @file       stream.hpp
/// @author     João André
///
/// @brief      I/O stream utilities, including shift operator overloads
///
/// @todo       Missing some overloads for std::volume and std::st_subset_base
///
/// @note       print_into() and load_from() could be included and employed (should work with iteratable types in any case),
///             but these implementations specialize towards storage types (e.g. formatting)
///
//------------------------------------------------------------------------------

#ifndef STORAGE_INCLUDE_STORAGE_STREAM_HPP_
#define STORAGE_INCLUDE_STORAGE_STREAM_HPP_

#include <iostream>
#include "storage/matrix.hpp"
#include "storage/volume.hpp"
#include "storage/subset.hpp"

//------------------------------------------------------------------------------
// @note        Default delimiter, employed by left/right shift operators
//              Conditional check allows user to compile trignoclient w/ a different default delimiter
//
#ifndef DEFAULT_DELIMITER
#define DEFAULT_DELIMITER ','
#endif


namespace std {

//------------------------------------------------------------------------------
/// @brief      Writes a matrix to an output stream (e.g. cout) in a serialized form.
///
/// @param      ostream    The stream to write data to.
/// @param[in]  mat        Matrix to write.
/// @param[in]  delimiter  Delimiter character. Defaults to DEFAULT_DELIMITER.
/// @param[in]  formatted  Wether to write matrix w/ formatted output (each row on new line, shape at bottom right corner).
///                        Defaults to true.
///
/// @tparam     T          Element data type. *Must* have its own output stream operator overload defined.
///
/// @return     Modified output stream.
///
template < typename T >
ostream& write(ostream& os, const matrix< T >& mat, char delimiter = DEFAULT_DELIMITER, bool formatted = true);

template < typename T >
ostream& write(ostream& os, const volume< T >& vol, char delimiter = DEFAULT_DELIMITER, bool formatted = true);

template < typename CT >
ostream& write(ostream& os, const storage::st_subset_base< CT >& st_subset, char delimiter = DEFAULT_DELIMITER, bool formatted = true);


//------------------------------------------------------------------------------
/// @brief      Reads from an input stream into a matrix object.
///
/// @param      istream       The stream to read data from.
/// @param      mat           Matrix to be populated.
/// @param[in]  delimiter     Delimiter character. Defaults to DEFAULT_DELIMITER.
/// @param[in]  ignore_break  Indicates if line break are to be ignored, otherwise input stops at first break. Defaults to false.
///
/// @tparam     T             Element data type. *Must* have its own input stream operator overload defined.
///
/// @return     Modified input stream.
///
/// @note       Matrix needs to be initialized beforehand, it is *not* reshapen. Only the number of elements of the matrix are read.
///
template < typename T >
istream& read(istream& is, matrix< T >& mat, char delimiter = DEFAULT_DELIMITER, bool ignore_break = false);

template < typename T >
istream& read(istream& is, volume< T >& vol, char delimiter = DEFAULT_DELIMITER, bool ignore_break = false);

template < typename T >
istream& read(istream& is, storage::st_subset_base< T >& st_subset, char delimiter = DEFAULT_DELIMITER, bool ignore_break = false);


//------------------------------------------------------------------------------
/// @brief      Left shift operator overload, for output streams.
///
/// @param      istream  The stream to read data from.
/// @param      mat      Matrix to be populated.
///
/// @tparam     T        Element data type. *Must* have its own output stream operator overload defined.
///
/// @return     Modified output stream.
///
/// @note       Wraps around read() with default arguments, for convenience.
///
template < typename T >
ostream& operator<<(ostream& ostream, const std::matrix< T >& mat) {
    std::cout.precision(5);
    return write(ostream, mat, '\t');
}

template < typename T >
ostream& operator<<(ostream& ostream, const std::volume< T >& vol) {
    std::cout.precision(5);
    return write(ostream, vol, '\t');
}

template < typename CT >
ostream& operator<<(ostream& ostream, const storage::st_subset_base< CT >& subset) {
    std::cout.precision(5);
    return write(ostream, subset, '\t');
}

// // template < typename T, typename ST, typename = typename enable_if< is_same< ST, matrix< T > >::value || is_same< ST, volume< T > >::value || is_same< ST, storage::st_subset_base< T > >::value >::type >
// template < typename ST, typename T, typename = typename enable_if< is_same< ST, storage::st_subset_base< T > >::value >::type >
// ostream& operator<<(ostream& ostream, const ST< T >& subset) {
//     std::cout.precision(5);
//     return write(ostream, subset, '\t');
// }

//------------------------------------------------------------------------------
/// @brief      Left shift operator overload, for output streams.
///
/// @param      ostream  The stream to write data to.
/// @param[in]  mat      Matrix object to print/write.
///
/// @tparam     T        Element data type. *Must* have its own input stream operator overload defined.
///
/// @return     Modified input stream.
///
/// @note       Wraps around write() with default arguments, for convenience.
///
template < typename T >
std::istream& operator>>(std::istream& istream, std::matrix< T >& mat) {
    return read(istream, mat);
}

/// ... add std::volume<> overload
/// ... add std::st_subset_base<> overload


//------------------------------------------------------------------------------
/// @cond

template < typename T >
ostream& write(ostream& ostream, const matrix< T >& mat, char delimiter, bool formatted) {
    for (size_t row = 0; row < mat.rows(); row++) {
        ostream << mat(row, 0);
        for (size_t col = 1; col < mat.cols(); col++) {
            ostream << delimiter << mat(row, col);
        }
        if (formatted && row < mat.rows() - 1) {
            ostream << "\n";
        }
    }
    if (formatted) {
        ostream << " [" << mat.rows() << " x " << mat.cols() << "]";
    }
    return ostream;
}

/// ... add std::volume<> overload

template < typename CT >
ostream& write(ostream& ostream, const storage::st_subset_base< CT >& subset, char delimiter, bool formatted) {
    ostream << subset[0];
    for (size_t idx = 1; idx < subset.size(); idx++) {
        ostream << delimiter << subset[idx];
    }
    if (formatted) {
        ostream << "\n";
        ostream << " [" << subset.size() << "]";
    }
    return ostream;
}

template < typename T >
istream& read(istream& istream, matrix< T >& mat, char delimiter, bool ignore_break) {
    for (auto& val : mat.container()) {
        istream >> val;
    }
    return istream;
}

/// ... add std::volume<> overload
/// ... add std::st_subset_base<> overload

/// @endcond
//------------------------------------------------------------------------------

}  // namespace std

#endif  // STORAGE_INCLUDE_STORAGE_STREAM_HPP_
