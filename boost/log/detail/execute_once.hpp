/*
 *          Copyright Andrey Semashev 2007 - 2010.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   execute_once.hpp
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

#include <boost/log/utility/unique_identifier_name.hpp>

#if defined(BOOST_THREAD_PLATFORM_WIN32)

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace aux {

struct execute_once_flag
{
    // Flag status values from Boost.Thread
    enum
    {
        uninitialized = 0,
        being_initialized = 0x7f0725e3,
        initialized = 0xc15730e2
    };

    void* event_handle;
    long status;
    long count;
};

#define BOOST_LOG_ONCE_INIT { 0, boost::log::aux::execute_once_flag::uninitialized, 0 }

class execute_once_sentry
{
private:
    execute_once_flag& m_Flag;
    mutable bool m_fCounted;

public:
    explicit execute_once_sentry(execute_once_flag& f) :
        m_Flag(f),
        m_fCounted(false)
    {
    }

    ~execute_once_sentry()
    {
        if (m_Flag.status != execute_once_flag::initialized)
            rollback();
    }

    bool executed() const
    {
        return (m_Flag.status == execute_once_flag::initialized || enter_once_block());
    }

    BOOST_LOG_EXPORT void commit();

private:
    //  Non-copyable, non-assignable
    execute_once_sentry(execute_once_sentry const&);
    execute_once_sentry& operator= (execute_once_sentry const&);

    BOOST_LOG_EXPORT bool enter_once_block() const;
    BOOST_LOG_EXPORT void rollback();
};

} // namespace aux

} // namespace log

} // namespace boost

#elif defined(BOOST_THREAD_PLATFORM_PTHREAD)

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace aux {

struct execute_once_flag
{
    enum
    {
        uninitialized = 0,
        being_initialized,
        initialized
    }
    status;
};

#define BOOST_LOG_ONCE_INIT { boost::log::aux::execute_once_flag::uninitialized }

class execute_once_sentry
{
private:
    execute_once_flag& m_Flag;

public:
    explicit execute_once_sentry(execute_once_flag& f) : m_Flag(f)
    {
    }

    ~execute_once_sentry()
    {
        if (m_Flag.status != execute_once_flag::initialized)
            rollback();
    }

    bool executed() const
    {
        return (m_Flag.status == execute_once_flag::initialized || enter_once_block());
    }

    BOOST_LOG_EXPORT void commit();

private:
    //  Non-copyable, non-assignable
    execute_once_sentry(execute_once_sentry const&);
    execute_once_sentry& operator= (execute_once_sentry const&);

    BOOST_LOG_EXPORT bool enter_once_block() const;
    BOOST_LOG_EXPORT void rollback();
};

} // namespace aux

} // namespace log

} // namespace boost

#else
#error Boost.Log: unsupported threading API
#endif

#else // BOOST_LOG_NO_THREADS

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace aux {

struct execute_once_flag
{
    bool status;
};

#define BOOST_LOG_ONCE_INIT { false }

class execute_once_sentry
{
private:
    execute_once_flag& m_Flag;

public:
    explicit execute_once_sentry(execute_once_flag& f) : m_Flag(f)
    {
    }

    bool executed() const
    {
        return m_Flag.status;
    }

    void commit()
    {
        m_Flag.status = true;
    }

private:
    //  Non-copyable, non-assignable
    execute_once_sentry(execute_once_sentry const&);
    execute_once_sentry& operator= (execute_once_sentry const&);
};

} // namespace aux

} // namespace log

} // namespace boost

#endif // BOOST_LOG_NO_THREADS

#define BOOST_LOG_EXECUTE_ONCE_FLAG_INTERNAL(flag_var, sentry_var)\
    for (boost::log::aux::execute_once_sentry sentry_var((flag_var));\
        !sentry_var.executed(); sentry_var.commit())

#define BOOST_LOG_EXECUTE_ONCE_INTERNAL(flag_var, sentry_var)\
    static boost::log::aux::execute_once_flag flag_var = BOOST_LOG_ONCE_INIT;\
    BOOST_LOG_EXECUTE_ONCE_FLAG_INTERNAL(flag_var, sentry_var)

/*!
 * \def BOOST_LOG_EXECUTE_ONCE_FLAG(flag_var)
 *
 * Begins a code block to be executed only once, with concurrency protection.
 * User has to provide the flag variable that controls whether the block has already
 * been executed.
 */
#define BOOST_LOG_EXECUTE_ONCE_FLAG(flag_var)\
    BOOST_LOG_EXECUTE_ONCE_INTERNAL(\
        flag_var,\
        BOOST_LOG_UNIQUE_IDENTIFIER_NAME(boost_log_execute_once_sentry_))

/*!
 * \def BOOST_LOG_EXECUTE_ONCE()
 *
 * Begins a code block to be executed only once, with concurrency protection.
 */
#define BOOST_LOG_EXECUTE_ONCE()\
    BOOST_LOG_EXECUTE_ONCE_INTERNAL(\
        BOOST_LOG_UNIQUE_IDENTIFIER_NAME(boost_log_execute_once_flag_),\
        BOOST_LOG_UNIQUE_IDENTIFIER_NAME(boost_log_execute_once_sentry_))

#endif // BOOST_LOG_DETAIL_EXECUTE_ONCE_HPP_INCLUDED_
