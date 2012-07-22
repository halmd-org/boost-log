/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   support/exception.hpp
 * \author Andrey Semashev
 * \date   18.07.2009
 *
 * This header enables Boost.Exception support for Boost.Log.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_SUPPORT_EXCEPTION_HPP_INCLUDED_
#define BOOST_LOG_SUPPORT_EXCEPTION_HPP_INCLUDED_

#include <boost/exception/info.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/attributes/attribute_name.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/utility/type_info_wrapper.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

/*!
 * Attribute name exception information
 */
typedef error_info< struct attribute_name_info_tag, attribute_name > attribute_name_info;

/*!
 * Type info exception information
 */
typedef error_info< struct type_info_info_tag, type_info_wrapper > type_info_info;

/*!
 * Current scope exception information
 */
typedef error_info< struct current_scope_info_tag, attributes::named_scope_list > current_scope_info;

/*!
 * The function returns an error information object that contains current stack of scopes.
 * This information can then be attached to an exception and extracted at the catch site.
 * The extracted scope list won't be affected by any scope changes that may happen during
 * the exception propagation.
 *
 * \note See the \c basic_named_scope attribute documentation on how to maintain scope list.
 */
inline current_scope_info current_scope()
{
    return current_scope_info(attributes::named_scope::get_scopes());
}

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_SUPPORT_EXCEPTION_HPP_INCLUDED_
