/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   syslog_backend.hpp
 * \author Andrey Semashev
 * \date   08.01.2008
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_SINKS_SYSLOG_BACKEND_HPP_INCLUDED_HPP_
#define BOOST_LOG_SINKS_SYSLOG_BACKEND_HPP_INCLUDED_HPP_

#include <string>
#include <locale>
#include <ostream>
#include <boost/shared_ptr.hpp>
#include <boost/function/function3.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/sinks/basic_sink_backend.hpp>
#include <boost/log/sinks/threading_models.hpp>
#include <boost/log/sinks/syslog_constants.hpp>
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

//! An implementation of a syslog sink backend
template< typename CharT >
class BOOST_LOG_EXPORT basic_syslog_backend :
    public basic_sink_backend< CharT, frontend_synchronization_tag >
{
    //! Base type
    typedef basic_sink_backend< CharT, frontend_synchronization_tag > base_type;
    //! Implementation type
    struct implementation;

public:
    //! Character type
    typedef typename base_type::char_type char_type;
    //! String type to be used as a message text holder
    typedef typename base_type::string_type string_type;
    //! Attribute values view type
    typedef typename base_type::attribute_values_view attribute_values_view;
    //! Output stream type
    typedef std::basic_ostream< char_type > stream_type;
    //! Formatter function type
    typedef boost::function3<
        void,
        stream_type&,
        attribute_values_view const&,
        string_type const&
    > formatter_type;

private:
    //! Pointer to the implementation
    implementation* m_pImpl;

public:
    //! Constructor
	explicit basic_syslog_backend(
	    syslog::facility_t facility = syslog::user,
	    syslog::options_t options = syslog::no_delay);
    //! Destructor
    ~basic_syslog_backend();

    //! The method sets formatter functor object
    void set_formatter(formatter_type const& fmt);
    //! The method resets the formatter
    void reset_formatter();

    //! The method returns the current locale
    std::locale getloc() const;
    //! The method sets the locale used during formatting
    std::locale imbue(std::locale const& loc);

    //! The method writes the message to the sink
    void write_message(attribute_values_view const& attributes, string_type const& message);
};

typedef basic_syslog_backend< char > syslog_backend;
typedef basic_syslog_backend< wchar_t > wsyslog_backend;

} // namespace sinks

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_SINKS_SYSLOG_BACKEND_HPP_INCLUDED_HPP_
