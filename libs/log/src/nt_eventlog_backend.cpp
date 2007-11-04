/*!
 * (C) 2007 Luca Regini
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   nt_eventlog_backend.cpp
 * \author Luca Regini
 * \author Andrey Semashev
 * \date   02.09.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#include <windows.h>
#include <algorithm>
#include <boost/log/sinks/nt_eventlog_backend.hpp>

namespace boost {

namespace log {

namespace sinks {

namespace {

    inline void report_event(HANDLE event_handler, const char* message, WORD information, WORD category)
    {
        ReportEventA(event_handler,
            EVENTLOG_SUCCESS, // information event
            0, // No custom category
            0, // No eventID
            NULL, // No sid
            1, // Number of messages in the message array
            0, // No binary data to log
            &message, // Pointer to string array LPCWSTR if UNICODE is defined LPCSTR otherwise
            NULL // Pointer to data
            );
    }

    inline void report_event(HANDLE event_handler, const wchar_t* message, WORD information, WORD category)
    {
        ReportEventW(event_handler,
            EVENTLOG_SUCCESS, // information event
            0, // No custom category
            0, // No eventID
            NULL, // No sid
            1, // Number of messages in the message array
            0, // No binary data to log
            &message, // Pointer to string array LPCWSTR if UNICODE is defined LPCSTR otherwise
            NULL // Pointer to data
            );
    }

    inline HANDLE register_event_source(const char* source, const char* server)
    {
        return RegisterEventSourceA(server, source);
    }

    inline HANDLE register_event_source(const wchar_t* source, const wchar_t* server)
    {
        return RegisterEventSourceW(server, source);
    }

} // namespace

template< typename CharT >
basic_nt_eventlog_backend< CharT >::basic_nt_eventlog_backend()
{
}

template< typename CharT >
basic_nt_eventlog_backend< CharT >::~basic_nt_eventlog_backend() 
{
    std::for_each(m_source_handles.begin(), m_source_handles.end(), &DeregisterEventSource);
}

template< typename CharT >
bool basic_nt_eventlog_backend< CharT >::add_source(const char_type* source, const char_type* server)
{
    const HANDLE h = register_event_source(server, source);
    if (h == NULL)
        return false;

    try
    {
        m_source_handles.push_back(h);
    }
    catch (std::exception&)
    {
        DeregisterEventSource(h);
        return false;
    }

    return true;
}

//! The method writes the message to the sink
template< typename CharT >
void basic_nt_eventlog_backend< CharT >::do_write_message(
    string_type const& message, attribute_values_view const& attributes)
{
    typename string_type::const_pointer const p = message.data();
    typename string_type::size_type const s = message.size();
    for (std::size_t i = 0; i < m_source_handles.size(); ++i)
    {
        report_event(m_source_handles[i], p, EVENTLOG_SUCCESS, 0);
    }
}

//! Explicitly instantiate sink implementation
template class BOOST_LOG_EXPORT basic_nt_eventlog_backend< char >;
template class BOOST_LOG_EXPORT basic_nt_eventlog_backend< wchar_t >;

} // namespace sinks

} // namespace log

} // namespace boost
