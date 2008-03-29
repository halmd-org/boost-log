/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   from_stream.hpp
 * \author Andrey Semashev
 * \date   22.03.2008
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_INIT_FROM_STREAM_HPP_INCLUDED_
#define BOOST_LOG_INIT_FROM_STREAM_HPP_INCLUDED_

#include <iosfwd>
#include <boost/log/detail/prologue.hpp>

#ifdef _MSC_VER
#pragma warning(push)
// 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
// non dll-interface class 'A' used as base for dll-interface class 'B'
#pragma warning(disable: 4275)
#endif // _MSC_VER

namespace boost {

namespace log {

//! The function initializes the logging library from a stream containing logging settings
template< typename CharT >
BOOST_LOG_EXPORT void init_from_stream(std::basic_istream< CharT >& strm);

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_INIT_FROM_STREAM_HPP_INCLUDED_
