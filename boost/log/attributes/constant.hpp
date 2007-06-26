/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   constant.hpp
 * \author Andrey Semashev
 * \date   15.04.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_ATTRIBUTES_CONSTANT_HPP_INCLUDED_
#define BOOST_LOG_ATTRIBUTES_CONSTANT_HPP_INCLUDED_

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/attributes/attribute.hpp>

namespace boost {

namespace log {

namespace attributes {

//! A class of an attribute that holds a single constant value
template< typename T >
class constant :
    public attribute,
    public attribute_value,
    public enable_shared_from_this< constant< T > >
{
public:
    //! A held constant type
    typedef T held_type;

private:
    //! The constant
    const held_type m_Value;

public:
    //! Constructor
    explicit constant(held_type const& value) : m_Value(value) {}

    //! The method returns the actual attribute value. It must not return NULL.
    shared_ptr< attribute_value > get_value()
    {
        return this->shared_from_this();
    }

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
};

} // namespace attributes

} // namespace log

} // namespace boost

#endif // BOOST_LOG_ATTRIBUTES_CONSTANT_HPP_INCLUDED_
