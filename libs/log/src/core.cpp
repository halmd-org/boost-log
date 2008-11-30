/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * \file   core.cpp
 * \author Andrey Semashev
 * \date   19.04.2007
 *
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#include <memory>
#include <stack>
#include <vector>
#include <algorithm>
#include <boost/ref.hpp>
#include <boost/none.hpp>
#include <boost/compatibility/cpp_c_headers/cstddef>
#include <boost/log/core.hpp>
#include <boost/log/sinks/sink.hpp>
#include <boost/log/attributes/attribute_values_view.hpp>
#include <boost/log/detail/singleton.hpp>
#if !defined(BOOST_LOG_NO_THREADS)
#include <boost/thread/tss.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/log/detail/shared_lock_guard.hpp>
#endif

namespace boost {

namespace BOOST_LOG_NAMESPACE {

//! Logging system implementation
template< typename CharT >
struct basic_core< CharT >::implementation :
    public log::aux::lazy_singleton<
        implementation,
        shared_ptr< basic_core< CharT > >
    >
{
public:
    //! Base type of singleton holder
    typedef log::aux::lazy_singleton<
        implementation,
        shared_ptr< basic_core< CharT > >
    > base_type;

    //! Front-end class type
    typedef basic_core< char_type > core_type;
#if !defined(BOOST_LOG_NO_THREADS)
    //! Read lock type
    typedef log::aux::shared_lock_guard< shared_mutex > scoped_read_lock;
    //! Write lock type
    typedef lock_guard< shared_mutex > scoped_write_lock;
#endif

    //! Sinks container type
    typedef std::vector< shared_ptr< sink_type > > sink_list;

    //! Thread-specific data
    struct thread_data
    {
        //! A structure that holds a particular logging record data
        struct pending_record
        {
            //! A list of sinks that will accept the record (mutable to emulate moving semantics)
            mutable sink_list AcceptingSinks;
            //! Attribute values view
            values_view_type AttributeValues;

            //! Constructor
            pending_record(
                attribute_set_type const& SourceAttrs,
                attribute_set_type const& ThreadAttrs,
                attribute_set_type const& GlobalAttrs
            ) : AttributeValues(SourceAttrs, ThreadAttrs, GlobalAttrs)
            {
            }
            //! Copy constructor (acts as move)
            pending_record(pending_record const& that) : AttributeValues(that.AttributeValues)
            {
                AcceptingSinks.swap(that.AcceptingSinks);
            }
        };

        //! A stack of log records being processed
        std::stack< pending_record > PendingRecords;

        //! Thread-specific attribute set
        attribute_set_type ThreadAttributes;
    };

public:
#if !defined(BOOST_LOG_NO_THREADS)
    //! Synchronization mutex
    shared_mutex Mutex;
#endif

    //! List of sinks involved into output
    sink_list Sinks;

    //! Global attribute set
    attribute_set_type GlobalAttributes;
#if !defined(BOOST_LOG_NO_THREADS)
    //! Thread-specific data
    thread_specific_ptr< thread_data > pThreadData;
#else
    //! Thread-specific data
    std::auto_ptr< thread_data > pThreadData;
#endif

    //! The global state of logging
    volatile bool Enabled;
    //! Global filter
    filter_type Filter;

public:
    //! Constructor
    implementation() : Enabled(true) {}

    //! The method returns the current thread-specific data
    thread_data* get_thread_data()
    {
        thread_data* p = pThreadData.get();
        if (!p)
        {
            init_thread_data();
            p = pThreadData.get();
        }
        return p;
    }

    //! The function initializes the logging system
    static void init_instance()
    {
        base_type::get_instance().reset(new core_type());
    }

private:
    //! The method initializes thread-specific data
    void init_thread_data()
    {
#if !defined(BOOST_LOG_NO_THREADS)
        scoped_write_lock lock(Mutex);
#endif
        if (!pThreadData.get())
        {
            std::auto_ptr< thread_data > p(new thread_data());
            pThreadData.reset(p.get());
            p.release();
        }
    }
};


//! Logging system constructor
template< typename CharT >
basic_core< CharT >::basic_core()
    : pImpl(new implementation())
{
}

//! Logging system destructor
template< typename CharT >
basic_core< CharT >::~basic_core()
{
    delete pImpl;
}

//! The method returns a pointer to the logging system instance
template< typename CharT >
shared_ptr< basic_core< CharT > > basic_core< CharT >::get()
{
    return implementation::get();
}

//! The method enables or disables logging and returns the previous state of logging flag
template< typename CharT >
bool basic_core< CharT >::set_logging_enabled(bool enabled)
{
#if !defined(BOOST_LOG_NO_THREADS)
    typename implementation::scoped_write_lock lock(pImpl->Mutex);
#endif
    const bool old_value = pImpl->Enabled;
    pImpl->Enabled = enabled;
    return old_value;
}

//! The method adds a new sink
template< typename CharT >
void basic_core< CharT >::add_sink(shared_ptr< sink_type > const& s)
{
#if !defined(BOOST_LOG_NO_THREADS)
    typename implementation::scoped_write_lock lock(pImpl->Mutex);
#endif
    typename implementation::sink_list::iterator it =
        std::find(pImpl->Sinks.begin(), pImpl->Sinks.end(), s);
    if (it == pImpl->Sinks.end())
        pImpl->Sinks.push_back(s);
}

//! The method removes the sink from the output
template< typename CharT >
void basic_core< CharT >::remove_sink(shared_ptr< sink_type > const& s)
{
#if !defined(BOOST_LOG_NO_THREADS)
    typename implementation::scoped_write_lock lock(pImpl->Mutex);
#endif
    typename implementation::sink_list::iterator it =
        std::find(pImpl->Sinks.begin(), pImpl->Sinks.end(), s);
    if (it != pImpl->Sinks.end())
        pImpl->Sinks.erase(it);
}


//! The method adds an attribute to the global attribute set
template< typename CharT >
std::pair< typename basic_core< CharT >::attribute_set_type::iterator, bool >
basic_core< CharT >::add_global_attribute(string_type const& name, shared_ptr< attribute > const& attr)
{
#if !defined(BOOST_LOG_NO_THREADS)
    typename implementation::scoped_write_lock lock(pImpl->Mutex);
#endif
    return pImpl->GlobalAttributes.insert(typename attribute_set_type::key_type(name), attr);
}

//! The method removes an attribute from the global attribute set
template< typename CharT >
void basic_core< CharT >::remove_global_attribute(typename attribute_set_type::iterator it)
{
#if !defined(BOOST_LOG_NO_THREADS)
    typename implementation::scoped_write_lock lock(pImpl->Mutex);
#endif
    pImpl->GlobalAttributes.erase(it);
}

//! The method returns the complete set of currently registered global attributes
template< typename CharT >
typename basic_core< CharT >::attribute_set_type basic_core< CharT >::get_global_attributes() const
{
#if !defined(BOOST_LOG_NO_THREADS)
    typename implementation::scoped_write_lock lock(pImpl->Mutex);
#endif
    return pImpl->GlobalAttributes;
}
//! The method replaces the complete set of currently registered global attributes with the provided set
template< typename CharT >
void basic_core< CharT >::set_global_attributes(attribute_set_type const& attrs) const
{
#if !defined(BOOST_LOG_NO_THREADS)
    typename implementation::scoped_write_lock lock(pImpl->Mutex);
#endif
    pImpl->GlobalAttributes = attrs;
}

//! The method adds an attribute to the thread-specific attribute set
template< typename CharT >
std::pair< typename basic_core< CharT >::attribute_set_type::iterator, bool >
basic_core< CharT >::add_thread_attribute(string_type const& name, shared_ptr< attribute > const& attr)
{
    typename implementation::thread_data* p = pImpl->get_thread_data();
    return p->ThreadAttributes.insert(typename attribute_set_type::key_type(name), attr);
}

//! The method removes an attribute from the thread-specific attribute set
template< typename CharT >
void basic_core< CharT >::remove_thread_attribute(typename attribute_set_type::iterator it)
{
    typename implementation::thread_data* p = pImpl->pThreadData.get();
    if (p)
        p->ThreadAttributes.erase(it);
}

//! The method returns the complete set of currently registered thread-specific attributes
template< typename CharT >
typename basic_core< CharT >::attribute_set_type basic_core< CharT >::get_thread_attributes() const
{
    typename implementation::thread_data* p = pImpl->get_thread_data();
    return p->ThreadAttributes;
}
//! The method replaces the complete set of currently registered thread-specific attributes with the provided set
template< typename CharT >
void basic_core< CharT >::set_thread_attributes(attribute_set_type const& attrs) const
{
    typename implementation::thread_data* p = pImpl->get_thread_data();
    p->ThreadAttributes = attrs;
}

//! An internal method to set the global filter
template< typename CharT >
void basic_core< CharT >::set_filter(filter_type const& filter)
{
#if !defined(BOOST_LOG_NO_THREADS)
    typename implementation::scoped_write_lock lock(pImpl->Mutex);
#endif
    pImpl->Filter = filter;
}

//! The method removes the global logging filter
template< typename CharT >
void basic_core< CharT >::reset_filter()
{
#if !defined(BOOST_LOG_NO_THREADS)
    typename implementation::scoped_write_lock lock(pImpl->Mutex);
#endif
    pImpl->Filter.clear();
}

//! The method opens a new record to be written and returns true if the record was opened
template< typename CharT >
bool basic_core< CharT >::open_record(attribute_set_type const& source_attributes)
{
    // Try a quick win first
    if (pImpl->Enabled) try
    {
        typename implementation::thread_data* tsd = pImpl->get_thread_data();
#if !defined(BOOST_LOG_NO_THREADS)
        // Lock the core to be safe against any attribute or sink set modifications
        typename implementation::scoped_read_lock lock(pImpl->Mutex);
#endif

        if (pImpl->Enabled && !pImpl->Sinks.empty())
        {
            // Construct a record
            typedef typename implementation::thread_data::pending_record pending_record;
            pending_record record(source_attributes, tsd->ThreadAttributes, pImpl->GlobalAttributes);

            if (pImpl->Filter.empty() || pImpl->Filter(record.AttributeValues))
            {
                // The global filter passed, trying the sinks
                record.AcceptingSinks.reserve(pImpl->Sinks.size());
                typename implementation::sink_list::iterator it = pImpl->Sinks.begin(), end = pImpl->Sinks.end();
                for (; it != end; ++it)
                {
                    try
                    {
                        if (it->get()->will_consume(record.AttributeValues))
                            record.AcceptingSinks.push_back(*it);
                    }
                    catch (...)
                    {
                        // Assume that the sink is incapable to receive messages now
                    }
                }

                if (!record.AcceptingSinks.empty())
                {
                    // Some sinks are willing to process the record
                    record.AttributeValues.freeze();
                    tsd->PendingRecords.push(record);
                    return true;
                }
            }
        }
    }
    catch (...)
    {
        // Something has gone wrong. As the library should impose minimum influence
        // on the user's code, we simply mimic here that the record is not needed.
    }

    return false;
}

//! The method pushes the record and closes it
template< typename CharT >
void basic_core< CharT >::push_record(string_type const& message_text)
{
    typename implementation::thread_data* tsd = pImpl->get_thread_data();
    try
    {
        if (tsd->PendingRecords.empty())
        {
            // If push_record was called with no prior call to open_record, we call it here
            attribute_set_type empty_source_attributes;
            if (!this->open_record(empty_source_attributes))
                return;
        }

        typename implementation::thread_data::pending_record& record =
            tsd->PendingRecords.top();

        typename implementation::sink_list::iterator it = record.AcceptingSinks.begin(),
            end = record.AcceptingSinks.end();
        for (; it != end; ++it) try
        {
            (*it)->consume(record.AttributeValues, message_text);
        }
        catch (...)
        {
        }
    }
    catch (...)
    {
    }

    // Inevitably, close the record
    tsd->PendingRecords.pop();
}

//! The method cancels the record
template< typename CharT >
void basic_core< CharT >::cancel_record()
{
    typename implementation::thread_data* tsd = pImpl->pThreadData.get();
    if (tsd && !tsd->PendingRecords.empty())
    {
        tsd->PendingRecords.pop();
    }
}

//  Explicitly instantiate core implementation
#ifdef BOOST_LOG_USE_CHAR
template class BOOST_LOG_EXPORT basic_core< char >;
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
template class BOOST_LOG_EXPORT basic_core< wchar_t >;
#endif

} // namespace log

} // namespace boost
