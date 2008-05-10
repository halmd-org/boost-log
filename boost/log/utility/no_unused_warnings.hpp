/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   no_unused_warnings.hpp
 * \author Andrey Semashev
 * \date   10.05.2008
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_UTILITY_NO_UNUSED_WARNINGS_HPP_INCLUDED_
#define BOOST_LOG_UTILITY_NO_UNUSED_WARNINGS_HPP_INCLUDED_

#include <boost/log/detail/prologue.hpp>

namespace boost {

namespace log {

namespace aux {

template< typename T >
inline void no_unused_warnings(T const&) {}

} // namespace aux

} // namespace log

} // namespace boost

//! The macro suppresses compiler warnings for var being unused
#define BOOST_LOG_NO_UNUSED_WARNINGS(var) ::boost::log::aux::no_unused_warnings(var)

#endif // BOOST_LOG_UTILITY_NO_UNUSED_WARNINGS_HPP_INCLUDED_
