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
#include <boost/log/detail/prologue.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/attributes/basic_attribute_value.hpp>

namespace boost {

namespace log {

namespace attributes {

//! A class of an attribute that holds a single constant value
template< typename T >
class constant :
    public attribute,
    public basic_attribute_value< T >
{
    //! Base type
    typedef basic_attribute_value< T > base_type;

public:
    //! A held constant type
    typedef typename base_type::held_type held_type;

public:
    //! Constructor
    explicit constant(held_type const& value) : base_type(value) {}

    //! The method returns the actual attribute value. It must not return NULL.
    shared_ptr< attribute_value > get_value()
    {
        return this->BOOST_NESTED_TEMPLATE shared_from_this< base_type >();
    }

    //! The method is called when the attribute value is passed to another thread
    shared_ptr< attribute_value > detach_from_thread()
    {
        // We have to create a copy of the constant because the attribute object
        // can be created on the stack and get destroyed even if there are shared_ptrs that point to it.
        return shared_ptr< attribute_value >(new base_type(static_cast< base_type const& >(*this)));
    }
};

} // namespace attributes

} // namespace log

} // namespace boost

#endif // BOOST_LOG_ATTRIBUTES_CONSTANT_HPP_INCLUDED_
