//------------------------------------------------------------------------------
/// @file       exception.hpp
/// @author     João André
///
/// @brief      Header file providing custom exceptions 'std::not_implemented' and 'std::timed_out'.
///
/// std::not_implemented is a useful execption derived from std::logic_error intended to be used
/// while implementations are still being developed and not ready for deployment.
///
/// std::timed_out is derived from std::runtime_error intended to ease handling IO read/writes i.e.
/// signaling a recoverable specific timed context.
/// 
//------------------------------------------------------------------------------

#ifndef _TRIGNOCLIENT_INCLUDE_TRIGNOCLIENT_STD_EXCEPTION_HPP_
#define _TRIGNOCLIENT_INCLUDE_TRIGNOCLIENT_STD_EXCEPTION_HPP_

#include <string>
#include <stdexcept>

namespace std {

//------------------------------------------------------------------------------
/// @brief      This class describes a 'not implemented' exception to signal currently missing features.
///
class not_implemented : public logic_error {
 public:
    //--------------------------------------------------------------------------
    /// @brief      Construct a new instance.
    ///
    explicit not_implemented(const std::string& what = "") : logic_error("Function not yet implemented" + (what.empty() ? (": " + what) : ".")) { /* ... */ }

    //--------------------------------------------------------------------------
    /// @brief      Destroys the object.
    ///
    // virtual ~not_implemented() throw() { /* ... */ }

    //--------------------------------------------------------------------------
    /// @brief      Get description string
    ///
    // virtual char const * what() const { return "Function not yet implemented."; }
};



//------------------------------------------------------------------------------
/// @brief      This class describes a 'timed out' exception to be used time-sensitive implementations.
///
class timed_out : public runtime_error {
 public:
    //--------------------------------------------------------------------------
    /// @brief      Construct a new instance.
    ///
    explicit timed_out(const std::string& what = "") : runtime_error("Operation timed out" + (what.empty() ? (": " + what) : ".")) { /* ... */ }

    //--------------------------------------------------------------------------
    /// @brief      Destroys the object.
    ///
    // virtual ~timed_out() throw() { /* ... */ }

    //--------------------------------------------------------------------------
    /// @brief      Get description string
    ///
    // virtual char const * what() const { return "Function has timed out."; }
};

}  // namespace std

#endif  // _TRIGNOCLIENT_INCLUDE_TRIGNOCLIENT_STD_EXCEPTION_HPP_
