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
#include <boost/bind/apply.hpp>
#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_raw.hpp>
#include <boost/spirit/include/qi_eoi.hpp>
#include <boost/spirit/include/qi_symbols.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <boost/log/detail/date_time_format_parser.hpp>
#include <boost/log/utility/functional/as_action.hpp>
#include "spirit_encoding.hpp"

namespace qi = boost::spirit::qi;

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace aux {

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

    static const char_type* short_year_placeholder() { return "%y"; }
    static const char_type* full_year_placeholder() { return "%Y"; }
    static const char_type* numeric_month_placeholder() { return "%m"; }
    static const char_type* short_month_placeholder() { return "%b"; }
    static const char_type* full_month_placeholder() { return "%B"; }
    static const char_type* month_day_leading_zero_placeholder() { return "%d"; }
    static const char_type* month_day_leading_space_placeholder() { return "%e"; }
    static const char_type* numeric_week_day_placeholder() { return "%w"; }
    static const char_type* short_week_day_placeholder() { return "%a"; }
    static const char_type* full_week_day_placeholder() { return "%A"; }

    static const char_type* iso_time_format() { return "%H%M%S"; }
    static const char_type* extended_iso_time_format() { return "%H:%M:%S"; }
    static const char_type* default_time_format() { return "%H:%M:%S.f"; }

    static const char_type* extended_iso_time_placeholder() { return "%T"; }

    static const char_type* hours_placeholder() { return "%O"; }
    static const char_type* hours_24_leading_zero_placeholder() { return "%H"; }
    static const char_type* hours_24_leading_space_placeholder() { return "%k"; }
    static const char_type* hours_12_leading_zero_placeholder() { return "%I"; }
    static const char_type* hours_12_leading_space_placeholder() { return "%l"; }
    static const char_type* minutes_placeholder() { return "%M"; }
    static const char_type* seconds_placeholder() { return "%S"; }
    static const char_type* fractional_seconds_placeholder() { return "%f"; }
    static const char_type* am_pm_lowercase_placeholder() { return "%P"; }
    static const char_type* am_pm_uppercase_placeholder() { return "%p"; }
    static const char_type* duration_minus_placeholder() { return "%-"; }
    static const char_type* duration_sign_placeholder() { return "%+"; }
    static const char_type* iso_timezone_placeholder() { return "%q"; }
    static const char_type* extended_iso_timezone_placeholder() { return "%Q"; }

    static const char_type* percent_placeholder() { return "%%"; }
    static iterator_range< const char_type* > percent_string() { return boost::as_literal("%"); }
};

#endif // BOOST_LOG_USE_CHAR

#ifdef BOOST_LOG_USE_WCHAR_T

template< >
struct string_constants< wchar_t >
{
    typedef wchar_t char_type;

    static const char_type* iso_date_format() { return L"%Y%m%d"; }
    static const char_type* extended_iso_date_format() { return L"%Y-%m-%d"; }

    static const char_type* short_year_placeholder() { return L"%y"; }
    static const char_type* full_year_placeholder() { return L"%Y"; }
    static const char_type* numeric_month_placeholder() { return L"%m"; }
    static const char_type* short_month_placeholder() { return L"%b"; }
    static const char_type* full_month_placeholder() { return L"%B"; }
    static const char_type* month_day_leading_zero_placeholder() { return L"%d"; }
    static const char_type* month_day_leading_space_placeholder() { return L"%e"; }
    static const char_type* numeric_week_day_placeholder() { return L"%w"; }
    static const char_type* short_week_day_placeholder() { return L"%a"; }
    static const char_type* full_week_day_placeholder() { return L"%A"; }

    static const char_type* iso_time_format() { return L"%H%M%S"; }
    static const char_type* extended_iso_time_format() { return L"%H:%M:%S"; }
    static const char_type* default_time_format() { return L"%H:%M:%S.f"; }

    static const char_type* extended_iso_time_placeholder() { return L"%T"; }

