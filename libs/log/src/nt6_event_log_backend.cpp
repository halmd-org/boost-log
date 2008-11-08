/*!
 * (C) 2008 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   nt6_event_log_backend.cpp
 * \author Andrey Semashev
 * \date   07.11.2008
 * 
 * \brief  A logging sink backend that uses Windows NT 6 (Vista/2008 Server) API
 *         for signalling application events.
 */

#if defined(BOOST_LOG_USE_WINNT6_API)

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600 // _WIN32_WINNT_LONGHORN
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <windows.h>
#include <evntprov.h>
#include <winmeta.h>

#ifdef _MSC_VER
#pragma comment(lib, "advapi32.lib")
#endif

#include <stdexcept>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/throw_exception.hpp>
#include <boost/log/sinks/nt6_event_log_backend.hpp>
#include <boost/log/sinks/nt6_event_log_constants.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace sinks {

namespace winapi {

    //  Windows events severity levels
    BOOST_LOG_EXPORT const level_t log_always = { WINEVENT_LEVEL_LOG_ALWAYS };
    BOOST_LOG_EXPORT const level_t critical = { WINEVENT_LEVEL_CRITICAL };
    BOOST_LOG_EXPORT const level_t error = { WINEVENT_LEVEL_ERROR };
    BOOST_LOG_EXPORT const level_t warning = { WINEVENT_LEVEL_WARNING };
    BOOST_LOG_EXPORT const level_t info = { WINEVENT_LEVEL_INFO };
    BOOST_LOG_EXPORT const level_t verbose = { WINEVENT_LEVEL_VERBOSE };

} // namespace winapi

template< typename CharT >
struct basic_nt6_event_log_backend< CharT >::implementation
{
    //! A handle for the registered event provider
    REGHANDLE m_ProviderHandle;
    //! A level mapping functtor
    level_mapper_type m_LevelMapper;

    implementation() : m_ProviderHandle(0)
    {
    }

    //! The function returns log level for the record
    unsigned char get_level(values_view_type const& values) const
    {
        unsigned char level = WINEVENT_LEVEL_LOG_ALWAYS;
        if (!m_LevelMapper.empty())
            level = m_LevelMapper(values).value;
        return level;
    }
};

template< typename CharT >
basic_nt6_event_log_backend< CharT >::event_enabled_filter::event_enabled_filter(boost::shared_ptr< implementation > const& impl) :
    m_pImpl(impl)
{
}

template< typename CharT >
BOOST_LOG_EXPORT bool basic_nt6_event_log_backend< CharT >::event_enabled_filter::operator() (values_view_type const& values) const
{
    boost::shared_ptr< implementation > impl = m_pImpl.lock();
    if (!!impl)
        return (EventProviderEnabled(impl->m_ProviderHandle, impl->get_level(values), 0ULL /* keyword */) != FALSE);
    else
        return false;
}


template< typename CharT >
basic_nt6_event_log_backend< CharT >::basic_nt6_event_log_backend(GUID const& provider_id) :
    m_pImpl(boost::make_shared< implementation >())
{
    if (EventRegister(&provider_id, NULL, NULL, &m_pImpl->m_ProviderHandle) != ERROR_SUCCESS)
        boost::throw_exception(std::runtime_error("Could not register event provider"));
}

template< typename CharT >
basic_nt6_event_log_backend< CharT >::~basic_nt6_event_log_backend()
{
    EventUnregister(m_pImpl->m_ProviderHandle);
}

template< typename CharT >
typename basic_nt6_event_log_backend< CharT >::event_enabled_filter
basic_nt6_event_log_backend< CharT >::get_event_enabled_filter() const
{
    return event_enabled_filter(m_pImpl);
}

//! The method installs the WinAPI record level mapping function object
template< typename CharT >
void basic_nt6_event_log_backend< CharT >::set_level_mapper(level_mapper_type const& mapper)
{
    m_pImpl->m_LevelMapper = mapper;
}

//! The method puts the formatted message to the event log
template< typename CharT >
void basic_nt6_event_log_backend< CharT >::do_write_message(values_view_type const& values, target_string_type const& formatted_message)
{
    EventWriteString(m_pImpl->m_ProviderHandle, m_pImpl->get_level(values), 0ULL /* keyword */, formatted_message.c_str());
}

#ifdef BOOST_LOG_USE_CHAR
template class BOOST_LOG_EXPORT basic_nt6_event_log_backend< char >;
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
template class BOOST_LOG_EXPORT basic_nt6_event_log_backend< wchar_t >;
#endif

} // namespace sinks

} // namespace log

} // namespace boost

#endif // defined(BOOST_LOG_USE_WINNT6_API)
