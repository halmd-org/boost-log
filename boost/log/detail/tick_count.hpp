/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   tick_count.hpp
 * \author Andrey Semashev
 * \date   31.07.2011
 *
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_DETAIL_TICK_COUNT_HPP_INCLUDED_
#define BOOST_LOG_DETAIL_TICK_COUNT_HPP_INCLUDED_

#include <boost/cstdint.hpp>
#include <boost/log/detail/prologue.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace aux {

#if defined(BOOST_WINDOWS) && !defined(__CYGWIN__)
typedef uint64_t (__stdcall* get_tick_count_t)();
#else
typedef uint64_t (*get_tick_count_t)();
#endif

/*!
 * The function returns a timestamp, in milliseconds since an unspecified
 * time point. This timer is guaranteed to be monotonic, it should not
 * be affected by clock changes, either manual or seasonal. Also, it
 * should be as fast as possible.
 */
extern BOOST_LOG_EXPORT get_tick_count_t get_tick_count;

} // namespace aux

} // namespace log

} // namespace boost

#endif // BOOST_LOG_DETAIL_TICK_COUNT_HPP_INCLUDED_
