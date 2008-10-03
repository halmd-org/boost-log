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
#include <boost/mpl/vector.hpp>
#include <boost/mpl/lambda.hpp>
#include <boost/mpl/reverse_fold.hpp>
#include <boost/mpl/placeholders.hpp> // for usage convenience, inspite that it's not used in this header directly
#include <boost/utility/addressof.hpp>
#include <boost/preprocessor/facilities/empty.hpp>
#include <boost/preprocessor/facilities/identity.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/detail/attachable_sstream_buf.hpp>
#include <boost/log/detail/shared_lock_guard.hpp>
#include <boost/log/detail/multiple_lock.hpp>
#include <boost/log/attributes/attribute_set.hpp>
#include <boost/log/core.hpp>
#if !defined(BOOST_LOG_NO_THREADS)
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#endif

#ifndef BOOST_LOG_MAX_CTOR_FORWARD_ARGS
//! The maximum number of arguments that can be forwarded by the logger constructor to its bases
#define BOOST_LOG_MAX_CTOR_FORWARD_ARGS 16
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

//! Multi-thread locking model with maximum locking capabilities
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

} // namespace aux

/*!
 * \brief Logging record pump implementation
 *
 * The pump is used to format the logging record message text and then
 * push it to the logging core. It is constructed on each attempt to write
 * a log record and destroyed afterwards.
 *
 * The pump class template is instantiated on the logger type.
 */
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
    typedef aux::stream_provider< char_type > stream_provider_type;
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

/*!
 * \brief Basic logger class
 *
 * The \c basic_logger class template serves as a base class for all loggers
 * provided by the library. It can also be used as a base for user-defined
 * loggers. The template parameters are:
 *
 * \li \c CharT - logging character type
 * \li \c FinalT - final type of the logger that eventually derives from
 *     the \c basic_logger. There may be other classes in the hierarchy
 *     between the final class and \c basic_logger.
 * \li \c ThreadingModelT - threading model policy. Must provide methods
 *     of the Boost.Thread locking concept used in \c basic_logger class
 *     and all its derivatives in the hierarchy up to the \c FinalT class.
 *     The \c basic_logger class itself requires methods of the
 *     SharedLockable concept. The threading model policy must also be
 *     default and copy-constructible and support member function \c swap.
 *     There are currently two policies provided: \c single_thread_model
 *     and \c multi_thread_model.
 *
 * The logger implements fundamental facilities of loggers, such as storing
 * source-specific attribute set and formatting log record messages. The basic
 * logger interacts with the logging core in order to apply filtering and
 * pass records to sinks.
 */
