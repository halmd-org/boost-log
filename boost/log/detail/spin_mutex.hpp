/*
 *          Copyright Andrey Semashev 2007 - 2010.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   spin_mutex.hpp
 * \author Andrey Semashev
 * \date   01.08.2010
 *
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_DETAIL_SPIN_MUTEX_HPP_INCLUDED_
#define BOOST_LOG_DETAIL_SPIN_MUTEX_HPP_INCLUDED_

#include <boost/log/detail/prologue.hpp>

#ifndef BOOST_LOG_NO_THREADS

#if defined(BOOST_THREAD_POSIX) // This one can be defined by users, so it should go first
#define BOOST_LOG_SPIN_MUTEX_USE_PTHREAD
#elif defined(BOOST_WINDOWS)
#define BOOST_LOG_SPIN_MUTEX_USE_WINAPI
#elif defined(BOOST_HAS_PTHREADS)
#define BOOST_LOG_SPIN_MUTEX_USE_PTHREAD
#endif

#if defined(BOOST_LOG_SPIN_MUTEX_USE_WINAPI)

#include <boost/detail/interlocked.hpp>

#if defined(BOOST_USE_WINDOWS_H)

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#include <windows.h>

#else // defined(BOOST_USE_WINDOWS_H)

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace aux {

extern "C" {

__declspec(dllimport) int __stdcall SwitchToThread();

} // extern "C"

} // namespace aux

} // namespace log

} // namespace boost

#endif // BOOST_USE_WINDOWS_H

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace aux {

//! A simple spinning mutex
class spin_mutex
{
private:
    enum _ { spin_count = 8 };

    long m_State;

public:
    spin_mutex() : m_State(0) {}

    bool try_lock()
    {
        return (BOOST_INTERLOCKED_COMPARE_EXCHANGE(&m_State, 1L, 0L) == 0L);
    }

    void lock()
    {
        register unsigned int spins = 0;
        while (true)
        {
            if (try_lock())
                break;
            if (++spins < spin_count)
            {
#if defined(__INTEL_COMPILER) || defined(_MSC_VER)
                __asm
                {
                    pause
                    pause
                    pause
                    pause
                }
#elif defined(__GNUC__)
                __asm__ __volatile__("pause;");
                __asm__ __volatile__("pause;");
                __asm__ __volatile__("pause;");
                __asm__ __volatile__("pause;");
#endif
            }
            else
            {
                spins = 0;
                SwitchToThread();
            }
        }
    }

    void unlock()
    {
        BOOST_INTERLOCKED_EXCHANGE(&m_State, 0);
    }

private:
    //  Non-copyable
    spin_mutex(spin_mutex const&);
    spin_mutex& operator= (spin_mutex const&);
};

} // namespace aux

} // namespace log

} // namespace boost

#elif defined(BOOST_LOG_LWRWMUTEX_USE_PTHREAD)

#include <pthread.h>
#include <boost/assert.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace aux {

#if defined(_POSIX_SPIN_LOCKS)

//! A simple spinning mutex
class spin_mutex
{
private:
    pthread_spinlock_t m_State;

public:
    spin_mutex()
    {
        BOOST_VERIFY(pthread_spin_init(&m_State) == 0);
    }
    ~spin_mutex()
    {
        pthread_spin_destroy(&m_State);
    }
    bool try_lock()
    {
        return (pthread_spin_trylock(&m_State) == 0);
    }
    void lock()
    {
        BOOS_VERIFY(pthread_spin_lock(&m_State) == 0);
    }
    void unlock()
    {
        pthread_spin_unlock(&m_State);
    }

private:
    //  Non-copyable
    spin_mutex(spin_mutex const&);
    spin_mutex& operator= (spin_mutex const&);
};

#else // defined(_POSIX_SPIN_LOCKS)

//! Backup implementation in case if pthreads don't support spin locks
class spin_mutex
{
private:
    pthread_mutex_t m_State;

public:
    spin_mutex()
    {
        BOOST_VERIFY(pthread_mutex_init(&m_State, NULL) == 0);
    }
    ~spin_mutex()
    {
        pthread_mutex_destroy(&m_State);
    }
    bool try_lock()
    {
        return (pthread_mutex_trylock(&m_State) == 0);
    }
    void lock()
    {
        BOOS_VERIFY(pthread_mutex_lock(&m_State) == 0);
    }
    void unlock()
    {
        pthread_mutex_unlock(&m_State);
    }

private:
    //  Non-copyable
    spin_mutex(spin_mutex const&);
    spin_mutex& operator= (spin_mutex const&);
};

#endif // defined(_POSIX_SPIN_LOCKS)

} // namespace aux

} // namespace log

} // namespace boost

#endif

#endif // BOOST_LOG_NO_THREADS

#endif // BOOST_LOG_DETAIL_SPIN_MUTEX_HPP_INCLUDED_
