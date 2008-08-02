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

#ifndef BOOST_LOG_DOXYGEN_PASS

    BOOST_PARAMETER_KEYWORD(tag, severity)

#else // BOOST_LOG_DOXYGEN_PASS

    //! The keyword is used to pass severity level to the severity logger methods
    implementation_defined severity;

#endif // BOOST_LOG_DOXYGEN_PASS

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

/*!
 * \brief Logger class with severity level support
 * 
 * The logger registers a special attribute with an integral value type on construction.
 * This attribute will provide severity level for each log record being made through the logger.
 * The severity level can be omitted on logging record construction, in which case the default
 * level will be used. The default level can also be customized by passing it to the logger constructor.
 */
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
    /*!
     * Default constructor. The constructed logger will have a severity attribute registered.
     * The default level for log records will be 0.
     */
    basic_severity_logger() :
        base_type(),
        m_DefaultSeverity(0),
        m_pSeverity(severity_attribute::get())
    {
        base_type::add_attribute_unlocked(
            aux::severity_attribute_name< char_type >::get(),
            m_pSeverity);
    }
    /*!
     * Copy constructor
     */
    basic_severity_logger(basic_severity_logger const& that) :
        base_type(static_cast< base_type const& >(that)),
        m_DefaultSeverity(that.m_DefaultSeverity),
        m_pSeverity(severity_attribute::get())
    {
        base_type::attributes()[aux::severity_attribute_name< char_type >::get()] = m_pSeverity;
    }
    /*!
     * Constructor with named arguments. Allows to setup the default level for log records.
     * 
     * \param args A set of named arguments. The following arguments are supported:
     *             \li \c severity - default severity value
     */
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

    /*!
     * The method opens a new logging record with the default severity
     */
    bool open_record()
    {
        m_pSeverity->set_value(m_DefaultSeverity);
        return base_type::open_record();
    }

    /*!
     * The method opens a new logging record. Record level can be specified as one of the named arguments.
     * 
     * \param args A set of named arguments. The following arguments are supported:
     *             \li \c severity - log record severity level
     */
    template< typename ArgsT >
    bool open_record(ArgsT const& args)
    {
        m_pSeverity->set_value(args[keywords::severity | m_DefaultSeverity]);
        return base_type::open_record();
    }

protected:
    /*!
     * Severity attribute accessor
     */
    shared_ptr< severity_attribute > const& severity() const { return m_pSeverity; }
    /*!
     * Default severity value getter
     */
    severity_attribute::held_type default_severity() const { return m_DefaultSeverity; }

    /*!
     * Unlocked \c open_record
     */
    bool open_record_unlocked()
    {
        m_pSeverity->set_value(m_DefaultSeverity);
        return base_type::open_record_unlocked();
    }
    /*!
     * Unlocked \c open_record
     */
    template< typename ArgsT >
    bool open_record_unlocked(ArgsT const& args)
    {
        m_pSeverity->set_value(args[keywords::severity | m_DefaultSeverity]);
        return base_type::open_record_unlocked();
    }

    //! Unlocked \c swap
    void swap_unlocked(basic_severity_logger& that)
    {
        base_type::swap_unlocked(static_cast< base_type& >(that));
        std::swap(m_DefaultSeverity, that.m_DefaultSeverity);
    }
};

#ifndef BOOST_LOG_DOXYGEN_PASS

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

#else // BOOST_LOG_DOXYGEN_PASS

/*!
 * \brief Narrow-char logger. Functionally equivalent to \c basic_severity_logger.
 * 
 * See \c basic_severity_logger class template for a more detailed description
 */
class severity_logger :
    public basic_severity_logger<
        basic_logger< char, severity_logger, single_thread_model >
    >
{
public:
    /*!
     * Default constructor
     */
    severity_logger();
    /*!
     * Copy constructor
     */
    severity_logger(severity_logger const& that);
    /*!
     * Constructor with named arguments
     */
    template< typename... ArgsT >
    explicit severity_logger(ArgsT... const& args);
    /*!
     * Assignment operator
     */
    severity_logger& operator= (severity_logger const& that)
    /*!
     * Swaps two loggers
     */
    void swap(severity_logger& that);
};

/*!
 * \brief Narrow-char thread-safe logger. Functionally equivalent to \c basic_severity_logger.
 * 
 * See \c basic_severity_logger class template for a more detailed description
 */
class severity_logger_mt :
    public basic_severity_logger<
        basic_logger< char, severity_logger_mt, multi_thread_model >
    >
{
public:
    /*!
     * Default constructor
     */
    severity_logger_mt();
    /*!
     * Copy constructor
     */
    severity_logger_mt(severity_logger_mt const& that);
    /*!
     * Constructor with named arguments
     */
    template< typename... ArgsT >
    explicit severity_logger_mt(ArgsT... const& args);
    /*!
     * Assignment operator
     */
    severity_logger_mt& operator= (severity_logger_mt const& that)
    /*!
     * Swaps two loggers
     */
    void swap(severity_logger_mt& that);
};

/*!
 * \brief Wide-char logger. Functionally equivalent to \c basic_severity_logger.
 * 
 * See \c basic_severity_logger class template for a more detailed description
 */
class wseverity_logger :
    public basic_severity_logger<
        basic_logger< wchar_t, wseverity_logger, single_thread_model >
    >
{
public:
    /*!
     * Default constructor
     */
    wseverity_logger();
    /*!
     * Copy constructor
     */
    wseverity_logger(wseverity_logger const& that);
    /*!
     * Constructor with named arguments
     */
    template< typename... ArgsT >
    explicit wseverity_logger(ArgsT... const& args);
    /*!
     * Assignment operator
     */
    wseverity_logger& operator= (wseverity_logger const& that)
    /*!
     * Swaps two loggers
     */
    void swap(wseverity_logger& that);
};

/*!
 * \brief Wide-char thread-safe logger. Functionally equivalent to \c basic_severity_logger.
 * 
 * See \c basic_severity_logger class template for a more detailed description
 */
class wseverity_logger_mt :
    public basic_severity_logger<
        basic_logger< wchar_t, wseverity_logger_mt, multi_thread_model >
    >
{
public:
    /*!
     * Default constructor
     */
    wseverity_logger_mt();
    /*!
     * Copy constructor
     */
    wseverity_logger_mt(wseverity_logger_mt const& that);
    /*!
     * Constructor with named arguments
     */
    template< typename... ArgsT >
    explicit wseverity_logger_mt(ArgsT... const& args);
    /*!
     * Assignment operator
     */
    wseverity_logger_mt& operator= (wseverity_logger_mt const& that)
    /*!
     * Swaps two loggers
     */
    void swap(wseverity_logger_mt& that);
};

#endif // BOOST_LOG_DOXYGEN_PASS

} // namespace sources

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

//! The macro allows to put a record with a specific severity level into log
#define BOOST_LOG_SEV(logger, svty)\
    BOOST_LOG_WITH_PARAMS((logger), (::boost::log::sources::keywords::severity = (svty)))

#endif // BOOST_LOG_SOURCES_SEVERITY_LOGGER_HPP_INCLUDED_
