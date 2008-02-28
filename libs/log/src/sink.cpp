/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   sink.cpp
 * \author Andrey Semashev
 * \date   03.11.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#include <list>
#include <boost/none.hpp>
#include <boost/optional.hpp>
#include <boost/utility/in_place_factory.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/log/sinks/sink.hpp>

namespace boost {

namespace log {

namespace sinks {

namespace aux {

//! Sink implementation data
template< typename CharT >
class asynchronous_sink_impl< CharT >::implementation
{
private:
    //! Synchronization mutex type
    typedef mutex mutex_type;
    //! Lock type
    typedef unique_lock< mutex_type > scoped_lock;

    //! Character type
    typedef CharT char_type;
    //! String type to be used as a message text holder
    typedef std::basic_string< char_type > string_type;
    //! Attribute values view type
    typedef basic_attribute_values_view< char_type > attribute_values_view;

    //! Enqueued record aggregate
    struct enqueued_record;
    friend struct enqueued_record;
    struct enqueued_record
    {
        //! Log record attributes
        attribute_values_view Attributes;
        //! Log message
        string_type Message;

        enqueued_record(attribute_values_view const& a, string_type const& m)
            : Attributes(a), Message(m)
        {
        }
    };

    //! Pending records queue
    typedef std::list< enqueued_record > enqueued_records;

    //! A simple functor to start internal thread and not involve Boost.Bind
    struct thread_starter;
    friend struct thread_starter;
    struct thread_starter
    {
        explicit thread_starter(implementation* pThis) : m_pThis(pThis) {}
        void operator() () const { m_pThis->output_thread(); }

    private:
        implementation* m_pThis;
    };

private:
    //! The flag shows that the output thread should finish
    volatile bool m_Finishing;
    //! Opaque pointer to the sink backend
    void* m_pBackend;
    //! Pointer to the write_message callback
    write_message_callback_t m_WriteMessageCallback;

    //! Records queue waiting to be output
    enqueued_records m_EnqueuedRecords;

    //! Synchronization mutex
    mutex_type m_Mutex;
    //! Sleep condition
    condition_variable m_Condition;

    //! Mutex to support locked_backend_ptr
    shared_backend_lock::mutex_type m_SharedBackendMutex;
    //! Shared lock to support locked_backend_ptr
    optional< shared_backend_lock > m_SharedBackendLock;

    //! Record output thread
    optional< thread > m_Thread;

public:
    //! Constructor
    implementation(void* p, write_message_callback_t wmc) :
        m_Finishing(false),
        m_pBackend(p),
        m_WriteMessageCallback(wmc)
    {
        m_Thread = boost::in_place(thread_starter(this));
    }
    //! Destructor
    ~implementation()
    {
        if (!!m_Thread)
        {
            {
                scoped_lock lock(m_Mutex);
                m_Finishing = true;
            }
            m_Condition.notify_one();
            m_Thread->join();
            m_Thread = none;
        }
    }

    //! The method puts the record into the queue
    void enqueue_message(attribute_values_view const& attributes, string_type const& message)
    {
        // Make sure that no references to the thread-specific data is left in attribute values
        for (typename attribute_values_view::const_iterator it = attributes.begin(), end = attributes.end(); it != end; ++it)
        {
            // Yep, a bit hackish. I'll need a better backdoor to do it gacefully.
            it->second->detach_from_thread().swap(
                    const_cast< typename attribute_values_view::mapped_type& >(it->second));
        }

        // Put the record into the queue
        {
            scoped_lock _(m_Mutex);
            m_EnqueuedRecords.push_back(enqueued_record(attributes, message));
        }

        m_Condition.notify_one();
    }

    //! Accessor to the shared lock for the locked_backend_ptr support
    optional< shared_backend_lock >& get_shared_backend_lock()
    {
        shared_backend_lock::scoped_lock lock(m_SharedBackendMutex);
        if (!m_SharedBackendLock)
            m_SharedBackendLock = boost::in_place(boost::ref(lock));
        return m_SharedBackendLock;
    }

private:
    //! Output thread routine
    void output_thread()
    {
        enqueued_records ExtractedRecords;

        while (!m_Finishing)
        {
            {
                scoped_lock lock(m_Mutex);
                if (m_EnqueuedRecords.empty())
                    m_Condition.wait(lock);

                ExtractedRecords.splice(ExtractedRecords.end(), m_EnqueuedRecords);
            }

            while (!ExtractedRecords.empty())
            {
                try
                {
                    enqueued_record const& rec = ExtractedRecords.front();
                    shared_backend_lock::scoped_lock lock(m_SharedBackendMutex);
                    m_WriteMessageCallback(m_pBackend, rec.Attributes, rec.Message);
                }
                catch (...)
                {
                    // We do nothing here. There's nothing we can do, actually.
                }

                ExtractedRecords.pop_front();
            }
        }
    }
};

//! Constructor
template< typename CharT >
asynchronous_sink_impl< CharT >::asynchronous_sink_impl(void* pBackend, write_message_callback_t wmc)
    : pImpl(new implementation(pBackend, wmc))
{
}

//! Destructor
template< typename CharT >
asynchronous_sink_impl< CharT >::~asynchronous_sink_impl()
{
    delete pImpl;
}

//! The method puts the record into the queue
template< typename CharT >
void asynchronous_sink_impl< CharT >::enqueue_message(attribute_values_view const& attributes, string_type const& message)
{
    pImpl->enqueue_message(attributes, message);
}

//! Accessor to the shared lock for the locked_backend_ptr support
template< typename CharT >
optional< shared_backend_lock >& asynchronous_sink_impl< CharT >::get_shared_backend_lock() const
{
    return pImpl->get_shared_backend_lock();
}

template class BOOST_LOG_EXPORT asynchronous_sink_impl< char >;
template class BOOST_LOG_EXPORT asynchronous_sink_impl< wchar_t >;

} // namespace aux

} // namespace sinks

} // namespace log

} // namespace boost
