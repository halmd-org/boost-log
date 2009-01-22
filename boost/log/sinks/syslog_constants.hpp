/*
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * This header is the Boost.Log library implementation, see the library documentation
 * at http://www.boost.org/libs/log/doc/log.html.
 */
/*!
 * \file   syslog_constants.hpp
 * \author Andrey Semashev
 * \date   08.01.2008
 * 
 * The header contains definition of constants related to Syslog API. The constants can be
 * used in other places without the Syslog backend.
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
    //! A tagged integal type that represents set of option flags for the syslog API initialization
    typedef boost::log::aux::tagged_integer< int, options_tag > options_t;
    /*!
     * The function constructs options from an integer
     */
    inline options_t make_options(int opt)
    {
        options_t options = { opt };
        return options;
    }

    struct level_tag;
    //! A tagged integal type that represents log record level for the syslog API
    typedef boost::log::aux::tagged_integer< int, level_tag > level_t;
    /*!
     * The function constructs log record level from an integer
     */
    inline level_t make_level(int lev)
    {
        level_t level = { lev };
        return level;
    }

    struct facility_tag;
    //! A tagged integal type that represents log source facility for the syslog API
    typedef boost::log::aux::tagged_integer< int, facility_tag > facility_t;
    /*!
     * The function constructs log source facility from an integer
     */
    inline facility_t make_facility(int fac)
    {
        facility_t facility = { fac };
        return facility;
    }

    //  Syslog initialization options
    BOOST_LOG_EXPORT extern const options_t console_fallback;       //!< Equivalent to LOG_CONS in syslog API
    BOOST_LOG_EXPORT extern const options_t no_delay;               //!< Equivalent to LOG_NDELAY in syslog API
    BOOST_LOG_EXPORT extern const options_t no_wait;                //!< Equivalent to LOG_NOWAIT in syslog API
    BOOST_LOG_EXPORT extern const options_t print_stderr;           //!< Equivalent to LOG_PERROR in syslog API
    BOOST_LOG_EXPORT extern const options_t log_pid;                //!< Equivalent to LOG_PID in syslog API

    //  Syslog record levels
    BOOST_LOG_EXPORT extern const level_t emergency;                //!< Equivalent to LOG_EMERG in syslog API
    BOOST_LOG_EXPORT extern const level_t alert;                    //!< Equivalent to LOG_ALERT in syslog API
    BOOST_LOG_EXPORT extern const level_t critical;                 //!< Equivalent to LOG_CRIT in syslog API
    BOOST_LOG_EXPORT extern const level_t error;                    //!< Equivalent to LOG_ERROR in syslog API
    BOOST_LOG_EXPORT extern const level_t warning;                  //!< Equivalent to LOG_WARNING in syslog API
    BOOST_LOG_EXPORT extern const level_t notice;                   //!< Equivalent to LOG_NOTICE in syslog API
    BOOST_LOG_EXPORT extern const level_t info;                     //!< Equivalent to LOG_INFO in syslog API
    BOOST_LOG_EXPORT extern const level_t debug;                    //!< Equivalent to LOG_DEBUG in syslog API

    //  Syslog facility codes
    BOOST_LOG_EXPORT extern const facility_t daemon;                //!< Equivalent to LOG_DAEMON (LOG_USER if not supported) in syslog API
    BOOST_LOG_EXPORT extern const facility_t user;                  //!< Equivalent to LOG_USER in syslog API
    BOOST_LOG_EXPORT extern const facility_t mail;                  //!< Equivalent to LOG_MAIL (LOG_USER if not supported) in syslog API
    BOOST_LOG_EXPORT extern const facility_t news;                  //!< Equivalent to LOG_NEWS (LOG_USER if not supported) in syslog API
    BOOST_LOG_EXPORT extern const facility_t local0;                //!< Equivalent to LOG_LOCAL0 in syslog API
    BOOST_LOG_EXPORT extern const facility_t local1;                //!< Equivalent to LOG_LOCAL1 in syslog API
    BOOST_LOG_EXPORT extern const facility_t local2;                //!< Equivalent to LOG_LOCAL2 in syslog API
    BOOST_LOG_EXPORT extern const facility_t local3;                //!< Equivalent to LOG_LOCAL3 in syslog API
    BOOST_LOG_EXPORT extern const facility_t local4;                //!< Equivalent to LOG_LOCAL4 in syslog API
    BOOST_LOG_EXPORT extern const facility_t local5;                //!< Equivalent to LOG_LOCAL5 in syslog API
    BOOST_LOG_EXPORT extern const facility_t local6;                //!< Equivalent to LOG_LOCAL6 in syslog API
    BOOST_LOG_EXPORT extern const facility_t local7;                //!< Equivalent to LOG_LOCAL7 in syslog API

} // namespace syslog

} // namespace sinks

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_SINKS_SYSLOG_CONSTANTS_HPP_INCLUDED_HPP_
