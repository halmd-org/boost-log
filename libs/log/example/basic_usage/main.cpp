/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   main.cpp
 * \author Andrey Semashev
 * \date   11.11.2007
 * 
 * \brief  An example of basic library usage. See the library tutorial for expanded
 *         comments on this code. It may also be worthwhile reading the Wiki requirements page:
 *         http://www.crystalclearsoftware.com/cgi-bin/boost_wiki/wiki.pl?Boost.Logging
 */

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif // _CRT_SECURE_NO_WARNINGS

// #define BOOST_LOG_USE_CHAR
// #define BOOST_ALL_DYN_LINK 1
// #define BOOST_LOG_DYN_LINK 1

#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <boost/log/common.hpp>

#include <boost/log/utility/init/to_file.hpp>
#include <boost/log/utility/init/to_console.hpp>
#include <boost/log/utility/init/common_attributes.hpp>

#include <boost/log/formatters/format.hpp>
#include <boost/log/formatters/attr.hpp>
#include <boost/log/formatters/date_time.hpp>
#include <boost/log/formatters/message.hpp>

#include <boost/log/attributes/counter.hpp>
#include <boost/log/attributes/constant.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/attributes/clock.hpp>
#include <boost/log/attributes/timer.hpp>

#include <boost/log/filters/attr.hpp>
#include <boost/log/filters/has_attr.hpp>

namespace logging = boost::log;
namespace fmt = boost::log::formatters;
namespace flt = boost::log::filters;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
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

int main(int argc, char* argv[])
{
    // This is a simple tutorial/example of Boost.Log usage

    // The first thing we have to do to get using the library is
    // to set up the logging sinks - i.e. where the logs will be written to.
    logging::init_log_to_file("sample.log");
    logging::init_log_to_console();

    // Also let's add some commonly used attributes, like timestamp and record counter.
    logging::add_common_attributes();

    // Now our logs will be written both to the console and to the file.
    // Let's do a quick test and output something. We have to create a logger for this.
    src::logger lg;

    // Let's pretend we also want to profile our code, so add a special timer attribute.
    lg.add_attribute("Uptime", boost::make_shared< attrs::timer >());

    // And output...
    BOOST_LOG(lg) << "Hello, World!";


    // Now, to set logging severity we could perfectly use our previously created logger "lg".
    // But no make it more convenient and efficient there is a special extended logger class.
    // Its implementation may serve as an example of extending basic library functionality.
    // You may add your specific capabilities to the logger by deriving your class from it.
    src::severity_logger< severity_level > slg;

    // These two lines test filtering based on severity
    BOOST_LOG_SEV(slg, normal) << "A normal severity message, will not pass to the output";
    BOOST_LOG_SEV(slg, error) << "An error severity message, will pass to the output";

    {
        // Next we try if the second condition of the filter works
        // We mark following lines with a tag
        BOOST_LOG_SCOPED_THREAD_TAG("Tag", std::string, "IMPORTANT MESSAGES");

        // We may omit the severity and use the shorter BOOST_LOG macro. The logger "slg"
        // has the default severity that may be specified on its construction. We didn't
        // do it, so it is 0 by default. Therefore this record will have "normal" severity.
        // The only reason this record will be output is the "Tag" attribute we added above.
        BOOST_LOG(slg) << "Some really urgent line";
    }

    pSink->reset_filter();

    // And moreover, it is possible to nest logging records. For example, this will
    // be processed in the order of evaluation:
    BOOST_LOG(lg) << "The result of foo is " << foo(lg);

    return 0;
}
