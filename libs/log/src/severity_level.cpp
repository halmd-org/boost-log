/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * \file   severity_level.cpp
 * \author Andrey Semashev
 * \date   10.05.2008
 *
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#include <boost/log/sources/severity_feature.hpp>

#if !defined(BOOST_LOG_NO_THREADS) && !defined(__GNUC__) && !defined(BOOST_MSVC)
#include <boost/log/detail/singleton.hpp>
#include <boost/log/detail/thread_specific.hpp>
#define BOOST_LOG_NO_NATIVE_TLS
#endif

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace sources {

namespace aux {

#if defined(BOOST_LOG_NO_THREADS)

static int g_Severity = 0;

#elif defined(__GNUC__)

static __thread int g_Severity = 0;

#elif defined(BOOST_MSVC)

// With MSVC we don't support Boost.Log as a delayed-loaded dll, so we can use __declspec(thread) here
static __declspec(thread) int g_Severity = 0;

#else

//! Severity level storage class
class severity_level_holder :
    public boost::log::aux::lazy_singleton< severity_level_holder, boost::log::aux::thread_specific< int > >
{
};

#endif


#if !defined(BOOST_LOG_NO_NATIVE_TLS)

//! The method returns the severity level for the current thread
BOOST_LOG_EXPORT int get_severity_level()
{
    return g_Severity;
}

//! The method sets the severity level for the current thread
BOOST_LOG_EXPORT void set_severity_level(int level)
{
    g_Severity = level;
}

#else // !defined(BOOST_LOG_NO_NATIVE_TLS)

//! The method returns the severity level for the current thread
BOOST_LOG_EXPORT int get_severity_level()
{
    return severity_level_holder::get().get();
}

//! The method sets the severity level for the current thread
BOOST_LOG_EXPORT void set_severity_level(int level)
{
    severity_level_holder::get().set(level);
}

#endif // !defined(BOOST_LOG_NO_NATIVE_TLS)

} // namespace aux

} // namespace sources

} // namespace log

} // namespace boost
