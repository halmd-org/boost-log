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
 * \file   basic_logger.hpp
 * \author Andrey Semashev
 * \date   08.03.2007
 * 
 * The header contains implementation of a base class for loggers. Convenience macros
 * for defining custom loggers are also provided.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_SOURCES_BASIC_LOGGER_HPP_INCLUDED_
#define BOOST_LOG_SOURCES_BASIC_LOGGER_HPP_INCLUDED_

#include <exception>
#include <string>
#include <utility>
#include <ostream>
#include <boost/assert.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility/addressof.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/detail/attachable_sstream_buf.hpp>
#include <boost/log/detail/shared_lock_guard.hpp>
#include <boost/log/detail/multiple_lock.hpp>
#include <boost/log/logging_core.hpp>
#include <boost/log/attributes/attribute_set.hpp>
#if !defined(BOOST_LOG_NO_THREADS)
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#endif

#ifdef _MSC_VER
#pragma warning(push)
// 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
#endif // _MSC_VER

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace sources {

template< typename CharT, typename FinalT, typename ThreadingModelT >
class basic_logger;

//! Single thread locking model
struct single_thread_model
{
    // We provide methods for the most advanced locking concept: UpgradeLockable
    void lock_shared() const {}
    bool try_lock_shared() const { return true; }
    template< typename TimeT >
    bool timed_lock_shared(TimeT const&) const { return true; }
    void unlock_shared() const {}
    void lock() const {}
    bool try_lock() const { return true; }
    template< typename TimeT >
    bool timed_lock(TimeT const&) const { return true; }
    void unlock() const {}
    void lock_upgrade() const {}
    bool try_lock_upgrade() const { return true; }
    template< typename TimeT >
    bool timed_lock_upgrade(TimeT const&) const { return true; }
    void unlock_upgrade() const {}
    void unlock_upgrade_and_lock() const {}
    void unlock_and_lock_upgrade() const {}
    void unlock_and_lock_shared() const {}
    void unlock_upgrade_and_lock_shared() const {}

    void swap(single_thread_model&) {}
};

#if !defined(BOOST_LOG_NO_THREADS)

//! Multi-thread locking model
struct multi_thread_model
{
    multi_thread_model() {}
    multi_thread_model(multi_thread_model const&) {}
    multi_thread_model& operator= (multi_thread_model const&) { return *this; }

    void lock_shared() const { m_Mutex.lock_shared(); }
    bool try_lock_shared() const { return m_Mutex.try_lock_shared(); }
    template< typename TimeT >
    bool timed_lock_shared(TimeT const& t) const { return m_Mutex.timed_lock_shared(t); }
    void unlock_shared() const { m_Mutex.unlock_shared(); }
    void lock() const { m_Mutex.lock(); }
    bool try_lock() const { return m_Mutex.try_lock(); }
    template< typename TimeT >
    bool timed_lock(TimeT const& t) const { return m_Mutex.timed_lock(t); }
    void unlock() const { m_Mutex.unlock(); }
    void lock_upgrade() const { m_Mutex.lock_upgrade(); }
    bool try_lock_upgrade() const { return m_Mutex.try_lock_upgrade(); }
    template< typename TimeT >
    bool timed_lock_upgrade(TimeT const& t) const { return m_Mutex.timed_lock_upgrade(t); }
    void unlock_upgrade() const { m_Mutex.unlock_upgrade(); }
    void unlock_upgrade_and_lock() const { m_Mutex.unlock_upgrade_and_lock(); }
    void unlock_and_lock_upgrade() const { m_Mutex.unlock_and_lock_upgrade(); }
    void unlock_and_lock_shared() const { m_Mutex.unlock_and_lock_shared(); }
    void unlock_upgrade_and_lock_shared() const { m_Mutex.unlock_upgrade_and_lock_shared(); }

    void swap(multi_thread_model&) {}

private:
    //! Synchronization primitive
    mutable shared_mutex m_Mutex;
};

#endif // !defined(BOOST_LOG_NO_THREADS)

namespace aux {

    //! Internal class that provides formatting streams for record pumps
    template< typename CharT >
    struct stream_provider
    {
        //! Character type
        typedef CharT char_type;
        //! String type to be used as a message text holder
        typedef std::basic_string< char_type > string_type;
        //! Stream device type
        typedef log::aux::basic_ostringstreambuf< char_type > ostream_buf;
        //! Output stream type
        typedef std::basic_ostream< char_type > ostream_type;

        //! Formatting stream compound
        struct stream_compound
        {
            stream_compound* next;

            //! The string to be written to
            string_type message;
            //! The streambuf
            ostream_buf stream_buf;
            //! Output stream
            ostream_type stream;

            stream_compound() :
                next(NULL),
                stream_buf(message),
                stream(&stream_buf)
            {
            }
        };

        //! The method returns an allocated stream compound
        BOOST_LOG_EXPORT static stream_compound* allocate_compound();
        //! The method releases a compound
        BOOST_LOG_EXPORT static void release_compound(stream_compound* compound) /* throw() */;

    private:
        //  Non-constructible, non-copyable, non-assignable
        stream_provider();
        stream_provider(stream_provider const&);
        stream_provider& operator= (stream_provider const&);
    };


    //! Record pump implementation
    template< typename LoggerT >
    class record_pump
    {
    public:
        //! The metafunction allows to adopt the pump to another logger type
        template< typename T >
        struct rebind
        {
            typedef record_pump< T > other;
        };

    private:
        //! Logger type
        typedef LoggerT logger_type;
        //! Character type
        typedef typename logger_type::char_type char_type;
        //! Stream compound provider
        typedef stream_provider< char_type > stream_provider_type;
        //! Stream compound type
        typedef typename stream_provider_type::stream_compound stream_compound;

    protected:
        //! A reference to the logger
        mutable logger_type* m_pLogger;
        //! Stream compound
        mutable stream_compound* m_pStreamCompound;

    public:
        //! Constructor
        explicit record_pump(logger_type* p) :
            m_pLogger(p),
            m_pStreamCompound(stream_provider_type::allocate_compound())
        {
        }
        //! Copy constructor (implemented as move)
        record_pump(record_pump const& that) :
            m_pLogger(that.m_pLogger),
            m_pStreamCompound(that.m_pStreamCompound)
        {
            that.m_pLogger = 0;
            that.m_pStreamCompound = 0;
        }
        //! Destructor. Pushes the composed message to log.
        ~record_pump()
        {
            if (m_pLogger)
            {
                try
                {
                    m_pStreamCompound->stream.flush();
                    m_pLogger->push_record(m_pStreamCompound->message);
                }
                catch (std::exception&)
                {
                    m_pLogger->cancel_record();
                }

                stream_provider_type::release_compound(m_pStreamCompound); // doesn't throw
            }
        }

        //! Forwarding output operators
        template< typename T >
        record_pump const& operator<< (T const& value) const
        {
            BOOST_ASSERT(m_pStreamCompound != 0);
            m_pStreamCompound->stream << value;
            return *this;
        }

    private:
        //! Closed assignment
        record_pump& operator= (record_pump const&);
    };

} // namespace aux

//! Logger class
template< typename CharT, typename FinalT, typename ThreadingModelT >
class basic_logger :
    public ThreadingModelT
{
    friend class aux::record_pump< FinalT >;

public:
    //! Character type
    typedef CharT char_type;
    //! Final logger type
    typedef FinalT final_type;

    //! String type to be used as a message text holder
    typedef std::basic_string< char_type > string_type;
    //! Attribute set type
    typedef basic_attribute_set< char_type > attribute_set_type;
    //! Logging system core type
    typedef basic_logging_core< char_type > logging_core_type;
    //! Threading model type
    typedef ThreadingModelT threading_model;

protected:
    //! Record pump type
    typedef aux::record_pump< final_type > record_pump_type;

private:
    //! A pointer to the logging system
    shared_ptr< logging_core_type > m_pLoggingSystem;

    //! Logger-specific attribute set
    attribute_set_type m_Attributes;

public:
    //! Constructor
    basic_logger() :
        m_pLoggingSystem(logging_core_type::get())
    {
    }
    //! Copy constructor
    basic_logger(basic_logger const& that) :
        m_pLoggingSystem(logging_core_type::get()),
        m_Attributes(that.m_Attributes)
    {
    }
    //! Constructor with arguments
    template< typename ArgsT >
    explicit basic_logger(ArgsT const& args) :
        m_pLoggingSystem(logging_core_type::get())
    {
    }

    //! Logging stream getter
    record_pump_type strm()
    {
        return strm_unlocked();
    }

    //! The method adds an attribute to the logger
    std::pair< typename attribute_set_type::iterator, bool > add_attribute(
        string_type const& name, shared_ptr< attribute > const& attr)
    {
#if !defined(BOOST_LOG_NO_THREADS)
        lock_guard< threading_model > _(threading_base());
#endif
        return add_attribute_unlocked(name, attr);
    }
    //! The method removes an attribute from the logger
    void remove_attribute(typename attribute_set_type::iterator it)
    {
#if !defined(BOOST_LOG_NO_THREADS)
        lock_guard< threading_model > _(threading_base());
#endif
        remove_attribute_unlocked(it);
    }

    //! The method removes all attributes from the logger
    void remove_all_attributes()
    {
#if !defined(BOOST_LOG_NO_THREADS)
        lock_guard< threading_model > _(threading_base());
#endif
        remove_all_attributes_unlocked();
    }

    //! The method checks if the message passes filters to be output by at least one sink and opens a record if it does
    bool open_record()
    {
#if !defined(BOOST_LOG_NO_THREADS)
        log::aux::shared_lock_guard< threading_model > _(threading_base());
#endif
        return open_record_unlocked();
    }
    //! The method checks if the message passes filters to be output by at least one sink and opens a record if it does
    template< typename ArgsT >
    bool open_record(ArgsT const& args)
    {
#if !defined(BOOST_LOG_NO_THREADS)
        log::aux::shared_lock_guard< threading_model > _(threading_base());
#endif
        return open_record_unlocked(args);
    }
    //! The method pushes the constructed message to the sinks and closes the record
    void push_record(string_type const& message)
    {
        push_record_unlocked(message);
    }
    //! The method cancels the currently opened record
    void cancel_record()
    {
        cancel_record_unlocked();
    }

protected:
    //! An accessor to the logging system pointer
    shared_ptr< logging_core_type > const& core() const { return m_pLoggingSystem; }
    //! An accessor to the logger attributes
    attribute_set_type& attributes() { return m_Attributes; }
    //! An accessor to the logger attributes
    attribute_set_type const& attributes() const { return m_Attributes; }
    //! An accessor to the threading model base
    threading_model& threading_base() { return *this; }
    //! An accessor to the threading model base
    threading_model const& threading_base() const { return *this; }
    //! An accessor to the final logger type
    final_type* final_this()
    {
        BOOST_LOG_ASSUME(this != NULL);
        return static_cast< final_type* >(this);
    }
    //! An accessor to the final logger type
    final_type const* final_this() const
    {
        BOOST_LOG_ASSUME(this != NULL);
        return static_cast< final_type const* >(this);
    }

    //! Unlocked swap
    void swap_unlocked(basic_logger& that)
    {
        threading_base().swap(that.threading_base());
        m_Attributes.swap(that.m_Attributes);
    }

    //! Logging stream getter
    record_pump_type strm_unlocked()
    {
        return record_pump_type(final_this());
    }

    //! The method adds an attribute to the logger
    std::pair< typename attribute_set_type::iterator, bool > add_attribute_unlocked(
        string_type const& name, shared_ptr< attribute > const& attr)
    {
        return m_Attributes.insert(std::make_pair(name, attr));
    }
    //! The method removes an attribute from the logger
    void remove_attribute_unlocked(typename attribute_set_type::iterator it)
    {
        m_Attributes.erase(it);
    }

    //! The method removes all attributes from the logger
    void remove_all_attributes_unlocked()
    {
        m_Attributes.clear();
    }

    //! The method checks if the message passes filters to be output by at least one sink and opens a record if it does
    bool open_record_unlocked()
    {
        return m_pLoggingSystem->open_record(m_Attributes);
    }
    //! The method checks if the message passes filters to be output by at least one sink and opens a record if it does
    template< typename ArgsT >
    bool open_record_unlocked(ArgsT const& args)
    {
        return m_pLoggingSystem->open_record(m_Attributes);
    }
    //! The method pushes the constructed message to the sinks and closes the record
    void push_record_unlocked(string_type const& message)
    {
        m_pLoggingSystem->push_record(message);
    }
    //! The method cancels the currently opened record
    void cancel_record_unlocked()
    {
        m_pLoggingSystem->cancel_record();
    }

private:
    //! Assignment (should be implemented through copy and swap in the final class)
    basic_logger& operator= (basic_logger const&);
};

//! Free-standing swap for loggers
template< typename CharT, typename FinalT, typename ThreadingModelT >
inline void swap(
    basic_logger< CharT, FinalT, ThreadingModelT >& left,
    basic_logger< CharT, FinalT, ThreadingModelT >& right)
{
    static_cast< FinalT& >(left).swap(static_cast< FinalT& >(right));
}

#ifdef BOOST_LOG_USE_CHAR

//! Narrow-char logger
class logger :
    public basic_logger< char, logger, single_thread_model >
{
public:
    logger& operator= (logger const& that)
    {
        if (this != &that)
        {
            logger tmp(that);
            swap_unlocked(tmp);
        }
        return *this;
    }

    void swap(logger& that)
    {
        swap_unlocked(that);
    }
};

#if !defined(BOOST_LOG_NO_THREADS)

//! Narrow-char thread-safe logger
class logger_mt :
    public basic_logger< char, logger_mt, multi_thread_model >
{
    typedef basic_logger< char, logger_mt, multi_thread_model > base_type;

public:
    logger_mt() {}
    logger_mt(logger_mt const& that) :
        base_type((
            log::aux::shared_lock_guard< const threading_model >(that.threading_base()),
            static_cast< base_type const& >(that)
        ))
    {
    }

    logger_mt& operator= (logger_mt const& that)
    {
        if (this != &that)
        {
            logger_mt tmp(that);
            lock_guard< threading_model > _(threading_base());
            swap_unlocked(tmp);
        }
        return *this;
    }

    void swap(logger_mt& that)
    {
        log::aux::multiple_unique_lock2<
            threading_model,
            threading_model
        > _(threading_base(), that.threading_base());
        swap_unlocked(that);
    }
};

#endif // !defined(BOOST_LOG_NO_THREADS)
#endif // BOOST_LOG_USE_CHAR

#ifdef BOOST_LOG_USE_WCHAR_T

//! Wide-char logger
class wlogger :
    public basic_logger< wchar_t, wlogger, single_thread_model >
{
public:
    wlogger& operator= (wlogger const& that)
    {
        if (this != &that)
        {
            wlogger tmp(that);
            swap_unlocked(tmp);
        }
        return *this;
    }

    void swap(wlogger& that)
    {
        swap_unlocked(that);
    }
};

#if !defined(BOOST_LOG_NO_THREADS)

//! Wide-char thread-safe logger
class wlogger_mt :
    public basic_logger< wchar_t, wlogger_mt, multi_thread_model >
{
    typedef basic_logger< wchar_t, wlogger_mt, multi_thread_model > base_type;

public:
    wlogger_mt() {}
    wlogger_mt(wlogger_mt const& that) :
        base_type((
            log::aux::shared_lock_guard< const threading_model >(that.threading_base()),
            static_cast< base_type const& >(that)
        ))
    {
    }

    wlogger_mt& operator= (wlogger_mt const& that)
    {
        if (this != &that)
        {
            wlogger_mt tmp(that);
            lock_guard< threading_model > _(threading_base());
            swap_unlocked(tmp);
        }
        return *this;
    }

    void swap(wlogger_mt& that)
    {
        log::aux::multiple_unique_lock2<
            threading_model,
            threading_model
        > _(threading_base(), that.threading_base());
        swap_unlocked(that);
    }
};

#endif // !defined(BOOST_LOG_NO_THREADS)
#endif // BOOST_LOG_USE_WCHAR_T

} // namespace sources

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

