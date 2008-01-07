/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   text_ostream_backend.hpp
 * \author Andrey Semashev
 * \date   22.04.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_SINKS_TEXT_OSTREAM_BACKEND_HPP_INCLUDED_
#define BOOST_LOG_SINKS_TEXT_OSTREAM_BACKEND_HPP_INCLUDED_

#include <ostream>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/sinks/basic_sink_backend.hpp>
#include <boost/log/sinks/record_writer.hpp>

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

//! A basic implementation of a text output stream logging sink backend
template< typename CharT >
class BOOST_LOG_EXPORT basic_text_ostream_backend :
    public basic_formatting_sink_backend< CharT >
{
    //! Base type
    typedef basic_formatting_sink_backend< CharT > base_type;

public:
    //! Character type
    typedef typename base_type::char_type char_type;
    //! String type to be used as a message text holder
    typedef typename base_type::string_type string_type;
    //! Attribute values view type
    typedef typename base_type::attribute_values_view attribute_values_view;
    //! Output stream type
    typedef typename base_type::stream_type stream_type;

private:
    //! Structure with data regarding a single stream
    struct stream_info
    {
        shared_ptr< stream_type > strm;
        record_writer* record_listener;
    };
    //! Type of the container that holds all aggregated streams
    typedef std::vector< stream_info > ostream_sequence;

private:
    //! Output stream list
    ostream_sequence m_Streams;
    //! Auto-flush flag
    bool m_fAutoFlush;

public:
    //! Constructor
	basic_text_ostream_backend();
    //! Destructor
    ~basic_text_ostream_backend();

    //! The method adds a new stream to the sink
    void add_stream(shared_ptr< stream_type > const& strm);
    //! The method removes a stream from the sink
    void remove_stream(shared_ptr< stream_type > const& strm);

    //! Sets the flag to automatically flush buffers after each logged line
    void auto_flush(bool f = true);

private:
    //! The method writes the message to the sink
    void do_write_message(attribute_values_view const& attributes, string_type const& formatted_message);
};

typedef basic_text_ostream_backend< char > text_ostream_backend;
typedef basic_text_ostream_backend< wchar_t > wtext_ostream_backend;

} // namespace sinks

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_SINKS_TEXT_OSTREAM_BACKEND_HPP_INCLUDED_
