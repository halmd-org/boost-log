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

#include "windows_version.hpp"
#include <windows.h>

#if defined(BOOST_LOG_USE_WINNT6_API)

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace aux {

BOOST_LOG_ANONYMOUS_NAMESPACE {

    SRWLOCK g_ExecuteOnceMutex = SRWLOCK_INIT;
    CONDITION_VARIABLE g_ExecuteOnceCond = CONDITION_VARIABLE_INIT;

} // namespace

BOOST_LOG_EXPORT bool execute_once_sentry::enter_once_block() const
{
    AcquireSRWLockExclusive(&g_ExecuteOnceMutex);

    execute_once_flag volatile& flag = m_Flag;
    while (flag.status != execute_once_flag::initialized)
    {
        if (flag.status == execute_once_flag::uninitialized)
        {
            flag.status = execute_once_flag::being_initialized;
            ReleaseSRWLockExclusive(&g_ExecuteOnceMutex);

            // Invoke the initializer block
            return false;
        }
        else
        {
            while (flag.status == execute_once_flag::being_initialized)
            {
                BOOST_VERIFY(SleepConditionVariableSRW(
                    &g_ExecuteOnceCond, &g_ExecuteOnceMutex, INFINITE, 0));
            }
        }
    }

    ReleaseSRWLockExclusive(&g_ExecuteOnceMutex);

    return true;
}

BOOST_LOG_EXPORT void execute_once_sentry::commit()
{
    AcquireSRWLockExclusive(&g_ExecuteOnceMutex);

    // The initializer executed successfully
    m_Flag.status = execute_once_flag::initialized;

    ReleaseSRWLockExclusive(&g_ExecuteOnceMutex);
    WakeAllConditionVariable(&g_ExecuteOnceCond);
}

BOOST_LOG_EXPORT void execute_once_sentry::rollback()
{
    AcquireSRWLockExclusive(&g_ExecuteOnceMutex);

    // The initializer failed, marking the flag as if it hasn't run at all
    m_Flag.status = execute_once_flag::uninitialized;

    ReleaseSRWLockExclusive(&g_ExecuteOnceMutex);
    WakeAllConditionVariable(&g_ExecuteOnceCond);
}

} // namespace aux

} // namespace log

} // namespace boost

#else // defined(BOOST_LOG_USE_WINNT6_API)

#include <boost/compatibility/cpp_c_headers/cstdlib> // atexit
#include <boost/detail/interlocked.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/condition_variable.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace aux {

