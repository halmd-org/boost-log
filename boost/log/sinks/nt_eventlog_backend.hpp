/*!
 * (C) 2007 Luca Regini
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   nt_eventlog_backend.hpp
 * \author Luca Regini
 * \author Andrey Semashev
 * \date   02.09.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_SINKS_NT_EVENTLOG_BACKEND_HPP_INCLUDED_HPP_
#define BOOST_LOG_SINKS_NT_EVENTLOG_BACKEND_HPP_INCLUDED_HPP_

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/sinks/basic_sink_backend.hpp>

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

//! An implementation of a Windows NT event logging sink backend
template< typename CharT >
class BOOST_LOG_EXPORT basic_nt_eventlog_backend :
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

private:
    //! Event source handles
    std::vector< void* > m_source_handles;

public:
    //! Constructor
	basic_nt_eventlog_backend();
    //! Destructor
    ~basic_nt_eventlog_backend();

    //! The method adds another eventlog source
    bool add_source(const char_type* source, const char_type* server = NULL);

private:
    //! The method writes the message to the sink
    void do_write_message(attribute_values_view const& attributes, string_type const& message);
};

typedef basic_nt_eventlog_backend< char > nt_eventlog_backend;
typedef basic_nt_eventlog_backend< wchar_t > wnt_eventlog_backend;

} // namespace sinks

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_SINKS_NT_EVENTLOG_BACKEND_HPP_INCLUDED_HPP_
