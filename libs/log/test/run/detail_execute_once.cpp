/*
 *          Copyright Andrey Semashev 2007 - 2010.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   detail_execute_once.cpp
 * \author Andrey Semashev
 * \date   24.06.2010
 *
 * \brief  This header contains tests for once-blocks implementation.
 *
 * The test was adopted from test_once.cpp test of Boost.Thread.
 */

#define BOOST_TEST_MODULE detail_execute_once

#include <boost/log/detail/execute_once.hpp>

#if !defined(BOOST_LOG_NO_THREADS)

#include <boost/test/included/unit_test.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>

namespace logging = boost::log;

enum config
{
    THREAD_COUNT = 20,
    LOOP_COUNT = 100
};

boost::mutex m;
typedef boost::lock_guard< boost::mutex > scoped_lock;

logging::aux::execute_once_flag flag = BOOST_LOG_ONCE_INIT;
int var_to_init_once_flag = 0;

void initialize_variable()
{
    // ensure that if multiple threads get in here, they are serialized, so we can see the effect
    scoped_lock lock(m);
    ++var_to_init_once_flag;
}


void execute_once_flag_thread()
{
    int my_once_value = 0;
    for (unsigned i = 0; i < LOOP_COUNT; ++i)
    {
        BOOST_LOG_EXECUTE_ONCE_FLAG(flag)
        {
            initialize_variable();
        }

        my_once_value = var_to_init_once_flag;
        if (my_once_value != 1)
        {
            break;
        }
    }
    scoped_lock lock(m);
    BOOST_CHECK_EQUAL(my_once_value, 1);
}

// The test checks if the BOOST_LOG_EXECUTE_ONCE_FLAG macro works
BOOST_AUTO_TEST_CASE(execute_once_flag)
{
    boost::thread_group group;

    try
    {
        for (unsigned i = 0; i < THREAD_COUNT; ++i)
        {
            group.create_thread(&execute_once_flag_thread);
        }
        group.join_all();
    }
    catch (...)
    {
        group.interrupt_all();
        group.join_all();
        throw;
    }

    BOOST_CHECK_EQUAL(var_to_init_once_flag, 1);
}

int var_to_init_once = 0;

void execute_once_thread()
{
    int my_once_value = 0;
    for (unsigned i = 0; i < LOOP_COUNT; ++i)
    {
        BOOST_LOG_EXECUTE_ONCE()
        {
            scoped_lock lock(m);
            ++var_to_init_once;
        }

        my_once_value = var_to_init_once;
        if (my_once_value != 1)
        {
            break;
        }
    }

    scoped_lock lock(m);
    BOOST_CHECK_EQUAL(my_once_value, 1);
}

// The test checks if the BOOST_LOG_EXECUTE_ONCE macro works
BOOST_AUTO_TEST_CASE(execute_once)
{
    boost::thread_group group;

    try
    {
        for (unsigned i = 0; i < THREAD_COUNT; ++i)
        {
            group.create_thread(&execute_once_thread);
        }
        group.join_all();
    }
    catch(...)
    {
        group.interrupt_all();
        group.join_all();
        throw;
    }

    BOOST_CHECK_EQUAL(var_to_init_once, 1);
}


struct my_exception
{
};

unsigned pass_counter = 0;
unsigned exception_counter = 0;

void execute_once_with_exception_thread()
{
    try
    {
        BOOST_LOG_EXECUTE_ONCE()
        {
            scoped_lock lock(m);
            ++pass_counter;
            if (pass_counter < 3)
            {
                throw my_exception();
            }
        }
    }
    catch (my_exception&)
    {
        scoped_lock lock(m);
        ++exception_counter;
    }
}

// The test verifies that the execute_once flag is not set if an exception is thrown from the once-block
BOOST_AUTO_TEST_CASE(execute_once_retried_on_exception)
{
    boost::thread_group group;

    try
    {
        for (unsigned i = 0; i < THREAD_COUNT; ++i)
        {
            group.create_thread(&execute_once_with_exception_thread);
        }
        group.join_all();
    }
    catch(...)
    {
        group.interrupt_all();
        group.join_all();
        throw;
    }

    BOOST_CHECK_EQUAL(pass_counter, 3u);
    BOOST_CHECK_EQUAL(exception_counter, 2u);
}

#endif // BOOST_LOG_NO_THREADS
