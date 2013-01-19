/*
 *          Copyright Andrey Semashev 2007 - 2013.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   add_value.hpp
 * \author Andrey Semashev
 * \date   26.11.2012
 *
 * This header contains the \c add_value manipulator.
 */

#ifndef BOOST_LOG_UTILITY_MANIPULATORS_ADD_VALUE_HPP_INCLUDED_
#define BOOST_LOG_UTILITY_MANIPULATORS_ADD_VALUE_HPP_INCLUDED_

#include <boost/type_traits/remove_cv.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/log/detail/config.hpp>
#include <boost/log/detail/embedded_string_type.hpp>
#include <boost/log/attributes/attribute_name.hpp>
#include <boost/log/attributes/attribute_value_impl.hpp>
#include <boost/log/expressions/keyword_fwd.hpp>
#include <boost/log/sources/record_ostream.hpp>

#ifdef BOOST_LOG_HAS_PRAGMA_ONCE
#pragma once
#endif

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

//! Attribute value manipulator
template< typename RefT >
class add_value_manip
{
public:
    //! Stored reference type
    typedef RefT reference_type;
    //! Attribute value type
    typedef typename remove_cv< typename remove_reference< reference_type >::type >::type value_type;

private:
    //! Attribute value
    reference_type m_value;
    //! Attribute name
    attribute_name m_name;

public:
    //! Initializing constructor
    add_value_manip(attribute_name const& name, reference_type value) : m_value(value), m_name(name)
    {
    }

    //! Returns attribute name
    attribute_name get_name() const { return m_name; }
    //! Returns attribute value
    reference_type get_value() const { return m_value; }
};

//! The operator attaches an attribute value to the log record
template< typename CharT, typename RefT >
inline basic_record_ostream< CharT >& operator<< (basic_record_ostream< CharT >& strm, add_value_manip< RefT > const& manip)
{
    typedef typename aux::make_embedded_string_type< typename add_value_manip< RefT >::value_type >::type value_type;
    strm.get_record().attribute_values().insert(manip.get_name(), attributes::make_attribute_value< value_type >(manip.get_value()));
    return strm;
}

//! The function creates a manipulator that attaches an attribute value to a log record
#if !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)

template< typename T >
inline add_value_manip< T&& > add_value(attribute_name const& name, T&& value)
{
    return add_value_manip< T&& >(name, value);
}

//! \overload
template< typename DescriptorT, template< typename > class ActorT >
inline add_value_manip< typename DescriptorT::value_type&& >
add_value(expressions::attribute_keyword< DescriptorT, ActorT > const&, typename DescriptorT::value_type&& value)
{
    return add_value_manip< typename DescriptorT::value_type&& >(DescriptorT::get_name(), value);
}

//! \overload
template< typename DescriptorT, template< typename > class ActorT >
inline add_value_manip< typename DescriptorT::value_type& >
add_value(expressions::attribute_keyword< DescriptorT, ActorT > const&, typename DescriptorT::value_type& value)
{
    return add_value_manip< typename DescriptorT::value_type& >(DescriptorT::get_name(), value);
}

#else // !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)

template< typename T >
inline add_value_manip< T const& > add_value(attribute_name const& name, T const& value)
{
    return add_value_manip< T const& >(name, value);
}

template< typename DescriptorT, template< typename > class ActorT >
inline add_value_manip< typename DescriptorT::value_type const& >
add_value(expressions::attribute_keyword< DescriptorT, ActorT > const&, typename DescriptorT::value_type const& value)
{
    return add_value_manip< typename DescriptorT::value_type const& >(DescriptorT::get_name(), value);
}

#endif // !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_UTILITY_MANIPULATORS_ADD_VALUE_HPP_INCLUDED_
