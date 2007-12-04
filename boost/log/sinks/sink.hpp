/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   sink.hpp
 * \author Andrey Semashev
 * \date   22.04.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_SINKS_SINK_HPP_INCLUDED_
#define BOOST_LOG_SINKS_SINK_HPP_INCLUDED_

#include <cassert>
#include <string>
#include <boost/ref.hpp>
#include <boost/none.hpp>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/static_assert.hpp>
#include <boost/mpl/or.hpp>
#include <boost/function/function1.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/utility/in_place_factory.hpp>
#include <boost/detail/atomic_count.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/sinks/threading_models.hpp>
#include <boost/log/attributes/attribute_values_view.hpp>

#ifdef _MSC_VER
#pragma warning(push)
// 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
// non dll-interface class 'A' used as base for dll-interface class 'B'
#pragma warning(disable: 4275)
#endif // _MSC_VER

namespace boost {

namespace log {

namespace sinks {

//! A base class for a logging sink facade
template< typename CharT >
class BOOST_LOG_NO_VTABLE sink : noncopyable
{
public:
    //! Character type
    typedef CharT char_type;
    //! String type to be used as a message text holder
    typedef std::basic_string< char_type > string_type;
    //! Attribute values view type
    typedef basic_attribute_values_view< char_type > attribute_values_view;
    //! Filter function type
    typedef function1< bool, attribute_values_view const& > filter_type;

private:
    //! Mutex type
    typedef shared_mutex filter_mutex_type;
    //! Read lock type
    typedef shared_lock< filter_mutex_type > scoped_read_lock;
    //! Write lock type
    typedef unique_lock< filter_mutex_type > scoped_write_lock;

private:
    //! Synchronization mutex
    filter_mutex_type m_FilterMutex;
    //! Filter
    filter_type m_Filter;

public:
    virtual ~sink() {}

    //! The method sets the sink-specific filter
    template< typename T >
    void set_filter(T const& filter)
    {
        scoped_write_lock _(m_FilterMutex);
        m_Filter = filter;
    }
    //! The method removes the sink-specific filter
    void reset_filter()
    {
        scoped_write_lock _(m_FilterMutex);
        m_Filter.clear();
    }

    //! The method returns true if the attribute values pass the filter
    bool will_write_message(attribute_values_view const& attributes)
    {
        scoped_read_lock _(m_FilterMutex);
        return (m_Filter.empty() || m_Filter(attributes));
    }

    //! The method writes the message to the sink
    virtual void write_message(attribute_values_view const& attributes, string_type const& message) = 0;
};

//! Non-blocking logging sink facade
template< typename SinkBackendT >
class unlocked_sink :
    public sink< typename SinkBackendT::char_type >
{
    typedef sink< typename SinkBackendT::char_type > base_type;

public:
    //! Sink implementation type
    typedef SinkBackendT sink_backend_type;
    BOOST_STATIC_ASSERT((is_model_supported< typename sink_backend_type::threading_model, backend_synchronization_tag >::value));

    typedef typename base_type::attribute_values_view attribute_values_view;
    typedef typename base_type::string_type string_type;

    //! A pointer type that locks the backend until it's destroyed (does not lock for this sink frontend!)
    typedef shared_ptr< sink_backend_type > locked_backend_ptr;

private:
    //! Pointer to the sink backend implementation
    shared_ptr< sink_backend_type > m_pBackend;

public:
    //! Default constructor - requires the backend to be default-constructible
    unlocked_sink() : m_pBackend(new sink_backend_type()) {}
    //! Constructor with ability to attach user-constructed backend
    explicit unlocked_sink(shared_ptr< sink_backend_type > const& backend) : m_pBackend(backend)
    {
        assert(!!m_pBackend);
    }

    //! The method writes the message to the sink
    void write_message(attribute_values_view const& attributes, string_type const& message)
    {
        m_pBackend->write_message(attributes, message);
    }
    //! Locking accessor to the attached backend
    locked_backend_ptr locked_backend() const { return m_pBackend; }
};

namespace aux {

