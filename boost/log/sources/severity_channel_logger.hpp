/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   severity_channel_logger.hpp
 * \author Andrey Semashev
 * \date   28.02.2008
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_SOURCES_SEVERITY_CHANNEL_LOGGER_HPP_INCLUDED_
#define BOOST_LOG_SOURCES_SEVERITY_CHANNEL_LOGGER_HPP_INCLUDED_

#include <boost/log/detail/prologue.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/channel_logger.hpp>

#ifdef _MSC_VER
#pragma warning(push)
// 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
#endif // _MSC_VER

namespace boost {

namespace log {

namespace sources {

#ifdef BOOST_LOG_USE_CHAR

//! Narrow-char logger with severity level and channel support
BOOST_LOG_DECLARE_LOGGER(severity_channel_logger, (basic_severity_logger)(basic_channel_logger));
//! Narrow-char thread-safe logger with severity level and channel support
BOOST_LOG_DECLARE_LOGGER_MT(severity_channel_logger_mt, (basic_severity_logger)(basic_channel_logger));

#endif

#ifdef BOOST_LOG_USE_WCHAR_T

//! Wide-char logger with severity level and channel support
BOOST_LOG_DECLARE_WLOGGER(wseverity_channel_logger, (basic_severity_logger)(basic_channel_logger));
//! Wide-char thread-safe logger with severity level and channel support
BOOST_LOG_DECLARE_WLOGGER_MT(wseverity_channel_logger_mt, (basic_severity_logger)(basic_channel_logger));

#endif

} // namespace sources

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_SOURCES_SEVERITY_CHANNEL_LOGGER_HPP_INCLUDED_
