/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   syslog_constants.hpp
 * \author Andrey Semashev
 * \date   08.01.2008
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_SINKS_SYSLOG_CONSTANTS_HPP_INCLUDED_HPP_
#define BOOST_LOG_SINKS_SYSLOG_CONSTANTS_HPP_INCLUDED_HPP_

#include <boost/log/detail/prologue.hpp>
#include <boost/log/detail/tagged_integer.hpp>

#ifdef _MSC_VER
#pragma warning(push)
// 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
// non dll-interface class 'A' used as base for dll-interface class 'B'
#pragma warning(disable: 4275)
#endif // _MSC_VER

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace sinks {

namespace syslog {

    struct options_tag;
    typedef log::aux::tagged_integer< int, options_tag > options_t;
    inline options_t make_options(int opt)
    {
        options_t options = { opt };
        return options;
    }

    struct level_tag;
    typedef log::aux::tagged_integer< int, level_tag > level_t;
    inline level_t make_level(int lev)
    {
        level_t level = { lev };
        return level;
    }

    struct facility_tag;
    typedef log::aux::tagged_integer< int, facility_tag > facility_t;
    inline facility_t make_facility(int fac)
    {
        facility_t facility = { fac };
        return facility;
    }

    //  Syslog initialization options
    BOOST_LOG_EXPORT extern const options_t console_fallback;       // LOG_CONS
    BOOST_LOG_EXPORT extern const options_t no_delay;               // LOG_NDELAY
    BOOST_LOG_EXPORT extern const options_t no_wait;                // LOG_NOWAIT
    BOOST_LOG_EXPORT extern const options_t print_stderr;           // LOG_PERROR
    BOOST_LOG_EXPORT extern const options_t log_pid;                // LOG_PID

    //  Syslog record levels
    BOOST_LOG_EXPORT extern const level_t emergency;                // LOG_EMERG
    BOOST_LOG_EXPORT extern const level_t alert;                    // LOG_ALERT
    BOOST_LOG_EXPORT extern const level_t critical;                 // LOG_CRIT
    BOOST_LOG_EXPORT extern const level_t error;                    // LOG_ERROR
    BOOST_LOG_EXPORT extern const level_t warning;                  // LOG_WARNING
    BOOST_LOG_EXPORT extern const level_t notice;                   // LOG_NOTICE
    BOOST_LOG_EXPORT extern const level_t info;                     // LOG_INFO
    BOOST_LOG_EXPORT extern const level_t debug;                    // LOG_DEBUG

    //  Syslog facility codes
    BOOST_LOG_EXPORT extern const facility_t daemon;                // LOG_DAEMON (LOG_USER if not supported)
    BOOST_LOG_EXPORT extern const facility_t user;                  // LOG_USER
    BOOST_LOG_EXPORT extern const facility_t mail;                  // LOG_MAIL (LOG_USER if not supported)
    BOOST_LOG_EXPORT extern const facility_t news;                  // LOG_NEWS (LOG_USER if not supported)
    BOOST_LOG_EXPORT extern const facility_t local0;                // LOG_LOCAL0
    BOOST_LOG_EXPORT extern const facility_t local1;                // LOG_LOCAL1
    BOOST_LOG_EXPORT extern const facility_t local2;                // LOG_LOCAL2
    BOOST_LOG_EXPORT extern const facility_t local3;                // LOG_LOCAL3
    BOOST_LOG_EXPORT extern const facility_t local4;                // LOG_LOCAL4
    BOOST_LOG_EXPORT extern const facility_t local5;                // LOG_LOCAL5
    BOOST_LOG_EXPORT extern const facility_t local6;                // LOG_LOCAL6
    BOOST_LOG_EXPORT extern const facility_t local7;                // LOG_LOCAL7

} // namespace syslog

} // namespace sinks

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_SINKS_SYSLOG_CONSTANTS_HPP_INCLUDED_HPP_