BOOST_LOG_ANONYMOUS_NAMESPACE {

    struct BOOST_LOG_NO_VTABLE execute_once_impl_base
    {
        virtual ~execute_once_impl_base() {}
        virtual bool enter_once_block(execute_once_flag volatile& flag) = 0;
        virtual void commit(execute_once_flag& flag) = 0;
        virtual void rollback(execute_once_flag& flag) = 0;
    };

    class execute_once_impl_nt6 :
        public execute_once_impl_base
    {
    public:
        struct SRWLOCK { void* p; };
        struct CONDITION_VARIABLE { void* p; };

        typedef void (__stdcall *InitializeSRWLock_t)(SRWLOCK*);
        typedef void (__stdcall *AcquireSRWLockExclusive_t)(SRWLOCK*);
        typedef void (__stdcall *ReleaseSRWLockExclusive_t)(SRWLOCK*);
        typedef void (__stdcall *InitializeConditionVariable_t)(CONDITION_VARIABLE*);
        typedef BOOL (__stdcall *SleepConditionVariableSRW_t)(CONDITION_VARIABLE*, SRWLOCK*, DWORD, ULONG);
        typedef void (__stdcall *WakeAllConditionVariable_t)(CONDITION_VARIABLE*);

    private:
        SRWLOCK m_Mutex;
        CONDITION_VARIABLE m_Cond;

        AcquireSRWLockExclusive_t m_pAcquireSRWLockExclusive;
        ReleaseSRWLockExclusive_t m_pReleaseSRWLockExclusive;
        SleepConditionVariableSRW_t m_pSleepConditionVariableSRW;
        WakeAllConditionVariable_t m_pWakeAllConditionVariable;

    public:
        execute_once_impl_nt6(
            InitializeSRWLock_t pInitializeSRWLock,
            AcquireSRWLockExclusive_t pAcquireSRWLockExclusive,
            ReleaseSRWLockExclusive_t pReleaseSRWLockExclusive,
            InitializeConditionVariable_t pInitializeConditionVariable,
            SleepConditionVariableSRW_t pSleepConditionVariableSRW,
            WakeAllConditionVariable_t pWakeAllConditionVariable
        ) :
            m_pAcquireSRWLockExclusive(pAcquireSRWLockExclusive),
            m_pReleaseSRWLockExclusive(pReleaseSRWLockExclusive),
            m_pSleepConditionVariableSRW(pSleepConditionVariableSRW),
            m_pWakeAllConditionVariable(pWakeAllConditionVariable)
        {
            pInitializeSRWLock(&m_Mutex);
            pInitializeConditionVariable(&m_Cond);
        }

        bool enter_once_block(execute_once_flag volatile& flag)
        {
            m_pAcquireSRWLockExclusive(&m_Mutex);

            while (flag.status != execute_once_flag::initialized)
            {
                if (flag.status == execute_once_flag::uninitialized)
                {
                    flag.status = execute_once_flag::being_initialized;
                    m_pReleaseSRWLockExclusive(&m_Mutex);

                    // Invoke the initializer block
                    return false;
                }
                else
                {
                    while (flag.status == execute_once_flag::being_initialized)
                    {
                        BOOST_VERIFY(m_pSleepConditionVariableSRW(
                            &m_Cond, &m_Mutex, INFINITE, 0));
                    }
                }
            }

            m_pReleaseSRWLockExclusive(&m_Mutex);

            return true;
        }

        void commit(execute_once_flag& flag)
        {
            m_pAcquireSRWLockExclusive(&m_Mutex);

            // The initializer executed successfully
            flag.status = execute_once_flag::initialized;

            m_pReleaseSRWLockExclusive(&m_Mutex);
            m_pWakeAllConditionVariable(&m_Cond);
        }

        void rollback(execute_once_flag& flag)
        {
            m_pAcquireSRWLockExclusive(&m_Mutex);

            // The initializer failed, marking the flag as if it hasn't run at all
            flag.status = execute_once_flag::uninitialized;

            m_pReleaseSRWLockExclusive(&m_Mutex);
            m_pWakeAllConditionVariable(&m_Cond);
        }
    };

    class execute_once_impl_nt5 :
        public execute_once_impl_base
    {
    private:
        mutex m_Mutex;
        condition_variable m_Cond;

    public:
        bool enter_once_block(execute_once_flag volatile& flag)
        {
            unique_lock< mutex > lock(m_Mutex);

            while (flag.status != execute_once_flag::initialized)
            {
                if (flag.status == execute_once_flag::uninitialized)
                {
                    flag.status = execute_once_flag::being_initialized;

                    // Invoke the initializer block
                    return false;
                }
                else
                {
                    while (flag.status == execute_once_flag::being_initialized)
                    {
                        m_Cond.wait(lock);
                    }
                }
            }

            return true;
        }

        void commit(execute_once_flag& flag)
        {
            {
                lock_guard< mutex > _(m_Mutex);
                flag.status = execute_once_flag::initialized;
            }
            m_Cond.notify_all();
        }

        void rollback(execute_once_flag& flag)
        {
            {
                lock_guard< mutex > _(m_Mutex);
                flag.status = execute_once_flag::uninitialized;
            }
            m_Cond.notify_all();
        }
    };

    execute_once_impl_base* create_execute_once_impl()
    {
        HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
        if (hKernel32)
        {
            execute_once_impl_nt6::InitializeSRWLock_t pInitializeSRWLock =
                (execute_once_impl_nt6::InitializeSRWLock_t)GetProcAddress(hKernel32, "InitializeSRWLock");
            if (pInitializeSRWLock)
            {
                execute_once_impl_nt6::AcquireSRWLockExclusive_t pAcquireSRWLockExclusive =
                    (execute_once_impl_nt6::AcquireSRWLockExclusive_t)GetProcAddress(hKernel32, "AcquireSRWLockExclusive");
                if (pAcquireSRWLockExclusive)
                {
                    execute_once_impl_nt6::ReleaseSRWLockExclusive_t pReleaseSRWLockExclusive =
                        (execute_once_impl_nt6::ReleaseSRWLockExclusive_t)GetProcAddress(hKernel32, "ReleaseSRWLockExclusive");
                    if (pReleaseSRWLockExclusive)
                    {
                        execute_once_impl_nt6::InitializeConditionVariable_t pInitializeConditionVariable =
                            (execute_once_impl_nt6::InitializeConditionVariable_t)GetProcAddress(hKernel32, "InitializeConditionVariable");
                        if (pInitializeConditionVariable)
                        {
                            execute_once_impl_nt6::SleepConditionVariableSRW_t pSleepConditionVariableSRW =
                                (execute_once_impl_nt6::SleepConditionVariableSRW_t)GetProcAddress(hKernel32, "SleepConditionVariableSRW");
                            if (pSleepConditionVariableSRW)
                            {
                                execute_once_impl_nt6::WakeAllConditionVariable_t pWakeAllConditionVariable =
                                    (execute_once_impl_nt6::WakeAllConditionVariable_t)GetProcAddress(hKernel32, "WakeAllConditionVariable");
                                if (pWakeAllConditionVariable)
                                {
                                    return new execute_once_impl_nt6(
                                        pInitializeSRWLock,
                                        pAcquireSRWLockExclusive,
                                        pReleaseSRWLockExclusive,
                                        pInitializeConditionVariable,
                                        pSleepConditionVariableSRW,
                                        pWakeAllConditionVariable);
                                }
                            }
                        }
                    }
                }
            }
        }

        return new execute_once_impl_nt5();
    }

    execute_once_impl_base* g_pExecuteOnceImpl = NULL;

    void destroy_execute_once_impl()
    {
        execute_once_impl_base* impl = (execute_once_impl_base*)
            BOOST_INTERLOCKED_EXCHANGE_POINTER((void**)&g_pExecuteOnceImpl, NULL);
        delete impl;
    }

    execute_once_impl_base* get_execute_once_impl()
    {
        execute_once_impl_base* impl = g_pExecuteOnceImpl;
        if (!impl)
        {
            execute_once_impl_base* new_impl = create_execute_once_impl();
            impl = (execute_once_impl_base*)
                BOOST_INTERLOCKED_COMPARE_EXCHANGE_POINTER((void**)&g_pExecuteOnceImpl, (void*)new_impl, NULL);
            if (impl)
            {
                delete new_impl;
            }
            else
            {
                std::atexit(&destroy_execute_once_impl);
                return new_impl;
            }
        }

        return impl;
    }

} // namespace

BOOST_LOG_EXPORT bool execute_once_sentry::enter_once_block() const
{
    return get_execute_once_impl()->enter_once_block(m_Flag);
}

BOOST_LOG_EXPORT void execute_once_sentry::commit()
{
    get_execute_once_impl()->commit(m_Flag);
}

BOOST_LOG_EXPORT void execute_once_sentry::rollback()
{
    get_execute_once_impl()->rollback(m_Flag);
}

} // namespace aux

} // namespace log

} // namespace boost

#endif // defined(BOOST_LOG_USE_WINNT6_API)

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
