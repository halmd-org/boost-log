/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   message.hpp
 * \author Andrey Semashev
 * \date   22.04.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_FORMATTERS_MESSAGE_HPP_INCLUDED_
#define BOOST_LOG_FORMATTERS_MESSAGE_HPP_INCLUDED_

#include <boost/log/detail/prologue.hpp>
#include <boost/log/formatters/basic_formatters.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace formatters {

//! Message formatter class
template< typename CharT >
class fmt_message :
    public basic_formatter< CharT, fmt_message< CharT > >
{
private:
    //! Base type
    typedef basic_formatter< CharT, fmt_message< CharT > > base_type;

public:
    //! String type
    typedef typename base_type::string_type string_type;
    //! Stream type
    typedef typename base_type::ostream_type ostream_type;
    //! Attribute values set type
    typedef typename base_type::values_view_type values_view_type;

public:
    //! Output operator
    void operator() (ostream_type& strm, values_view_type const&, string_type const& msg) const
    {
        strm << msg;
    }
};

#ifdef BOOST_LOG_USE_CHAR
//! Formatter generator
inline fmt_message< char > message()
{
    return fmt_message< char >();
}
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
//! Formatter generator
inline fmt_message< wchar_t > wmessage()
{
    return fmt_message< wchar_t >();
}
#endif

} // namespace formatters

} // namespace log

} // namespace boost

#endif // BOOST_LOG_FORMATTERS_MESSAGE_HPP_INCLUDED_
