/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   date_time_types.hpp
 * \author Andrey Semashev
 * \date   13.03.2008
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_DATE_TIME_TYPES_HPP_INCLUDED_
#define BOOST_LOG_DATE_TIME_TYPES_HPP_INCLUDED_

#include <ctime>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/joint_view.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/detail/prologue.hpp>

namespace boost {

namespace log {

//! An MPL-sequence of natively supported date and time types of attributes
typedef mpl::vector<
    std::time_t,
    std::tm
>::type native_date_time_types;

//! An MPL-sequence of Boost date and time types of attributes
typedef mpl::vector<
    posix_time::ptime
>::type boost_date_time_types;

//! An MPL-sequence with the complete list of the supported date and time types
typedef mpl::joint_view<
    native_date_time_types,
    boost_date_time_types
>::type date_time_types;

//! An MPL-sequence of natively supported time duration types of attributes
typedef mpl::vector<
    double // result of difftime
>::type native_time_duration_types;

//! An MPL-sequence of Boost time duration types of attributes
typedef mpl::vector<
    posix_time::time_duration
>::type boost_time_duration_types;

//! An MPL-sequence with the complete list of the supported time duration types
typedef mpl::joint_view<
    native_time_duration_types,
    boost_time_duration_types
>::type time_duration_types;

} // namespace log

} // namespace boost

#endif // BOOST_LOG_DATE_TIME_TYPES_HPP_INCLUDED_
