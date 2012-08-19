/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   stream.hpp
 * \author Andrey Semashev
 * \date   24.07.2012
 *
 * The header contains implementation of a stream placeholder in template expressions.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_EXPRESSIONS_STREAM_HPP_INCLUDED_
#define BOOST_LOG_EXPRESSIONS_STREAM_HPP_INCLUDED_

#include <boost/phoenix/core/argument.hpp>
#include <boost/log/detail/prologue.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace expressions {

/*!
 * Stream placeholder type in formatter template expressions.
 */
typedef phoenix::expression::argument< 2 >::type stream_type;

/*!
 * Stream placeholder in formatter template expressions.
 */
const stream_type stream = {};

} // namespace expressions

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#if defined(BOOST_LOG_EXPRESSIONS_ATTR_HPP_INCLUDED_)
#include <boost/log/detail/attr_output_impl.hpp>
#endif

#endif // BOOST_LOG_EXPRESSIONS_STREAM_HPP_INCLUDED_
