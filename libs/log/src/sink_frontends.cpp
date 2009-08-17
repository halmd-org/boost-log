/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * \file   sink_frontends.cpp
 * \author Andrey Semashev
 * \date   03.11.2007
 *
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#include <boost/assert.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/log/sinks/basic_sink_frontend.hpp>
#include <boost/log/sinks/unlocked_frontend.hpp>

#if !defined(BOOST_LOG_NO_THREADS)
#include <utility>
#include <stdexcept>
#include <boost/ref.hpp>
#include <boost/bind.hpp>
#include <boost/none.hpp>
#include <boost/optional.hpp>
#include <boost/checked_delete.hpp>
#include <boost/intrusive/options.hpp>
#include <boost/intrusive/list.hpp>
#include <boost/intrusive/trivial_value_traits.hpp>
#include <boost/utility/in_place_factory.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/exceptions.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/log/detail/light_rw_mutex.hpp>
#include <boost/log/detail/shared_lock_guard.hpp>
#include <boost/log/detail/throw_exception.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/async_frontend.hpp>
#include "atomic_queue.hpp"
#endif // !defined(BOOST_LOG_NO_THREADS)

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace sinks {

/////////////////////////////////////////////////////////////////////
//  Basic sink frontend implementation
/////////////////////////////////////////////////////////////////////
//! Implementation data
template< typename CharT >
struct basic_sink_frontend< CharT >::implementation
{
#if !defined(BOOST_LOG_NO_THREADS)
    //! Mutex type
    typedef boost::log::aux::light_rw_mutex mutex_type;
    //! Read lock type
    typedef boost::log::aux::shared_lock_guard< mutex_type > scoped_read_lock;
    //! Write lock type
    typedef lock_guard< mutex_type > scoped_write_lock;

    //! Synchronization mutex
    mutex_type m_Mutex;
#endif
    //! Filter
    filter_type m_Filter;
    //! Exception handler
    exception_handler_type m_ExceptionHandler;

    virtual ~implementation() {}
};

//! The constructor installs the pointer to the frontend implementation
template< typename CharT >
basic_sink_frontend< CharT >::basic_sink_frontend(implementation* p) : m_pImpl(p)
{
}

//! Destructor
template< typename CharT >
basic_sink_frontend< CharT >::~basic_sink_frontend()
{
    delete m_pImpl;
}

//! The method sets sink-specific filter functional object
template< typename CharT >
void basic_sink_frontend< CharT >::set_filter(filter_type const& filter)
{
#if !defined(BOOST_LOG_NO_THREADS)
    typename implementation::scoped_write_lock _(m_pImpl->m_Mutex);
#endif
    m_pImpl->m_Filter = filter;
}

//! The method resets the filter
template< typename CharT >
void basic_sink_frontend< CharT >::reset_filter()
{
#if !defined(BOOST_LOG_NO_THREADS)
    typename implementation::scoped_write_lock _(m_pImpl->m_Mutex);
#endif
    m_pImpl->m_Filter.clear();
}

//! The method sets an exception handler function
template< typename CharT >
void basic_sink_frontend< CharT >::set_exception_handler(exception_handler_type const& handler)
{
#if !defined(BOOST_LOG_NO_THREADS)
    typename implementation::scoped_write_lock _(m_pImpl->m_Mutex);
#endif
    m_pImpl->m_ExceptionHandler = handler;
}

//! The method returns \c true if no filter is set or the attribute values pass the filter
template< typename CharT >
bool basic_sink_frontend< CharT >::will_consume(values_view_type const& attributes)
{
#if !defined(BOOST_LOG_NO_THREADS)
    typename implementation::scoped_read_lock _(m_pImpl->m_Mutex);
#endif
    try
    {
        return (m_pImpl->m_Filter.empty() || m_pImpl->m_Filter(attributes));
    }
#if !defined(BOOST_LOG_NO_THREADS)
    catch (thread_interrupted&)
    {
        throw;
    }
#endif
    catch (...)
    {
        if (m_pImpl->m_ExceptionHandler.empty())
            throw;
        m_pImpl->m_ExceptionHandler();
        return false;
    }
}

#ifdef BOOST_LOG_USE_CHAR
template class basic_sink_frontend< char >;
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
template class basic_sink_frontend< wchar_t >;
#endif


namespace aux {


/////////////////////////////////////////////////////////////////////
//  Unlocked sink frontend implementation
/////////////////////////////////////////////////////////////////////
//! Implementation data
template< typename CharT >
struct unlocked_frontend< CharT >::implementation :
    public basic_sink_frontend< CharT >::implementation
{
    //! Pointer to the backend
    shared_ptr< void > m_pBackend;
    //! Pointer to the consume trampoline function
    consume_trampoline_t m_Consume;

    implementation(shared_ptr< void > const& backend, consume_trampoline_t consume_tramp) :
        m_pBackend(backend),
        m_Consume(consume_tramp)
    {
    }
};

template< typename CharT >
unlocked_frontend< CharT >::unlocked_frontend(shared_ptr< void > const& backend, consume_trampoline_t consume_tramp) :
    base_type(new implementation(backend, consume_tramp))
{
    BOOST_ASSERT(!!backend);
}

template< typename CharT >
shared_ptr< void > const& unlocked_frontend< CharT >::get_backend() const
{
    return this->BOOST_NESTED_TEMPLATE get_impl< implementation >()->m_pBackend;
}

template< typename CharT >
void unlocked_frontend< CharT >::consume(record_type const& record)
{
    register implementation* pImpl = this->BOOST_NESTED_TEMPLATE get_impl< implementation >();
    try
    {
        (pImpl->m_Consume)(pImpl->m_pBackend.get(), record);
    }
#if !defined(BOOST_LOG_NO_THREADS)
    catch (thread_interrupted&)
    {
        throw;
    }
#endif // !defined(BOOST_LOG_NO_THREADS)
    catch (...)
    {
#if !defined(BOOST_LOG_NO_THREADS)
        typename implementation::scoped_read_lock _(pImpl->m_Mutex);
#endif
        if (pImpl->m_ExceptionHandler.empty())
            throw;
        pImpl->m_ExceptionHandler();
    }
}

#ifdef BOOST_LOG_USE_CHAR
template class unlocked_frontend< char >;
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
template class unlocked_frontend< wchar_t >;
#endif


#if !defined(BOOST_LOG_NO_THREADS)

/////////////////////////////////////////////////////////////////////
//  Synchronous sink frontend implementation
/////////////////////////////////////////////////////////////////////
//! Implementation data
template< typename CharT >
struct synchronous_frontend< CharT >::implementation :
    public basic_sink_frontend< CharT >::implementation,
    public boost::log::aux::locking_ptr_counter_base
{
    //! Synchronization mutex
    mutex m_BackendMutex;
    //! Pointer to the backend
    shared_ptr< void > m_pBackend;
    //! Pointer to the consume trampoline function
    consume_trampoline_t m_Consume;

    implementation(shared_ptr< void > const& backend, consume_trampoline_t consume_tramp) :
        m_pBackend(backend),
        m_Consume(consume_tramp)
    {
    }

    // locking_ptr_counter_base methods
    void lock() { m_BackendMutex.lock(); }
    bool try_lock() { return m_BackendMutex.try_lock(); }
    void unlock() { m_BackendMutex.unlock(); }
};

template< typename CharT >
synchronous_frontend< CharT >::synchronous_frontend(shared_ptr< void > const& backend, consume_trampoline_t consume_tramp) :
    base_type(new implementation(backend, consume_tramp))
{
    BOOST_ASSERT(!!backend);
}

template< typename CharT >
shared_ptr< void > const& synchronous_frontend< CharT >::get_backend() const
{
    return this->BOOST_NESTED_TEMPLATE get_impl< implementation >()->m_pBackend;
}

template< typename CharT >
boost::log::aux::locking_ptr_counter_base& synchronous_frontend< CharT >::get_backend_locker() const
{
    return *this->BOOST_NESTED_TEMPLATE get_impl< implementation >();
}

template< typename CharT >
void synchronous_frontend< CharT >::consume(record_type const& record)
{
    register implementation* pImpl = this->BOOST_NESTED_TEMPLATE get_impl< implementation >();
    try
    {
        lock_guard< mutex > _(pImpl->m_BackendMutex);
        (pImpl->m_Consume)(pImpl->m_pBackend.get(), record);
    }
    catch (thread_interrupted&)
    {
        throw;
    }
    catch (...)
    {
        typename implementation::scoped_read_lock _(pImpl->m_Mutex);
        if (pImpl->m_ExceptionHandler.empty())
            throw;
        pImpl->m_ExceptionHandler();
    }
}

template< typename CharT >
bool synchronous_frontend< CharT >::try_consume(record_type const& record)
{
    register implementation* pImpl = this->BOOST_NESTED_TEMPLATE get_impl< implementation >();
    try
    {
        unique_lock< mutex > lock(pImpl->m_BackendMutex, try_to_lock);
        if (lock.owns_lock())
        {
            (pImpl->m_Consume)(pImpl->m_pBackend.get(), record);
            return true;
        }
        else
            return false;
    }
    catch (thread_interrupted&)
    {
        throw;
    }
    catch (...)
    {
        typename implementation::scoped_read_lock _(pImpl->m_Mutex);
        if (pImpl->m_ExceptionHandler.empty())
            throw;
        pImpl->m_ExceptionHandler();
        return false;
    }
}

#ifdef BOOST_LOG_USE_CHAR
template class synchronous_frontend< char >;
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
template class synchronous_frontend< wchar_t >;
#endif


/////////////////////////////////////////////////////////////////////
//  Asynchronous sink frontend implementation
/////////////////////////////////////////////////////////////////////

namespace {

    //! A simple scope guard that automatically clears processing thread id
    struct thread_id_cleanup
    {
        explicit thread_id_cleanup(optional< thread::id >& tid, mutex& mtx, condition_variable* cond = NULL) :
            m_ThreadID(tid),
            m_Mutex(mtx),
            m_pCond(cond),
            m_Active(false)
        {
        }
        ~thread_id_cleanup()
        {
            if (m_Active)
            {
                lock_guard< mutex > _(m_Mutex);
                m_ThreadID = none;
                if (m_pCond)
                    m_pCond->notify_all();
            }
        }
        void activate()
        {
            m_Active = true;
        }

    private:
        optional< thread::id >& m_ThreadID;
        mutex& m_Mutex;
        condition_variable* m_pCond;
        bool m_Active;
    };

} // namespace

//! Implementation data
template< typename CharT >
struct asynchronous_frontend< CharT >::implementation :
    public basic_sink_frontend< CharT >::implementation,
    public boost::log::aux::locking_ptr_counter_base
{
public:
    //! Base type
    typedef typename basic_sink_frontend< CharT >::implementation base_type;
    //! Pending records queue
    typedef boost::log::aux::atomic_queue< record_type > enqueued_records;

    //! Node traits for putting enqueued records into a Boost.Intrusive list
    struct enqueued_records_node_traits
    {
       typedef typename enqueued_records::node node;
       typedef node* node_ptr;
       typedef const node* const_node_ptr;

       static node* get_next(const node* n) { return n->m_pNext; }
       static void set_next(node* n, node* next) { n->m_pNext = next; }
       static node* get_previous(const node *n) { return n->m_pPrev; }
       static void set_previous(node* n, node* prev) { n->m_pPrev = prev; }
    };
    //! Boost.Intrusive list type for enqueued records
    typedef intrusive::list<
        typename enqueued_records::node,
        intrusive::value_traits< intrusive::trivial_value_traits< enqueued_records_node_traits, intrusive::normal_link > >,
        intrusive::constant_time_size< false >
    > records_list;

public:
    //! The flag shows that the output thread should finish
    volatile bool m_Finishing;
    //! Frontend synchronization mutex
    mutex m_FrontendMutex;
    //! Backend synchronization mutex
    mutex m_BackendMutex;
    //! Sleep condition
    condition_variable m_Condition;
    //! The condition for synchronizing with \c run finishing
    condition_variable m_FinishingCondition;

    //! Pointer to the backend
    shared_ptr< void > m_pBackend;
    //! Pointer to the consume trampoline function
    consume_trampoline_t m_Consume;

    //! Records queue waiting to be output
    enqueued_records m_EnqueuedRecords;

    //! Dedicated backend feeding thread
    optional< thread > m_Thread;
    //! Thread identifier that is being inside the \c run or \c feed_records
    optional< thread::id > m_FeedingThreadID;

public:
    //! Constructor
    implementation(shared_ptr< void > const& backend, consume_trampoline_t consume_tramp) :
        m_Finishing(false),
        m_pBackend(backend),
        m_Consume(consume_tramp)
    {
    }

    // locking_ptr_counter_base methods
    void lock() { m_BackendMutex.lock(); }
    bool try_lock() { return m_BackendMutex.try_lock(); }
    void unlock() { m_BackendMutex.unlock(); }

    //! The function returns \c true if the frontend already runs a thread
    bool is_thread_running() const
    {
        return !!m_Thread;
    }

    //! Output thread routine
    void run()
    {
        thread_id_cleanup cleanup(m_FeedingThreadID, m_FrontendMutex, &m_FinishingCondition);
        {
            lock_guard< mutex > lock(m_FrontendMutex);
            if (m_FeedingThreadID)
                boost::log::aux::throw_exception(std::logic_error("Asynchronous sink frontend already runs a thread"));
            m_FeedingThreadID = this_thread::get_id();
            cleanup.activate();
            m_Finishing = false;
        }

        records_list records;
        while (!m_Finishing) try
        {
            if (eject_records(records))
            {
                feed_records(records);
            }
            else
            {
                unique_lock< mutex > lock(m_FrontendMutex);
                if (!m_Finishing)
                    m_Condition.wait(lock);
                else
                    break;
            }
        }
        catch (...)
        {
            typedef typename enqueued_records::node node;
            records.clear_and_dispose(checked_deleter< node >());
            throw;
        }
    }

    //! The method softly interrupts record feeding loop
    bool stop()
    {
        unique_lock< mutex > lock(m_FrontendMutex);
        if (m_FeedingThreadID)
        {
            m_Finishing = true;
            m_Condition.notify_one();
            m_FinishingCondition.wait(lock);

            if (m_Thread)
            {
                m_Thread->join();
                m_Thread = none;
            }
            return true;
        }
        else
            return false;
    }

    //! The function feeds records to the backend
    void feed_records()
    {
        thread_id_cleanup cleanup(m_FeedingThreadID, m_FrontendMutex);
        {
            lock_guard< mutex > lock(m_FrontendMutex);
            if (m_FeedingThreadID)
                boost::log::aux::throw_exception(std::logic_error("Asynchronous sink frontend already runs a thread"));
            m_FeedingThreadID = this_thread::get_id();
            cleanup.activate();
        }

        records_list records;
        if (eject_records(records)) try
        {
            feed_records(records);
        }
        catch (...)
        {
            typedef typename enqueued_records::node node;
            records.clear_and_dispose(checked_deleter< node >());
            throw;
        }
    }

    //! The function ejects records from the queue and puts them into list
    bool eject_records(records_list& records)
    {
        typedef typename enqueued_records::node node;
        std::pair< node*, node* > range = m_EnqueuedRecords.eject_nodes();
        if (range.first != NULL)
        {
            // Introduce artifical "end" node
            node end_node;
            range.first->m_pPrev = range.second->m_pNext = &end_node;
            end_node.m_pPrev = range.second;
            end_node.m_pNext = range.second;
            // Inject all nodes into the list
            records_list::node_algorithms::transfer(records.end().pointed_node(), range.first, &end_node);
            return true;
        }
        else
            return false;
    }

private:
    //! The function feeds records to the backend
    void feed_records(records_list& records)
    {
        while (!records.empty()) try
        {
            typedef typename enqueued_records::node node;
            record_type rec;
            rec.swap(records.front().m_Value);
            records.pop_front_and_dispose(checked_deleter< node >());
            lock_guard< mutex > _(m_BackendMutex);
            m_Consume(m_pBackend.get(), rec);
        }
        catch (thread_interrupted&)
        {
            throw;
        }
        catch (...)
        {
            typename base_type::scoped_read_lock _(this->m_Mutex);
            if (this->m_ExceptionHandler.empty())
                throw;
            this->m_ExceptionHandler();
        }
    }
};

//! Constructor
template< typename CharT >
asynchronous_frontend< CharT >::asynchronous_frontend(
    shared_ptr< void > const& backend, consume_trampoline_t consume_tramp, bool start_thread) :
    base_type(new implementation(backend, consume_tramp))
{
    BOOST_ASSERT(!!backend);
    if (start_thread)
    {
        implementation* pImpl = this->BOOST_NESTED_TEMPLATE get_impl< implementation >();
        pImpl->m_Thread = boost::in_place(boost::bind(&implementation::run, pImpl));
    }
}

//! Destructor
template< typename CharT >
asynchronous_frontend< CharT >::~asynchronous_frontend()
{
    stop();
}

template< typename CharT >
shared_ptr< void > const& asynchronous_frontend< CharT >::get_backend() const
{
    return this->BOOST_NESTED_TEMPLATE get_impl< implementation >()->m_pBackend;
}

template< typename CharT >
boost::log::aux::locking_ptr_counter_base& asynchronous_frontend< CharT >::get_backend_locker() const
{
    return *this->BOOST_NESTED_TEMPLATE get_impl< implementation >();
}

template< typename CharT >
void asynchronous_frontend< CharT >::consume(record_type const& record)
{
    register implementation* pImpl = this->BOOST_NESTED_TEMPLATE get_impl< implementation >();
    try
    {
        record_type rec = record;
        rec.detach_from_thread();
        pImpl->m_EnqueuedRecords.push(rec);
        pImpl->m_Condition.notify_one();
    }
    catch (thread_interrupted&)
    {
        throw;
    }
    catch (...)
    {
        typename implementation::scoped_read_lock _(pImpl->m_Mutex);
        if (pImpl->m_ExceptionHandler.empty())
            throw;
        pImpl->m_ExceptionHandler();
    }
}

template< typename CharT >
bool asynchronous_frontend< CharT >::try_consume(record_type const& record)
{
    register implementation* pImpl = this->BOOST_NESTED_TEMPLATE get_impl< implementation >();
    try
    {
        record_type rec = record;
        rec.detach_from_thread();
        if (pImpl->m_EnqueuedRecords.try_push(rec))
        {
            pImpl->m_Condition.notify_one();
            return true;
        }
        else
            return false;
    }
    catch (thread_interrupted&)
    {
        throw;
    }
    catch (...)
    {
        typename implementation::scoped_read_lock _(pImpl->m_Mutex);
        if (pImpl->m_ExceptionHandler.empty())
            throw;
        pImpl->m_ExceptionHandler();
        return false;
    }
}

//! The method starts record feeding loop
template< typename CharT >
void asynchronous_frontend< CharT >::run()
{
    register implementation* pImpl = this->BOOST_NESTED_TEMPLATE get_impl< implementation >();
    pImpl->run();
}

//! The method softly interrupts record feeding loop
template< typename CharT >
bool asynchronous_frontend< CharT >::stop()
{
    implementation* pImpl = this->BOOST_NESTED_TEMPLATE get_impl< implementation >();
    return pImpl->stop();
}

//! The method feeds log records that may have been buffered to the backend and returns
template< typename CharT >
void asynchronous_frontend< CharT >::feed_records()
{
    register implementation* pImpl = this->BOOST_NESTED_TEMPLATE get_impl< implementation >();
    pImpl->feed_records();
}

#ifdef BOOST_LOG_USE_CHAR
template class asynchronous_frontend< char >;
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
template class asynchronous_frontend< wchar_t >;
#endif

#endif // !defined(BOOST_LOG_NO_THREADS)

} // namespace aux

} // namespace sinks

} // namespace log

} // namespace boost
