/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   counter.hpp
 * \author Andrey Semashev
 * \date   01.05.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_ATTRIBUTES_COUNTER_HPP_INCLUDED_
#define BOOST_LOG_ATTRIBUTES_COUNTER_HPP_INCLUDED_

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/detail/atomic_count.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/attributes/basic_attribute_value.hpp>

namespace boost {

namespace log {

namespace attributes {

//! A class of an attribute that counts an integral value
template<
    typename T,
    bool fDescending = false
>
class counter :
    public attribute
{
public:
    //! A held counter type
    typedef T held_type;

private:
    //! Counter value type
    typedef basic_attribute_value< held_type > counter_value;

private:
    //! Initial value
    const held_type m_InitialValue;
    //! The counter
    boost::detail::atomic_count m_Counter;

public:
    //! Constructor
    explicit counter(held_type const& initial = 0) : m_InitialValue(initial), m_Counter(0) {}

    //! The method returns the actual attribute value. It must not return NULL.
    shared_ptr< attribute_value > get_value()
    {
        register long NextValue = (--m_Counter) + 1;

        if (fDescending)
            return boost::make_shared< counter_value >(m_InitialValue + NextValue);
        else
            return boost::make_shared< counter_value >(m_InitialValue - NextValue);
    }
};

} // namespace attributes

} // namespace log

} // namespace boost

#endif // BOOST_LOG_ATTRIBUTES_COUNTER_HPP_INCLUDED_
