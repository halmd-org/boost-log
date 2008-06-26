/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   filter_parser.hpp
 * \author Andrey Semashev
 * \date   31.03.2008
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_UTILITY_INIT_FILTER_PARSER_HPP_INCLUDED_
#define BOOST_LOG_UTILITY_INIT_FILTER_PARSER_HPP_INCLUDED_

#include <string>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/logging_core.hpp>

#ifdef _MSC_VER
#pragma warning(push)
// 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
// non dll-interface class 'A' used as base for dll-interface class 'B'
#pragma warning(disable: 4275)
#endif // _MSC_VER

namespace boost {

namespace BOOST_LOG_NAMESPACE {

//! The function parses a filter from the string
template< typename CharT >
BOOST_LOG_EXPORT typename basic_logging_core< CharT >::filter_type
parse_filter(const CharT* begin, const CharT* end);

//! The function parses a filter from the string
template< typename CharT, typename TraitsT, typename AllocatorT >
inline typename basic_logging_core< CharT >::filter_type
parse_filter(std::basic_string< CharT, TraitsT, AllocatorT > const& str)
{
    const CharT* p = str.c_str();
    return parse_filter(p, p + str.size());
}

//! The function parses a filter from the string
template< typename CharT >
inline typename basic_logging_core< CharT >::filter_type parse_filter(const CharT* str)
{
    return parse_filter(str, str + std::char_traits< CharT >::length(str));
}

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_UTILITY_INIT_FILTER_PARSER_HPP_INCLUDED_
