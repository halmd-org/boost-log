/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   decomposed_time.hpp
 * \author Andrey Semashev
 * \date   07.11.2012
 *
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_DETAIL_DECOMPOSED_TIME_HPP_INCLUDED_
#define BOOST_LOG_DETAIL_DECOMPOSED_TIME_HPP_INCLUDED_

#include <ctime>
#include <string>
#include <vector>
#include <limits>
#include <locale>
#include <boost/cstdint.hpp>
#include <boost/move/move.hpp>
#include <boost/spirit/include/generate.hpp>
#include <boost/spirit/include/karma_uint.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/detail/date_time_format_parser.hpp>
#include <boost/log/detail/light_function.hpp>
#include <boost/log/utility/formatting_stream.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace aux {

//! Date and time suitable for formatting
struct decomposed_time
{
    // Subseconds are microseconds
    enum _
    {
        subseconds_per_second = 1000000,
        subseconds_digits10 = 6
    };

    uint32_t year, month, day, hours, minutes, seconds, subseconds;
    bool negative;

    decomposed_time() : year(0), month(1), day(1), hours(0), minutes(0), seconds(0), subseconds(0), negative(false)
    {
    }

    decomposed_time(uint32_t y, uint32_t mo, uint32_t d, uint32_t h, uint32_t mi, uint32_t s, uint32_t ss = 0, bool neg = false) :
        year(y), month(mo), day(d), hours(h), minutes(mi), seconds(s), subseconds(ss), negative(neg)
    {
    }

    unsigned int week_day() const
    {
        unsigned int a = (14u - month) / 12u;
        unsigned int y = year - a;
        unsigned int m = month + 12u * a - 2u;
        return (day + y + (y / 4u) - (y / 100u) + (y / 400u) + (31u * m) / 12u) % 7u;
    }

    unsigned int year_day() const
    {
        bool is_leap_year = (!(year % 4u)) && ((year % 100u) || (!(year % 400u)));
        static const unsigned int first_day_offset[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
        return first_day_offset[month - 1] + day + (month > 2 && is_leap_year);
    }
};

inline std::tm to_tm(decomposed_time const& t)
{
    std::tm res = {};
    res.tm_year = static_cast< int >(t.year) - 1900;
    res.tm_mon = t.month - 1;
    res.tm_mday = t.day;
    res.tm_hour = t.hours;
    res.tm_min = t.minutes;
    res.tm_sec = t.seconds;
    res.tm_wday = t.week_day();
    res.tm_yday = t.year_day();
    res.tm_isdst = -1;

    return res;
}

template< typename T, typename CharT >
class date_time_formatter
{
    BOOST_COPYABLE_AND_MOVABLE_ALT(date_time_formatter)

public:
    typedef void result_type;
    typedef T value_type;
    typedef CharT char_type;
    typedef basic_formatting_ostream< char_type > stream_type;

private:
    typedef light_function2< void, stream_type&, value_type const& > formatter_type;
    typedef std::vector< formatter_type > formatters;

private:
    formatters m_formatters;

public:
    BOOST_LOG_DEFAULTED_FUNCTION(date_time_formatter(), {})
    date_time_formatter(date_time_formatter const& that) : m_formatters(that.m_formatters) {}
    date_time_formatter(BOOST_RV_REF(date_time_formatter) that) { m_formatters.swap(that.m_formatters); }

    date_time_formatter& operator= (date_time_formatter that)
    {
        this->swap(that);
        return *this;
    }

    result_type operator() (stream_type& strm, value_type const& value) const
    {
        // Some formatters will put characters directly to the underlying string, so we have to flush stream buffers before formatting
        strm.flush();
        for (typename formatters::const_iterator it = m_formatters.begin(), end = m_formatters.end(); strm.good() && it != end; ++it)
        {
            (*it)(strm, value);
        }
    }

#if !defined(BOOST_NO_RVALUE_REFERENCES) && !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)
    template< typename FunT >
    void add_formatter(FunT&& fun)
    {
        m_formatters.emplace_back(boost::forward< FunT >(fun));
    }
#else
    template< typename FunT >
    void add_formatter(FunT const& fun)
    {
        m_formatters.push_back(fun);
    }
#endif

