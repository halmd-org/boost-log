/*
 *          Copyright Andrey Semashev 2007 - 2013.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   main.cpp
 * \author Andrey Semashev
 * \date   01.12.2012
 *
 * \brief  An example of wide character logging.
 */

// #define BOOST_LOG_USE_CHAR
// #define BOOST_ALL_DYN_LINK 1
// #define BOOST_LOG_DYN_LINK 1

#include <iostream>

#include <boost/locale/generator.hpp>

#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>

#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include <boost/log/attributes/timer.hpp>
#include <boost/log/attributes/named_scope.hpp>

#include <boost/log/sources/logger.hpp>

#include <boost/log/support/date_time.hpp>

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

using boost::shared_ptr;

// Here we define our application severity levels.
enum severity_level
{
    normal,
    notification,
    warning,
    error,
    critical
};

// The formatting logic for the severity level
template< typename CharT, typename TraitsT >
inline std::basic_ostream< CharT, TraitsT >& operator<< (
    std::basic_ostream< CharT, TraitsT >& strm, severity_level lvl)
{
    static const char* const str[] =
    {
        "normal",
        "notification",
        "warning",
        "error",
        "critical"
    };
    if (static_cast< std::size_t >(lvl) < (sizeof(str) / sizeof(*str)))
        strm << str[lvl];
    else
        strm << static_cast< int >(lvl);
    return strm;
}

// Declare attribute keywords
BOOST_LOG_ATTRIBUTE_KEYWORD(_severity, "Severity", severity_level)
BOOST_LOG_ATTRIBUTE_KEYWORD(_timestamp, "TimeStamp", boost::posix_time::ptime)
BOOST_LOG_ATTRIBUTE_KEYWORD(_uptime, "Uptime", attrs::timer::value_type)
BOOST_LOG_ATTRIBUTE_KEYWORD(_scope, "Scope", attrs::named_scope::value_type)

int main(int argc, char* argv[])
{
    // Setup the locale for character code conversion
    std::locale loc = boost::locale::generator()("en_US.UTF-8");

    // The sink will perform character code conversion as needed, according to the locale set with imbue()
    logging::add_file_log
    (
        "sample.log",
        keywords::format = expr::stream
            << expr::format_date_time(_timestamp, "%Y-%m-%d, %H:%M:%S.%f")
            << " [" << expr::format_date_time(_uptime, "%O:%M:%S")
            << "] [" << expr::format_named_scope(_scope, keywords::format = "%n (%f:%l)")
            << "] <" << _severity.or_default(normal)
            << "> " << expr::message
    )->imbue(loc);

    // Let's add some commonly used attributes, like timestamp and record counter.
    logging::add_common_attributes();
    logging::core::get()->add_thread_attribute("Scope", attrs::named_scope());

    BOOST_LOG_FUNCTION();

    // Now our logs will be written both to the console and to the file.
    // Let's do a quick test and output something. We have to create a logger for this.
    src::wlogger lg;

    // And output...
    BOOST_LOG(lg) << L"Hello, World!";

    const wchar_t national_chars[] = { 0x041f, 0x0440, 0x0438, 0x0432, 0x0435, 0x0442, L',', L' ', 0x043c, 0x0438, 0x0440, L'!', 0 };
    BOOST_LOG(lg) << national_chars;

    // Now, let's try logging with severity
    src::wseverity_logger< severity_level > slg;

    // Let's pretend we also want to profile our code, so add a special timer attribute.
    slg.add_attribute("Uptime", attrs::timer());

    BOOST_LOG_SEV(slg, normal) << L"A normal severity message, will not pass to the file";
    BOOST_LOG_SEV(slg, warning) << L"A warning severity message, will pass to the file";
    BOOST_LOG_SEV(slg, error) << L"An error severity message, will pass to the file";

    return 0;
}
