/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   date_time.hpp
 * \author Andrey Semashev
 * \date   11.03.2008
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_FORMATTERS_DATE_TIME_HPP_INCLUDED_
#define BOOST_LOG_FORMATTERS_DATE_TIME_HPP_INCLUDED_

#include <time.h>
#include <ctime>
#include <string>
#include <memory>
#include <locale>
#include <ostream>
#include <iterator>
#include <limits>
#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>
#include <boost/utility/addressof.hpp>
#include <boost/date_time/time.hpp>
#include <boost/date_time/date.hpp>
#include <boost/date_time/time_facet.hpp>
#include <boost/date_time/date_facet.hpp>
#include <boost/date_time/compiler_config.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/detail/cleanup_scope_guard.hpp>
#include <boost/log/detail/attachable_sstream_buf.hpp>
#include <boost/log/attributes/extractors.hpp>
#include <boost/log/formatters/basic_formatters.hpp>
#include <boost/log/type_dispatch/date_time_types.hpp>

namespace boost {

namespace log {

namespace formatters {

namespace aux {

    //! A special base class that defines the default format strings
    template< typename >
    struct date_time_format_defaults;
    template< >
    struct date_time_format_defaults< char > : noncopyable
    {
        static const char* default_date_format() { return "%Y-%b-%d"; }
        static const char* default_time_format() { return "%H:%M:%S%f"; }
        static const char* default_date_time_format() { return "%Y-%b-%d %H:%M:%S%f"; }
        static const char* default_time_duration_format() { return "%-%H:%M:%S%f"; }
        static const char* default_time_period_format() { return "[%1% - %2%]"; }
    }
    template< >
    struct date_time_format_defaults< wchar_t > : noncopyable
    {
        static const wchar_t* default_date_format() { return L"%Y-%b-%d"; }
        static const wchar_t* default_time_format() { return L"%H:%M:%S%f"; }
        static const wchar_t* default_date_time_format() { return L"%Y-%b-%d %H:%M:%S%f"; }
        static const wchar_t* default_time_duration_format() { return L"%-%H:%M:%S%f"; }
        static const wchar_t* default_time_period_format() { return L"[%1% - %2%]"; }
    }

    //! Base class for formatters
    template< typename CharT >
    class date_time_formatter_base :
        protected date_time_format_defaults< CharT >
    {
    public:
        //! Character type
        typedef CharT char_type;
        //! String type
        typedef std::basic_string< char_type > string_type;
        //! Stream buffer type
        typedef log::aux::basic_ostringstreambuf< char_type > ostreambuf_type;
        //! Stream type
        typedef std::basic_ostream< char_type > ostream_type;

    protected:
        //! Format specifier
        const string_type m_Format;
        //! Formatting buffer string
        string_type m_Buffer;
        //! Stream buffer
        ostreambuf_type m_StreamBuf;
        //! Formatting stream
        ostream_type m_Stream;

    public:
        //! Constructor with format setup
        explicit date_time_formatter_base(const char_type* pFormat)
            : m_Format(pFormat), m_StreamBuf(m_Buffer), m_Stream(&m_StreamBuf)
        {
        }

        //! Cleanup method
        void clear() { m_Buffer.clear(); }
        //! Returns the formatted string
        string_type const& get() const { return m_Buffer; }

    protected:
        //! The method puts the date/time object into stream
        template< typename FacetT, typename T >
        void to_stream(T const& value)
        {
            std::locale loc = m_Stream.getloc();
            if (!std::has_facet< FacetT >(loc))
            {
                // Add the formatting facet
                std::auto_ptr< FacetT > facet(new FacetT(m_Format.c_str()));
                m_Stream.imbue(std::locale(loc, facet.get()));
                facet.release();
                loc = m_Stream.getloc();
            }

            // Perform formatting
            std::ostreambuf_iterator< char_type > osb_it(m_Stream);
            std::use_facet< FacetT >(loc).put(osb_it, m_Stream, m_Stream.fill(), value);
        }
    };

    //! Time point formatting object
    template< typename CharT >
    class basic_date_time_formatter :
        public date_time_formatter_base< CharT >
    {
        //! Base type
        typedef date_time_formatter_base< CharT > base_type;

    public:
        //! Function object return type
        typedef void result_type;
        //! Character type
        typedef typename base_type::char_type char_type;
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        //! Default constructor
        basic_date_time_formatter() : base_type(base_type::default_date_time_format()) {}
        //! Constructor with format setup
        explicit basic_date_time_formatter(string_type const& fmt) : base_type(fmt.c_str()) {}

        //! Formatting method for time_t
        void operator()(std::time_t value)
        {
            std::tm converted;
#ifdef BOOST_DATE_TIME_HAS_REENTRANT_STD_FUNCTIONS
            gmtime_r(&value, &converted);
#else
            posix_time::ptime psx = posix_time::from_time_t(value);
            converted = posix_time::to_tm(psx);
#endif
            (*this)(converted);
        }
        //! Formatting method for tm
        void operator()(std::tm const& value)
        {
            std::ostreambuf_iterator< char_type > osb_it(this->m_Stream);
            const char_type* const pFormat = this->m_Format.c_str();
            std::use_facet< std::time_put< char_type > >(this->m_Stream).put(
                osb_it,
                this->m_Stream,
                this->m_Stream.fill(),
                &value,
                pFormat,
                pFormat + this->m_Format.size());
            this->m_Stream.flush();
        }
        //! Formatting method for Boost.DateTime date object
        template< typename T >
        void operator()(T const& value)
        {
            this->to_stream_dispatch(value, boost::addressof(value));
            this->m_Stream.flush();
        }

    private:
        template< typename T, typename CalendarT, typename DurationT >
        void to_stream_dispatch(
            T const& value, boost::date_time::date< T, CalendarT, DurationT > const*)
        {
            typedef boost::date_time::date_facet< T, char_type > facet_t;
            this->BOOST_NESTED_TEMPLATE to_stream< facet_t >(value);
        }
        template< typename T, typename TimeSystemT >
        void to_stream_dispatch(
            T const& value, boost::date_time::base_time< T, TimeSystemT > const*)
        {
            typedef boost::date_time::time_facet< T, char_type > facet_t;
            this->BOOST_NESTED_TEMPLATE to_stream< facet_t >(value);
        }
        template< typename T >
        void to_stream_dispatch(T const& value, ...)
        {
            // Call a user-defined function to format the unknown type of date or time
            boost_log_format_date_time(
                this->m_Stream, value, static_cast< string_type const& >(this->m_Format));
        }
    };

    //! Date formatting object
    template< typename CharT >
    class basic_date_formatter :
        public basic_date_time_formatter< CharT >
    {
        //! Base type
        typedef basic_date_time_formatter< CharT > base_type;

    public:
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        //! Default constructor
        basic_date_formatter() : base_type(base_type::default_date_format()) {}
        //! Constructor with format setup
        explicit basic_date_formatter(string_type const& fmt) : base_type(fmt) {}
    };

    //! Time formatting object
    template< typename CharT >
    class basic_time_formatter :
        public basic_date_time_formatter< CharT >
    {
        //! Base type
        typedef basic_date_time_formatter< CharT > base_type;

    public:
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        //! Default constructor
        basic_time_formatter() : base_type(base_type::default_time_format()) {}
        //! Constructor with format setup
        explicit basic_time_formatter(string_type const& fmt) : base_type(fmt) {}
    };

    //! Time period formatting object
    template< typename CharT >
    class basic_time_duration_formatter :
        public date_time_formatter_base< CharT >
    {
        //! Base type
        typedef date_time_formatter_base< CharT > base_type;

    public:
        //! Function object return type
        typedef void result_type;
        //! Character type
        typedef typename base_type::char_type char_type;
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        //! Default constructor
        basic_time_duration_formatter() : base_type(base_type::default_time_duration_format()) {}
        //! Constructor with format setup
        explicit basic_time_duration_formatter(string_type const& fmt) : base_type(fmt.c_str()) {}

        //! Formatting method for result of difftime
        void operator()(double value)
        {
            if (value < (std::numeric_limits< long >::max)())
            {
                (*this)(posix_time::seconds(static_cast< long >(value)));
            }
            else
            {
                const long hrs = static_cast< long >(value / 3600.0);
                (*this)(posix_time::hours(hrs)
                    + posix_time::seconds(static_cast< long >(value % 3600.0)));
            }
        }
        //! Formatting method for Boost.DateTime POSIX time duration object
        void operator()(posix_time::time_duration const& value)
        {
            // We have to hardcode the POSIX time duration type since
            // the time facet requires the time type to be instantiated.
            // And this type we cannot know from the time duration type.
            // I guess, I'll have to write time duration formatting by hand
            // one day to bypass the Boost.DateTime limitation...
            typedef boost::date_time::time_facet< posix_time::ptime, char_type > facet_t;
            this->BOOST_NESTED_TEMPLATE to_stream< facet_t >(value);
        }
        //! Formatting method for other time duration objects
        template< typename T >
        void operator()(T const& value)
        {
            // Call a user-defined function to format the unknown type of time duration
            boost_log_format_time_duration(
                this->m_Stream, value, static_cast< string_type const& >(this->m_Format));
        }
    };

} // namespace aux

//! Date and time attribute formatter
template<
    typename CharT,
    typename AttributeValueTypesT,
    template< typename > class FormatterImplT
>
class fmt_date_time_facade :
    public basic_formatter<
        CharT,
        fmt_date_time_facade< CharT, AttributeValueTypesT, FormatterImplT >
    >
{
    //! Base type
    typedef basic_formatter<
        CharT,
        fmt_date_time_facade< CharT, AttributeValueTypesT, FormatterImplT >
    > base_type;

public:
    //! Character type
    typedef typename base_type::char_type char_type;
    //! String type
    typedef typename base_type::string_type string_type;
    //! Stream type
    typedef typename base_type::ostream_type ostream_type;
    //! Boost.Format object type
    typedef typename base_type::format_type format_type;
    //! Attribute values set type
    typedef typename base_type::attribute_values_view attribute_values_view;

private:
    //! Actual formatter type
    typedef FormatterImplT< char_type > formatter_type;

private:
    //! Attribute value extractor
    attributes::attribute_value_extractor< char_type, AttributeValueTypesT > m_Extractor;
    //! Pointer to the formatter implementation
    mutable std::auto_ptr< formatter_type > m_pFormatter;

public:
    //! Constructor
    explicit fmt_date_time_facade(string_type const& name)
        : m_Extractor(name), m_pFormatter(new formatter_type())
    {
    }
    //! Constructor with date and time format specification
    fmt_date_time_facade(string_type const& name, string_type const& fmt)
        : m_Extractor(name), m_pFormatter(new formatter_type(fmt))
    {
    }

    //! Copy constructor (acts as move) - the constructor is formally needed in order to compose lazy formatter expression
    fmt_date_time_facade(fmt_date_time_facade const& that)
        : m_Extractor(that.m_Extractor), m_pFormatter(that.m_pFormatter.release())
    {
    }
    //! Assignment (acts as move)
    fmt_date_time_facade& operator= (fmt_date_time_facade const& that)
    {
        m_Extractor = that.m_Extractor;
        m_pFormatter.reset(that.m_pFormatter.release());
        return *this;
    }

    //! Output stream operator
    void operator() (ostream_type& strm, attribute_values_view const& attrs, string_type const&) const
    {
        BOOST_ASSERT(m_pFormatter.get() != 0);
        log::aux::cleanup_guard< formatter_type > _(*m_pFormatter);
        m_Extractor(attrs, *m_pFormatter);
        strm.write(m_pFormatter->get().data(), m_pFormatter->get().size());
    }
    //! Output stream operator
    void operator() (format_type& fmt, attribute_values_view const& attrs, string_type const&) const
    {
        BOOST_ASSERT(m_pFormatter.get() != 0);
        log::aux::cleanup_guard< formatter_type > _(*m_pFormatter);
        m_Extractor(attrs, *m_pFormatter);
        fmt % m_pFormatter->get();
    }
};

//! Formatter generator
inline fmt_date_time_facade<
    char,
    date_time_types,
    aux::basic_date_time_formatter
> date_time(std::basic_string< char > const& name)
{
    return fmt_date_time_facade<
        char,
        date_time_types,
        aux::basic_date_time_formatter
    >(name);
}
//! Formatter generator
inline fmt_date_time_facade<
    wchar_t,
    date_time_types,
    aux::basic_date_time_formatter
> date_time(std::basic_string< wchar_t > const& name)
{
    return fmt_date_time_facade<
        wchar_t,
        date_time_types,
        aux::basic_date_time_formatter
    >(name);
}

//! Formatter generator with ability to specify an exact attribute value type(s)
template< typename AttributeValueTypesT >
inline fmt_date_time_facade<
    char,
    AttributeValueTypesT,
    aux::basic_date_time_formatter
> date_time(std::basic_string< char > const& name)
{
    return fmt_date_time_facade<
        char,
        AttributeValueTypesT,
        aux::basic_date_time_formatter
    >(name);
}
//! Formatter generator with ability to specify an exact attribute value type(s)
template< typename AttributeValueTypesT >
inline fmt_date_time_facade<
    wchar_t,
    AttributeValueTypesT,
    aux::basic_date_time_formatter
> date_time(std::basic_string< wchar_t > const& name)
{
    return fmt_date_time_facade<
        wchar_t,
        AttributeValueTypesT,
        aux::basic_date_time_formatter
    >(name);
}

//! Formatter generator
inline fmt_date_time_facade<
    char,
    date_time_types,
    aux::basic_date_time_formatter
> date_time(std::basic_string< char > const& name, std::basic_string< char > const& fmt)
{
    return fmt_date_time_facade<
        char,
        date_time_types,
        aux::basic_date_time_formatter
    >(name, fmt);
}
//! Formatter generator
inline fmt_date_time_facade<
    wchar_t,
    date_time_types,
    aux::basic_date_time_formatter
> date_time(std::basic_string< wchar_t > const& name, std::basic_string< wchar_t > const& fmt)
{
    return fmt_date_time_facade<
        wchar_t,
        date_time_types,
        aux::basic_date_time_formatter
    >(name, fmt);
}

//! Formatter generator with ability to specify an exact attribute value type(s)
template< typename AttributeValueTypesT >
inline fmt_date_time_facade<
    char,
    AttributeValueTypesT,
    aux::basic_date_time_formatter
> date_time(std::basic_string< char > const& name, std::basic_string< char > const& fmt)
{
    return fmt_date_time_facade<
        char,
        AttributeValueTypesT,
        aux::basic_date_time_formatter
    >(name, fmt);
}
//! Formatter generator with ability to specify an exact attribute value type(s)
template< typename AttributeValueTypesT >
inline fmt_date_time_facade<
    wchar_t,
    AttributeValueTypesT,
    aux::basic_date_time_formatter
> date_time(std::basic_string< wchar_t > const& name, std::basic_string< wchar_t > const& fmt)
{
    return fmt_date_time_facade<
        wchar_t,
        AttributeValueTypesT,
        aux::basic_date_time_formatter
    >(name, fmt);
}


//! Formatter generator
inline fmt_date_time_facade<
    char,
    time_duration_types,
    aux::basic_time_duration_formatter
> time_duration(std::basic_string< char > const& name)
{
    return fmt_date_time_facade<
        char,
        time_duration_types,
        aux::basic_time_duration_formatter
    >(name);
}
//! Formatter generator
inline fmt_date_time_facade<
    wchar_t,
    time_duration_types,
    aux::basic_time_duration_formatter
> time_duration(std::basic_string< wchar_t > const& name)
{
    return fmt_date_time_facade<
        wchar_t,
        time_duration_types,
        aux::basic_time_duration_formatter
    >(name);
}

//! Formatter generator with ability to specify an exact attribute value type(s)
template< typename AttributeValueTypesT >
inline fmt_date_time_facade<
    char,
    AttributeValueTypesT,
    aux::basic_time_duration_formatter
> time_duration(std::basic_string< char > const& name)
{
    return fmt_date_time_facade<
        char,
        AttributeValueTypesT,
        aux::basic_time_duration_formatter
    >(name);
}
//! Formatter generator with ability to specify an exact attribute value type(s)
template< typename AttributeValueTypesT >
inline fmt_date_time_facade<
    wchar_t,
    AttributeValueTypesT,
    aux::basic_time_duration_formatter
> time_duration(std::basic_string< wchar_t > const& name)
{
    return fmt_date_time_facade<
        wchar_t,
        AttributeValueTypesT,
        aux::basic_time_duration_formatter
    >(name);
}

//! Formatter generator
inline fmt_date_time_facade<
    char,
    time_duration_types,
    aux::basic_time_duration_formatter
> time_duration(std::basic_string< char > const& name, std::basic_string< char > const& fmt)
{
    return fmt_date_time_facade<
        char,
        time_duration_types,
        aux::basic_time_duration_formatter
    >(name, fmt);
}
//! Formatter generator
inline fmt_date_time_facade<
    wchar_t,
    time_duration_types,
    aux::basic_time_duration_formatter
> time_duration(std::basic_string< wchar_t > const& name, std::basic_string< wchar_t > const& fmt)
{
    return fmt_date_time_facade<
        wchar_t,
        time_duration_types,
        aux::basic_time_duration_formatter
    >(name, fmt);
}

//! Formatter generator with ability to specify an exact attribute value type(s)
template< typename AttributeValueTypesT >
inline fmt_date_time_facade<
    char,
    AttributeValueTypesT,
    aux::basic_time_duration_formatter
> time_duration(std::basic_string< char > const& name, std::basic_string< char > const& fmt)
{
    return fmt_date_time_facade<
        char,
        AttributeValueTypesT,
        aux::basic_time_duration_formatter
    >(name, fmt);
}
//! Formatter generator with ability to specify an exact attribute value type(s)
template< typename AttributeValueTypesT >
inline fmt_date_time_facade<
    wchar_t,
    AttributeValueTypesT,
    aux::basic_time_duration_formatter
> time_duration(std::basic_string< wchar_t > const& name, std::basic_string< wchar_t > const& fmt)
{
    return fmt_date_time_facade<
        wchar_t,
        AttributeValueTypesT,
        aux::basic_time_duration_formatter
    >(name, fmt);
}

} // namespace formatters

} // namespace log

} // namespace boost

#endif // BOOST_LOG_FORMATTERS_DATE_TIME_HPP_INCLUDED_