template< typename CharT, typename FinalT, typename ThreadingModelT >
class basic_logger :
    public ThreadingModelT
{
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
    typedef basic_core< char_type > core_type;
    //! Threading model type
    typedef ThreadingModelT threading_model;

protected:
    /*!
     * \brief Record pump type
     *
     * This pump is used to format the logging record message text and then
     * push it to the logging core.
     */
    typedef record_pump< final_type > record_pump_type;

private:
    //! A pointer to the logging system
    shared_ptr< core_type > m_pCore;

    //! Logger-specific attribute set
    attribute_set_type m_Attributes;

public:
    /*!
     * Constructor. Initializes internal data structures of the basic logger class,
     * acquires reference to the logging core.
     */
    basic_logger() :
        m_pCore(core_type::get())
    {
    }
    /*!
     * Copy constructor. Copies all attributes from the source logger.
     *
     * \note Not thread-safe. The source logger must be locked in the final class before copying.
     *
     * \param that Source logger
     */
    basic_logger(basic_logger const& that) :
        m_pCore(core_type::get()),
        m_Attributes(that.m_Attributes)
    {
    }
    /*!
     * Constructor with named arguments. The constructor ignores all arguments. The result of
     * construction is equivalent to default construction.
     */
    template< typename ArgsT >
    explicit basic_logger(ArgsT const& args) :
        m_pCore(core_type::get())
    {
    }

    /*!
     * Logging pump getter. The result of this method can be used to format log record message.
     * The message will be pushed to the logging core on the result destruction.
     *
     * \return Logging pump
     */
    record_pump_type strm()
    {
        return strm_unlocked();
    }

    /*!
     * The method adds an attribute to the source-specific attribute set. The attribute will be implicitly added to
     * every log record made with the current logger.
     *
     * \param name The attribute name.
     * \param attr Pointer to the attribute. Must not be NULL.
     * \return A pair of values. If the second member is \c true, then the attribute is added and the first member points to the
     *         attribute. Otherwise the attribute was not added and the first member points to the attribute that prevents
     *         addition.
     */
    std::pair< typename attribute_set_type::iterator, bool > add_attribute(
        string_type const& name, shared_ptr< attribute > const& attr)
    {
#if !defined(BOOST_LOG_NO_THREADS)
        lock_guard< threading_model > _(threading_base());
#endif
        return add_attribute_unlocked(name, attr);
    }
    /*!
     * The method removes an attribute from the thread-specific attribute set.
     *
     * \pre The attribute was added with the add_attribute call for this instance of the logger.
     * \post The attribute is no longer registered as a source-specific attribute for this logger. The iterator is invalidated after removal.
     *
     * \param it Iterator to the previously added attribute.
     */
    void remove_attribute(typename attribute_set_type::iterator it)
    {
#if !defined(BOOST_LOG_NO_THREADS)
        lock_guard< threading_model > _(threading_base());
#endif
        remove_attribute_unlocked(it);
    }

    /*!
     * The method removes all attributes from the logger. All iterators to the removed attributes are invalidated.
     */
    void remove_all_attributes()
    {
#if !defined(BOOST_LOG_NO_THREADS)
        lock_guard< threading_model > _(threading_base());
#endif
        remove_all_attributes_unlocked();
    }

    /*!
     * The method opens a new log record in the logging core.
     *
     * \return \c true if the logging record is opened successfully, \c false otherwise.
     */
    bool open_record()
    {
#if !defined(BOOST_LOG_NO_THREADS)
        log::aux::shared_lock_guard< threading_model > _(threading_base());
#endif
        return open_record_unlocked();
    }
    /*!
     * The method opens a new log record in the logging core.
     *
     * \param args A set of additional named arguments. The parameter is ignored.
     * \return \c true if the logging record is opened successfully, \c false otherwise.
     */
    template< typename ArgsT >
    bool open_record(ArgsT const& args)
    {
#if !defined(BOOST_LOG_NO_THREADS)
        log::aux::shared_lock_guard< threading_model > _(threading_base());
#endif
        return open_record_unlocked(args);
    }
    /*!
     * The method pushes the constructed message to the logging core
     *
     * \param message The formatted log record message
     */
    void push_record(string_type const& message)
    {
        push_record_unlocked(message);
    }
    /*!
     * The method cancels the currently opened record in the logging core
     */
    void cancel_record()
    {
        cancel_record_unlocked();
    }

protected:
    /*!
     * An accessor to the logging system pointer
     */
    shared_ptr< core_type > const& core() const { return m_pCore; }
    /*!
     * An accessor to the logger attributes
     */
    attribute_set_type& attributes() { return m_Attributes; }
    /*!
     * An accessor to the logger attributes
     */
    attribute_set_type const& attributes() const { return m_Attributes; }
    /*!
     * An accessor to the threading model base
     */
    threading_model& threading_base() { return *this; }
    /*!
     * An accessor to the threading model base
     */
    threading_model const& threading_base() const { return *this; }
    /*!
     * An accessor to the final logger
     */
    final_type* final_this()
    {
        BOOST_LOG_ASSUME(this != NULL);
        return static_cast< final_type* >(this);
    }
    /*!
     * An accessor to the final logger
     */
    final_type const* final_this() const
    {
        BOOST_LOG_ASSUME(this != NULL);
        return static_cast< final_type const* >(this);
    }

    /*!
     * Unlocked \c swap
     */
    void swap_unlocked(basic_logger& that)
    {
        threading_base().swap(that.threading_base());
        m_Attributes.swap(that.m_Attributes);
    }

    /*!
     * Unlocked \c strm
     */
    record_pump_type strm_unlocked()
    {
        return record_pump_type(final_this());
    }

    /*!
     * Unlocked \c add_attribute
     */
    std::pair< typename attribute_set_type::iterator, bool > add_attribute_unlocked(
        string_type const& name, shared_ptr< attribute > const& attr)
    {
        return m_Attributes.insert(std::make_pair(name, attr));
    }
    /*!
     * Unlocked \c remove_attribute
     */
    void remove_attribute_unlocked(typename attribute_set_type::iterator it)
    {
        m_Attributes.erase(it);
    }

    /*!
     * Unlocked \c remove_all_attributes
     */
    void remove_all_attributes_unlocked()
    {
        m_Attributes.clear();
    }

    /*!
     * Unlocked \c open_record
     */
    bool open_record_unlocked()
    {
        return m_pCore->open_record(m_Attributes);
    }
    /*!
     * Unlocked \c open_record
     */
    template< typename ArgsT >
    bool open_record_unlocked(ArgsT const& args)
    {
        return m_pCore->open_record(m_Attributes);
    }
    /*!
     * Unlocked \c push_record
     */
    void push_record_unlocked(string_type const& message)
    {
        m_pCore->push_record(message);
    }
    /*!
     * Unlocked \c cancel_record
     */
    void cancel_record_unlocked()
    {
        m_pCore->cancel_record();
    }

private:
    //! Assignment (should be implemented through copy and swap in the final class)
    basic_logger& operator= (basic_logger const&);
};

