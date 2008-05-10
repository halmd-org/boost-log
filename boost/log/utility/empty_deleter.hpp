/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   empty_deleter.hpp
 * \author Andrey Semashev
 * \date   22.04.2007
 * 
 * \brief  This header contains an empty_deleter implementation. This is an empty
 *         function object that receives a pointer and does nothing with it.
 *         Such empty deletion strategy may be convenient, for example, when
 *         constructing shared_ptrs that point to some object that should not be
 *         deleted (i.e. a variable on the stack or some global singleton, like std::cout).
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_UTILITY_EMPTY_DELETER_HPP_INCLUDED_
#define BOOST_LOG_UTILITY_EMPTY_DELETER_HPP_INCLUDED_

namespace boost {

namespace log {

struct empty_deleter
{
    void operator() (const volatile void*) const volatile {}
};

} // namespace log

} // namespace boost

#endif // BOOST_LOG_UTILITY_EMPTY_DELETER_HPP_INCLUDED_
