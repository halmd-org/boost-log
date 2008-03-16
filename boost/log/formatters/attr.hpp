/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   attr.hpp
 * \author Andrey Semashev
 * \date   22.04.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_FORMATTERS_ATTR_HPP_INCLUDED_
#define BOOST_LOG_FORMATTERS_ATTR_HPP_INCLUDED_

#include <string>
#include <boost/format.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/detail/cleanup_scope_guard.hpp>
#include <boost/log/attributes/extractors.hpp>
#include <boost/log/formatters/basic_formatters.hpp>
#include <boost/log/type_dispatch/standard_types.hpp>

namespace boost {

namespace log {

namespace formatters {

//! Abstract type attribute formatter
template< typename CharT, typename AttributeValueTypesT >
class fmt_attr :
    public basic_formatter< CharT, fmt_attr< CharT, AttributeValueTypesT > >
{
    //! Base type
    typedef basic_formatter< CharT, fmt_attr< CharT, AttributeValueTypesT > > base_type;

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
    //! Output stream operator
    struct ostream_op
    {
        typedef void result_type;
        explicit ostream_op(ostream_type& strm) : m_Stream(strm) {}
        template< typename T >
        void operator() (T const& value) const
        {
            m_Stream << value;
        }

    private:
        ostream_type& m_Stream;
    };
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

private:
    //! Attribute value extractor
    attributes::attribute_value_extractor< char_type, AttributeValueTypesT > m_Extractor;

public:
    //! Constructor
    explicit fmt_attr(string_type const& name) : m_Extractor(name) {}

    //! Output stream operator
    void operator() (ostream_type& strm, attribute_values_view const& attrs, string_type const&) const
    {
        ostream_op op(strm);
        m_Extractor(attrs, op);
    }
    //! Output stream operator
    void operator() (format_type& fmt, attribute_values_view const& attrs, string_type const&) const
    {
        format_op op(fmt);
        if (!m_Extractor(attrs, op))
        {
            // Not very nice but we have to put something
            // into the formatter if there is no attribute value found
            const char_type empty_string[1] = { 0 };
            fmt % empty_string;
        }
    }
};

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
        log::aux::cleanup_guard< format_type > _(m_Formatter);
        format_op op(m_Formatter);
        m_Extractor(attrs, op);
        strm << m_Formatter;
    }
    //! Output stream operator
    void operator() (format_type& fmt, attribute_values_view const& attrs, string_type const&) const
    {
        log::aux::cleanup_guard< format_type > _(m_Formatter);
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

} // namespace formatters

} // namespace log

} // namespace boost

#endif // BOOST_LOG_FORMATTERS_ATTR_HPP_INCLUDED_
