/*
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * This header is the Boost.Log library implementation, see the library documentation
 * at http://www.boost.org/libs/log/doc/log.html.
 */
/*!
 * \file   severity_logger.hpp
 * \author Andrey Semashev
 * \date   08.03.2007
 * 
 * The header contains implementation of a logger with severity level support.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_SOURCES_SEVERITY_LOGGER_HPP_INCLUDED_
#define BOOST_LOG_SOURCES_SEVERITY_LOGGER_HPP_INCLUDED_

#include <algorithm>
#include <boost/shared_ptr.hpp>
#include <boost/parameter/keyword.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/detail/singleton.hpp>
#if !defined(BOOST_LOG_NO_THREADS)
#include <boost/log/detail/thread_specific.hpp>
#endif
#include <boost/log/sources/basic_logger.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/attributes/basic_attribute_value.hpp>

#ifdef _MSC_VER
#pragma warning(push)
// 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
// non dll-interface class 'A' used as base for dll-interface class 'B'
#pragma warning(disable: 4275)
#endif // _MSC_VER

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace sources {

namespace keywords {

    BOOST_PARAMETER_KEYWORD(tag, severity)

} // namespace keywords

namespace aux {

    //! A helper traits to get severity attribute name constant in the proper type
    template< typename >
    struct severity_attribute_name;

#ifdef BOOST_LOG_USE_CHAR
    template< >
    struct severity_attribute_name< char >
    {
        static const char* get() { return "Severity"; }
    };
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
    template< >
    struct severity_attribute_name< wchar_t >
    {
        static const wchar_t* get() { return L"Severity"; }
    };
#endif

    //! Severity level attribute implementation
    class severity_level :
        public attribute,
        public attribute_value,
        public enable_shared_from_this< severity_level >,
        public log::aux::lazy_singleton< severity_level, shared_ptr< severity_level > >
    {
        friend class log::aux::lazy_singleton< severity_level, shared_ptr< severity_level > >;
        typedef log::aux::lazy_singleton< severity_level, shared_ptr< severity_level > > singleton_base;

    public:
        typedef int held_type;

    private:
#if !defined(BOOST_LOG_NO_THREADS)
        //! The actual severity level value
        log::aux::thread_specific< held_type > m_Value;
#else
        //! The actual severity level value
        held_type m_Value;
#endif

    public:
        virtual ~severity_level();

        //! Returns an instance of the attribute
        static BOOST_LOG_EXPORT shared_ptr< severity_level > get();

        //! The method returns the actual attribute value. It must not return NULL.
        virtual shared_ptr< attribute_value > get_value();
        //! The method sets the actual level
        void set_value(held_type level)
        {
            m_Value = level;
        }

        //! The method dispatches the value to the given object
        virtual bool dispatch(type_dispatcher& dispatcher);
        //! The method is called when the attribute value is passed to another thread
        virtual shared_ptr< attribute_value > detach_from_thread();

    private:
        severity_level();
        //! Initializes the singleton instance
        static void init_instance();
    };

} // namespace aux

//! Logger class
template< typename BaseT >
class basic_severity_logger :
    public BaseT
{
    //! Base type
    typedef BaseT base_type;

public:
    //! Character type
    typedef typename base_type::char_type char_type;
    //! Final type
    typedef typename base_type::final_type final_type;
    //! Attribute set type
    typedef typename base_type::attribute_set_type attribute_set_type;

    //! Severity attribute type
    typedef aux::severity_level severity_attribute;

private:
    //! Default severity
    severity_attribute::held_type m_DefaultSeverity;
    //! Severity attribute
    shared_ptr< severity_attribute > m_pSeverity;

public:
    //! Constructor
    basic_severity_logger() :
        base_type(),
        m_DefaultSeverity(0),
        m_pSeverity(severity_attribute::get())
    {
        base_type::add_attribute_unlocked(
            aux::severity_attribute_name< char_type >::get(),
            m_pSeverity);
    }
    //! Copy constructor
    basic_severity_logger(basic_severity_logger const& that) :
        base_type(static_cast< base_type const& >(that)),
        m_DefaultSeverity(that.m_DefaultSeverity),
        m_pSeverity(severity_attribute::get())
    {
        base_type::attributes()[aux::severity_attribute_name< char_type >::get()] = m_pSeverity;
    }
    //! Constructor with arguments
    template< typename ArgsT >
    explicit basic_severity_logger(ArgsT const& args) :
        base_type(args),
        m_DefaultSeverity(args[keywords::severity | 0]),
        m_pSeverity(severity_attribute::get())
    {
        base_type::add_attribute_unlocked(
            aux::severity_attribute_name< char_type >::get(),
            m_pSeverity);
    }

    //! The method opens a new logging record with the default severity
    bool open_record()
    {
        m_pSeverity->set_value(m_DefaultSeverity);
        return base_type::open_record();
    }

    //! The method allows to assign a severity to the opening record
    template< typename ArgsT >
    bool open_record(ArgsT const& args)
    {
        m_pSeverity->set_value(args[keywords::severity | m_DefaultSeverity]);
        return base_type::open_record();
    }

protected:
    //! Severity attribute accessor
    shared_ptr< severity_attribute > const& severity() const { return m_pSeverity; }
    //! Default severity value getter
    severity_attribute::held_type default_severity() const { return m_DefaultSeverity; }

    //! The method checks if the message passes filters to be output by at least one sink and opens a record if it does
    bool open_record_unlocked()
    {
        m_pSeverity->set_value(m_DefaultSeverity);
        return base_type::open_record_unlocked();
    }
    //! The method checks if the message passes filters to be output by at least one sink and opens a record if it does
    template< typename ArgsT >
    bool open_record_unlocked(ArgsT const& args)
    {
        m_pSeverity->set_value(args[keywords::severity | m_DefaultSeverity]);
        return base_type::open_record_unlocked();
    }

    //! Unlocked swap
    void swap_unlocked(basic_severity_logger& that)
    {
        base_type::swap_unlocked(static_cast< base_type& >(that));
        std::swap(m_DefaultSeverity, that.m_DefaultSeverity);
    }

private:
    //! Assignment (should be implemented in the final type as copy and swap)
    basic_severity_logger& operator= (basic_severity_logger const&);
};

#ifdef BOOST_LOG_USE_CHAR

//! Narrow-char logger with severity level support
BOOST_LOG_DECLARE_LOGGER(severity_logger, (basic_severity_logger));

#if !defined(BOOST_LOG_NO_THREADS)
//! Narrow-char thread-safe logger with severity level support
BOOST_LOG_DECLARE_LOGGER_MT(severity_logger_mt, (basic_severity_logger));
#endif

#endif

#ifdef BOOST_LOG_USE_WCHAR_T

//! Wide-char logger with severity level support
BOOST_LOG_DECLARE_WLOGGER(wseverity_logger, (basic_severity_logger));

#if !defined(BOOST_LOG_NO_THREADS)
//! Wide-char thread-safe logger with severity level support
BOOST_LOG_DECLARE_WLOGGER_MT(wseverity_logger_mt, (basic_severity_logger));
#endif

#endif

} // namespace sources

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#define BOOST_LOG_SEV(logger, svty)\
    BOOST_LOG_WITH_PARAMS((logger), (::boost::log::sources::keywords::severity = (svty)))

#endif // BOOST_LOG_SOURCES_SEVERITY_LOGGER_HPP_INCLUDED_