    void swap(date_time_formatter& that)
    {
        m_formatters.swap(that.m_formatters);
    }
};

template< typename FormatterT, typename CharT >
class decomposed_time_formatter_builder :
    public date_time_format_parser_callback< CharT >
{
public:
    typedef date_time_format_parser_callback< CharT > base_type;
    typedef typename base_type::char_type char_type;
    typedef FormatterT formatter_type;
    typedef typename formatter_type::value_type value_type;
    typedef typename formatter_type::stream_type stream_type;
    typedef typename stream_type::string_type string_type;
    typedef typename stream_type::streambuf_type streambuf_type;

private:
    struct literal_formatter
    {
        typedef void result_type;

        explicit literal_formatter(iterator_range< const char_type* > const& lit) : m_literal(lit.begin(), lit.end())
        {
        }

        result_type operator() (stream_type& strm, value_type const&) const
        {
            string_type& str = *static_cast< streambuf_type* >(strm.rdbuf())->storage();
            str.append(m_literal);
        }

    private:
        string_type m_literal;
    };

    struct integer_formatter_base
    {
        static void put(string_type& str, uint32_t value, unsigned int width, char_type fill_char)
        {
            char_type buf[std::numeric_limits< uint32_t >::digits10 + 2];
            char_type* p = buf;

            typedef spirit::karma::uint_generator< uint32_t, 10 > uint_gen;
            spirit::karma::generate(p, uint_gen(), value);
            const std::size_t len = p - buf;
            if (len < width)
                str.insert(str.end(), width - len, fill_char);
            str.append(buf, p);
        }
    };

    template< uint32_t decomposed_time::*MemberV, unsigned int WidthV, char_type FillCharV >
    struct integer_formatter :
        public integer_formatter_base
    {
        typedef void result_type;

        result_type operator() (stream_type& strm, value_type const& value) const
        {
            string_type& str = *static_cast< streambuf_type* >(strm.rdbuf())->storage();
            integer_formatter_base::put(str, static_cast< decomposed_time const& >(value).*MemberV, WidthV, FillCharV);
        }
    };

    struct short_year_formatter :
        public integer_formatter_base
    {
        typedef void result_type;

        result_type operator() (stream_type& strm, value_type const& value) const
        {
            string_type& str = *static_cast< streambuf_type* >(strm.rdbuf())->storage();
            integer_formatter_base::put(str, static_cast< decomposed_time const& >(value).year % 100u, 2, '0');
        }
    };

    template< char FormatV >
    struct locale_formatter
    {
        typedef void result_type;

        result_type operator() (stream_type& strm, value_type const& value) const
        {
            typedef std::time_put< char_type > facet_type;
            typedef typename facet_type::iter_type iter_type;
            std::tm t = to_tm(static_cast< decomposed_time const& >(value));
            std::use_facet< facet_type >(strm.getloc()).put(iter_type(strm), strm, ' ', &t, FormatV);
            strm.flush();
        }
    };

    struct numeric_week_day_formatter :
        public integer_formatter_base
    {
        typedef void result_type;

        result_type operator() (stream_type& strm, value_type const& value) const
        {
            string_type& str = *static_cast< streambuf_type* >(strm.rdbuf())->storage();
            integer_formatter_base::put(str, static_cast< decomposed_time const& >(value).week_day(), 1, '0');
        }
    };

    template< char_type FillCharV >
    struct hours_12_formatter :
        public integer_formatter_base
    {
        typedef void result_type;

        result_type operator() (stream_type& strm, value_type const& value) const
        {
            string_type& str = *static_cast< streambuf_type* >(strm.rdbuf())->storage();
            integer_formatter_base::put(str, (static_cast< decomposed_time const& >(value).hours % 12u) + 1u, 2, FillCharV);
        }
    };

    template< bool UpperCaseV >
    struct am_pm_formatter
    {
        typedef void result_type;

        result_type operator() (stream_type& strm, value_type const& value) const
        {
            static const char_type am[] = { static_cast< char_type >(UpperCaseV ? 'A' : 'a'), static_cast< char_type >(UpperCaseV ? 'M' : 'm'), static_cast< char_type >(0) };
            static const char_type pm[] = { static_cast< char_type >(UpperCaseV ? 'P' : 'p'), static_cast< char_type >(UpperCaseV ? 'M' : 'm'), static_cast< char_type >(0) };

            string_type& str = *static_cast< streambuf_type* >(strm.rdbuf())->storage();
            str.append(((static_cast< decomposed_time const& >(value).hours > 11) ? pm : am), 2u);
        }
    };

    template< bool DisplayPositiveV >
    struct sign_formatter
    {
        typedef void result_type;

        result_type operator() (stream_type& strm, value_type const& value) const
        {
            if (static_cast< decomposed_time const& >(value).negative)
                str.push_back('-');
            else if (DisplayPositiveV)
                str.push_back('+');
        }
    };

protected:
    formatter_type& m_formatter;

public:
    explicit decomposed_time_formatter_builder(formatter_type& fmt) : m_formatter(fmt)
    {
    }

    void on_literal(iterator_range< const char_type* > const& lit)
    {
        m_formatter.add_formatter(literal_formatter(lit));
    }

    void on_short_year()
    {
        m_formatter.add_formatter(short_year_formatter());
    }

    void on_full_year()
    {
        m_formatter.add_formatter(integer_formatter< &decomposed_time::year, 4, '0' >());
    }

    void on_numeric_month()
    {
        m_formatter.add_formatter(integer_formatter< &decomposed_time::month, 2, '0' >());
    }

    void on_short_month()
    {
        m_formatter.add_formatter(locale_formatter< 'b' >());
    }

    void on_full_month()
    {
        m_formatter.add_formatter(locale_formatter< 'B' >());
    }

    void on_month_day(bool leading_zero)
    {
        if (leading_zero)
            m_formatter.add_formatter(integer_formatter< &decomposed_time::day, 2, '0' >());
        else
            m_formatter.add_formatter(integer_formatter< &decomposed_time::day, 2, ' ' >());
    }

    void on_numeric_week_day()
    {
        m_formatter.add_formatter(numeric_week_day_formatter());
    }

    void on_short_week_day()
    {
        m_formatter.add_formatter(locale_formatter< 'a' >());
    }

    void on_full_week_day()
    {
        m_formatter.add_formatter(locale_formatter< 'A' >());
    }

    void on_hours(bool leading_zero)
    {
        if (leading_zero)
            m_formatter.add_formatter(integer_formatter< &decomposed_time::hours, 2, '0' >());
        else
            m_formatter.add_formatter(integer_formatter< &decomposed_time::hours, 2, ' ' >());
    }

    void on_hours_12(bool leading_zero)
    {
        if (leading_zero)
            m_formatter.add_formatter(hours_12_formatter< '0' >());
        else
            m_formatter.add_formatter(hours_12_formatter< ' ' >());
    }

    void on_minutes()
    {
        m_formatter.add_formatter(integer_formatter< &decomposed_time::minutes, 2, '0' >());
    }

    void on_seconds()
    {
        m_formatter.add_formatter(integer_formatter< &decomposed_time::seconds, 2, '0' >());
    }

    void on_fractional_seconds()
    {
        m_formatter.add_formatter(integer_formatter< &decomposed_time::subseconds, decomposed_time::subseconds_digits10, '0' >());
    }

    void on_am_pm(bool upper_case)
    {
        if (upper_case)
            m_formatter.add_formatter(am_pm_formatter< true >());
        else
            m_formatter.add_formatter(am_pm_formatter< false >());
    }

    void on_duration_sign(bool display_positive)
    {
        if (display_positive)
            m_formatter.add_formatter(sign_formatter< true >());
        else
            m_formatter.add_formatter(sign_formatter< false >());
    }

    void on_iso_time_zone()
    {
    }

    void on_extended_iso_time_zone()
    {
    }
};

} // namespace aux

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_DETAIL_DECOMPOSED_TIME_HPP_INCLUDED_
