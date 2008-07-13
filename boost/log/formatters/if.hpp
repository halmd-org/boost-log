/*
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * This header is the Boost.Log library implementation, see the library documentation
 * at http://www.boost.org/libs/log/doc/log.html.
 */
/*!
 * \file   if.hpp
 * \author Andrey Semashev
 * \date   12.01.2008
 * 
 * The header contains implementation of a conditional formatter.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_FORMATTERS_IF_HPP_INCLUDED_
#define BOOST_LOG_FORMATTERS_IF_HPP_INCLUDED_

#include <boost/log/detail/prologue.hpp>
#include <boost/log/formatters/basic_formatters.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace formatters {

//! Conditional 'if-else' formatter
template< typename FilterT, typename ThenT, typename ElseT >
class fmt_if_else :
    public basic_formatter< typename ThenT::char_type, fmt_if_else< FilterT, ThenT, ElseT > >
{
private:
    //! Base type
    typedef basic_formatter< typename ThenT::char_type, fmt_if_else< FilterT, ThenT, ElseT > > base_type;

public:
    //! String type
    typedef typename base_type::string_type string_type;
    //! Stream type
    typedef typename base_type::ostream_type ostream_type;
    //! Attribute values set type
    typedef typename base_type::values_view_type values_view_type;

private:    
    FilterT m_Filter;
    ThenT m_Then;
    ElseT m_Else;

public:
    //! Constructor
    fmt_if_else(FilterT const& flt, ThenT const& th, ElseT const& el) : m_Filter(flt), m_Then(th), m_Else(el) {}

    //! Output operator
    void operator() (ostream_type& strm, values_view_type const& values, string_type const& message) const
    {
        if (m_Filter(values))
            m_Then(strm, values, message);
        else
            m_Else(strm, values, message);
    }
};

//! Conditional 'if' formatter
template< typename FilterT, typename FormatterT >
class fmt_if :
    public basic_formatter< typename FormatterT::char_type, fmt_if< FilterT, FormatterT > >
{
private:
    //! Base type
    typedef basic_formatter< typename FormatterT::char_type, fmt_if< FilterT, FormatterT > > base_type;

public:
    //! String type
    typedef typename base_type::string_type string_type;
    //! Stream type
    typedef typename base_type::ostream_type ostream_type;
    //! Attribute values set type
    typedef typename base_type::values_view_type values_view_type;

private:
    class else_gen
    {
        friend class fmt_if;

    private:
        FilterT m_Filter;
        FormatterT m_Formatter;

    public:
        //! Constructor
        else_gen(FilterT const& flt, FormatterT const& fmt) : m_Filter(flt), m_Formatter(fmt) {}
        //! If-else formatter generation operator
        template< typename ElseT >
        fmt_if_else< FilterT, FormatterT, ElseT > operator[] (ElseT const& el) const
        {
            return fmt_if_else< FilterT, FormatterT, ElseT >(m_Filter, m_Formatter, el);
        }
    };

public:
    //! Else generation object
    else_gen else_;

public:
    //! Constructor
    fmt_if(FilterT const& flt, FormatterT const& fmt) : else_(flt, fmt) {}

    //! Output operator
    void operator() (ostream_type& strm, values_view_type const& values, string_type const& message) const
    {
        if (else_.m_Filter(values))
            else_.m_Formatter(strm, values, message);
    }
};

namespace aux {

    //! Conditional formatter generator
    template< typename FilterT >
    class fmt_if_gen
    {
        FilterT m_Filter;

    public:
        explicit fmt_if_gen(FilterT const& filter) : m_Filter(filter) {}
        template< typename FormatterT >
        fmt_if< FilterT, FormatterT > operator[] (FormatterT const& fmt) const
        {
            return fmt_if< FilterT, FormatterT >(m_Filter, fmt);
        }
    };

} // namespace aux

//! Generator function
template< typename FilterT >
inline aux::fmt_if_gen< FilterT > if_(FilterT const& flt)
{
    return aux::fmt_if_gen< FilterT >(flt);
}

} // namespace boost

} // namespace log

} // namespace formatters

#endif // BOOST_LOG_FORMATTERS_IF_HPP_INCLUDED_
