/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   unique_identifier_name.hpp
 * \author Andrey Semashev
 * \date   30.04.2008
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_UTILITY_UNIQUE_IDENTIFIER_NAME_HPP_INCLUDED_
#define BOOST_LOG_UTILITY_UNIQUE_IDENTIFIER_NAME_HPP_INCLUDED_

#include <boost/detail/workaround.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/log/detail/prologue.hpp>

#define BOOST_LOG_UNIQUE_IDENTIFIER_NAME_INTERNAL_(prefix, postfix)\
    BOOST_PP_CAT(prefix, postfix)
#define BOOST_LOG_UNIQUE_IDENTIFIER_NAME_INTERNAL(prefix, postfix)\
    BOOST_LOG_UNIQUE_IDENTIFIER_NAME_INTERNAL_(prefix, postfix)

// In VC 7.0 and later when compiling with /ZI option __LINE__ macro is corrupted
#if BOOST_WORKAROUND(BOOST_MSVC, >=1300)
#  define BOOST_LOG_UNIQUE_IDENTIFIER_NAME(prefix)\
    BOOST_LOG_UNIQUE_IDENTIFIER_NAME_INTERNAL(prefix, __COUNTER__)
#else
#  define BOOST_LOG_UNIQUE_IDENTIFIER_NAME(prefix)\
    BOOST_LOG_UNIQUE_IDENTIFIER_NAME_INTERNAL(prefix, __LINE__)
#endif // BOOST_WORKAROUND(BOOST_MSVC, >= 1300)

#endif // BOOST_LOG_UTILITY_UNIQUE_IDENTIFIER_NAME_HPP_INCLUDED_