//! The macro writes a record to the log
#define BOOST_LOG(logger)\
    if (!(logger).open_record())\
        ((void)0);\
    else\
        (logger).strm()

//! The macro writes a record to the log and allows to pass additional arguments to the logger
#define BOOST_LOG_WITH_PARAMS(logger, params_seq)\
    if (!(logger).open_record((BOOST_PP_SEQ_ENUM(params_seq))))\
        ((void)0);\
    else\
        (logger).strm()


#define BOOST_LOG_DECLARE_LOGGER_TYPE_INTERNAL1(r, data, elem) elem<
#define BOOST_LOG_DECLARE_LOGGER_TYPE_INTERNAL2(r, data, elem) >

#ifndef BOOST_LOG_MAX_CTOR_FORWARD_ARGS
//! The maximum number of arguments that can be forwarded by the logger constructor to its bases
#define BOOST_LOG_MAX_CTOR_FORWARD_ARGS 16
#endif

#define BOOST_LOG_CTOR_FORWARD(z, n, data)\
    template< BOOST_PP_ENUM_PARAMS(n, typename T) >\
    explicit data(BOOST_PP_ENUM_BINARY_PARAMS(n, T, const& arg)) : base_type((BOOST_PP_ENUM_PARAMS(n, arg))) {}

#if !defined(BOOST_LOG_NO_THREADS)