    static const char_type* hours_placeholder() { return L"%O"; }
    static const char_type* hours_24_leading_zero_placeholder() { return L"%H"; }
    static const char_type* hours_24_leading_space_placeholder() { return L"%k"; }
    static const char_type* hours_12_leading_zero_placeholder() { return L"%I"; }
    static const char_type* hours_12_leading_space_placeholder() { return L"%l"; }
    static const char_type* minutes_placeholder() { return L"%M"; }
    static const char_type* seconds_placeholder() { return L"%S"; }
    static const char_type* fractional_seconds_placeholder() { return L"%f"; }
    static const char_type* am_pm_lowercase_placeholder() { return L"%P"; }
    static const char_type* am_pm_uppercase_placeholder() { return L"%p"; }
    static const char_type* duration_minus_placeholder() { return L"%-"; }
    static const char_type* duration_sign_placeholder() { return L"%+"; }
    static const char_type* iso_timezone_placeholder() { return L"%q"; }
    static const char_type* extended_iso_timezone_placeholder() { return L"%Q"; }

    static const char_type* percent_placeholder() { return L"%%"; }
    static iterator_range< const char_type* > percent_string() { return boost::as_literal(L"%"); }
};

#endif // BOOST_LOG_USE_WCHAR_T

template< typename CharT >
inline void add_predefined_date_formats(date_format_parser_callback< CharT >& callback, qi::symbols< CharT, boost::function< void () > >& syms)
{
    typedef date_format_parser_callback< CharT > callback_type;
    typedef string_constants< CharT > constants;

    syms.add(constants::iso_date_format(), boost::bind(&callback_type::on_iso_date, boost::ref(callback)));
    syms.add(constants::extended_iso_date_format(), boost::bind(&callback_type::on_extended_iso_date, boost::ref(callback)));
}

template< typename CharT >
inline void add_date_placeholders(date_format_parser_callback< CharT >& callback, qi::symbols< CharT, boost::function< void () > >& syms)
{
    typedef date_format_parser_callback< CharT > callback_type;
    typedef string_constants< CharT > constants;

    syms.add(constants::short_year_placeholder(), boost::bind(&callback_type::on_short_year, boost::ref(callback)));
    syms.add(constants::full_year_placeholder(), boost::bind(&callback_type::on_full_year, boost::ref(callback)));
    syms.add(constants::numeric_month_placeholder(), boost::bind(&callback_type::on_numeric_month, boost::ref(callback)));
    syms.add(constants::short_month_placeholder(), boost::bind(&callback_type::on_short_month, boost::ref(callback)));
    syms.add(constants::full_month_placeholder(), boost::bind(&callback_type::on_full_month, boost::ref(callback)));
    syms.add(constants::month_day_leading_zero_placeholder(), boost::bind(&callback_type::on_month_day, boost::ref(callback), true));
    syms.add(constants::month_day_leading_space_placeholder(), boost::bind(&callback_type::on_month_day, boost::ref(callback), false));
    syms.add(constants::numeric_week_day_placeholder(), boost::bind(&callback_type::on_numeric_week_day, boost::ref(callback)));
    syms.add(constants::short_week_day_placeholder(), boost::bind(&callback_type::on_short_week_day, boost::ref(callback)));
    syms.add(constants::full_week_day_placeholder(), boost::bind(&callback_type::on_full_week_day, boost::ref(callback)));
}

template< typename CharT >
inline void add_predefined_time_formats(time_format_parser_callback< CharT >& callback, qi::symbols< CharT, boost::function< void () > >& syms)
{
    typedef time_format_parser_callback< CharT > callback_type;
    typedef string_constants< CharT > constants;

    syms.add(constants::default_time_format(), boost::bind(&callback_type::on_default_time, boost::ref(callback)));
    syms.add(constants::iso_time_format(), boost::bind(&callback_type::on_iso_time, boost::ref(callback)));
    syms.add(constants::extended_iso_time_format(), boost::bind(&callback_type::on_extended_iso_time, boost::ref(callback)));
}

