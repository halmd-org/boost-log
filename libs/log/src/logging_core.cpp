/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   logging_core.cpp
 * \author Andrey Semashev
 * \date   19.04.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#include <cstddef>
#include <memory>
#include <vector>
#include <algorithm>
#include <boost/ref.hpp>
#include <boost/none.hpp>
#include <boost/optional.hpp>
#include <boost/utility/in_place_factory.hpp>
#include <boost/thread/tss.hpp>
#include <boost/thread/once.hpp>
#include <boost/thread/read_write_mutex.hpp>
#include <boost/log/logging_core.hpp>
#include <boost/log/attributes/attribute_values_view.hpp>

namespace boost {

namespace log {

//! Logging system implementation
template< typename CharT >
struct basic_logging_core< CharT >::implementation
{
public:
    //! Front-end class type
    typedef basic_logging_core< char_type > logging_core_type;
    //! Read lock type
    typedef read_write_mutex::scoped_read_lock scoped_read_lock;
    //! Write lock type
    typedef read_write_mutex::scoped_write_lock scoped_write_lock;

    //! Sinks container type
    typedef std::vector< shared_ptr< sink_type > > sink_list;

    //! Thread-specific data
    struct thread_data
    {
        struct pending_record
        {
            //! A lock to prevent logging system state to change until the record is pushed
            scoped_read_lock Lock;
            //! A list of sinks that will accept the record
            std::vector< sink_type* > AcceptingSinks;

            pending_record(read_write_mutex& mutex) : Lock(mutex)
            {
            }
        };

        //! A record that is being validated and pushed to the sinks
        optional< pending_record > PendingRecord;
        /*!
        *    \brief Attribute values view
        *
        *    It should really reside inside pending_record, but for performance reasons
        *    it was made a longer-living object to reduce the number of memory allocations.
        *    This view is filled up at each record opening and cleared on the record closure.
        */
        attribute_values_view RecordAttributeValues;

        //! Thread-specific attribute set
        attribute_set ThreadAttributes;
    };

public:
    //! Synchronization mutex
    read_write_mutex Mutex;

    //! List of sinks involved into output
    sink_list Sinks;

    //! Global attribute set
    attribute_set GlobalAttributes;
    //! Thread-specific data
    thread_specific_ptr< thread_data > pThreadData;

    //! Global filter
    filter_type Filter;

public:
    //! Constructor
    implementation() : Mutex(/*read_write_scheduling_policy::writer_priority*/) {}

    //! The method initializes thread-specific data
    void init_thread_data()
    {
        scoped_write_lock lock(Mutex);

        if (!pThreadData.get())
        {
            std::auto_ptr< thread_data > p(new thread_data());
            pThreadData.reset(p.get());
            p.release();
        }
    }