/*!
 *  \brief The macro declares a logger class that inherits a number of base classes
 * 
 *  \param type_name The name of the logger class to declare
 *  \param char_type The character type of the logger. Either char or wchar_t expected.
 *  \param base_seq A Boost.Preprocessor sequence of type identifiers of the base classes templates
 *  \param threading A threading model class
 */
#define BOOST_LOG_DECLARE_LOGGER_TYPE(type_name, char_type, base_seq, threading)\
    class type_name :\
        public BOOST_PP_SEQ_FOR_EACH(BOOST_LOG_DECLARE_LOGGER_TYPE_INTERNAL1, ~, base_seq)\
            ::boost::log::sources::basic_logger< char_type, type_name, threading >\
            BOOST_PP_SEQ_FOR_EACH(BOOST_LOG_DECLARE_LOGGER_TYPE_INTERNAL2, ~, base_seq)\
    {\
        typedef BOOST_PP_SEQ_FOR_EACH(BOOST_LOG_DECLARE_LOGGER_TYPE_INTERNAL1, ~, base_seq)\
            ::boost::log::sources::basic_logger< char_type, type_name, threading >\
            BOOST_PP_SEQ_FOR_EACH(BOOST_LOG_DECLARE_LOGGER_TYPE_INTERNAL2, ~, base_seq) base_type;\
    public:\
        typedef base_type::threading_model threading_model;\
    public:\
        type_name() {}\
        type_name(type_name const& that) :\
            base_type((\
                ::boost::log::aux::shared_lock_guard< const threading_model >(that.threading_base()),\
                static_cast< base_type const& >(that)\
            ))\
        {\
        }\
        BOOST_PP_REPEAT_FROM_TO(1, BOOST_LOG_MAX_CTOR_FORWARD_ARGS, BOOST_LOG_CTOR_FORWARD, type_name)\
        type_name& operator= (type_name const& that)\
        {\
            if (this != ::boost::addressof(that))\
            {\
                type_name tmp(that);\
                ::boost::lock_guard< threading_model > _(threading_base());\
                swap_unlocked(tmp);\
            }\
            return *this;\
        }\
        void swap(type_name& that)\
        {\
            ::boost::log::aux::multiple_unique_lock2<\
                threading_model,\
                threading_model\
            > _(threading_base(), that.threading_base());\
            swap_unlocked(that);\
        }\
    }

