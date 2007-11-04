/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   basic_sink_backend.hpp
 * \author Andrey Semashev
 * \date   04.11.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_SINKS_BASIC_SINK_BACKEND_HPP_INCLUDED_
#define BOOST_LOG_SINKS_BASIC_SINK_BACKEND_HPP_INCLUDED_

#include <string>
#include <locale>
#include <ostream>
#include <boost/noncopyable.hpp>
#include <boost/static_assert.hpp>
#include <boost/mpl/or.hpp>
#include <boost/function/function3.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/sinks/threading_models.hpp>
#include <boost/log/attributes/attribute_values_view.hpp>
#include <boost/log/detail/attachable_sstream_buf.hpp>

namespace boost {

namespace log {

namespace sinks {

//! A basic implementation of a logging sink backend
template< typename CharT, typename ThreadingModelTagT >
struct basic_sink_backend : noncopyable
{
    //! Character type
    typedef CharT char_type;
    //! String type to be used as a message text holder
    typedef std::basic_string< char_type > string_type;
    //! Attribute values view type
    typedef basic_attribute_values_view< char_type > attribute_values_view;

    //! Threading model tag
    typedef ThreadingModelTagT threading_model;
};

//! A basic implementation of a logging sink backend with message formatting support
template< typename CharT, typename ThreadingModelTagT = frontend_synchronization_tag >
class BOOST_LOG_NO_VTABLE basic_formatting_sink_backend :
    public basic_sink_backend< CharT, ThreadingModelTagT >
{
    typedef basic_sink_backend< CharT, ThreadingModelTagT > base_type;

public:
    //  Type imports from the base class
    typedef typename base_type::char_type char_type;
    typedef typename base_type::string_type string_type;
    typedef typename base_type::attribute_values_view attribute_values_view;
    typedef typename base_type::threading_model threading_model;

    //  This type of sink backends require synchronization on the frontend side
    BOOST_STATIC_ASSERT((mpl::or_<
        is_model_supported< threading_model, single_thread_tag >,
        is_model_supported< threading_model, frontend_synchronization_tag >
    >::value));

    //! Output stream type
    typedef std::basic_ostream< char_type > stream_type;

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

public:
    //! Default constructor
    basic_formatting_sink_backend() :
        m_StreamBuf(m_FormattedRecord),
        m_FormattingStream(&m_StreamBuf),
        m_Formatter(&basic_formatting_sink_backend::default_formatter)
    {
    }

    //! The method sets formatter functor object
    template< typename T >
    void set_formatter(T const& fmt)
    {
        m_Formatter = fmt;
    }

    //! The method sets the locale used during formatting
    std::locale imbue(std::locale const& loc)
    {
        return m_FormattingStream.imbue(loc);
    }

    //! The method writes the message to the sink - do not override in derived classes
    void write_message(attribute_values_view const& attributes, string_type const& message)
    {
        // Scope guard to automatically clear the storage
        struct clear_invoker
        {
            string_type& m_T;
            explicit clear_invoker(string_type& t) : m_T(t) {}
            ~clear_invoker() { m_T.clear(); }
        };
        clear_invoker _(m_FormattedRecord);

        // Perform the formatting
        m_Formatter(m_FormattingStream, attributes, message);
        m_FormattingStream.flush();

        // Pass the formatted string to the backend implementation
        do_write_message(m_FormattedRecord, attributes);
    }

protected:
    //! A backend-defined implementation of the formatted message storing
    virtual void do_write_message(string_type const& formatted_message, attribute_values_view const& attributes) = 0;

private:
    //! Default formatter
    static void default_formatter(stream_type& strm, attribute_values_view const&, string_type const& message)
    {
        strm << message << '\n';
    }
};

} // namespace sinks

} // namespace log

} // namespace boost

#endif // BOOST_LOG_SINKS_BASIC_SINK_BACKEND_HPP_INCLUDED_
