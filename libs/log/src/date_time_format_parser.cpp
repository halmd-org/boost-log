/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   date_time_format_parser.cpp
 * \author Andrey Semashev
 * \date   30.09.2012
 *
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#include <boost/ref.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_lit.hpp>
#include <boost/spirit/include/qi_raw.hpp>
#include <boost/spirit/include/qi_lexeme.hpp>
#include <boost/spirit/include/qi_as.hpp>
#include <boost/spirit/include/qi_symbols.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <boost/log/utility/date_time_format_parser.hpp>
#include "spirit_encoding.hpp"

namespace qi = boost::spirit::qi;

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

BOOST_LOG_ANONYMOUS_NAMESPACE {

template< typename CharT >
struct string_constants;

#ifdef BOOST_LOG_USE_CHAR

template< >
struct string_constants< char >
{
    typedef char char_type;

    static const char_type* iso_date_format() { return "%Y%m%d"; }
    static const char_type* extended_iso_date_format() { return "%Y-%m-%d"; }

    static const char_type* iso_time_format() { return "%H%M%S"; }
    static const char_type* extended_iso_time_format() { return "%H:%M:%S"; }
    static const char_type* default_time_format() { return "%H:%M:%S.f"; }
};

#endif // BOOST_LOG_USE_CHAR

#ifdef BOOST_LOG_USE_WCHAR_T

template< >
struct string_constants< wchar_t >
{
    typedef wchar_t char_type;

    static const char_type* iso_date_format() { return L"%Y%m%d"; }
    static const char_type* extended_iso_date_format() { return L"%Y-%m-%d"; }

    static const char_type* iso_time_format() { return L"%H%M%S"; }
    static const char_type* extended_iso_time_format() { return L"%H:%M:%S"; }
    static const char_type* default_time_format() { return L"%H:%M:%S.f"; }
};

#endif // BOOST_LOG_USE_WCHAR_T

template< typename CharT >
inline void add_predefined_date_formats(date_format_parser_callback< CharT >& callback, qi::symbols< CharT, boost::function< void (iterator_range< const CharT* > const&) > >& syms)
{
    typedef date_format_parser_callback< CharT > callback_type;
    typedef string_constants< CharT > constants;

    syms.add(constants::iso_date_format(), boost::bind(&callback_type::on_iso_date, boost::ref(callback)));
    syms.add(constants::extended_iso_date_format(), boost::bind(&callback_type::on_extended_iso_date, boost::ref(callback)));
}

template< typename CharT >
inline void add_predefined_time_formats(time_format_parser_callback< CharT >& callback, qi::symbols< CharT, boost::function< void (iterator_range< const CharT* > const&) > >& syms)
{
    typedef time_format_parser_callback< CharT > callback_type;
    typedef string_constants< CharT > constants;

    syms.add(constants::default_time_format(), boost::bind(&callback_type::on_default_time, boost::ref(callback)));
    syms.add(constants::iso_time_format(), boost::bind(&callback_type::on_iso_time, boost::ref(callback)));
    syms.add(constants::extended_iso_time_format(), boost::bind(&callback_type::on_extended_iso_time, boost::ref(callback)));
}

} // namespace

//! Parses the date format string and invokes the callback object
template< typename CharT >
void parse_date_format(const CharT* begin, const CharT* end, date_format_parser_callback< CharT >& callback)
{

}

//! Parses the time format string and invokes the callback object
template< typename CharT >
void parse_time_format(const CharT* begin, const CharT* end, time_format_parser_callback< CharT >& callback)
{

}

//! Parses the date and time format string and invokes the callback object
template< typename CharT >
void parse_date_time_format(const CharT* begin, const CharT* end, date_time_format_parser_callback< CharT >& callback)
{

}

#ifdef BOOST_LOG_USE_CHAR

template BOOST_LOG_API
void parse_date_format(const char* begin, const char* end, date_format_parser_callback< char >& callback);
template BOOST_LOG_API
void parse_time_format(const char* begin, const char* end, time_format_parser_callback< char >& callback);
template BOOST_LOG_API
void parse_date_time_format(const char* begin, const char* end, date_time_format_parser_callback< char >& callback);

#endif // BOOST_LOG_USE_CHAR

#ifdef BOOST_LOG_USE_WCHAR_T

template BOOST_LOG_API
void parse_date_format(const wchar_t* begin, const wchar_t* end, date_format_parser_callback< wchar_t >& callback);
template BOOST_LOG_API
void parse_time_format(const wchar_t* begin, const wchar_t* end, time_format_parser_callback< wchar_t >& callback);
template BOOST_LOG_API
void parse_date_time_format(const wchar_t* begin, const wchar_t* end, date_time_format_parser_callback< wchar_t >& callback);

#endif // BOOST_LOG_USE_WCHAR_T

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost
