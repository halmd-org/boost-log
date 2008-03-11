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

#include <ctime>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/detail/cleanup_scope_guard.hpp>
#include <boost/log/detail/attachable_sstream_buf.hpp>
#include <boost/log/attributes/extractors.hpp>
#include <boost/log/formatters/basic_formatters.hpp>
#include <boost/log/type_dispatch/standard_types.hpp>

#ifdef _MSC_VER
#pragma warning(push)
// 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
// non dll-interface class 'A' used as base for dll-interface class 'B'
#pragma warning(disable: 4275)
#endif // _MSC_VER

namespace boost {

namespace log {

namespace formatters {

namespace aux {

    //! Base class for formatters
    template< typename CharT >
    class date_time_formatter_base
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
        //! Formatting buffer string
        string_type m_Buffer;
        //! Stream buffer
        ostreambuf_type m_StreamBuf;
        //! Formatting stream
        ostream_type m_Stream;

    public:
        //! Default constructor
        date_time_formatter_base();
        //! Constructor with format setup
        explicit date_time_formatter_base(string_type const& fmt);

        //! Cleanup method
        void clear() { m_Buffer.clear(); }
        //! Returns the formatted string
        string_type const& get() const { return m_Buffer; }

    private:
        //  Copying and assignment prohibited
        date_time_formatter_base(date_time_formatter_base const&);
        date_time_formatter_base& operator= (date_time_formatter_base const&);
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
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        //! Default constructor
        BOOST_LOG_EXPORT basic_date_time_formatter();
        //! Constructor with format setup
        BOOST_LOG_EXPORT explicit basic_date_time_formatter(string_type const& fmt);

        //! Formatting method for time_t
        BOOST_LOG_EXPORT void operator()(std::time_t value);
        //! Formatting method for tm
        BOOST_LOG_EXPORT void operator()(std::tm const& value);
        //! Formatting method for DATE type on Windows
        BOOST_LOG_EXPORT void operator()(double value);
        //! Formatting method for Boost.DateTime POSIX time object
        BOOST_LOG_EXPORT void operator()(posix_time::ptime const& value);
    };

