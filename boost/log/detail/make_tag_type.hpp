/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   make_tag_type.hpp
 * \author Andrey Semashev
 * \date   02.09.2012
 *
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_DETAIL_MAKE_TAG_TYPE_HPP_INCLUDED_
#define BOOST_LOG_DETAIL_MAKE_TAG_TYPE_HPP_INCLUDED_

#include <boost/type.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace aux {

//! The metafunction creates a tag type
template< typename T >
struct make_tag_type
{
    typedef boost::type< T > type;
};

template< typename T >
struct make_tag_type< boost::type< T > >
{
    typedef boost::type< T > type;
};

} // namespace aux

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_DETAIL_MAKE_TAG_TYPE_HPP_INCLUDED_