/*!
 * Free-standing swap for all loggers
 */
template< typename CharT, typename FinalT, typename ThreadingModelT >
inline void swap(
    basic_logger< CharT, FinalT, ThreadingModelT >& left,
    basic_logger< CharT, FinalT, ThreadingModelT >& right)
{
    static_cast< FinalT& >(left).swap(static_cast< FinalT& >(right));
}

namespace aux {

/*!
 * \brief A helper metafunction that is used to inherit all logger features into the final logger
 */
struct inherit_logger_features
{
    template< typename PrevT, typename T >
    struct apply
    {
        typedef typename mpl::lambda< T >::type::BOOST_NESTED_TEMPLATE apply< PrevT >::type type;
    };
};

} // namespace aux

//! \cond

#define BOOST_LOG_CTOR_FORWARD_INTERNAL(z, n, data)\
    template< BOOST_PP_ENUM_PARAMS(n, typename T) >\
    explicit data(BOOST_PP_ENUM_BINARY_PARAMS(n, T, const& arg)) :\
        base_type((BOOST_PP_ENUM_PARAMS(n, arg))) {}


#define BOOST_LOG_CTOR_FORWARD(z, n, class_type)\
    template< BOOST_PP_ENUM_PARAMS(n, typename T) >\
    explicit class_type(BOOST_PP_ENUM_BINARY_PARAMS(n, T, const& arg)) :\
        class_type::logger_base((BOOST_PP_ENUM_PARAMS(n, arg))) {}

#define BOOST_LOG_FORWARD_LOGGER_PARAMETRIZED_CONSTRUCTORS_IMPL(class_type, typename_keyword)\
    public:\
        BOOST_PP_REPEAT_FROM_TO(1, BOOST_LOG_MAX_CTOR_FORWARD_ARGS, BOOST_LOG_CTOR_FORWARD, class_type)

#define BOOST_LOG_FORWARD_LOGGER_CONSTRUCTORS_IMPL(class_type, typename_keyword)\
    public:\
        class_type() {}\
        class_type(class_type const& that) : class_type::logger_base(\
            static_cast< typename_keyword() class_type::logger_base const& >(that)) {}\
        BOOST_LOG_FORWARD_LOGGER_PARAMETRIZED_CONSTRUCTORS_IMPL(class_type, typename_keyword)

//! \endcond

#define BOOST_LOG_FORWARD_LOGGER_PARAMETRIZED_CONSTRUCTORS(class_type)\
    BOOST_LOG_FORWARD_LOGGER_PARAMETRIZED_CONSTRUCTORS_IMPL(class_type, BOOST_PP_EMPTY)

