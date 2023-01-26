#ifndef STORAGE_INCLUDE_STORAGE_IO_HPP_
#define STORAGE_INCLUDE_STORAGE_IO_HPP_

#include <iostream>
#include "matrix.hpp"
#include "volume.hpp"
#include "subset.hpp"

//------------------------------------------------------------------------------
// @note        Default vaue delimiter, employed by left/right shift operators
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


//------------------------------------------------------------------------------

/// @cond

template < typename T >
ostream& write(ostream& ostream, const matrix< T >& mat, char delimiter, bool formatted) {
    for (size_t row = 0; row < mat.rows(); row++) {
        ostream << mat(row, 0);
        for (size_t col = 1; col < mat.cols(); col++) {
            ostream << delimiter << mat(row, col);
            // if (!formatted || col < mat.cols() - 1) {
            //     ostream << ", ";
            // }
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


template < typename T >
istream& read(istream& istream, matrix< T >& mat, char delimiter, bool ignore_break) {
    for (auto& val : mat.container()) {
        istream >> val;
    }
    return istream;
}



/// @endcond

}  // namespace std

#endif  // STORAGE_INCLUDE_STORAGE_IO_HPP_