#else // !defined(BOOST_LOG_NO_THREADS)

/*!
 *  \brief The macro declares a logger class that inherits a number of base classes
 * 
 *  \param type_name The name of the logger class to declare
 *  \param char_type The character type of the logger. Either char or wchar_t expected.
 *  \param base_seq A Boost.Preprocessor sequence of type identifiers of the base classes templates
 *  \param threading A threading model class
 */
#define BOOST_LOG_DECLARE_LOGGER_TYPE(type_name, char_type, base_seq, threading)\
    class type_name :\
        public BOOST_PP_SEQ_FOR_EACH(BOOST_LOG_DECLARE_LOGGER_TYPE_INTERNAL1, ~, base_seq)\
            ::boost::log::sources::basic_logger< char_type, type_name, threading >\
            BOOST_PP_SEQ_FOR_EACH(BOOST_LOG_DECLARE_LOGGER_TYPE_INTERNAL2, ~, base_seq)\
    {\
        typedef BOOST_PP_SEQ_FOR_EACH(BOOST_LOG_DECLARE_LOGGER_TYPE_INTERNAL1, ~, base_seq)\
            ::boost::log::sources::basic_logger< char_type, type_name, threading >\
            BOOST_PP_SEQ_FOR_EACH(BOOST_LOG_DECLARE_LOGGER_TYPE_INTERNAL2, ~, base_seq) base_type;\
    public:\
        typedef base_type::threading_model threading_model;\
    public:\
        type_name() {}\
        type_name(type_name const& that) :\
            base_type(static_cast< base_type const& >(that))\
        {\
        }\
        BOOST_PP_REPEAT_FROM_TO(1, BOOST_LOG_MAX_CTOR_FORWARD_ARGS, BOOST_LOG_CTOR_FORWARD, type_name)\
        type_name& operator= (type_name const& that)\
        {\
            if (this != ::boost::addressof(that))\
            {\
                type_name tmp(that);\
                swap_unlocked(tmp);\
            }\
            return *this;\
        }\
        void swap(type_name& that)\
        {\
            swap_unlocked(that);\
        }\
    }

