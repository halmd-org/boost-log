/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   value_extraction_fwd.hpp
 * \author Andrey Semashev
 * \date   01.03.2008
 *
 * The header contains forward declaration of tools for extracting an attribute value
 * from the view.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_ATTRIBUTES_VALUE_EXTRACTION_FWD_HPP_INCLUDED_
#define BOOST_LOG_ATTRIBUTES_VALUE_EXTRACTION_FWD_HPP_INCLUDED_

#include <boost/log/detail/prologue.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace result_of {

/*!
 * \brief A metafunction that allows to acquire the result of the value extraction
 */
template< typename T, typename DefaultT >
struct extract_or_default;

/*!
 * \brief A metafunction that allows to acquire the result of the value extraction
 */
template< typename T >
struct extract_or_throw;

/*!
 * \brief A metafunction that allows to acquire the result of the value extraction
 */
template< typename T >
struct extract;

} // namespace result_of

/*!
 * \brief Attribute value extraction policy that returns an empty value if no value can be extracted
 */
template< typename T >
class extract_value_or_none;

/*!
 * \brief Attribute value extraction policy that throws an exception if no value can be extracted
 */
template< typename T >
class extract_value_or_throw;

/*!
 * \brief Attribute value extraction policy that returns an default value if no value can be extracted
 */
template< typename T, typename DefaultT = T >
class extract_value_or_default;

/*!
 * \brief Generic attribute value extractor
 */
template< typename ExtractionPolicyT >
class value_extractor;

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_ATTRIBUTES_VALUE_EXTRACTION_FWD_HPP_INCLUDED_