#define BOOST_LOG_FORWARD_LOGGER_PARAMETRIZED_CONSTRUCTORS_TEMPLATE(class_type)\
    BOOST_LOG_FORWARD_LOGGER_PARAMETRIZED_CONSTRUCTORS_IMPL(class_type, BOOST_PP_IDENTITY(typename))

#define BOOST_LOG_FORWARD_LOGGER_CONSTRUCTORS(class_type)\
    BOOST_LOG_FORWARD_LOGGER_CONSTRUCTORS_IMPL(class_type, BOOST_PP_EMPTY)

#define BOOST_LOG_FORWARD_LOGGER_CONSTRUCTORS_TEMPLATE(class_type)\
    BOOST_LOG_FORWARD_LOGGER_CONSTRUCTORS_IMPL(class_type, BOOST_PP_IDENTITY(typename))

/*!
 * \brief A composite logger that inherits a number of features
 *
 * The composite logger is a helper class that simplifies feature composition into a final logger.
 * The user's logger class is expected to derive from the composite logger class, instantiated with
 * the character type, the user's logger class, threading model and the list of the required features.
 * The former three parameters are passed to the \c basic_logger class template. The feature list
 * must be an MPL type sequence, where each element is an unary MPL metafunction class, that upon
 * applying on its argument derives from it. Every logger feature provided by the library can
 * participate in the feature list if its corresponding template parameter for the base class is
 * set to the \c mpl::_1 placeholder.
 */
template< typename CharT, typename FinalT, typename ThreadingModelT, typename FeaturesT >
class basic_composite_logger :
    public mpl::reverse_fold<
        FeaturesT,
        basic_logger< CharT, FinalT, ThreadingModelT >,
        aux::inherit_logger_features
    >::type
{
    //! Base type (the hierarchy of features)
    typedef typename mpl::reverse_fold<
        FeaturesT,
        basic_logger< CharT, FinalT, ThreadingModelT >,
        aux::inherit_logger_features
    >::type base_type;

protected:
    //! The composite logger type (for use in the user's logger class)
    typedef basic_composite_logger logger_base;

public:
    //! Threading model being used
    typedef typename base_type::threading_model threading_model;

#if !defined(BOOST_LOG_NO_THREADS)

public:
    /*!
     * Default constructor (default-constructs all features)
     */
    basic_composite_logger() {}
    /*!
     * Copy constructor
     */
    basic_composite_logger(basic_composite_logger const& that) :
        base_type((
            log::aux::shared_lock_guard< const threading_model >(that.threading_base()),
            static_cast< base_type const& >(that)
        ))
    {
    }
    //  Parametrized constructors that pass all named arguments to the features
    BOOST_PP_REPEAT_FROM_TO(1, BOOST_LOG_MAX_CTOR_FORWARD_ARGS, BOOST_LOG_CTOR_FORWARD_INTERNAL, basic_composite_logger)

    /*!
     * Assignment for the final class. Threadsafe, provides strong exception guarantee.
     */
    FinalT& operator= (FinalT const& that)
    {
        if (this != boost::addressof(that))
        {
            // We'll have to explicitly create the copy in order to make sure it's unlocked when we attempt to lock *this
            FinalT tmp(that);
            lock_guard< threading_model > _(this->threading_base());
            this->swap_unlocked(tmp);
        }
        return static_cast< FinalT& >(*this);
    }
    /*!
     * Thread-safe implementation of swap
     */
    void swap(basic_composite_logger& that)
    {
        log::aux::multiple_unique_lock2<
            threading_model,
            threading_model
        > _(this->threading_base(), that.threading_base());
        this->swap_unlocked(that);
    }
};

