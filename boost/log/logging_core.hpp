/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   logging_core.hpp
 * \author Andrey Semashev
 * \date   19.04.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_LOGGING_CORE_HPP_INCLUDED_
#define BOOST_LOG_LOGGING_CORE_HPP_INCLUDED_

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/function/function1.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/sinks/sink.hpp>
#include <boost/log/attributes/attribute_set.hpp>

#ifdef _MSC_VER
#pragma warning(push)
 // non dll-interface class 'A' used as base for dll-interface class 'B'
#pragma warning(disable: 4275)
#endif // _MSC_VER

namespace boost {

namespace log {

//! Logging system core class
template< typename CharT >
class BOOST_LOG_EXPORT basic_logging_core : noncopyable
{
public:
    //! Character type
    typedef CharT char_type;
    //! String type to be used as a message text holder
    typedef std::basic_string< char_type > string_type;
    //! Attribute set type
    typedef basic_attribute_set< char_type > attribute_set;
    //! Attribute values view type
    typedef basic_attribute_values_view< char_type > attribute_values_view;
    //! Sink interface type
    typedef sink< char_type > sink_type;
    //! Filter function type
    typedef function1< bool, attribute_values_view const& > filter_type;

private:
    //! Implementation type
    struct implementation;
    friend struct implementation;

private:
    //! A pointer to the implementation
    implementation* pImpl;

private:
    //! Constructor
    basic_logging_core();

public:
    //! Destructor
    ~basic_logging_core();

    //! The method returns a pointer to the logging system instance
    static shared_ptr< basic_logging_core > get();

    //! The method should be called in every non-boost thread on its finish to cleanup some thread-specific data
    void thread_cleanup();

    //! The method sets the global logging filter
    template< typename T >
    void set_filter(T const& filter)
    {
        this->set_filter_impl(filter_type(filter));
    }
    //! The method removes the global logging filter
    void reset_filter();

    //! The method adds a new sink
    void add_sink(shared_ptr< sink_type > const& s);
    //! The method removes the sink from the output
    void remove_sink(shared_ptr< sink_type > const& s);

    //! The method adds an attribute to the global attribute set
    typename attribute_set::iterator add_global_attribute(
        string_type const& name, shared_ptr< attribute > const& attr);
    //! The method removes an attribute from the global attribute set
    void remove_global_attribute(typename attribute_set::iterator it);

    //! The method adds an attribute to the thread-specific attribute set
    typename attribute_set::iterator add_thread_attribute(
        string_type const& name, shared_ptr< attribute > const& attr);
    //! The method removes an attribute from the thread-specific attribute set
    void remove_thread_attribute(typename attribute_set::iterator it);

    //! The method opens a new record to be written and returns true if the record was opened
    bool open_record(attribute_set const& source_attributes);
    //! The method pushes the record and closes it
    void push_record(string_type const& message_text);
    //! The method cancels the record
    void cancel_record();

private:
    //! An internal method to set the global filter
    void set_filter_impl(filter_type const& filter);
};

typedef basic_logging_core< char > logging_core;
typedef basic_logging_core< wchar_t > wlogging_core;

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_LOGGING_CORE_HPP_INCLUDED_
