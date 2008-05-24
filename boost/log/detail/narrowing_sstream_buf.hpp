/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   narrowing_sstream_buf.hpp
 * \author Andrey Semashev
 * \date   01.05.2008
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_DETAIL_NARROWING_SSTREAM_BUF_HPP_INCLUDED_
#define BOOST_LOG_DETAIL_NARROWING_SSTREAM_BUF_HPP_INCLUDED_

#include <boost/log/detail/prologue.hpp>
#ifdef BOOST_LOG_USE_WCHAR_T
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/code_converter.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#endif
#ifdef BOOST_LOG_USE_CHAR
#include <boost/log/detail/attachable_sstream_buf.hpp>
#endif

namespace boost {

namespace log {

namespace aux {

#ifdef BOOST_LOG_USE_WCHAR_T

//! The stream buffer that performs character code conversion before storing data to a string
template<
    typename TraitsT = std::char_traits< wchar_t >,
    typename AllocatorT = std::allocator< wchar_t >
>
struct narrowing_ostringstreambuf :
    public iostreams::stream_buffer<
        iostreams::code_converter<
            iostreams::back_insert_device< std::string >
        >
    >
{
    typedef iostreams::back_insert_device< std::string > device_t;
    typedef iostreams::code_converter< device_t > convert_t;
    typedef iostreams::stream_buffer< convert_t > base_type;

    explicit narrowing_ostringstreambuf(std::string& str) :
        base_type(convert_t(device_t(str)))
    {
    }
};

#endif

//! Stream buffer type generator
template<
    typename CharT,
    typename TraitsT = std::char_traits< CharT >,
    typename AllocatorT = std::allocator< CharT >
>
struct make_narrowing_ostringstreambuf;

#ifdef BOOST_LOG_USE_CHAR
template< typename TraitsT, typename AllocatorT >
struct make_narrowing_ostringstreambuf< char, TraitsT, AllocatorT >
{
    typedef basic_ostringstreambuf< char, TraitsT, AllocatorT > type;
};
#endif

#ifdef BOOST_LOG_USE_WCHAR_T
template< typename TraitsT, typename AllocatorT >
struct make_narrowing_ostringstreambuf< wchar_t, TraitsT, AllocatorT >
{
    typedef narrowing_ostringstreambuf< TraitsT, AllocatorT > type;
};
#endif

} // namespace aux

} // namespace log

} // namespace boost

#endif // BOOST_LOG_DETAIL_NARROWING_SSTREAM_BUF_HPP_INCLUDED_
