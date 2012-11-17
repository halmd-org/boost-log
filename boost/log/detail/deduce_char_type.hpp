/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   deduce_char_type.hpp
 * \author Andrey Semashev
 * \date   17.11.2012
 *
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#ifndef BOOST_LOG_DETAIL_DEDUCE_CHAR_TYPE_HPP_INCLUDED_
#define BOOST_LOG_DETAIL_DEDUCE_CHAR_TYPE_HPP_INCLUDED_

#include <boost/log/detail/prologue.hpp>

#ifdef BOOST_LOG_HAS_PRAGMA_ONCE
#pragma once
#endif

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace aux {

template< typename T >
struct deduced_char_type;

template< >
struct deduced_char_type< char >
{
    typedef char type;
};

template< >
struct deduced_char_type< const char >
{
    typedef char type;
};

template< >
struct deduced_char_type< wchar_t >
{
    typedef wchar_t type;
};

template< >
struct deduced_char_type< const wchar_t >
{
    typedef wchar_t type;
};

//! Auxiliary traits to detect character type from a string
template< typename RangeT >
struct deduce_char_type :
    public deduced_char_type< typename RangeT::value_type >
{
};

template< typename T >
struct deduce_char_type< T* > :
    public deduced_char_type< T >
{
};

template< typename T, unsigned int CountV >
struct deduce_char_type< T(&)[CountV] > :
    public deduced_char_type< T >
{
};

template< typename T, unsigned int CountV >
struct deduce_char_type< T[CountV] > :
    public deduced_char_type< T >
{
};

} // namespace aux

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_DETAIL_DEDUCE_CHAR_TYPE_HPP_INCLUDED_
