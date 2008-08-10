/*
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * This header is the Boost.Log library implementation, see the library documentation
 * at http://www.boost.org/libs/log/doc/log.html.
 */
/*!
 * \file   filter_parser.hpp
 * \author Andrey Semashev
 * \date   31.03.2008
 * 
 * The header contains definition of a filter parser function.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_UTILITY_INIT_FILTER_PARSER_HPP_INCLUDED_
#define BOOST_LOG_UTILITY_INIT_FILTER_PARSER_HPP_INCLUDED_

#include <string>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/core.hpp>

#ifdef _MSC_VER
#pragma warning(push)
// 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
// non dll-interface class 'A' used as base for dll-interface class 'B'
#pragma warning(disable: 4275)
#endif // _MSC_VER

namespace boost {

namespace BOOST_LOG_NAMESPACE {

/*!
 * The function parses a filter from the sequence of characters
 * 
 * \pre <tt>begin <= end</tt>
 * \param begin Pointer to the first character of the sequence. Must not be NULL.
 * \param end Pointer to the after-the-last character of the sequence. Must not be NULL.
 * \return A function object that can be used as a filter.
 * \throw An <tt>std::exception</tt>-based exception, if a filter cannot be recognized in the character sequence.
 */
template< typename CharT >
BOOST_LOG_EXPORT
#ifndef BOOST_LOG_BROKEN_TEMPLATE_DEFINITION_MATCHING
typename basic_core< CharT >::filter_type
#else
function1< bool, basic_attribute_values_view< CharT > const& >
#endif
parse_filter(const CharT* begin, const CharT* end);

/*!
 * The function parses a filter from the string
 * 
 * \param str A string that contains filter description
 * \return A function object that can be used as a filter.
 * \throw An <tt>std::exception</tt>-based exception, if a filter cannot be recognized in the character sequence.
 */
template< typename CharT, typename TraitsT, typename AllocatorT >
inline typename basic_core< CharT >::filter_type
parse_filter(std::basic_string< CharT, TraitsT, AllocatorT > const& str)
{
    const CharT* p = str.c_str();
    return parse_filter(p, p + str.size());
}

/*!
 * The function parses a filter from the string
 * 
 * \param str A string that contains filter description. Must point to a zero-terminated character sequence,
 *            must not be NULL.
 * \return A function object that can be used as a filter.
 * \throw An <tt>std::exception</tt>-based exception, if a filter cannot be recognized in the character sequence.
 */
template< typename CharT >
inline typename basic_core< CharT >::filter_type parse_filter(const CharT* str)
{
    return parse_filter(str, str + std::char_traits< CharT >::length(str));
}

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_UTILITY_INIT_FILTER_PARSER_HPP_INCLUDED_
