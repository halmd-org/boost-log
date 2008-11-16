/*!
 * (C) 2008 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   event_log_backend.cpp
 * \author Andrey Semashev
 * \date   07.11.2008
 * 
 * \brief  A logging sink backend that uses Windows NT event log API
 *         for signalling application events.
 */

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500 // Windows 2000
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <windows.h>
#include <psapi.h>
#include "simple_event_log.h"

#ifdef _MSC_VER
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "advapi32.lib")
#endif

#include <memory>
#include <string>
#include <vector>
#include <stdexcept>
#include <boost/throw_exception.hpp>
#include <boost/log/sinks/event_log_backend.hpp>
#include <boost/log/sinks/event_log_constants.hpp>
#ifndef BOOST_LOG_NO_THREADS
#include <boost/ref.hpp>
#include <boost/bind.hpp>
#include <boost/thread/once.hpp>
#endif // BOOST_LOG_NO_THREADS
#include "event_log_registry.hpp"

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace sinks {

namespace event_log {

    //  Windows event types
    BOOST_LOG_EXPORT const event_type_t success = { EVENTLOG_SUCCESS };
    BOOST_LOG_EXPORT const event_type_t info = { EVENTLOG_INFORMATION_TYPE };
    BOOST_LOG_EXPORT const event_type_t warning = { EVENTLOG_WARNING_TYPE };
    BOOST_LOG_EXPORT const event_type_t error = { EVENTLOG_ERROR_TYPE };

} // namespace event_log

namespace {

#ifdef BOOST_LOG_USE_CHAR
    //! A simple forwarder to the ReportEvent API
    inline BOOL report_event(
        HANDLE hEventLog,
        WORD wType,
        WORD wCategory,
        DWORD dwEventID,
        PSID lpUserSid,
        WORD wNumStrings,
        DWORD dwDataSize,
        const char** lpStrings,
        LPVOID lpRawData)
    {
        return ReportEventA(hEventLog, wType, wCategory, dwEventID, lpUserSid, wNumStrings, dwDataSize, lpStrings, lpRawData);
    }
    //! A simple forwarder to the GetModuleFileName API
    inline DWORD get_module_file_name(HMODULE hModule, char* lpFilename, DWORD nSize)
    {
        return GetModuleFileNameA(hModule, lpFilename, nSize);
    }
    //! A simple forwarder to the RegisterEventSource API
    inline HANDLE register_event_source(const char* lpUNCServerName, const char* lpSourceName)
    {
        return RegisterEventSourceA(lpUNCServerName, lpSourceName);
    }
    //! The function completes default source name for the sink backend
    inline void complete_default_simple_event_log_source_name(std::string& name)
    {
        name += " simple event source";
    }
#endif // BOOST_LOG_USE_CHAR

#ifdef BOOST_LOG_USE_WCHAR_T
    //! A simple forwarder to the ReportEvent API
    inline BOOL report_event(
        HANDLE hEventLog,
        WORD wType,
        WORD wCategory,
        DWORD dwEventID,
        PSID lpUserSid,
        WORD wNumStrings,
        DWORD dwDataSize,
        const wchar_t** lpStrings,
        LPVOID lpRawData)
    {
        return ReportEventW(hEventLog, wType, wCategory, dwEventID, lpUserSid, wNumStrings, dwDataSize, lpStrings, lpRawData);
    }
    //! A simple forwarder to the GetModuleFileName API
    inline DWORD get_module_file_name(HMODULE hModule, wchar_t* lpFilename, DWORD nSize)
    {
        return GetModuleFileNameW(hModule, lpFilename, nSize);
    }
    //! A simple forwarder to the RegisterEventSource API
    inline HANDLE register_event_source(const wchar_t* lpUNCServerName, const wchar_t* lpSourceName)
    {
        return RegisterEventSourceW(lpUNCServerName, lpSourceName);
    }
    //! The function completes default source name for the sink backend
    inline void complete_default_simple_event_log_source_name(std::wstring& name)
    {
        name += L" simple event source";
    }
#endif // BOOST_LOG_USE_WCHAR_T

    //! The function finds the handle for the current module
    void init_self_module_handle(HMODULE& handle)
    {
        // Acquire all modules of the current process
        HANDLE hProcess = GetCurrentProcess();
        std::vector< HMODULE > modules;
        DWORD module_count = 1024;
        do
        {
            modules.resize(module_count, HMODULE(0));
            BOOL res = EnumProcessModules(
                hProcess,
                &modules[0],
                static_cast< DWORD >(modules.size() * sizeof(HMODULE)),
                &module_count);
            module_count /= sizeof(HMODULE);

            if (!res)
                boost::throw_exception(std::runtime_error("Could not enumerate process modules"));
        }
        while (module_count > modules.size());
        modules.resize(module_count, HMODULE(0));

        // Now find the current module among them
        void* p = (void*)&init_self_module_handle;
        for (std::size_t i = 0, n = modules.size(); i < n; ++i)
        {
            MODULEINFO info;
            if (!GetModuleInformation(hProcess, modules[i], &info, sizeof(info)))
                boost::throw_exception(std::runtime_error("Could not acquire module information"));

            if (info.lpBaseOfDll <= p && (static_cast< unsigned char* >(info.lpBaseOfDll) + info.SizeOfImage) > p)
            {
                // Found it
                handle = modules[i];
                break;
            }
        }

        if (!handle)
            boost::throw_exception(std::runtime_error("Could not find self module information"));
    }

    //! Retrieves the full name of the current module (be that dll or exe)
    template< typename CharT >
    std::basic_string< CharT > get_current_module_name()
    {
        static HMODULE hSelfModule = 0;
#ifndef BOOST_LOG_NO_THREADS
        static once_flag flag = BOOST_ONCE_INIT;
        boost::call_once(flag, boost::bind(&init_self_module_handle, boost::ref(hSelfModule)));
#else
        if (!hSelfModule)
            init_self_module_handle(hSelfModule);
#endif // BOOST_LOG_NO_THREADS

        // Get the module file name
        CharT buf[MAX_PATH];
        DWORD size = get_module_file_name(hSelfModule, buf, sizeof(buf) / sizeof(*buf));
        if (size == 0)
            boost::throw_exception(std::runtime_error("Could not get module file name"));

        return std::basic_string< CharT >(buf, buf + size);
    }

} // namespace


//! Sink backend implementation
template< typename CharT >
struct basic_simple_event_log_backend< CharT >::implementation
{
    //! A handle for the registered event provider
    HANDLE m_SourceHandle;
    //! A level mapping functor
    event_type_mapper_type m_LevelMapper;

    implementation() : m_SourceHandle(0)
    {
    }
};

//! Default constructor. Registers event source Boost.Log <Boost version> in the Application log.
template< typename CharT >
basic_simple_event_log_backend< CharT >::basic_simple_event_log_backend() :
    m_pImpl(construct(get_default_log_name(), get_default_source_name(), false))
{
}

//! Destructor
template< typename CharT >
basic_simple_event_log_backend< CharT >::~basic_simple_event_log_backend()
{
    DeregisterEventSource(m_pImpl->m_SourceHandle);
    delete m_pImpl;
}

//! Constructs backend implementation
template< typename CharT >
typename basic_simple_event_log_backend< CharT >::implementation*
basic_simple_event_log_backend< CharT >::construct(
    string_type const& log_name, string_type const& source_name, bool force)
{
    aux::registry_params< char_type > reg_params;
    reg_params.event_message_file = get_current_module_name< char_type >();
    reg_params.types_supported = DWORD(
        EVENTLOG_SUCCESS |
        EVENTLOG_INFORMATION_TYPE |
        EVENTLOG_WARNING_TYPE |
        EVENTLOG_ERROR_TYPE);
    aux::init_event_log_registry(log_name, source_name, force, reg_params);

    std::auto_ptr< implementation > p(new implementation());

    HANDLE hSource = register_event_source(NULL, source_name.c_str());
    if (!hSource)
        boost::throw_exception(std::runtime_error("Could not register event source"));

    p->m_SourceHandle = hSource;

    return p.release();
}

//! Returns default log name
template< typename CharT >
typename basic_simple_event_log_backend< CharT >::string_type
basic_simple_event_log_backend< CharT >::get_default_log_name()
{
    return aux::registry_traits< char_type >::make_default_log_name();
}

//! Returns default source name
template< typename CharT >
typename basic_simple_event_log_backend< CharT >::string_type
basic_simple_event_log_backend< CharT >::get_default_source_name()
{
    string_type source_name = aux::registry_traits< char_type >::make_default_source_name();
    complete_default_simple_event_log_source_name(source_name);
    return source_name;
}

//! The method installs the function object that maps application severity levels to WinAPI event types
template< typename CharT >
void basic_simple_event_log_backend< CharT >::set_event_type_mapper(event_type_mapper_type const& mapper)
{
    m_pImpl->m_LevelMapper = mapper;
}

//! The method puts the formatted message to the event log
template< typename CharT >
void basic_simple_event_log_backend< CharT >::do_write_message(values_view_type const& values, target_string_type const& formatted_message)
{
    const char_type* message = formatted_message.c_str();
    WORD event_type = EVENTLOG_INFORMATION_TYPE;
    if (!m_pImpl->m_LevelMapper.empty())
        event_type = m_pImpl->m_LevelMapper(values).value;

    DWORD event_id;
    switch (event_type)
    {
    case EVENTLOG_SUCCESS:
        event_id = BOOST_LOG_MSG_DEBUG; break;
    case EVENTLOG_WARNING_TYPE:
        event_id = BOOST_LOG_MSG_WARNING; break;
    case EVENTLOG_ERROR_TYPE:
        event_id = BOOST_LOG_MSG_ERROR; break;
    case EVENTLOG_INFORMATION_TYPE:
    default:
        event_id = BOOST_LOG_MSG_INFO; break;
    }

    report_event(
        m_pImpl->m_SourceHandle,    // Event log handle. 
        event_type,                 // Event type. 
        0,                          // Event category.  
        event_id,                   // Event identifier. 
        NULL,                       // No user security identifier. 
        1,                          // Number of substitution strings. 
        0,                          // No data. 
        &message,                   // Pointer to strings. 
        NULL);                      // No data.
}

#ifdef BOOST_LOG_USE_CHAR
template class BOOST_LOG_EXPORT basic_simple_event_log_backend< char >;
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
template class BOOST_LOG_EXPORT basic_simple_event_log_backend< wchar_t >;
#endif

} // namespace sinks

} // namespace log

} // namespace boost
