/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   support/date_time.hpp
 * \author Andrey Semashev
 * \date   07.11.2012
 *
 * This header enables Boost.DateTime support for Boost.Log.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_SUPPORT_DATE_TIME_HPP_INCLUDED_
#define BOOST_LOG_SUPPORT_DATE_TIME_HPP_INCLUDED_

#include <ctime>
#include <string>
#include <locale>
#include <ostream>
#include <iterator>
#include <boost/date_time/time.hpp>
#include <boost/date_time/date.hpp>
#include <boost/date_time/period.hpp>
#include <boost/date_time/time_facet.hpp>
#include <boost/date_time/date_facet.hpp>
#include <boost/date_time/compiler_config.hpp>
#include <boost/date_time/gregorian/conversion.hpp>
#include <boost/date_time/local_time/conversion.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/detail/date_time_format_parser.hpp>
#include <boost/log/detail/light_function.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace expressions {

namespace aux {

template< typename T, typename CharT, typename VoidT = void >
struct date_time_formatter_generator_traits;

template< typename T, typename CharT, typename VoidT >
struct date_time_formatter_generator_traits;


} // namespace aux

} // namespace expressions

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_SUPPORT_DATE_TIME_HPP_INCLUDED_
