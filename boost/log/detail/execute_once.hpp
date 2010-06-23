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
    void* event_handle;
    long status;
    long count;
};

#define BOOST_LOG_ONCE_INIT { 0, 0, 0 }

class execute_once_sentry
{
private:
    // Flag status values from Boost.Thread
    enum
    {
        running_value = 0x7f0725e3,
        function_complete_flag_value = 0xc15730e2
    };

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
        if (m_Flag.status != function_complete_flag_value)
            finalize_once();
    }

    bool executed() const
    {
        return (m_Flag.status == function_complete_flag_value || executed_once());
    }

private:
    //  Non-copyable, non-assignable
    execute_once_sentry(execute_once_sentry const&);
    execute_once_sentry& operator= (execute_once_sentry const&);

    BOOST_LOG_EXPORT bool executed_once() const;
    BOOST_LOG_EXPORT void finalize_once();
};

} // namespace aux

} // namespace log

} // namespace boost

#elif defined(BOOST_THREAD_PLATFORM_PTHREAD)

#include <boost/cstdint.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace aux {

struct execute_once_flag
{
    boost::uintmax_t epoch;
};

#define BOOST_LOG_ONCE_INIT { BOOST_ONCE_INITIAL_FLAG_VALUE }

class execute_once_sentry
{
private:
    execute_once_flag& m_Flag;
    boost::uintmax_t& m_ThisThreadEpoch;

public:
    BOOST_LOG_EXPORT explicit execute_once_sentry(execute_once_flag& f);

    ~execute_once_sentry()
    {
        if (m_Flag.epoch < m_ThisThreadEpoch)
            finalize_once();
    }

    bool executed() const
    {
        return (m_Flag.epoch >= m_ThisThreadEpoch || executed_once());
    }

private:
    //  Non-copyable, non-assignable
    execute_once_sentry(execute_once_sentry const&);
    execute_once_sentry& operator= (execute_once_sentry const&);

    BOOST_LOG_EXPORT bool executed_once() const;
    BOOST_LOG_EXPORT void finalize_once();
};

} // namespace aux

} // namespace log

} // namespace boost

#else
#error Boost.Log: unsupported threading API
#endif

#define BOOST_LOG_EXECUTE_ONCE_FLAG_INTERNAL(flag_var, sentry_var)\
    for (boost::log::aux::execute_once_sentry const& sentry_var =\
            boost::log::aux::execute_once_sentry((flag_var));\
        !sentry_var.executed();)

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

#define BOOST_LOG_EXECUTE_ONCE_INTERNAL(flag_var, sentry_var)\
    static boost::log::aux::execute_once_flag flag_var = BOOST_LOG_ONCE_INIT;\
    BOOST_LOG_EXECUTE_ONCE_FLAG_INTERNAL(flag_var, sentry_var)

/*!
 * \def BOOST_LOG_EXECUTE_ONCE()
 *
 * Begins a code block to be executed only once, with concurrency protection.
 */
#define BOOST_LOG_EXECUTE_ONCE()\
    BOOST_LOG_EXECUTE_ONCE_INTERNAL(\
        BOOST_LOG_UNIQUE_IDENTIFIER_NAME(boost_log_execute_once_flag_),\
        BOOST_LOG_UNIQUE_IDENTIFIER_NAME(boost_log_execute_once_sentry_))

#endif // BOOST_LOG_NO_THREADS

#endif // BOOST_LOG_DETAIL_EXECUTE_ONCE_HPP_INCLUDED_