//! An optimized composite logger version with no multithreading support
template< typename CharT, typename FinalT, typename FeaturesT >
class basic_composite_logger< CharT, FinalT, single_thread_model, FeaturesT > :
    public mpl::reverse_fold<
        FeaturesT,
        basic_logger< CharT, FinalT, single_thread_model >,
        aux::inherit_logger_features
    >::type
{
    typedef typename mpl::reverse_fold<
        FeaturesT,
        basic_logger< CharT, FinalT, single_thread_model >,
        aux::inherit_logger_features
    >::type base_type;

protected:
    typedef basic_composite_logger logger_base;

public:
    typedef typename base_type::threading_model threading_model;

#endif // !defined(BOOST_LOG_NO_THREADS)

public:
    basic_composite_logger() {}
    basic_composite_logger(basic_composite_logger const& that) :
        base_type(static_cast< base_type const& >(that))
    {
    }
    BOOST_PP_REPEAT_FROM_TO(1, BOOST_LOG_MAX_CTOR_FORWARD_ARGS, BOOST_LOG_CTOR_FORWARD_INTERNAL, basic_composite_logger)

    FinalT& operator= (FinalT that)
    {
        this->swap_unlocked(that);
        return static_cast< FinalT& >(*this);
    }
    void swap(basic_composite_logger& that)
    {
        this->swap_unlocked(that);
    }
};

#undef BOOST_LOG_CTOR_FORWARD_INTERNAL

#ifdef BOOST_LOG_USE_CHAR

/*!
 * \brief Narrow-char logger. Functionally equivalent to \c basic_logger.
 *
 * See \c basic_logger class template for a more detailed description.
 */
class logger :
    public basic_composite_logger< char, logger, single_thread_model, mpl::vector0< > >
{
    BOOST_LOG_FORWARD_LOGGER_CONSTRUCTORS(logger)
};

#if !defined(BOOST_LOG_NO_THREADS)

/*!
 * \brief Narrow-char thread-safe logger. Functionally equivalent to \c basic_logger.
 *
 * See \c basic_logger class template for a more detailed description.
 */
class logger_mt :
    public basic_composite_logger< char, logger_mt, multi_thread_model, mpl::vector0< > >
{
    BOOST_LOG_FORWARD_LOGGER_CONSTRUCTORS(logger_mt)
};

#endif // !defined(BOOST_LOG_NO_THREADS)
#endif // BOOST_LOG_USE_CHAR

#ifdef BOOST_LOG_USE_WCHAR_T

/*!
 * \brief Wide-char logger. Functionally equivalent to \c basic_logger.
 *
 * See \c basic_logger class template for a more detailed description.
 */
class wlogger :
    public basic_composite_logger< wchar_t, wlogger, single_thread_model, mpl::vector0< > >
{
    BOOST_LOG_FORWARD_LOGGER_CONSTRUCTORS(wlogger)
};

#if !defined(BOOST_LOG_NO_THREADS)

/*!
 * \brief Wide-char thread-safe logger. Functionally equivalent to \c basic_logger.
 *
 * See \c basic_logger class template for a more detailed description.
 */
class wlogger_mt :
    public basic_composite_logger< wchar_t, wlogger_mt, multi_thread_model, mpl::vector0< > >
{
    BOOST_LOG_FORWARD_LOGGER_CONSTRUCTORS(wlogger_mt)
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

//! The macro writes a record to the log and allows to pass additional named arguments to the logger
#define BOOST_LOG_WITH_PARAMS(logger, params_seq)\
    if (!(logger).open_record((BOOST_PP_SEQ_ENUM(params_seq))))\
        ((void)0);\
    else\
        (logger).strm()


//! \cond

#define BOOST_LOG_DECLARE_LOGGER_TYPE_INTERNAL(r, data, i, elem) BOOST_PP_COMMA_IF(i) elem< ::boost::mpl::_1 >

//! \endcond

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
        public ::boost::log::sources::basic_composite_logger<\
            char_type,\
            type_name,\
            threading,\
            ::boost::mpl::vector<\
                BOOST_PP_SEQ_FOR_EACH_I(BOOST_LOG_DECLARE_LOGGER_TYPE_INTERNAL, ~, base_seq)\
            >::type\
        >\
    {\
        BOOST_LOG_FORWARD_LOGGER_CONSTRUCTORS(type_name)\
    }



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
