/*
 * (C) 2009 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * This header is the Boost.Log library implementation, see the library documentation
 * at http://www.boost.org/libs/log/doc/log.html.
 */
/*!
 * \file   exception_handler_feature.hpp
 * \author Andrey Semashev
 * \date   17.07.2009
 *
 * The header contains implementation of an exception handler support feature.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_SOURCES_EXCEPTION_HANDLER_FEATURE_HPP_INCLUDED_
#define BOOST_LOG_SOURCES_EXCEPTION_HANDLER_FEATURE_HPP_INCLUDED_

#include <boost/function/function0.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/sources/threading_models.hpp> // strictest_lock
#if !defined(BOOST_LOG_NO_THREADS)
#include <boost/thread/locks.hpp>
#include <boost/thread/exceptions.hpp>
#endif

#ifdef _MSC_VER
#pragma warning(push)
// 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
#endif // _MSC_VER

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace sources {

/*!
 * \brief Exception handler feature implementation
 */
template< typename BaseT >
class basic_exception_handler_logger :
    public BaseT
{
    //! Base type
    typedef BaseT base_type;

public:
    //! Log record type
    typedef typename base_type::record_type record_type;
    //! Threading model being used
    typedef typename base_type::threading_model threading_model;
    //! Exception handler function type
    typedef function0< void > exception_handler_type;

private:
    //! Exception handler
    exception_handler_type m_ExceptionHandler;

public:
    /*!
     * Default constructor. The constructed logger does not have an exception handler.
     */
    basic_exception_handler_logger() : base_type()
    {
    }
    /*!
     * Copy constructor
     */
    basic_exception_handler_logger(basic_exception_handler_logger const& that) :
        base_type(static_cast< base_type const& >(that)),
        m_ExceptionHandler(that.m_ExceptionHandler)
    {
    }
    /*!
     * Constructor with arguments. Passes arguments to other features.
     */
    template< typename ArgsT >
    explicit basic_exception_handler_logger(ArgsT const& args) :
        base_type(args)
    {
    }

    /*!
     * The method sets exception handler function. The function will be called with no arguments
     * in case if an exception occurs during either \c open_record or \c push_record method
     * execution. Since exception handler is called from a \c catch statement, the exception
     * can be rethrown in order to determine its type.
     *
     * By default no handler is installed, thus any exception is propagated as usual.
     *
     * \sa <tt>utility/exception_handler.hpp</tt>
     * \param handler Exception handling function
     *
     * \note The exception handler can be invoked in several threads concurrently.
     *
     * \note Thread interruptions are not affected by exception handlers.
     */
    template< typename HandlerT >
    void set_exception_handler(HandlerT const& handler)
    {
#ifndef BOOST_LOG_NO_THREADS
        lock_guard< threading_model > _(this->get_threading_model());
#endif
        m_ExceptionHandler = handler;
    }

protected:
    //! Lock requirement for the open_record_unlocked method
    typedef typename strictest_lock<
        typename base_type::open_record_lock,
#ifndef BOOST_LOG_NO_THREADS
        boost::log::aux::shared_lock_guard< threading_model >
#else
        no_lock
#endif // !defined(BOOST_LOG_NO_THREADS)
    >::type open_record_lock;

    /*!
     * Unlocked \c open_record
     */
    template< typename ArgsT >
    record_type open_record_unlocked(ArgsT const& args)
    {
        try
        {
            return base_type::open_record_unlocked(args);
        }
#ifndef BOOST_LOG_NO_THREADS
        catch (thread_interrupted&)
        {
            throw;
        }
#endif
        catch (...)
        {
            if (m_ExceptionHandler.empty())
                throw;
            m_ExceptionHandler();
            return record_type();
        }
    }

    //! Lock requirement for the push_record_unlocked method
    typedef typename strictest_lock<
        typename base_type::push_record_lock,
#ifndef BOOST_LOG_NO_THREADS
        boost::log::aux::shared_lock_guard< threading_model >
#else
        no_lock
#endif // !defined(BOOST_LOG_NO_THREADS)
    >::type push_record_lock;

    /*!
     * Unlocked \c push_record
     */
    void push_record_unlocked(record_type const& record)
    {
        try
        {
            base_type::push_record(record);
        }
#ifndef BOOST_LOG_NO_THREADS
        catch (thread_interrupted&)
        {
            throw;
        }
#endif
        catch (...)
        {
            if (m_ExceptionHandler.empty())
                throw;
            m_ExceptionHandler();
        }
    }

    //! Lock requirement for the swap_unlocked method
    typedef typename strictest_lock<
        typename base_type::swap_lock,
#ifndef BOOST_LOG_NO_THREADS
        lock_guard< threading_model >
#else
        no_lock
#endif // !defined(BOOST_LOG_NO_THREADS)
    >::type swap_lock;

    /*!
     * Unlocked swap
     */
    void swap_unlocked(basic_channel_logger& that)
    {
        base_type::swap_unlocked(static_cast< base_type& >(that));
        m_ExceptionHandler.swap(that.m_ExceptionHandler);
    }
};

/*!
 * \brief Exception handler support feature
 *
 * The logger with this feature will provide an additional method to
 * install an exception handler functional object. This functional
 * object will be called if during either opening or pushing a record
 * an exception is thrown from the logging core.
 */
struct exception_handler
{
    template< typename BaseT >
    struct apply
    {
        typedef basic_exception_handler_logger< BaseT > type;
    };
};

} // namespace sources

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_SOURCES_EXCEPTION_HANDLER_FEATURE_HPP_INCLUDED_
