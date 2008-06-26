/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   prologue.hpp
 * \author Andrey Semashev
 * \date   08.03.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html. In this file
 *         an internal configuration macros are defined.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_DETAIL_PROLOGUE_HPP_INCLUDED_
#define BOOST_LOG_DETAIL_PROLOGUE_HPP_INCLUDED_

#include <boost/config.hpp>

#if defined(_MSC_VER) && !defined(_STLPORT_VERSION)
// MSVC 9.0 mandates packaging of STL classes, which apparently affects alignment and
// makes alignment_of< T >::value no longer be a power of 2 for types that derive from STL classes.
// This breaks type_with_alignment and everything that relies on it.
// This doesn't happen with non-native STLs, such as STLPort. Strangely, this doesn't show with
// STL classes themselves or most of the user-defined derived classes.
// Not sure if that happens with other MSVC versions.
// See: http://svn.boost.org/trac/boost/ticket/1946
#   define BOOST_LOG_BROKEN_STL_ALIGNMENT
#endif

#if defined(BOOST_MSVC)
// For some reason MSVC 9.0 fails to link the library if static integral constants are defined in cpp
#   define BOOST_LOG_BROKEN_STATIC_CONSTANTS_LINKAGE
#endif

#if (defined __SUNPRO_CC) && (__SUNPRO_CC <= 0x530) && !(defined BOOST_NO_COMPILER_CONFIG)
    // Sun C++ 5.3 can't handle the safe_bool idiom, so don't use it
#   define BOOST_NO_UNSPECIFIED_BOOL
#endif // (defined __SUNPRO_CC) && (__SUNPRO_CC <= 0x530) && !(defined BOOST_NO_COMPILER_CONFIG)

// Extended declaration macros. Used to implement compiler-specific optimizations.
#if defined(_MSC_VER)
#    define BOOST_LOG_FORCEINLINE __forceinline
#    define BOOST_LOG_NO_VTABLE __declspec(novtable)
#elif defined(__GNUC__)
#    if (__GNUC__ > 3)
#        define BOOST_LOG_FORCEINLINE inline __attribute__((always_inline))
#    else
#        define BOOST_LOG_FORCEINLINE inline
#    endif
#    define BOOST_LOG_NO_VTABLE
#else
#    define BOOST_LOG_FORCEINLINE inline
#    define BOOST_LOG_NO_VTABLE
#endif

// An MS-like compilers' extension that allows to optimize away the needless code
#if defined(_MSC_VER)
#    define BOOST_LOG_ASSUME(expr) __assume(expr)
#else
#    define BOOST_LOG_ASSUME(expr)
#endif

#if !defined(BOOST_LOG_BUILDING_THE_LIB)

// Detect if we're dealing with dll
#   if defined(BOOST_LOG_DYN_LINK) || defined(BOOST_ALL_DYN_LINK)
#        define BOOST_LOG_DLL
#   endif

#    if defined(BOOST_HAS_DECLSPEC) && defined(BOOST_LOG_DLL)
#        define BOOST_LOG_EXPORT __declspec(dllimport)
#    else
#        define BOOST_LOG_EXPORT
#    endif // defined(BOOST_HAS_DECLSPEC)
//
// Automatically link to the correct build variant where possible. 
// 
#    if !defined(BOOST_ALL_NO_LIB) && !defined(BOOST_LOG_NO_LIB)
#        define BOOST_LIB_NAME boost_log
#        if defined(BOOST_LOG_DLL)
#            define BOOST_DYN_LINK
#        endif
#        include <boost/config/auto_link.hpp>
#    endif  // auto-linking disabled

#else // !defined(BOOST_LOG_BUILDING_THE_LIB)

#    if defined(BOOST_HAS_DECLSPEC) && defined(BOOST_LOG_DLL)
#        define BOOST_LOG_EXPORT __declspec(dllexport)
#    elif defined(__GNUC__) && __GNUC__ >= 4 && (defined(linux) || defined(__linux) || defined(__linux__))
#        define BOOST_LOG_EXPORT __attribute__((visibility("default")))
#    else
#        define BOOST_LOG_EXPORT
#    endif

#endif // !defined(BOOST_LOG_BUILDING_THE_LIB)

// Define BOOST_LOG_USE_SYSLOG if any syslog API should be supported
#if !defined(BOOST_LOG_USE_SYSLOG) && defined(BOOST_LOG_USE_NATIVE_SYSLOG)
#    define BOOST_LOG_USE_SYSLOG
#endif // !defined(BOOST_LOG_USE_SYSLOG) && defined(BOOST_LOG_USE_NATIVE_SYSLOG)

#if !defined(BOOST_LOG_USE_CHAR) && !defined(BOOST_LOG_USE_WCHAR_T)
// By default we provide support for both char and wchar_t
#    define BOOST_LOG_USE_CHAR
#    define BOOST_LOG_USE_WCHAR_T
#endif // !defined(BOOST_LOG_USE_CHAR) && !defined(BOOST_LOG_USE_WCHAR_T)

// Check if multithreading is supported
#if !defined(BOOST_LOG_NO_THREADS) && !defined(BOOST_HAS_THREADS)
#   define BOOST_LOG_NO_THREADS
#endif // !defined(BOOST_LOG_NO_THREADS) && !defined(BOOST_HAS_THREADS)

namespace boost {

// Setup namespace name
#if !defined(BOOST_LOG_DOXYGEN_PASS)
#   if defined(BOOST_LOG_NO_THREADS)
namespace log_st {}
namespace log = log_st;
#       define BOOST_LOG_NAMESPACE log_st
#   else
namespace log_mt {}
namespace log = log_mt;
#       define BOOST_LOG_NAMESPACE log_mt
#   endif // defined(BOOST_LOG_NO_THREADS)
#else
namespace log {}
#   define BOOST_LOG_NAMESPACE log
#endif // !defined(BOOST_LOG_DOXYGEN_PASS)

} // namespace boost

#endif // BOOST_LOG_DETAIL_PROLOGUE_HPP_INCLUDED_
