/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   basic_sink.hpp
 * \author Andrey Semashev
 * \date   22.04.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_BASIC_SINK_HPP_INCLUDED_
#define BOOST_LOG_BASIC_SINK_HPP_INCLUDED_

#include <string>
#include <boost/thread/read_write_mutex.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/sinks/sink.hpp>
#include <boost/log/attributes/attribute_values_view.hpp>

namespace boost {

namespace log {

//! A basic implementation of a logging sink
template< typename CharT >
class BOOST_LOG_NO_VTABLE basic_sink :
    public sink< CharT >
{
    //! Base type
    typedef sink< CharT > base_type;

public:
    //! Character type
    typedef typename base_type::char_type char_type;
    //! String type to be used as a message text holder
    typedef typename base_type::string_type string_type;
    //! Attribute values view type
    typedef typename base_type::attribute_values_view attribute_values_view;
    //! Filter function type
    typedef typename base_type::filter_type filter_type;

    //! Mutex type
    typedef read_write_mutex mutex_type;
    //! Scoped read lock type
    typedef mutex_type::scoped_read_lock scoped_read_lock;
    //! Scoped write lock type
    typedef mutex_type::scoped_write_lock scoped_write_lock;

private:
    //! Synchronization mutex
    mutable mutex_type m_Mutex;
    //! Filter function
    filter_type m_Filter;

protected:
    basic_sink() : m_Mutex(/*read_write_scheduling_policy::writer_priority*/) {}

public:
    //! The method returns true if the attribute values pass the filter
    bool will_write_message(attribute_values_view const& attributes)
    {
        scoped_read_lock lock(m_Mutex);
        return will_write_message_unlocked(attributes);
    }

protected:
    //! Mutex accessor
    mutex_type& mutex() const { return m_Mutex; }

    //! The set_filter implementation
    void set_filter_impl(filter_type const& filter)
    {
        scoped_write_lock lock(m_Mutex);
        m_Filter = filter;
    }

    //! The method returns true if the attribute values pass the filter
    bool will_write_message_unlocked(attribute_values_view const& attributes)
    {
        if (!m_Filter.empty()) try
        {
            return m_Filter(attributes);
        }
        catch (...)
        {
            return false;
        }

        return true;
    }
};

} // namespace log

} // namespace boost

#endif // BOOST_LOG_BASIC_SINK_HPP_INCLUDED_