    //! Shared lock object to support locking_ptr
    struct shared_backend_lock
    {
        typedef recursive_mutex mutex_type;
        typedef unique_lock< mutex_type > scoped_lock;

        boost::detail::atomic_count m_RefCounter;
        scoped_lock m_Lock;

        shared_backend_lock(scoped_lock& l) : m_RefCounter(0), m_Lock(*l.mutex(), boost::defer_lock_t())
        {
            m_Lock.swap(l);
        }
    };

    //! A pointer type that locks the backend until it's destroyed
    template< typename SinkBackendT >
    class locking_ptr
    {
    public:
        //! Pointed type
        typedef SinkBackendT element_type;

    private:
        //! The pointer to the backend
        shared_ptr< element_type > m_pBackend;
        //! Reference to the shared lock of the backend
        optional< shared_backend_lock >& m_Lock;

    public:
        //! Constructor
        locking_ptr(shared_ptr< SinkBackendT > const& p, optional< shared_backend_lock >& l)
            : m_pBackend(p), m_Lock(l)
        {
            ++m_Lock->m_RefCounter;
        }
        //! Copy constructor
        locking_ptr(locking_ptr const& that) : m_pBackend(that.m_pBackend), m_Lock(that.m_Lock)
        {
            ++m_Lock->m_RefCounter;
        }
        //! Destructor
        ~locking_ptr()
        {
            if (--m_Lock->m_RefCounter == 0)
                m_Lock = none;
        }

        //! Indirection
        element_type* operator-> () const { return m_pBackend.get(); }
        //! Dereferencing
        element_type& operator* () const { return *m_pBackend; }

        //! Accessor to the raw pointer
        element_type* get() const { return m_pBackend.get(); }

    private:
        //! Assignment (closed)
        locking_ptr& operator= (locking_ptr const&);
    };

    //! Free raw pointer getter to assist generic programming
    template< typename SinkBackendT >
    inline SinkBackendT* get_pointer(locking_ptr< SinkBackendT > const& p)
    {
        return p.get();
    }

} // namespace aux

//! Synchronous logging sink facade
template< typename SinkBackendT >
class synchronous_sink :
    public sink< typename SinkBackendT::char_type >
{
    typedef sink< typename SinkBackendT::char_type > base_type;

    //! Mutex type
    typedef aux::shared_backend_lock::mutex_type mutex_type;
    //! Lock type
    typedef aux::shared_backend_lock::scoped_lock scoped_lock;

public:
    //! Sink implementation type
    typedef SinkBackendT sink_backend_type;
    BOOST_STATIC_ASSERT((is_model_supported< typename sink_backend_type::threading_model, frontend_synchronization_tag >::value));

    typedef typename base_type::attribute_values_view attribute_values_view;
    typedef typename base_type::string_type string_type;

    //! A pointer type that locks the backend until it's destroyed
    typedef aux::locking_ptr< sink_backend_type > locked_backend_ptr;

private:
    //! Synchronization mutex
    mutable mutex_type m_Mutex;
    //! Pointer to the sink backend implementation
    shared_ptr< sink_backend_type > m_pBackend;

    //! A temporary storage to allow locked_backend_ptr to work
    mutable optional< aux::shared_backend_lock > m_SharedBackendLock;

public:
    //! Default constructor - requires the backend to be default-constructible
    synchronous_sink() : m_pBackend(new sink_backend_type()) {}
    //! Constructor with ability to attach user-constructed backend
    explicit synchronous_sink(shared_ptr< sink_backend_type > const& backend) : m_pBackend(backend)
    {
        assert(!!m_pBackend);
    }

    //! The method writes the message to the sink
    void write_message(attribute_values_view const& attributes, string_type const& message)
    {
        scoped_lock _(m_Mutex);
        m_pBackend->write_message(attributes, message);
    }
    //! Locking accessor to the attached backend
    locked_backend_ptr locked_backend() const
    {
        scoped_lock lock(m_Mutex);
        if (!m_SharedBackendLock)
            m_SharedBackendLock = boost::in_place(boost::ref(lock));
        return locked_backend_ptr(m_pBackend, m_SharedBackendLock);
    }
};

namespace aux {

