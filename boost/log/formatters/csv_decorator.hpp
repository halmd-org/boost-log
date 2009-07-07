/*
 * (C) 2009 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * This header is the Boost.Log library implementation, see the library documentation
 * at http://www.boost.org/libs/log/doc/log.html.
 */
/*!
 * \file   csv_decorator.hpp
 * \author Andrey Semashev
 * \date   07.06.2009
 *
 * The header contains implementation of CSV-style decorator. See:
 * http://en.wikipedia.org/wiki/Comma-separated_values
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_FORMATTERS_CSV_DECORATOR_HPP_INCLUDED_
#define BOOST_LOG_FORMATTERS_CSV_DECORATOR_HPP_INCLUDED_

#include <boost/range/iterator_range.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/formatters/char_decorator.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace formatters {

namespace aux {

    template< typename >
    struct csv_decorator_traits;

#ifdef BOOST_LOG_USE_CHAR
    template< >
    struct csv_decorator_traits< char >
    {
        static const char* const g_From[1];
        static const char* const g_To[1];
    };
    const char* const csv_decorator_traits< char >::g_From[1] = { "\"" };
    const char* const csv_decorator_traits< char >::g_To[1] = { "\"\"" };
#endif // BOOST_LOG_USE_CHAR

#ifdef BOOST_LOG_USE_WCHAR_T
    template< >
    struct csv_decorator_traits< wchar_t >
    {
        static const wchar_t* const g_From[1];
        static const wchar_t* const g_To[1];
    };
    const wchar_t* const csv_decorator_traits< wchar_t >::g_From[1] = { L"\"" };
    const wchar_t* const csv_decorator_traits< wchar_t >::g_To[1] = { L"\"\"" };
#endif // BOOST_LOG_USE_WCHAR_T

    struct fmt_csv_decorator_gen
    {
        template< typename FormatterT >
        fmt_char_decorator< FormatterT > operator[] (FormatterT const& fmt) const
        {
            typedef fmt_char_decorator< FormatterT > decorator;
            typedef csv_decorator_traits< typename decorator::char_type > traits_t;
            return decorator(
                fmt,
                boost::make_iterator_range(traits_t::g_From),
                boost::make_iterator_range(traits_t::g_To));
        }
    };

} // namespace aux

#ifndef BOOST_LOG_DOXYGEN_PASS

const aux::fmt_csv_decorator_gen csv_dec = {};

#else // BOOST_LOG_DOXYGEN_PASS

/*!
 * CSV-style decorator generator object. The decorator doubles double quotes that may be found
 * in the output. See http://en.wikipedia.org/wiki/Comma-separated_values for more information on
 * the CSV format. The generator provides <tt>operator[]</tt> that can be used to construct
 * the actual decorator. For example:
 *
 * <code>
 * csv_dec[ attr< std::string >("MyAttr") ]
 * </code>
 */
implementation_defined csv_dec;

#endif // BOOST_LOG_DOXYGEN_PASS

} // namespace formatters

} // namespace log

} // namespace boost

#endif // BOOST_LOG_FORMATTERS_CSV_DECORATOR_HPP_INCLUDED_
