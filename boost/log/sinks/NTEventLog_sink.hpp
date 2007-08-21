/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   text_ostream_sink.hpp
 * \author Andrey Semashev
 * \date   22.04.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_NTEVENTLOGSINK_HPP_INCLUDED_
#define BOOST_LOG_NTEVENTLOGSINK_HPP_INCLUDED_

#include <ostream>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/function/function3.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/sinks/basic_sink.hpp>
#include <boost/log/detail/attachable_sstream_buf.hpp>

#ifdef _MSC_VER
#pragma warning(push)
// 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
// non dll-interface class 'A' used as base for dll-interface class 'B'
#pragma warning(disable: 4275)
#endif // _MSC_VER

namespace boost {

namespace log {

//! A basic implementation of a text output stream logging sink
template< typename CharT >
class BOOST_LOG_EXPORT NTEventLogSink : noncopyable,
    public basic_sink< CharT >
{
    //! Base type
    typedef basic_sink< CharT > base_type;

public:
    //! Character type
    typedef typename base_type::char_type char_type;
    //! String type to be used as a message text holder
    typedef typename base_type::string_type string_type;
    //! Attribute values view type
    typedef typename base_type::attribute_values_view attribute_values_view;
    //! Filter function type
    typedef typename base_type::filter_type filter_type;

    //! Mutex type
    typedef typename base_type::mutex_type mutex_type;
    //! Scoped read lock type
    typedef typename base_type::scoped_read_lock scoped_read_lock;
    //! Scoped write lock type
    typedef typename base_type::scoped_write_lock scoped_write_lock;

    //! Output stream type
    typedef std::basic_ostream< char_type > stream_type;

private:

private:
    //! Formatted log record storage
    string_type m_FormattedRecord;
    //! Stream buffer to fill the storage
    aux::basic_ostringstreambuf< char_type > m_StreamBuf;
    //! Formatting stream
    stream_type m_FormattingStream;


    //! Formatter functor
    boost::function3<
        void,
        stream_type&,
        attribute_values_view const&,
        string_type const&
    > m_Formatter;

	std::vector<void*> m_source_handlers;
	
	
public:
    //! Constructor
    NTEventLogSink();
    //! Destructor
    ~NTEventLogSink();

	bool add_source(const char* source);
  
    //! The method sets formatter functor object
    template< typename T >
    void set_formatter(T const& fmt)
    {
        m_Formatter = fmt;
    }

    //! The method sets the locale used during formatting
    std::locale imbue(std::locale const& loc);

    //! The method returns true if the attribute values pass the filter
    bool will_write_message(attribute_values_view const& attributes);
    //! The method writes the message to the sink
    void write_message(attribute_values_view const& attributes, string_type const& message);
};

typedef NTEventLogSink< char > eventLogSink;
typedef NTEventLogSink< wchar_t > weventLogSink ;

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_TEXT_OSTREAM_SINK_HPP_INCLUDED_
