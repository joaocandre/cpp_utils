#ifndef CPPUTILS_INCLUDE_EXTRA_CHRONO_HPP_
#define CPPUTILS_INCLUDE_EXTRA_CHRONO_HPP_

#include <chrono>
#include <string>
#include <algorithm>

namespace std::chrono {

//------------------------------------------------------------------------------
/// @brief      Class implementing a simple stopwatch to profile blocks of code.
///
/// @note       Wraps around/abstract from usage of <chrono> clocks.
///
class stopwatch {
 public:
    //--------------------------------------------------------------------------
    /// @brief      Start measurement.
    ///
    void tic() { _start = std::chrono::steady_clock::now(); }

    //--------------------------------------------------------------------------
    /// @brief      Stop measurement.
    ///
    void toc() { _end = std::chrono::steady_clock::now(); }

    //--------------------------------------------------------------------------
    /// @brief      Reset (ongoing) measurement.
    ///
    void reset() { tic(); _end = _start; }

    //--------------------------------------------------------------------------
    /// @brief      { function_description }
    ///
    /// @return     { description_of_the_return_value }
    ///
    double value() const { return std::chrono::duration< double > (_end - _start).count(); }
    double peek() const { auto now = std::chrono::steady_clock::now(); return std::chrono::duration< double > (now - _start).count(); }
    double operator()() const { return value(); }

 protected:
    std::chrono::time_point< std::chrono::steady_clock > _start;
    std::chrono::time_point< std::chrono::steady_clock > _end;
};


//------------------------------------------------------------------------------
/// @todo   add function that executes function N times and returns average duration.
///         check usage of lambdas in trignoclient library
///
/// ...

//------------------------------------------------------------------------------
/// @brief      Convert number of seconds to days.
///
/// @param[in]  seconds    Number of seconds to conver.
/// @param[in]  modifier   Value modifier applied to given *seconds* amount. Defaults to 0.
///                        Useful when convertting an absolute duration to time components (e.g. date).
///
inline double stod(double seconds, double mod = 0.0) {
    return ((seconds + mod) / (24 * 3600.0));
}

//------------------------------------------------------------------------------
/// @brief      Convert number of seconds to hours.
///
/// @param[in]  seconds    Number of seconds to conver.
/// @param[in]  modifier   Value modifier applied to given *seconds* amount. Defaults to 0.
///                        Useful when convertting an absolute duration to time components (e.g. date).
///
inline double stoh(double seconds, double mod = 0.0) {
    return ((seconds + mod) / 3600.0);
}

//------------------------------------------------------------------------------
/// @brief      Convert number of seconds to minutes.
///
/// @param[in]  seconds    Number of seconds to conver.
/// @param[in]  modifier   Value modifier applied to given *seconds* amount. Defaults to 0.
///                        Useful when convertting an absolute duration to time components (e.g. date).
///
inline double stom(double seconds, double mod = 0.0) {
    return ((seconds + mod) / 60.0);
}

// //------------------------------------------------------------------------------
// /// @brief      Convert number of seconds to a text string.
// ///             Optionally, can segment *sec* into day, hour, minute and second count, according to given *format*.
// ///
// /// @param[in]  etc          Duration value to convert (in seconds).
// /// @param[in]  format       Text format of returned string.
// /// @param[in]  null         Null character to place instead of numeric values when given *sec* is invalid i.e. < 0.0. Defaults to '-'.
// /// @param[in]  placeholder  Placeholder value to use before substituting w/ null charater when given *sec* is invalid i.e. < 0.0. Defaults to 0.
// ///                          Useful to parametrize as format may already include value (as character) and therefore substitution would mangle output.
// ///
// /// @return     Text string with value of given *seconds* in given *format*.
// ///
// template < size_t MaxLength = 50 >
// inline std::string progress::stos(double seconds, const std::string& format, char null, size_t placeholder) {
//     // auto-check if detailed or not (number of values)
//     // @note    adds unnecessary overhead, although it may be negligible
//     // @todo    make function dynamically segment input according to format (discard "detailed" flag)
//     // size_t n = std::count(format.begin(), format.end(), '%');
//     std::string out = "";
//     if (detailed) {
//         if (sec >= 0.0) {
//             // convert total second count to hours + minutes + seconds
//             float hours = stoh(sec);
//             float mins  = stom(3600.0 * (hours - static_cast< size_t >(hours)));
//             float secs  = 60.0 * (mins - static_cast< size_t >(mins));
//             out += std::format< 20 >(format, static_cast< size_t >(hours), static_cast< size_t >(mins), secs);
//         } else {
//             // use null values and replace '0' with null charater
//             // @note    only works if there are no additional '0' on format
//             out += std::format< 20 >(format, placeholder, placeholder, placeholder);
//             std::replace(out.begin(), out.end(), static_cast< char >(placeholder + 48), null);
//         }
//     } else {
//         if (sec >= 0.0) {
//             // print sec value directly
//             out += std::format< 20 >(format, sec);
//         } else {
//             // use null values and replace '0' with null charater
//             // @note    only works if there are no additional '0' on format
//             out += std::format< 20 >(format, placeholder);
//             std::replace(out.begin(), out.end(), static_cast< char >(placeholder + 48), null);
//         }
//     }

//     return out;
// }


}  // namespace std::chrono

#endif  // CPPUTILS_INCLUDE_EXTRA_CHRONO_HPP_
