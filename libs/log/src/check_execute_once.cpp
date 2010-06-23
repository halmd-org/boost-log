/*
 *          Copyright Andrey Semashev 2007 - 2010.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   check_execute_once.cpp
 * \author Andrey Semashev
 * \date   23.06.2010
 *
 * \brief  This file is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 *
 * The code in this file is based on the \c call_once function implementation in Boost.Thread.
 */

#include <boost/log/detail/check_execute_once.hpp>

#ifndef BOOST_LOG_NO_THREADS

#include <exception>
#include <boost/assert.hpp>
#include <boost/thread/once.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace aux {

#if defined(BOOST_THREAD_PLATFORM_WIN32)

BOOST_LOG_EXPORT void check_execute_once::slow_constructor()
{
    long status;
    void* event_handle = 0;

    while ((status = boost::detail::interlocked_read_acquire(&m_Flag.status))
          != function_complete_flag_value)
    {
        status = BOOST_INTERLOCKED_COMPARE_EXCHANGE(&m_Flag.status, running_value, 0);
        if (!status)
        {
            if (!event_handle)
            {
                event_handle = boost::detail::interlocked_read_acquire(&m_Flag.event_handle);
            }
            if (event_handle)
            {
                boost::detail::win32::ResetEvent(event_handle);
            }

            // Invoke the initializer block
            break;
        }

        if (!m_fCounted)
        {
            BOOST_INTERLOCKED_INCREMENT(&m_Flag.count);
            m_fCounted = true;
            status = boost::detail::interlocked_read_acquire(&m_Flag.status);
            if (status == function_complete_flag_value)
            {
                // The initializer block has already been executed
                break;
            }
            event_handle = boost::detail::interlocked_read_acquire(&m_Flag.event_handle);
            if (!event_handle)
            {
                event_handle = boost::detail::allocate_event_handle(m_Flag.event_handle);
                continue;
            }
        }
        BOOST_VERIFY(!boost::detail::win32::WaitForSingleObject(
            event_handle, boost::detail::win32::infinite));
    }
}

BOOST_LOG_EXPORT void check_execute_once::slow_destructor()
{
    void* event_handle = boost::detail::interlocked_read_acquire(&m_Flag.event_handle);
    bool keep_event = false;

    if (!std::uncaught_exception())
    {
        // The initializer executed successfully
        BOOST_INTERLOCKED_EXCHANGE(&m_Flag.status, function_complete_flag_value);

        if (!event_handle && boost::detail::interlocked_read_acquire(&m_Flag.count) > 0)
        {
            event_handle = boost::detail::allocate_event_handle(m_Flag.event_handle);
        }
    }
    else
    {
        // The initializer failed, marking the flag as if it hasn't run at all
        BOOST_INTERLOCKED_EXCHANGE(&m_Flag.status, 0);
        keep_event = true;
    }

    // Launch other threads that may have been blocked
    if (event_handle)
    {
        boost::detail::win32::SetEvent(event_handle);
    }

    if (m_fCounted && BOOST_INTERLOCKED_DECREMENT(&m_Flag.count)) == 0 && !keep_event)
    {
        if (!event_handle)
        {
            event_handle = boost::detail::interlocked_read_acquire(&m_Flag.event_handle);
        }
        if (event_handle)
        {
            BOOST_INTERLOCKED_EXCHANGE_POINTER(&m_Flag.event_handle, 0);
            boost::detail::win32::CloseHandle(event_handle);
        }
    }
}

#elif defined(BOOST_THREAD_PLATFORM_PTHREAD)

boost::uintmax_t const uninitialized_flag = BOOST_ONCE_INITIAL_FLAG_VALUE;
boost::uintmax_t const being_initialized = uninitialized_flag + 1;

BOOST_LOG_EXPORT void check_execute_once::slow_constructor()
{
    BOOST_VERIFY(!pthread_mutex_lock(&detail::once_epoch_mutex));

    while (m_Flag.epoch <= being_initialized)
    {
        if (m_Flag.epoch == uninitialized_flag)
        {
            m_Flag.epoch = being_initialized;
            BOOST_VERIFY(!pthread_mutex_unlock(&detail::once_epoch_mutex));

            // Invoke the initializer block
            break;
        }
        else
        {
            while (m_Flag.epoch == being_initialized)
            {
                BOOST_VERIFY(!pthread_cond_wait(&detail::once_epoch_cv, &detail::once_epoch_mutex));
            }
        }
    }
}

BOOST_LOG_EXPORT void check_execute_once::slow_destructor()
{
    BOOST_VERIFY(!pthread_mutex_lock(&detail::once_epoch_mutex));

    if (!std::uncaught_exception())
    {
        // The initializer executed successfully
        m_Flag.epoch = --detail::once_global_epoch;
        m_ThisThreadEpoch = detail::once_global_epoch;
    }
    else
    {
        // The initializer failed, marking the flag as if it hasn't run at all
        m_Flag.epoch = uninitialized_flag;
    }

    BOOST_VERIFY(!pthread_mutex_unlock(&detail::once_epoch_mutex));
    BOOST_VERIFY(!pthread_cond_broadcast(&detail::once_epoch_cv));
}

#else
#error Boost.Log: unsupported threading API
#endif

} // namespace aux

} // namespace log

} // namespace boost

#endif // BOOST_LOG_NO_THREADS
