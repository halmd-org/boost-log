/*
 *          Copyright Andrey Semashev 2007 - 2011.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   tick_count.cpp
 * \author Andrey Semashev
 * \date   31.07.2011
 *
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#include <boost/log/detail/tick_count.hpp>

#if defined(BOOST_WINDOWS) && !defined(__CYGWIN__)
#include "windows_version.hpp"
#include <windows.h>
#else
#include <unistd.h> // for config macros
#include <time.h>
#include <errno.h>
#include <boost/throw_exception.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#endif

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace aux {

#if defined(BOOST_WINDOWS) && !defined(__CYGWIN__)

#if _WIN32_WINNT >= 0x0600

// Directly use API from Vista and later
BOOST_LOG_EXPORT get_tick_count_t get_tick_count = &GetTickCount64;

#else // _WIN32_WINNT >= 0x0600

BOOST_LOG_ANONYMOUS_NAMESPACE {

#ifdef _MSC_VER
__declspec(align(16))
#elif defined(__GNUC__)
__attribute__((aligned(16)))
#endif
uint64_t g_ticks = 0;

union ticks_caster
{
    uint64_t as_uint64;
    struct
    {
        uint32_t ticks;
        uint32_t counter;
    }
    as_components;
};

#if defined(_MSC_VER) && !defined(_M_CEE_PURE)

#   if defined(_M_IX86)
//! Atomically loads and stores the 64-bit value
BOOST_LOG_FORCEINLINE void move64(const uint64_t* from, uint64_t* to)
{
    __asm
    {
        mov eax, from
        mov edx, to
        fild qword ptr [eax]
        fistp qword ptr [edx]
    };
}
#   else // == if defined(_M_AMD64)
//! Atomically loads and stores the 64-bit value
BOOST_LOG_FORCEINLINE void move64(const uint64_t* from, uint64_t* to)
{
    *to = *from;
}
#   endif

#elif defined(__GNUC__)

#   if defined(__i386__)
//! Atomically loads and stores the 64-bit value
BOOST_LOG_FORCEINLINE void move64(const uint64_t* from, uint64_t* to)
{
    __asm__ __volatile__
    (
        "fildq %1\n\t"
        "fistpq %0"
            : "=m" (*to)
            : "m" (*from)
            : "memory"
    );
}
#   else // == if defined(__x86_64__)
//! Atomically loads and stores the 64-bit value
BOOST_LOG_FORCEINLINE void move64(const uint64_t* from, uint64_t* to)
{
    *to = *from;
}
#   endif

#else

#error Boost.Log: Atomic operations are not defined for your compiler, sorry

#endif

//! Artifical implementation of GetTickCount64
uint64_t __stdcall get_tick_count64()
{
    ticks_caster state;
    move64(&g_ticks, &state.as_uint64);

    uint32_t new_ticks = GetTickCount();

    state.as_components.counter += new_ticks < state.as_components.ticks;
    state.as_components.ticks = new_ticks;

    move64(&state.as_uint64, &g_ticks);
    return state.as_uint64;
}

//! Performs runtime initialization of the GetTickCount API
uint64_t __stdcall get_tick_count_init()
{
    HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
    if (hKernel32)
    {
        get_tick_count_t p = (get_tick_count_t)GetProcAddress(hKernel32, "GetTickCount64");
        if (p)
        {
            // Use native API
            get_tick_count = p;
            return p();
        }
    }

    // No native API available
    get_tick_count = &get_tick_count64;
    return get_tick_count64();
}

} // namespace

// Use runtime API detection
BOOST_LOG_EXPORT get_tick_count_t get_tick_count = &get_tick_count_init;

#endif // _WIN32_WINNT >= 0x0600

#else

#   if defined(_POSIX_TIMERS) && _POSIX_TIMERS > 0

BOOST_LOG_ANONYMOUS_NAMESPACE {

/*!
 * GetTickCount64 implementation based on POSIX realtime clock.
 * Note that this implementation is only used as a last resort since
 * this timer can be manually set and may jump due to DST change.
 */
uint64_t get_tick_count_realtime_clock()
{
    timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0)
    {
        BOOST_THROW_EXCEPTION(boost::system::system_error(
            errno, boost::system::system_category(), "Failed to acquire current time"));
    }

    return static_cast< uint64_t >(ts.tv_sec) * 1000ULL + ts.tv_nsec / 1000000UL;
}

#       if defined(_POSIX_MONOTONIC_CLOCK)

//! GetTickCount64 implementation based on POSIX monotonic clock
uint64_t get_tick_count_monotonic_clock()
{
    timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
    {
        int err = errno;
        if (err == EINVAL)
        {
            // The current platform does not support monotonic timer.
            // Fall back to realtime clock, which is not exactly what we need
            // but is better than nothing.
            get_tick_count = &get_tick_count_realtime_clock;
            return get_tick_count_realtime_clock();
        }
        BOOST_THROW_EXCEPTION(boost::system::system_error(
            err, boost::system::system_category(), "Failed to acquire current time"));
    }

    return static_cast< uint64_t >(ts.tv_sec) * 1000ULL + ts.tv_nsec / 1000000UL;
}

#           define BOOST_LOG_DEFAULT_GET_TICK_COUNT get_tick_count_monotonic_clock

#       else // if defined(_POSIX_MONOTONIC_CLOCK)
#           define BOOST_LOG_DEFAULT_GET_TICK_COUNT get_tick_count_realtime_clock
#       endif // if defined(_POSIX_MONOTONIC_CLOCK)

} // namespace

// Use POSIX API
BOOST_LOG_EXPORT get_tick_count_t get_tick_count = &BOOST_LOG_DEFAULT_GET_TICK_COUNT;

#       undef BOOST_LOG_DEFAULT_GET_TICK_COUNT

#   else // if defined(_POSIX_TIMERS) && _POSIX_TIMERS > 0

#   error Boost.Log: POSIX timers not supported on your platform

#   endif // if defined(_POSIX_TIMERS) && _POSIX_TIMERS > 0

#endif

} // namespace aux

} // namespace log

} // namespace boost
