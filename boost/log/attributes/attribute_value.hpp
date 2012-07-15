/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   attribute_value.hpp
 * \author Andrey Semashev
 * \date   21.05.2010
 *
 * The header contains methods of the \c attribute_value class. Use this header
 * to introduce the complete \c attribute_value implementation into your code.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_ATTRIBUTES_ATTRIBUTE_VALUE_HPP_INCLUDED_
#define BOOST_LOG_ATTRIBUTES_ATTRIBUTE_VALUE_HPP_INCLUDED_

#include <boost/log/detail/prologue.hpp>
#include <boost/log/attributes/attribute_value_def.hpp>
#include <boost/log/attributes/value_extraction.hpp>
#include <boost/log/utility/type_dispatch/static_type_dispatcher.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

template< typename T, typename VisitorT >
inline bool attribute_value::visit(VisitorT visitor) const
{
    static_type_dispatcher< T > disp(visitor);
    return this->dispatch(disp);
}

template< typename T >
inline typename result_of::extract< T >::type attribute_value::extract() const
{
    return boost::log::extract_value_or_none< T >()(*this);
}

template< typename T >
inline typename result_of::extract_or_default< T, T >::type attribute_value::extract_or_default(T const& def_value) const
{
    return boost::log::extract_value_or_default< T, T >(def_value)(*this);
}

template< typename T, typename DefaultT >
inline typename result_of::extract_or_default< T, DefaultT >::type attribute_value::extract_or_default(DefaultT const& def_value) const
{
    return boost::log::extract_value_or_default< T, DefaultT >(def_value)(*this);
}

} // namespace log

} // namespace boost

#endif // BOOST_LOG_ATTRIBUTES_ATTRIBUTE_VALUE_HPP_INCLUDED_
