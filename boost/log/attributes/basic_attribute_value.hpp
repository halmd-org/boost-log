/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   basic_attribute_value.hpp
 * \author Andrey Semashev
 * \date   24.06.2007
 *
 * The header contains an implementation of an attribute value base class.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_ATTRIBUTES_BASIC_ATTRIBUTE_VALUE_HPP_INCLUDED_
#define BOOST_LOG_ATTRIBUTES_BASIC_ATTRIBUTE_VALUE_HPP_INCLUDED_

#include <boost/move/move.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/attributes/attribute_value.hpp>
#include <boost/log/utility/type_dispatch/type_dispatcher.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace attributes {

/*!
 * \brief Basic attribute value implementation class
 *
 * This class can be used as a boilerplate for simple attribute values. The class implements all needed
 * interfaces of attribute values and allows to store a single value of the type specified as a template parameter.
 * The stored value can be dispatched with type dispatching mechanism.
 */
template< typename T >
class basic_attribute_value :
    public attribute_value::impl
{
public:
    //! Value type
    typedef T value_type;

private:
    //! Attribute value
    value_type m_Value;

public:
    /*!
     * Constructor with initialization of the stored value
     */
    explicit basic_attribute_value(value_type const& v) : m_Value(v) {}
    /*!
     * Constructor with initialization of the stored value
     */
    explicit basic_attribute_value(BOOST_RV_REF(value_type) v) : m_Value(boost::move(v)) {}

    virtual bool dispatch(type_dispatcher& dispatcher)
    {
        type_dispatcher::callback< value_type > callback = dispatcher.get_callback< value_type >();
        if (callback)
        {
            callback(m_Value);
            return true;
        }
        else
            return false;
    }

    /*!
     * \return The attribute value type
     */
    type_info_wrapper get_type() const { return type_info_wrapper(typeid(value_type)); }

    /*!
     * \return Reference to the contained value.
     */
    value_type const& get() const { return m_Value; }
};

/*!
 * The function creates an attribute value from the specified object.
 */
template< typename T >
inline attribute_value make_attribute_value(BOOST_FWD_REF(T) v)
{
    return attribute_value(new basic_attribute_value< T >(boost::forward< T >(v)));
}

} // namespace attributes

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_ATTRIBUTES_BASIC_ATTRIBUTE_VALUE_HPP_INCLUDED_
