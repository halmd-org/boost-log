/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   format.hpp
 * \author Andrey Semashev
 * \date   16.03.2008
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_FORMATTERS_FORMAT_HPP_INCLUDED_
#define BOOST_LOG_FORMATTERS_FORMAT_HPP_INCLUDED_

#include <vector>
#include <ostream>
#include <boost/format.hpp>
#include <boost/function/function3.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/formatters/basic_formatters.hpp>
#include <boost/log/formatters/chain.hpp>
#include <boost/log/formatters/wrappers.hpp>
#include <boost/log/detail/cleanup_scope_guard.hpp>
#include <boost/log/detail/attachable_sstream_buf.hpp>

namespace boost {

namespace log {

namespace formatters {

//! Formatter to output objects into a Boost.Format object
template< typename CharT >
class fmt_format :
    public basic_formatter< CharT, fmt_format< CharT > >
{
private:
    //! Base type
    typedef basic_formatter< CharT, fmt_format< CharT > > base_type;

public:
    //! Char type
    typedef typename base_type::char_type char_type;
    //! String type
    typedef typename base_type::string_type string_type;
    //! Stream type
    typedef typename base_type::ostream_type ostream_type;
    //! Boost.Format type
    typedef basic_format< char_type > format_type;
    //! Attribute values set type
    typedef typename base_type::values_view_type values_view_type;

private:
    //! Formatter function object type
    typedef function3< void, ostream_type&, values_view_type const&, string_type const& > formatter_type;
    //! Sequence of formatters
    typedef std::vector< formatter_type > formatters;

private:
    //! Boost.Format object
    mutable format_type m_Format;
    //! Other formatters
    formatters m_Formatters;

    //! Formatting buffer
    mutable string_type m_Buffer;
    //! Stream buffer
    mutable log::aux::basic_ostringstreambuf< char_type > m_StreamBuf;
    //! Formatting stream
    mutable ostream_type m_Stream;

public:
    //! Constructor
    explicit fmt_format(format_type const& fmt) : m_Format(fmt), m_StreamBuf(m_Buffer), m_Stream(&m_StreamBuf)
    {
        m_Stream.exceptions(std::ios_base::badbit | std::ios_base::failbit);
    }
    //! Copy constructor
    fmt_format(fmt_format const& that) :
        m_Format(that.m_Format),
        m_Formatters(that.m_Formatters),
        m_StreamBuf(m_Buffer),
        m_Stream(&m_StreamBuf)
    {
        m_Stream.exceptions(std::ios_base::badbit | std::ios_base::failbit);
    }

    //! Output operator
    void operator() (ostream_type& strm, values_view_type const& attrs, string_type const& msg) const
    {
        log::aux::cleanup_guard< format_type > cleanup1(m_Format);
        log::aux::cleanup_guard< ostream_type > cleanup2(m_Stream);

        for (typename formatters::const_iterator it = m_Formatters.begin(), end = m_Formatters.end(); it != end; ++it)
        {
            log::aux::cleanup_guard< string_type > cleanup3(m_Buffer);
            (*it)(m_Stream, attrs, msg);
            m_Stream.flush();
            m_Format % m_Buffer;
        }

        strm << m_Format.str();
    }
    //! Composition operator
    template< typename FormatterT >
    fmt_format< char_type >& operator% (FormatterT const& fmt)
    {
        m_Formatters.push_back(formatter_type(fmt));
        return *this;
    }

private:
    //! Assignment prohibited
    fmt_format& operator= (fmt_format const& that);
};

//! Formatter generator
inline fmt_format< char > format(const char* fmt)
{
    typedef fmt_format< char >::format_type format_type;
    return fmt_format< char >(format_type(fmt));
}
//! Formatter generator
inline fmt_format< char > format(std::basic_string< char > const& fmt)
{
    typedef fmt_format< char >::format_type format_type;
    return fmt_format< char >(format_type(fmt));
}
//! Formatter generator
inline fmt_format< wchar_t > format(const wchar_t* fmt)
{
    typedef fmt_format< wchar_t >::format_type format_type;
    return fmt_format< wchar_t >(format_type(fmt));
}
//! Formatter generator
inline fmt_format< wchar_t > format(std::basic_string< wchar_t > const& fmt)
{
    typedef fmt_format< wchar_t >::format_type format_type;
    return fmt_format< wchar_t >(format_type(fmt));
}

} // namespace formatters

} // namespace log

} // namespace boost

#endif // BOOST_LOG_FORMATTERS_BASIC_FORMATTERS_HPP_INCLUDED_