    //! The function holds the reference to the logging system singleton
    static shared_ptr< logging_core_type >& get_instance()
    {
        static shared_ptr< logging_core_type > pInstance;
        return pInstance;
    }
    //! The function initializes the logging system
    static void init_logging_core()
    {
        get_instance().reset(new logging_core_type());
    }
};


//! Logging system constructor
template< typename CharT >
basic_logging_core< CharT >::basic_logging_core()
    : pImpl(new implementation())
{
}

//! Logging system destructor
template< typename CharT >
basic_logging_core< CharT >::~basic_logging_core()
{
    delete pImpl;
}

//! The method returns a pointer to the logging system instance
template< typename CharT >
shared_ptr< basic_logging_core< CharT > > basic_logging_core< CharT >::get()
{
    static once_flag flag = BOOST_ONCE_INIT;
    call_once(&implementation::init_logging_core, flag);
    return implementation::get_instance();
}

//! The method should be called in every non-boost thread on its finish to cleanup some thread-specific data
template< typename CharT >
void basic_logging_core< CharT >::thread_cleanup()
{
    typename implementation::scoped_write_lock lock(pImpl->Mutex);
    pImpl->pThreadData.reset();
}

//! The method adds a new sink
template< typename CharT >
void basic_logging_core< CharT >::add_sink(shared_ptr< sink_type > const& s)
{
    typename implementation::scoped_write_lock lock(pImpl->Mutex);
    typename implementation::sink_list::iterator it =
        std::find(pImpl->Sinks.begin(), pImpl->Sinks.end(), s);
    if (it == pImpl->Sinks.end())
        pImpl->Sinks.push_back(s);
}

//! The method removes the sink from the output
template< typename CharT >
void basic_logging_core< CharT >::remove_sink(shared_ptr< sink_type > const& s)
{
    typename implementation::scoped_write_lock lock(pImpl->Mutex);
    typename implementation::sink_list::iterator it =
        std::find(pImpl->Sinks.begin(), pImpl->Sinks.end(), s);
    if (it != pImpl->Sinks.end())
        pImpl->Sinks.erase(it);
}


//! The method adds an attribute to the global attribute set
template< typename CharT >
typename basic_logging_core< CharT >::attribute_set::iterator
basic_logging_core< CharT >::add_global_attribute(string_type const& name, shared_ptr< attribute > const& attr)
{
    typename implementation::scoped_write_lock lock(pImpl->Mutex);
    return pImpl->GlobalAttributes.insert(std::make_pair(name, attr));
}

//! The method removes an attribute from the global attribute set
template< typename CharT >
void basic_logging_core< CharT >::remove_global_attribute(typename attribute_set::iterator it)
{
    typename implementation::scoped_write_lock lock(pImpl->Mutex);
    pImpl->GlobalAttributes.erase(it);
}

//! The method adds an attribute to the thread-specific attribute set
template< typename CharT >
typename basic_logging_core< CharT >::attribute_set::iterator
basic_logging_core< CharT >::add_thread_attribute(string_type const& name, shared_ptr< attribute > const& attr)
{
    if (!pImpl->pThreadData.get())
        pImpl->init_thread_data();

    return pImpl->pThreadData->ThreadAttributes.insert(std::make_pair(name, attr));
}

//! The method removes an attribute from the thread-specific attribute set
template< typename CharT >
void basic_logging_core< CharT >::remove_thread_attribute(typename attribute_set::iterator it)
{
    typename implementation::thread_data* p = pImpl->pThreadData.get();
    if (p)
        p->ThreadAttributes.erase(it);
}

//! An internal method to set the global filter
template< typename CharT >
void basic_logging_core< CharT >::set_filter_impl(filter_type const& filter)
{
    typename implementation::scoped_write_lock lock(pImpl->Mutex);
    pImpl->Filter = filter;
}

//! The method opens a new record to be written and returns true if the record was opened
template< typename CharT >
bool basic_logging_core< CharT >::open_record(attribute_set const& source_attributes)
{
    if (!pImpl->pThreadData.get()) try
    {
        pImpl->init_thread_data();
    }
    catch (std::exception&)
    {
        return false;
    }

    typename implementation::thread_data& tsd = *pImpl->pThreadData;
    if (!!tsd.PendingRecord)
    {
        return false;
    }
    else try
    {
        // Lock the core to be safe against any attribute or sink set modifications
        // Construct the complete attribute values view
        tsd.PendingRecord = in_place(ref(pImpl->Mutex));
        typename implementation::thread_data::pending_record& record = tsd.PendingRecord.get();

        // Construct attribute values view
        tsd.RecordAttributeValues.adopt(
            source_attributes, tsd.ThreadAttributes, pImpl->GlobalAttributes);

        if ((pImpl->Filter.empty() || pImpl->Filter(tsd.RecordAttributeValues)) && !pImpl->Sinks.empty())
        {
            // The global filter passed, trying the sinks
            record.AcceptingSinks.reserve(pImpl->Sinks.size());
            typename implementation::sink_list::iterator it = pImpl->Sinks.begin();
            for (; it != pImpl->Sinks.end(); ++it)
            {
                try
                {
                    if (it->get()->will_write_message(tsd.RecordAttributeValues))
                        record.AcceptingSinks.push_back(it->get());
                }
                catch (...)
                {
                    // Assume that the sink is incapable to receive messages now
                }
            }

            if (!record.AcceptingSinks.empty())
                return true;
        }
    }
    catch (...)
    {
        // Something has gone wrong. As the library should impose minimum influence
        // on the user's code, we simply mimic here that the record is not needed.
    }

    tsd.PendingRecord = none;
    tsd.RecordAttributeValues.clear();
    return false;
}

//! The method pushes the record and closes it
template< typename CharT >
void basic_logging_core< CharT >::push_record(string_type const& message_text)
{
    if (!pImpl->pThreadData.get())
        pImpl->init_thread_data();

    typename implementation::thread_data& tsd = *pImpl->pThreadData;
    try
    {
        if (!tsd.PendingRecord)
        {
            // If push_record was called with no prior call to open_record, we call it here
            attribute_set empty_source_attributes;
            if (!this->open_record(empty_source_attributes))
                return;
        }

        typename implementation::thread_data::pending_record& record =
            tsd.PendingRecord.get();

        typename std::vector< sink_type* >::iterator it = record.AcceptingSinks.begin();
        for (; it != record.AcceptingSinks.end(); ++it)
        {
            try
            {
                (*it)->write_message(tsd.RecordAttributeValues, message_text);
            }
            catch (...)
            {
            }
        }
    }
    catch (...)
    {
    }

    // Inevitably, close the record
    tsd.PendingRecord = none;
    tsd.RecordAttributeValues.clear();
}

} // namespace log

} // namespace boost

//! Explicitly instantiate logging_core implementation
template class BOOST_LOG_EXPORT boost::log::basic_logging_core< char >;
template class BOOST_LOG_EXPORT boost::log::basic_logging_core< wchar_t >;
