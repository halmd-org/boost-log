/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   timer.hpp
 * \author Andrey Semashev
 * \date   02.12.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_ATTRIBUTES_TIMER_HPP_INCLUDED_
#define BOOST_LOG_ATTRIBUTES_TIMER_HPP_INCLUDED_

#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#if defined(BOOST_WINDOWS) && !defined(BOOST_LOG_NO_QUERY_PERFORMANCE_COUNTER)
#include <boost/cstdint.hpp>
#include <boost/thread/mutex.hpp>
#endif
#include <boost/log/detail/prologue.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/attributes/basic_attribute_value.hpp>
#include <boost/log/attributes/time_traits.hpp>

namespace boost {

namespace log {

namespace attributes {

#if defined(BOOST_WINDOWS) && !defined(BOOST_LOG_NO_QUERY_PERFORMANCE_COUNTER)

#ifdef _MSC_VER
#pragma warning(push)
 // non dll-interface class 'A' used as base for dll-interface class 'B'
#pragma warning(disable: 4275)
#endif // _MSC_VER

/*!
 * \brief A class of an attribute that makes an attribute value of the time interval since construction (a more precise version for Windows)
 * 
 * This version uses QueryPerformanceFrequence/QueryPerformanceCounter API to calculate a more precise value
 * of the elapsed time. There are known problems with these functions when used with some CPUs, notably AMD Athlon with
 * Cool'n'Quiet technology enabled. See the following links for for more information and possible resolutions:
 * 
 * http://support.microsoft.com/?scid=kb;en-us;895980
 * http://support.microsoft.com/?id=896256
 * 
 * In case if none of these solutions apply, you are free to #define BOOST_LOG_NO_QUERY_PERFORMANCE_COUNTER macro to
 * fall back to the default implementation based on Boost.DateTime.
 */
class BOOST_LOG_EXPORT timer :
    public attribute
{
public:
    //! Time type
    typedef utc_time_traits::time_type time_type;

private:
    //! Attribute value type
    typedef basic_attribute_value< time_type::time_duration_type > result_value;

private:
    //! Synchronization mutex
    mutex m_Mutex;
    //! Frequency factor for calculating duration
    uint64_t m_FrequencyFactor;
    //! Last value of the performance counter
    uint64_t m_LastCounter;
    //! Elapsed time duration
    time_type::time_duration_type m_Duration;

public:
    //! Constructor
    timer();
    //! The method returns the actual attribute value. It must not return NULL.
    shared_ptr< attribute_value > get_value();
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#else // defined(BOOST_WINDOWS) && !defined(BOOST_LOG_NO_QUERY_PERFORMANCE_COUNTER)

//! A class of an attribute that makes an attribute value of the time interval since construction
class timer :
    public attribute
{
public:
    //! Time type
    typedef utc_time_traits::time_type time_type;

private:
    //! Attribute value type
    typedef basic_attribute_value< time_type::time_duration_type > result_value;

private:
    //! Base time point
    const time_type m_BaseTimePoint;

public:
    //! Constructor
    timer() : m_BaseTimePoint(utc_time_traits::get_clock()) {}

    //! The method returns the actual attribute value. It must not return NULL.
    shared_ptr< attribute_value > get_value()
    {
        return shared_ptr< attribute_value >(
            new result_value(utc_time_traits::get_clock() - m_BaseTimePoint));
    }
};

#endif // defined(BOOST_WINDOWS) && !defined(BOOST_LOG_NO_QUERY_PERFORMANCE_COUNTER)

} // namespace attributes

} // namespace log

} // namespace boost

#endif // BOOST_LOG_ATTRIBUTES_TIMER_HPP_INCLUDED_
