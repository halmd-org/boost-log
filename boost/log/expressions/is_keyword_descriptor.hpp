/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   is_keyword_descriptor.hpp
 * \author Andrey Semashev
 * \date   14.07.2012
 *
 * The header contains attribute keyword descriptor detection trait.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_EXPRESSIONS_IS_KEYWORD_DESCRIPTOR_HPP_INCLUDED_
#define BOOST_LOG_EXPRESSIONS_IS_KEYWORD_DESCRIPTOR_HPP_INCLUDED_

#include <boost/mpl/bool.hpp>
#include <boost/log/detail/prologue.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace expressions {

/*!
 * Base class for keyword descriptors. All keyword descriptors must derive from this class to support the \c is_keyword_descriptor trait.
 */
struct keyword_descriptor
{
    typedef void _is_boost_log_keyword_descriptor;
};

/*!
 * The metafunction detects if the type \c T is a keyword descriptor
 */
template< typename T, typename VoidT = void >
struct is_keyword_descriptor :
    public mpl::false_
{
};

template< typename T >
struct is_keyword_descriptor< T, typename T::_is_boost_log_keyword_descriptor > :
    public mpl::true_
{
};

} // namespace expressions

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_EXPRESSIONS_IS_KEYWORD_DESCRIPTOR_HPP_INCLUDED_