template< typename CharT >
inline void add_time_placeholders(time_format_parser_callback< CharT >& callback, qi::symbols< CharT, boost::function< void () > >& syms)
{
    typedef time_format_parser_callback< CharT > callback_type;
    typedef string_constants< CharT > constants;

    syms.add(constants::extended_iso_time_placeholder(), boost::bind(&callback_type::on_extended_iso_time, boost::ref(callback)));
    syms.add(constants::hours_placeholder(), boost::bind(&callback_type::on_hours, boost::ref(callback), true));
    syms.add(constants::hours_24_leading_zero_placeholder(), boost::bind(&callback_type::on_hours, boost::ref(callback), true));
    syms.add(constants::hours_24_leading_space_placeholder(), boost::bind(&callback_type::on_hours, boost::ref(callback), false));
    syms.add(constants::hours_12_leading_zero_placeholder(), boost::bind(&callback_type::on_hours_12, boost::ref(callback), true));
    syms.add(constants::hours_12_leading_space_placeholder(), boost::bind(&callback_type::on_hours_12, boost::ref(callback), false));
    syms.add(constants::minutes_placeholder(), boost::bind(&callback_type::on_minutes, boost::ref(callback)));
    syms.add(constants::seconds_placeholder(), boost::bind(&callback_type::on_seconds, boost::ref(callback)));
    syms.add(constants::fractional_seconds_placeholder(), boost::bind(&callback_type::on_fractional_seconds, boost::ref(callback)));
    syms.add(constants::am_pm_lowercase_placeholder(), boost::bind(&callback_type::on_am_pm, boost::ref(callback), false));
    syms.add(constants::am_pm_uppercase_placeholder(), boost::bind(&callback_type::on_am_pm, boost::ref(callback), true));
    syms.add(constants::duration_minus_placeholder(), boost::bind(&callback_type::on_duration_sign, boost::ref(callback), false));
    syms.add(constants::duration_sign_placeholder(), boost::bind(&callback_type::on_duration_sign, boost::ref(callback), true));
    syms.add(constants::iso_timezone_placeholder(), boost::bind(&callback_type::on_iso_time_zone, boost::ref(callback)));
    syms.add(constants::extended_iso_timezone_placeholder(), boost::bind(&callback_type::on_extended_iso_time_zone, boost::ref(callback)));
}

template< typename CallbackT, typename CharT >
inline void add_common_placeholders(CallbackT& callback, qi::symbols< CharT, boost::function< void () > >& syms)
{
    typedef CallbackT callback_type;
    typedef string_constants< CharT > constants;

    syms.add(constants::percent_placeholder(), boost::bind(&callback_type::on_literal, boost::ref(callback), constants::percent_string()));
}

template< typename CharT, typename CallbackT >
inline bool parse_format(const CharT*& begin, const CharT* end, qi::symbols< CharT, boost::function< void () > > const& syms, CallbackT& callback)
{
    typedef CharT char_type;
    typedef CallbackT callback_type;

    return qi::parse(begin, end,
        *(
            syms[boost::log::as_action(boost::apply< void >())] |
            qi::raw[ static_cast< char_type >('%') >> qi::char_ ][boost::bind(&callback_type::on_placeholder, boost::ref(callback), _1)] |
            qi::raw[ *(qi::char_ - static_cast< char_type >('%')) ][boost::bind(&callback_type::on_literal, boost::ref(callback), _1)] |
            qi::raw[ static_cast< char_type >('%') >> qi::eoi ][boost::bind(&callback_type::on_literal, boost::ref(callback), _1)]
        )
    );
}

} // namespace

//! Parses the date format string and invokes the callback object
template< typename CharT >
void parse_date_format(const CharT* begin, const CharT* end, date_format_parser_callback< CharT >& callback)
{
    typedef CharT char_type;
    typedef date_format_parser_callback< char_type > callback_type;

    qi::symbols< char_type, boost::function< void () > > syms;
    add_common_placeholders(callback, syms);
    add_date_placeholders(callback, syms);
    add_predefined_date_formats(callback, syms);

    parse_format(begin, end, syms, callback);
}

//! Parses the time format string and invokes the callback object
template< typename CharT >
void parse_time_format(const CharT* begin, const CharT* end, time_format_parser_callback< CharT >& callback)
{
    typedef CharT char_type;
    typedef time_format_parser_callback< char_type > callback_type;

    qi::symbols< char_type, boost::function< void () > > syms;
    add_common_placeholders(callback, syms);
    add_time_placeholders(callback, syms);
    add_predefined_time_formats(callback, syms);

    parse_format(begin, end, syms, callback);
}

//! Parses the date and time format string and invokes the callback object
template< typename CharT >
void parse_date_time_format(const CharT* begin, const CharT* end, date_time_format_parser_callback< CharT >& callback)
{
    typedef CharT char_type;
    typedef date_time_format_parser_callback< char_type > callback_type;

    qi::symbols< char_type, boost::function< void () > > syms;
    add_common_placeholders(callback, syms);
    add_time_placeholders(callback, syms);
    add_date_placeholders(callback, syms);
    add_predefined_time_formats(callback, syms);
    add_predefined_date_formats(callback, syms);

    parse_format(begin, end, syms, callback);
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

} // namespace aux

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost
