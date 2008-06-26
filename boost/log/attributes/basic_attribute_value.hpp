/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   basic_attribute_value.hpp
 * \author Andrey Semashev
 * \date   24.06.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_ATTRIBUTES_BASIC_ATTRIBUTE_VALUE_HPP_INCLUDED_
#define BOOST_LOG_ATTRIBUTES_BASIC_ATTRIBUTE_VALUE_HPP_INCLUDED_

#include <boost/log/detail/prologue.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/detail/templated_shared_from_this.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace attributes {

//! Basic attribute value class
template< typename T >
class basic_attribute_value :
    public attribute_value,
    public log::aux::templated_shared_from_this
{
public:
    //! Value type
    typedef T held_type;

private:
    //! Attribute value
    held_type m_Value;

public:
    //! Constructor
    explicit basic_attribute_value(held_type const& v) : m_Value(v) {}

    //! The method dispatches the value to the given object. It returns true if the
    //! object was capable to consume the real attribute value type and false otherwise.
    bool dispatch(type_dispatcher& dispatcher)
    {
        register type_visitor< held_type >* visitor =
            dispatcher.get_visitor< held_type >();
        if (visitor)
        {
            visitor->visit(m_Value);
            return true;
        }
        else
            return false;
    }

    //! The method is called when the attribute value is passed to another thread (e.g.
    //! in case of asynchronous logging). The value should ensure it properly owns all thread-specific data.
    shared_ptr< attribute_value > detach_from_thread()
    {
        return this->shared_from_this< basic_attribute_value< held_type > >();
    }

    //! Returns the contained value
    held_type const& get() const { return m_Value; }
};

} // namespace attributes

} // namespace log

} // namespace boost

#endif // BOOST_LOG_ATTRIBUTES_BASIC_ATTRIBUTE_VALUE_HPP_INCLUDED_