    //! Time period formatting object
    template< typename CharT >
    class basic_time_period_formatter :
        public date_time_formatter_base< CharT >
    {
        //! Base type
        typedef date_time_formatter_base< CharT > base_type;

    public:
        //! Function object return type
        typedef void result_type;
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        //! Default constructor
        BOOST_LOG_EXPORT basic_time_period_formatter();
        //! Constructor with format setup
        BOOST_LOG_EXPORT explicit basic_time_period_formatter(string_type const& fmt);

        //! Formatting method for Boost.DateTime POSIX time period object
        BOOST_LOG_EXPORT void operator()(posix_time::time_period const& value);
    };

} // namespace aux

//! Date and time attribute formatter
template<
    typename CharT,
    typename AttributeValueTypesT,
    template< typename > class FormatterT
>
class fmt_date_time :
    public basic_formatter< CharT, fmt_date_time< CharT, AttributeValueTypesT, FormatterT > >
{
    //! Base type
    typedef basic_formatter< CharT, fmt_date_time< CharT, AttributeValueTypesT, FormatterT > > base_type;

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
    typedef FormatterT< char_type > formatter_type;

private:
    //! Attribute value extractor
    attributes::attribute_value_extractor< char_type, AttributeValueTypesT > m_Extractor;
    //! Pointer to the formatter implementation
    shared_ptr< formatter_type > m_pFormatter;

public:
    //! Constructor
    explicit fmt_date_time(string_type const& name) : m_Extractor(name), m_pFormatter(new formatter_type()) {}
    //! Constructor with date and time format specification
    fmt_date_time(string_type const& name, string_type const& fmt) : m_Extractor(name), m_pFormatter(new formatter_type(fmt)) {}

    //! Output stream operator
    void operator() (ostream_type& strm, attribute_values_view const& attrs, string_type const&) const
    {
        log::aux::cleanup_guard< formatter_type > _(*m_pFormatter);
        m_Extractor(attrs, *m_pFormatter);
        strm.write(m_pFormatter->get().data(), m_pFormatter->get().size());
    }
    //! Output stream operator
    void operator() (format_type& fmt, attribute_values_view const& attrs, string_type const&) const
    {
        log::aux::cleanup_guard< formatter_type > _(*m_pFormatter);
        m_Extractor(attrs, *m_pFormatter);
        fmt % m_pFormatter->get();
    }
};
/*
//! Formatter generator
inline fmt_attr<
    char,
    make_default_attribute_types< char >::type
> attr(std::basic_string< char > const& name)
{
    return fmt_attr< char, make_default_attribute_types< char >::type >(name);
}
//! Formatter generator
inline fmt_attr<
    wchar_t,
    make_default_attribute_types< wchar_t >::type
> attr(std::basic_string< wchar_t > const& name)
{
    return fmt_attr< wchar_t, make_default_attribute_types< wchar_t >::type >(name);
}

//! Formatter generator with ability to specify an exact attribute value type(s)
template< typename AttributeValueTypesT >
inline fmt_attr<
    char,
    AttributeValueTypesT
> attr(std::basic_string< char > const& name)
{
    return fmt_attr< char, AttributeValueTypesT >(name);
}
//! Formatter generator with ability to specify an exact attribute value type(s)
template< typename AttributeValueTypesT >
inline fmt_attr<
    wchar_t,
    AttributeValueTypesT
> attr(std::basic_string< wchar_t > const& name)
{
    return fmt_attr< wchar_t, AttributeValueTypesT >(name);
}


//! Abstract type attribute formatter with format specifier
template< typename CharT, typename AttributeValueTypesT >
class fmt_attr_formatted :
    public basic_formatter< CharT, fmt_attr_formatted< CharT, AttributeValueTypesT > >
{
    //! Base type
    typedef basic_formatter< CharT, fmt_attr_formatted< CharT, AttributeValueTypesT > > base_type;

public:
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
    //! Boost.Format binding operator
    struct format_op
    {
        typedef void result_type;
        explicit format_op(format_type& fmt) : m_Format(fmt) {}
        template< typename T >
        void operator() (T const& value) const
        {
            m_Format % value;
        }

    private:
        format_type& m_Format;
    };
    //! Cleanup scope guard
    struct cleanup_guard
    {
        explicit cleanup_guard(format_type& fmt) : m_Format(fmt) {}
        ~cleanup_guard() { m_Format.clear(); }

    private:
        format_type& m_Format;
    };

private:
    //! Attribute value extractor
    attributes::attribute_value_extractor< char_type, AttributeValueTypesT > m_Extractor;
    //! Formatter object
    mutable format_type m_Formatter;

public:
    //! Constructor
    explicit fmt_attr_formatted(string_type const& name, string_type const& fmt) : m_Extractor(name), m_Formatter(fmt) {}

    //! Output stream operator
    void operator() (ostream_type& strm, attribute_values_view const& attrs, string_type const&) const
    {
        cleanup_guard _(m_Formatter);
        format_op op(m_Formatter);
        m_Extractor(attrs, op);
        strm << m_Formatter;
    }
    //! Output stream operator
    void operator() (format_type& fmt, attribute_values_view const& attrs, string_type const&) const
    {
        cleanup_guard _(m_Formatter);
        format_op op(m_Formatter);
        m_Extractor(attrs, op);
        fmt % m_Formatter.str();
    }
};

//! Formatter generator
inline fmt_attr_formatted<
    char,
    make_default_attribute_types< char >::type
> attr(std::basic_string< char > const& name, std::basic_string< char > const& fmt)
{
    return fmt_attr_formatted< char, make_default_attribute_types< char >::type >(name, fmt);
}
//! Formatter generator
inline fmt_attr_formatted<
    wchar_t,
    make_default_attribute_types< wchar_t >::type
> attr(std::basic_string< wchar_t > const& name, std::basic_string< wchar_t > const& fmt)
{
    return fmt_attr_formatted< wchar_t, make_default_attribute_types< wchar_t >::type >(name, fmt);
}

//! Formatter generator with ability to specify an exact attribute value type(s)
template< typename AttributeValueTypesT >
inline fmt_attr_formatted<
    char,
    AttributeValueTypesT
> attr(std::basic_string< char > const& name, std::basic_string< char > const& fmt)
{
    return fmt_attr_formatted< char, AttributeValueTypesT >(name, fmt);
}
//! Formatter generator with ability to specify an exact attribute value type(s)
template< typename AttributeValueTypesT >
inline fmt_attr_formatted<
    wchar_t,
    AttributeValueTypesT
> attr(std::basic_string< wchar_t > const& name, std::basic_string< wchar_t > const& fmt)
{
    return fmt_attr_formatted< wchar_t, AttributeValueTypesT >(name, fmt);
}
*/
} // namespace formatters

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_FORMATTERS_DATE_TIME_HPP_INCLUDED_
