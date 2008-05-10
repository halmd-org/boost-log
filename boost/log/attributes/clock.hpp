/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   clock.hpp
 * \author Andrey Semashev
 * \date   01.12.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_ATTRIBUTES_CLOCK_HPP_INCLUDED_
#define BOOST_LOG_ATTRIBUTES_CLOCK_HPP_INCLUDED_

#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/attributes/basic_attribute_value.hpp>
#include <boost/log/attributes/time_traits.hpp>
#include <boost/log/utility/new_shared.hpp>

namespace boost {

namespace log {

namespace attributes {

//! A class of an attribute that makes an attribute value of the current date and time
template< typename TimeTraitsT >
class basic_clock :
    public attribute
{
public:
    //! A held functor type
    typedef typename TimeTraitsT::time_type time_type;

private:
    //! Attribute value type
    typedef basic_attribute_value< time_type > result_value;

public:
    //! The method returns the actual attribute value. It must not return NULL.
    shared_ptr< attribute_value > get_value()
    {
        return log::new_shared< result_value >(TimeTraitsT::get_clock());
    }
};

//! Attribute that returns current UTC time
typedef basic_clock< utc_time_traits > utc_clock;
//! Attribute that returns current local time
typedef basic_clock< local_time_traits > local_clock;

} // namespace attributes

} // namespace log

} // namespace boost

#endif // BOOST_LOG_ATTRIBUTES_CLOCK_HPP_INCLUDED_
