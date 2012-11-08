/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   support/date_time.hpp
 * \author Andrey Semashev
 * \date   07.11.2012
 *
 * This header enables Boost.DateTime support for Boost.Log.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_SUPPORT_DATE_TIME_HPP_INCLUDED_
#define BOOST_LOG_SUPPORT_DATE_TIME_HPP_INCLUDED_

#include <ctime>
#include <string>
#include <locale>
#include <ostream>
#include <iterator>
#include <boost/cstdint.hpp>
#include <boost/move/move.hpp>
#include <boost/date_time/time.hpp>
#include <boost/date_time/date.hpp>
#include <boost/date_time/period.hpp>
#include <boost/date_time/time_facet.hpp>
#include <boost/date_time/date_facet.hpp>
#include <boost/date_time/compiler_config.hpp>
#include <boost/date_time/gregorian/conversion.hpp>
#include <boost/date_time/local_time/conversion.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/detail/date_time_format_parser.hpp>
#include <boost/log/detail/light_function.hpp>
#include <boost/log/detail/decomposed_time.hpp>
#include <boost/log/utility/formatting_stream.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace expressions {

namespace aux {

template< typename T, typename CharT, typename VoidT = void >
struct date_time_formatter_generator_traits;

template< typename T >
struct decomposed_time_wrapper :
    public boost::log::aux::decomposed_time
{
    typedef boost::log::aux::decomposed_time base_type;
    typedef T value_type;
    value_type m_time;

    BOOST_LOG_DEFAULTED_FUNCTION(decomposed_ptime(), {})
    explicit decomposed_time_wrapper(value_type const& t) : m_time(t)
    {
        typedef typename value_type::date_time date_time;
        date_time d = t.date();
        this->year = d.year();
        this->month = d.month();
        this->day = d.day();

        typedef typename value_type::time_duration_type time_duration_type;
        time_duration_type dur = t.time_of_day();
        this->hours = dur.hours();
        this->minutes = dur.minutes();
        this->seconds = dur.seconds();

        typedef typename time_duration_type::traits_type traits_type;
        enum
        {
            adjustment_ratio = (traits_type::ticks_per_second > base_type::subseconds_per_second ?
                traits_type::ticks_per_second / base_type::subseconds_per_second :
                base_type::subseconds_per_second / traits_type::ticks_per_second)
        };
        uint64_t frac = dur.fractional_seconds();
        this->subseconds = (traits_type::ticks_per_second > base_type::subseconds_per_second ? frac / adjustment_ratio : frac * adjustment_ratio);
    }
};

template< typename TimeT, typename CharT >
struct date_time_formatter_generator_traits_base
{
    //! Character type
    typedef CharT char_type;
    //! String type
    typedef std::basic_string< char_type > string_type;
    //! Formatting stream type
    typedef basic_formatting_stream< char_type > stream_type;
    //! Value type
    typedef TimeT value_type;

    //! Formatter function
    typedef boost::log::aux::light_function2< void, stream_type&, value_type const& > formatter_function_type;

    //! Formatter implementation
    class formatter :
        public boost::log::aux::date_time_formatter< decomposed_time_wrapper< value_type >, char_type >
    {
        BOOST_COPYABLE_AND_MOVABLE_ALT(formatter)

    private:
        typedef boost::log::aux::date_time_formatter< decomposed_time_wrapper< value_type >, char_type > base_type;

    public:
        typedef typename base_type::result_type result_type;

    public:
        BOOST_LOG_DEFAULTED_FUNCTION(formatter(), {})
        formatter(formatter const& that) : base_type(static_cast< base_type const& >(that)) {}
        formatter(BOOST_RV_REF(formatter) that) { this->swap(that); }

        formatter& operator= (formatter that)
        {
            this->swap(that);
            return *this;
        }

        result_type operator() (stream_type& strm, value_type const& value) const
        {
            if (value.is_not_a_date_time())
                strm << "not-a-date-time";
            else if (value.is_pos_infinity())
                strm << "+infinity";
            else if (value.is_neg_infinity())
                strm << "-infinity";
            else
                base_type::operator() (strm, decomposed_time_wrapper< value_type >(value));
        }
    };

    //! The function parses format string and constructs formatter function
    static formatter_function_type parse(string_type const& format)
    {
        formatter fmt;
        boost::log::aux::decomposed_time_formatter_builder< formatter, char_type > builder(fmt);
        boost::log::aux::parse_date_time_format(format, builder);
        return formatter_function_type(boost::move(fmt));
    }
};

template< typename CharT, typename VoidT >
struct date_time_formatter_generator_traits< posix_time::ptime, CharT, VoidT > :
    public date_time_formatter_generator_traits_base< posix_time::ptime, CharT >
{
};

template< typename TimeT, typename TimeSystemT, typename CharT, typename VoidT >
struct date_time_formatter_generator_traits< date_time::base_time< TimeT, TimeSystemT >, CharT, VoidT > :
    public date_time_formatter_generator_traits_base< date_time::base_time< TimeT, TimeSystemT >, CharT >
{
};

template< typename TimeT, typename TimeZoneT, typename CharT, typename VoidT >
struct date_time_formatter_generator_traits< local_time::local_date_time_base< TimeT, TimeZoneT >, CharT, VoidT >
{
    typedef date_time_formatter_generator_traits< TimeT, CharT, VoidT > underlying_traits;

    //! Character type
    typedef CharT char_type;
    //! String type
    typedef std::basic_string< char_type > string_type;
    //! Formatting stream type
    typedef basic_formatting_stream< char_type > stream_type;
    //! Value type
    typedef TimeT value_type;

    //! Formatter function
    typedef boost::log::aux::light_function2< void, stream_type&, value_type const& > formatter_function_type;

    //! Formatter implementation
    class formatter :
        public underlying_traits::formatter
    {
        BOOST_COPYABLE_AND_MOVABLE_ALT(formatter)

    private:
        typedef typename underlying_traits::formatter base_type;

    public:
        typedef typename base_type::result_type result_type;

    public:
        BOOST_LOG_DEFAULTED_FUNCTION(formatter(), {})
        formatter(formatter const& that) : base_type(static_cast< base_type const& >(that)) {}
        formatter(BOOST_RV_REF(formatter) that) { this->swap(that); }

        formatter& operator= (formatter that)
        {
            this->swap(that);
            return *this;
        }

        result_type operator() (stream_type& strm, value_type const& value) const
        {
            base_type::operator() (strm, value.local_time());
        }
    };

    //! The function parses format string and constructs formatter function
    static formatter_function_type parse(string_type const& format)
    {
        formatter fmt;
        boost::log::aux::decomposed_time_formatter_builder< formatter, char_type > builder(fmt);
        boost::log::aux::parse_date_time_format(format, builder);
        return formatter_function_type(boost::move(fmt));
    }
};

} // namespace aux

} // namespace expressions

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_SUPPORT_DATE_TIME_HPP_INCLUDED_
