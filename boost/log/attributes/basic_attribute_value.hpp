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

namespace boost {

namespace log {

namespace attributes {

//! Basic attribute value class
template< typename T >
class basic_attribute_value :
    public attribute_value
{
private:
    //! Underlying allocator type
    typedef std::allocator< basic_attribute_value > underlying_allocator;
    //! Value type
    typedef T held_type;

private:
    //! Attribute value
    const held_type m_Value;

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

    //  Memory allocation functions
    static void* operator new(std::size_t)
    {
        underlying_allocator alloc;
        return alloc.allocate(1);
    }
    static void operator delete(void* p)
    {
        underlying_allocator alloc;
        alloc.deallocate(reinterpret_cast< typename underlying_allocator::pointer >(p), 1);
    }
};

} // namespace attributes

} // namespace log

} // namespace boost

#endif // BOOST_LOG_ATTRIBUTES_BASIC_ATTRIBUTE_VALUE_HPP_INCLUDED_
