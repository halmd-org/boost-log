/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   add_attr.hpp
 * \author Andrey Semashev
 * \date   26.11.2012
 *
 * This header contains the \c add_attr manipulator.
 */

#ifndef BOOST_LOG_UTILITY_MANIPULATORS_ADD_ATTR_HPP_INCLUDED_
#define BOOST_LOG_UTILITY_MANIPULATORS_ADD_ATTR_HPP_INCLUDED_

#include <boost/log/detail/prologue.hpp>
#include <boost/log/attributes/attribute_name.hpp>
#include <boost/log/attributes/basic_attribute_value.hpp>
#include <boost/log/sources/record_ostream.hpp>

#ifdef BOOST_LOG_HAS_PRAGMA_ONCE
#pragma once
#endif

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

//! Attribute value manipulator
template< typename T >
class add_attr_manip
{
public:
    //! Attribute value type
    typedef T value_type;

private:
#if !defined(BOOST_NO_RVALUE_REFERENCES) && !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)
    typedef value_type&& reference_type;
#else
    typedef value_type const& reference_type;
#endif
    //! Attribute value
    reference_type m_value;
    //! Attribute name
    attribute_name m_name;

public:
    //! Initializing constructor
    add_attr_manip(attribute_name const& name, reference_type value) : m_value(value), m_name(name)
    {
    }

    //! Returns attribute name
    attribute_name get_name() const { return m_name; }
    //! Returns attribute value
    reference_type get_value() const { return m_value; }
};

//! The operator attaches an attribute value to the log record
template< typename CharT, typename T >
inline basic_record_ostream< CharT >& operator<< (basic_record_ostream< CharT >& strm, add_attr_manip< T > const& manip)
{
    strm.get_record().attribute_values().insert(manip.get_name(), attributes::make_attribute_value(manip.get_value()));
    return strm;
}

//! The function creates a manipulator that attaches an attribute value to a log record
#if !defined(BOOST_NO_RVALUE_REFERENCES) && !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)
template< typename T >
inline add_attr_manip< T > add_attr(attribute_name const& name, T&& value)
{
    return add_attr_manip< T >(name, value);
}
#else
template< typename T >
inline add_attr_manip< T > add_attr(attribute_name const& name, T const& value)
{
    return add_attr_manip< T >(name, value);
}
#endif

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_UTILITY_MANIPULATORS_ADD_ATTR_HPP_INCLUDED_
