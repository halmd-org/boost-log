/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   make_expression_fwd.hpp
 * \author Andrey Semashev
 * \date   12.08.2012
 *
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_DETAIL_MAKE_EXPRESSION_FWD_HPP_INCLUDED_
#define BOOST_LOG_DETAIL_MAKE_EXPRESSION_FWD_HPP_INCLUDED_

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace expressions {

namespace aux {

template< typename LeftT, typename T, typename FallbackPolicyT, typename ParamsT, typename TagT >
struct make_output_expression;

} // namespace aux

} // namespace expressions

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_DETAIL_MAKE_EXPRESSION_FWD_HPP_INCLUDED_
