/*
 *          Copyright Andrey Semashev 2007 - 2010.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   execute_once.cpp
 * \author Andrey Semashev
 * \date   23.06.2010
 *
 * \brief  This file is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 *
 * The code in this file is based on the \c call_once function implementation in Boost.Thread.
 */

#include <boost/log/detail/execute_once.hpp>

#ifndef BOOST_LOG_NO_THREADS

#include <boost/assert.hpp>

#if defined(BOOST_THREAD_PLATFORM_WIN32)

#include <boost/detail/interlocked.hpp>
#include <boost/thread/win32/interlocked_read.hpp> // interlocked_read_acquire
#include "windows_version.hpp"
#include <windows.h>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace aux {

BOOST_LOG_ANONYMOUS_NAMESPACE {

    void* allocate_event_handle(void*& handle)
    {
        void* const new_handle = CreateEvent(NULL, TRUE, FALSE, NULL);
        void* event_handle =
            BOOST_INTERLOCKED_COMPARE_EXCHANGE_POINTER(&handle, new_handle, 0);
        if (event_handle)
        {
            CloseHandle(new_handle);
            return event_handle;
        }
        return new_handle;
    }

} // namespace

BOOST_LOG_EXPORT bool execute_once_sentry::enter_once_block() const
{
    long status;
    void* event_handle = 0;

    while ((status = boost::detail::interlocked_read_acquire(&m_Flag.status)) != execute_once_flag::initialized)
    {
        status = BOOST_INTERLOCKED_COMPARE_EXCHANGE(&m_Flag.status, execute_once_flag::being_initialized, 0);
        if (!status)
        {
            if (!event_handle)
            {
                event_handle = boost::detail::interlocked_read_acquire(&m_Flag.event_handle);
            }
            if (event_handle)
            {
                ResetEvent(event_handle);
            }

            // Invoke the initializer block
            return false;
        }

        if (!m_fCounted)
        {
            BOOST_INTERLOCKED_INCREMENT(&m_Flag.count);
            m_fCounted = true;
            status = boost::detail::interlocked_read_acquire(&m_Flag.status);
            if (status == execute_once_flag::initialized)
            {
                // The initializer block has already been executed
                return true;
            }
            event_handle = boost::detail::interlocked_read_acquire(&m_Flag.event_handle);
            if (!event_handle)
            {
                event_handle = allocate_event_handle(m_Flag.event_handle);
                continue;
            }
        }
        BOOST_VERIFY(WaitForSingleObject(event_handle, INFINITE) == 0);
    }

    return true;
}

BOOST_LOG_EXPORT void execute_once_sentry::commit()
{
    void* event_handle = boost::detail::interlocked_read_acquire(&m_Flag.event_handle);

    // The initializer executed successfully
    BOOST_INTERLOCKED_EXCHANGE(&m_Flag.status, execute_once_flag::initialized);

    if (!event_handle && boost::detail::interlocked_read_acquire(&m_Flag.count) > 0)
    {
        event_handle = allocate_event_handle(m_Flag.event_handle);
    }

    // Launch other threads that may have been blocked
    if (event_handle)
    {
        SetEvent(event_handle);
    }

    if (m_fCounted && (BOOST_INTERLOCKED_DECREMENT(&m_Flag.count) == 0))
    {
        if (!event_handle)
        {
            event_handle = boost::detail::interlocked_read_acquire(&m_Flag.event_handle);
        }
        if (event_handle)
        {
            BOOST_INTERLOCKED_EXCHANGE_POINTER(&m_Flag.event_handle, 0);
            CloseHandle(event_handle);
        }
    }
}

BOOST_LOG_EXPORT void execute_once_sentry::rollback()
{
    // The initializer failed, marking the flag as if it hasn't run at all
    BOOST_INTERLOCKED_EXCHANGE(&m_Flag.status, execute_once_flag::uninitialized);

    // Launch other threads that may have been blocked
    void* event_handle = boost::detail::interlocked_read_acquire(&m_Flag.event_handle);
    if (event_handle)
    {
        SetEvent(event_handle);
    }

    BOOST_INTERLOCKED_DECREMENT(&m_Flag.count);
}

} // namespace aux

} // namespace log

} // namespace boost

#elif defined(BOOST_THREAD_PLATFORM_PTHREAD)

#include <pthread.h>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace aux {

BOOST_LOG_ANONYMOUS_NAMESPACE {

static pthread_mutex_t g_ExecuteOnceMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_ExecuteOnceCond = PTHREAD_COND_INITIALIZER;

} // namespace

BOOST_LOG_EXPORT bool execute_once_sentry::enter_once_block() const
{
    BOOST_VERIFY(!pthread_mutex_lock(&g_ExecuteOnceMutex));

    execute_once_flag volatile& flag = m_Flag;
    while (flag.status != execute_once_flag::initialized)
    {
        if (flag.status == execute_once_flag::uninitialized)
        {
            flag.status = execute_once_flag::being_initialized;
            BOOST_VERIFY(!pthread_mutex_unlock(&g_ExecuteOnceMutex));

            // Invoke the initializer block
            return false;
        }
        else
        {
            while (flag.status == execute_once_flag::being_initialized)
            {
                BOOST_VERIFY(!pthread_cond_wait(&g_ExecuteOnceCond, &g_ExecuteOnceMutex));
            }
        }
    }

    BOOST_VERIFY(!pthread_mutex_unlock(&g_ExecuteOnceMutex));

    return true;
}

BOOST_LOG_EXPORT void execute_once_sentry::commit()
{
    BOOST_VERIFY(!pthread_mutex_lock(&g_ExecuteOnceMutex));

    // The initializer executed successfully
    m_Flag.status = execute_once_flag::initialized;

    BOOST_VERIFY(!pthread_mutex_unlock(&g_ExecuteOnceMutex));
    BOOST_VERIFY(!pthread_cond_broadcast(&g_ExecuteOnceCond));
}

BOOST_LOG_EXPORT void execute_once_sentry::rollback()
{
    BOOST_VERIFY(!pthread_mutex_lock(&g_ExecuteOnceMutex));

    // The initializer failed, marking the flag as if it hasn't run at all
    m_Flag.status = execute_once_flag::uninitialized;

    BOOST_VERIFY(!pthread_mutex_unlock(&g_ExecuteOnceMutex));
    BOOST_VERIFY(!pthread_cond_broadcast(&g_ExecuteOnceCond));
}

} // namespace aux

} // namespace log

} // namespace boost

#else
#error Boost.Log: unsupported threading API
#endif

#endif // BOOST_LOG_NO_THREADS