    //! Asynchronous logging sink implementation
    template< typename CharT >
    class BOOST_LOG_EXPORT asynchronous_sink_impl
    {
    public:
        //! Character type
        typedef CharT char_type;
        //! String type to be used as a message text holder
        typedef std::basic_string< char_type > string_type;
        //! Attribute values view type
        typedef basic_attribute_values_view< char_type > attribute_values_view;

        //! Callback type to call write_message in the backend
        typedef void (*write_message_callback_t)(void*, attribute_values_view const&, string_type const&); 

    private:
        class implementation;
        implementation* pImpl;
        
    public:
        asynchronous_sink_impl(void* pBackend, write_message_callback_t wmc);
        ~asynchronous_sink_impl();

        //! The method puts the record into the queue
        void enqueue_message(attribute_values_view const& attributes, string_type const& message);

        //! Accessor to the shared lock for the locked_backend_ptr support
        optional< shared_backend_lock >& get_shared_backend_lock() const;

    private:
        asynchronous_sink_impl(asynchronous_sink_impl const&);
        asynchronous_sink_impl& operator= (asynchronous_sink_impl const&);
    };

} // namespace aux

//! Asynchronous logging sink facade
template< typename SinkBackendT >
class asynchronous_sink :
    public sink< typename SinkBackendT::char_type >
{
    typedef sink< typename SinkBackendT::char_type > base_type;

public:
    //! Sink implementation type
    typedef SinkBackendT sink_backend_type;
    BOOST_STATIC_ASSERT((mpl::or_<
        is_model_supported< typename sink_backend_type::threading_model, single_thread_tag >,
        is_model_supported< typename sink_backend_type::threading_model, frontend_synchronization_tag >
    >::value));

    typedef typename base_type::char_type char_type;
    typedef typename base_type::attribute_values_view attribute_values_view;
    typedef typename base_type::string_type string_type;

    //! A pointer type that locks the backend until it's destroyed
    typedef aux::locking_ptr< sink_backend_type > locked_backend_ptr;

private:
    //! Pointer to the sink backend implementation
    shared_ptr< sink_backend_type > m_pBackend;
    //! Synchronization mutex
    aux::asynchronous_sink_impl< char_type > m_Impl;

public:
    //! Default constructor - requires the backend to be default-constructible
    asynchronous_sink() :
        m_pBackend(new sink_backend_type()),
        m_Impl(m_pBackend.get(), &asynchronous_sink::write_message_trampoline)
    {
    }
    //! Constructor with ability to attach user-constructed backend
    explicit asynchronous_sink(shared_ptr< sink_backend_type > const& backend) :
        m_pBackend(backend),
        m_Impl(m_pBackend.get(), &asynchronous_sink::write_message_trampoline)
    {
        assert(!!backend);
    }

    //! The method writes the message to the sink
    void write_message(attribute_values_view const& attributes, string_type const& message)
    {
        m_Impl.enqueue_message(attributes, message);
    }
    //! Locking accessor to the attached backend
    locked_backend_ptr locked_backend() const
    {
        return locked_backend_ptr(m_pBackend, m_Impl.get_shared_backend_lock());
    }

private:
    //! Trampoline function to invoke the backend
    static void write_message_trampoline(void* pBackend, attribute_values_view const& attributes, string_type const& message)
    {
        sink_backend_type* p = reinterpret_cast< sink_backend_type* >(pBackend);
        p->write_message(attributes, message);
    }
};

} // namespace sinks

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_SINKS_SINK_HPP_INCLUDED_
