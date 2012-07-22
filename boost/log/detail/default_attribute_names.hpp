/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   default_attribute_names.hpp
 * \author Andrey Semashev
 * \date   15.01.2012
 *
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_DETAIL_DEFAULT_ATTRIBUTE_NAMES_HPP_INCLUDED_
#define BOOST_LOG_DETAIL_DEFAULT_ATTRIBUTE_NAMES_HPP_INCLUDED_

#include <boost/log/detail/prologue.hpp>
#include <boost/log/attributes/attribute_name.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace aux {

namespace default_attribute_names {

BOOST_LOG_API attribute_name severity();
BOOST_LOG_API attribute_name channel();
BOOST_LOG_API attribute_name message();

} // namespace default_attribute_names

} // namespace aux

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_DETAIL_DEFAULT_ATTRIBUTE_NAMES_HPP_INCLUDED_
