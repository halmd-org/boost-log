/*
 *          Copyright Andrey Semashev 2007 - 2010.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   check_execute_once.hpp
 * \author Andrey Semashev
 * \date   23.06.2010
 *
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 *
 * The code in this file is based on the \c call_once function implementation in Boost.Thread.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_DETAIL_EXECUTE_ONCE_HPP_INCLUDED_
#define BOOST_LOG_DETAIL_EXECUTE_ONCE_HPP_INCLUDED_

#include <boost/log/detail/prologue.hpp>

#ifndef BOOST_LOG_NO_THREADS

#include <boost/log/utility/explicit_operator_bool.hpp>

#if defined(BOOST_THREAD_PLATFORM_WIN32)

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace aux {

struct check_execute_once_flag
{
    void* event_handle;
    long status;
    long count;
};

#define BOOST_LOG_ONCE_INIT { 0, 0, 0 }

class check_execute_once
{
private:
    // Flag status values from Boost.Thread
    enum
    {
        running_value = 0x7f0725e3,
        function_complete_flag_value = 0xc15730e2
    };

private:
    check_execute_once_flag& m_Flag;
    bool m_fCounted;

public:
    explicit check_execute_once(check_execute_once_flag& f) : m_Flag(f), m_fCounted(false)
    {
        if (m_Flag.status != function_complete_flag_value)
            slow_constructor();
    }

    ~check_execute_once()
    {
        if (m_Flag.status != function_complete_flag_value)
            slow_destructor();
    }

    BOOST_LOG_EXPLICIT_OPERATOR_BOOL()
    bool operator! () const
    {
        return (m_Flag.status == function_complete_flag_value);
    }

private:
    //  Non-copyable, non-assignable
    check_execute_once(check_execute_once const&);
    check_execute_once& operator= (check_execute_once const&);

    BOOST_LOG_EXPORT void slow_constructor();
    BOOST_LOG_EXPORT void slow_destructor();
};

} // namespace aux

} // namespace log

} // namespace boost

#elif defined(BOOST_THREAD_PLATFORM_PTHREAD)

#include <boost/cstdint.hpp>
#include <boost/thread/once.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace aux {

struct check_execute_once_flag
{
    boost::uintmax_t epoch;
};

#define BOOST_LOG_ONCE_INIT { BOOST_ONCE_INITIAL_FLAG_VALUE }

class check_execute_once
{
private:
    check_execute_once_flag& m_Flag;
    boost::uintmax_t& m_ThisThreadEpoch;

public:
    explicit check_execute_once(check_execute_once_flag& f) :
        m_Flag(f),
        m_ThisThreadEpoch(detail::get_once_per_thread_epoch())
    {
        if (m_Flag.epoch < m_ThisThreadEpoch)
            slow_constructor();
    }

    ~check_execute_once()
    {
        if (m_Flag.epoch < m_ThisThreadEpoch)
            slow_destructor();
    }

    BOOST_LOG_EXPLICIT_OPERATOR_BOOL()
    bool operator! () const
    {
        return (m_Flag.epoch >= m_ThisThreadEpoch);
    }

private:
    //  Non-copyable, non-assignable
    check_execute_once(check_execute_once const&);
    check_execute_once& operator= (check_execute_once const&);

    BOOST_LOG_EXPORT void slow_constructor();
    BOOST_LOG_EXPORT void slow_destructor();
};

} // namespace aux

} // namespace log

} // namespace boost

#else
#error Boost.Log: unsupported threading API
#endif

#endif // BOOST_LOG_NO_THREADS

#endif // BOOST_LOG_DETAIL_EXECUTE_ONCE_HPP_INCLUDED_
