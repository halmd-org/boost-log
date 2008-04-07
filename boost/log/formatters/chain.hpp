/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   chain.hpp
 * \author Andrey Semashev
 * \date   22.04.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_FORMATTERS_CHAIN_HPP_INCLUDED_
#define BOOST_LOG_FORMATTERS_CHAIN_HPP_INCLUDED_

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/formatters/basic_formatters.hpp>

namespace boost {

namespace log {

namespace formatters {

//! A formatter compound that encapsulates two other formatters
template< typename CharT, typename LeftFmtT, typename RightFmtT >
class fmt_chain :
    public basic_formatter< CharT, fmt_chain< CharT, LeftFmtT, RightFmtT > >
{
private:
    //! Base type
    typedef basic_formatter<
        CharT,
        fmt_chain< CharT, LeftFmtT, RightFmtT >
    > base_type;

public:
    //! Char type
    typedef typename base_type::char_type char_type;
    //! String type
    typedef typename base_type::string_type string_type;
    //! Attribute values set type
    typedef typename base_type::attribute_values_view attribute_values_view;

private:
    //! Left formatter
    LeftFmtT m_Left;
    //! Right formatter
    RightFmtT m_Right;

public:
    //! Constructor
    template< typename RightT >
    fmt_chain(LeftFmtT const& left, RightT const& right) : m_Left(left), m_Right(right) {}

    //! Output operator
    template< typename T >
    void operator() (T& strm, attribute_values_view const& attrs, string_type const& msg) const
    {
        m_Left(strm, attrs, msg);
        m_Right(strm, attrs, msg);
    }
};

} // namespace formatters

} // namespace log

} // namespace boost

#endif // BOOST_LOG_FORMATTERS_CHAIN_HPP_INCLUDED_