#endif // !defined(BOOST_LOG_NO_THREADS)

#ifdef BOOST_LOG_USE_CHAR

/*!
 *  \brief The macro declares a narrow-char logger class that inherits a number of base classes
 * 
 *  Equivalent to BOOST_LOG_DECLARE_LOGGER_TYPE(type_name, char, base_seq, single_thread_model)
 * 
 *  \param type_name The name of the logger class to declare
 *  \param base_seq A Boost.Preprocessor sequence of type identifiers of the base classes templates
 */
#define BOOST_LOG_DECLARE_LOGGER(type_name, base_seq)\
    BOOST_LOG_DECLARE_LOGGER_TYPE(type_name, char, base_seq, ::boost::log::sources::single_thread_model)

#if !defined(BOOST_LOG_NO_THREADS)

/*!
 *  \brief The macro declares a narrow-char thread-safe logger class that inherits a number of base classes
 * 
 *  Equivalent to BOOST_LOG_DECLARE_LOGGER_TYPE(type_name, char, base_seq, multi_thread_model)
 * 
 *  \param type_name The name of the logger class to declare
 *  \param base_seq A Boost.Preprocessor sequence of type identifiers of the base classes templates
 */
#define BOOST_LOG_DECLARE_LOGGER_MT(type_name, base_seq)\
    BOOST_LOG_DECLARE_LOGGER_TYPE(type_name, char, base_seq, ::boost::log::sources::multi_thread_model)

