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

#include <boost/format.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/formatters/basic_formatters.hpp>
#include <boost/log/formatters/chain.hpp>
#include <boost/log/formatters/wrappers.hpp>
#include <boost/log/detail/cleanup_scope_guard.hpp>

namespace boost {

namespace log {

namespace formatters {

//! Formatter to output objects into a Boost.Format object
template< typename CharT, typename ArgsT >
class fmt_format :
    public basic_formatter< CharT, fmt_format< CharT, ArgsT > >
{
private:
    //! Base type
    typedef basic_formatter< CharT, fmt_format< CharT, ArgsT > > base_type;

public:
    //! Char type
    typedef typename base_type::char_type char_type;
    //! String type
    typedef typename base_type::string_type string_type;
    //! Stream type
    typedef typename base_type::ostream_type ostream_type;
    //! Boost.Format type
    typedef typename base_type::format_type format_type;
    //! Attribute values set type
    typedef typename base_type::attribute_values_view attribute_values_view;

private:
    //! Boost.Format object
    mutable format_type m_Format;
    //! Other formatters
    ArgsT m_Args;

public:
    //! Constructor
    fmt_format(format_type const& fmt, ArgsT const& args) : m_Format(fmt), m_Args(args) {}

    //! Output operator
    void operator() (ostream_type& strm, attribute_values_view const& attrs, string_type const& msg) const
    {
        log::aux::cleanup_guard< format_type > _(m_Format);
        m_Args(m_Format, attrs, msg);
        strm << m_Format.str();
    }
    //! Composition operator
    template< typename NextArgT >
    fmt_format<
        char_type,
        fmt_chain<
            ArgsT,
            typename wrap_if_not_formatter< char_type, NextArgT >::type
        >
    > operator% (NextArgT const& arg) const
    {
        typedef typename wrap_if_not_formatter< char_type, NextArgT >::type arg_t;
        typedef fmt_chain< ArgsT, arg_t > fmt_chain_t;
        return fmt_format< char_type, fmt_chain_t >(
            m_Format, fmt_chain_t(m_Args, arg_t(arg)));
    }
};

//! Formatter object generator
template< typename CharT >
class fmt_format_gen
{
    //! Format object
    boost::basic_format< CharT > m_Format;

public:
    explicit fmt_format_gen(const CharT* fmt) : m_Format(fmt) {}
    explicit fmt_format_gen(std::basic_string< CharT > const& fmt) : m_Format(fmt.c_str()) {}

    //! Composition operator
    template< typename NextArgT >
    fmt_format<
        CharT,
        typename wrap_if_not_formatter< CharT, NextArgT >::type
    > operator% (NextArgT const& arg) const
    {
        typedef typename wrap_if_not_formatter< CharT, NextArgT >::type arg_t;
        return fmt_format< CharT, arg_t >(m_Format, arg_t(arg));
    }
};

//! Formatter generator
inline fmt_format_gen< char > format(const char* fmt)
{
    return fmt_format_gen< char >(fmt);
}
//! Formatter generator
inline fmt_format_gen< char > format(std::basic_string< char > const& fmt)
{
    return fmt_format_gen< char >(fmt);
}
//! Formatter generator
inline fmt_format_gen< wchar_t > format(const wchar_t* fmt)
{
    return fmt_format_gen< wchar_t >(fmt);
}
//! Formatter generator
inline fmt_format_gen< wchar_t > format(std::basic_string< wchar_t > const& fmt)
{
    return fmt_format_gen< wchar_t >(fmt);
}

} // namespace formatters

} // namespace log

} // namespace boost

#endif // BOOST_LOG_FORMATTERS_BASIC_FORMATTERS_HPP_INCLUDED_
