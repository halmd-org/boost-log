/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   standard_types.hpp
 * \author Andrey Semashev
 * \date   19.05.2007
 *
 * The header contains definition of standard types supported by the library by default.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_STANDARD_TYPES_HPP_INCLUDED_
#define BOOST_LOG_STANDARD_TYPES_HPP_INCLUDED_

#include <string>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/joint_view.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/utility/string_literal_fwd.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

/*!
 * An MPL-sequence of integral types of attributes, supported by default
 */
typedef mpl::vector<
    bool,
    char,
#if !defined(BOOST_NO_INTRINSIC_WCHAR_T)
    wchar_t,
#endif
    signed char,
    unsigned char,
    short,
    unsigned short,
    int,
    unsigned int,
    long,
    unsigned long
#if defined(BOOST_HAS_LONG_LONG)
    , long long
    , unsigned long long
#endif // defined(BOOST_HAS_LONG_LONG)
>::type integral_types;

/*!
 * An MPL-sequence of FP types of attributes, supported by default
 */
typedef mpl::vector<
    float,
    double,
    long double
>::type floating_point_types;

/*!
 * An MPL-sequence of all numeric types of attributes, supported by default
 */
typedef mpl::joint_view<
    integral_types,
    floating_point_types
>::type numeric_types;

/*!
 * An MPL-sequence of string types of attributes, supported by default
 */
typedef mpl::vector<
#ifdef BOOST_LOG_USE_CHAR
    std::string,
    string_literal
#ifdef BOOST_LOG_USE_WCHAR_T
    ,
#endif
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
    std::wstring,
    wstring_literal
#endif
>::type string_types;

/*!
 * An MPL-sequence of all attribute value types that are supported by the library by default.
 */
typedef mpl::joint_view<
    numeric_types,
    string_types
>::type default_attribute_types;

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_STANDARD_TYPES_HPP_INCLUDED_
