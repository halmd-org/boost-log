/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   sink.hpp
 * \author Andrey Semashev
 * \date   22.04.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_SINK_HPP_INCLUDED_
#define BOOST_LOG_SINK_HPP_INCLUDED_

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/function/function1.hpp>
#include <boost/log/detail/prologue.hpp>

namespace boost {

namespace log {

template< typename >
class basic_attribute_values_view;

//! A base class for a logging sink
template< typename CharT >
struct BOOST_LOG_NO_VTABLE sink
{
    //! Character type
    typedef CharT char_type;
    //! String type to be used as a message text holder
    typedef std::basic_string< char_type > string_type;
    //! Attribute values view type
    typedef basic_attribute_values_view< char_type > attribute_values_view;
    //! Filter function type
    typedef function1< bool, attribute_values_view const& > filter_type;

    virtual ~sink() {}

    //! The method sets the sink-specific filter
    template< typename T >
    void set_filter(T const& filter)
    {
        this->set_filter_impl(filter_type(filter));
    }

    //! The method returns true if the attribute values pass the filter
    virtual bool will_write_message(attribute_values_view const& attributes) = 0;
    //! The method writes the message to the sink
    virtual void write_message(attribute_values_view const& attributes, string_type const& message) = 0;

protected:
    //! The set_filter implementation
    virtual void set_filter_impl(filter_type const& filter) = 0;
};

} // namespace log

} // namespace boost

#endif // BOOST_LOG_SINK_HPP_INCLUDED_