#endif // !defined(BOOST_LOG_NO_THREADS)
#endif // BOOST_LOG_USE_CHAR

#ifdef BOOST_LOG_USE_WCHAR_T

/*!
 *  \brief The macro declares a wide-char logger class that inherits a number of base classes
 * 
 *  Equivalent to BOOST_LOG_DECLARE_LOGGER_TYPE(type_name, wchar_t, base_seq, single_thread_model)
 * 
 *  \param type_name The name of the logger class to declare
 *  \param base_seq A Boost.Preprocessor sequence of type identifiers of the base classes templates
 */
#define BOOST_LOG_DECLARE_WLOGGER(type_name, base_seq)\
    BOOST_LOG_DECLARE_LOGGER_TYPE(type_name, wchar_t, base_seq, ::boost::log::sources::single_thread_model)

#if !defined(BOOST_LOG_NO_THREADS)

/*!
 *  \brief The macro declares a wide-char thread-safe logger class that inherits a number of base classes
 * 
 *  Equivalent to BOOST_LOG_DECLARE_LOGGER_TYPE(type_name, wchar_t, base_seq, multi_thread_model)
 * 
 *  \param type_name The name of the logger class to declare
 *  \param base_seq A Boost.Preprocessor sequence of type identifiers of the base classes templates
 */
#define BOOST_LOG_DECLARE_WLOGGER_MT(type_name, base_seq)\
    BOOST_LOG_DECLARE_LOGGER_TYPE(type_name, wchar_t, base_seq, ::boost::log::sources::multi_thread_model)

#endif // !defined(BOOST_LOG_NO_THREADS)
#endif // BOOST_LOG_USE_WCHAR_T

#endif // BOOST_LOG_SOURCES_BASIC_LOGGER_HPP_INCLUDED_
