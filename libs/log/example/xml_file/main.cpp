/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   main.cpp
 * \author Andrey Semashev
 * \date   26.04.2008
 * 
 * \brief  An example of logging into a rotating XML text file.
 *         See the library tutorial for expanded comments on this code.
 *         It may also be worthwhile reading the Wiki requirements page:
 *         http://www.crystalclearsoftware.com/cgi-bin/boost_wiki/wiki.pl?Boost.Logging
 */

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif // _CRT_SECURE_NO_WARNINGS

// #define BOOST_LOG_DYN_LINK 1

#include <stdexcept>
#include <string>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <boost/log/core.hpp>
#include <boost/log/attributes/clock.hpp>
#include <boost/log/attributes/counter.hpp>
#include <boost/log/sources/basic_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sinks/sink.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/formatters/format.hpp>
#include <boost/log/formatters/attr.hpp>
#include <boost/log/formatters/date_time.hpp>
#include <boost/log/formatters/message.hpp>
#include <boost/log/formatters/xml_decorator.hpp>

namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace fmt = boost::log::formatters;
namespace keywords = boost::log::keywords;

using namespace boost::lambda;
using boost::shared_ptr;

enum { LOG_RECORDS_TO_WRITE = 2000 };

int main(int argc, char* argv[])
{
    try
    {
        // Create a text file sink
        typedef sinks::synchronous_sink< sinks::text_file_backend > file_sink;
        shared_ptr< file_sink > sink(new file_sink(
            keywords::file_name = "temp.log",       // temporary file name
            keywords::rotation_size = 16384         // rotation size, in characters
            ));

        // Set up where the rotated files will be stored
        sink->locked_backend()->file_collector(sinks::file::fifo_collector(
            keywords::file_name = "logs/%Y%m%d_%H%M%S_%5N.xml", // the resulting file name pattern
            keywords::max_size = 16 * 1024 * 1024,              // maximum total size of the stored files, in bytes
            keywords::min_free_space = 100 * 1024 * 1024,       // minimum free space on the drive, in bytes
            keywords::scan_method = sinks::file::scan_matching        // upon restart, scan the directory for files matching the file_name pattern
            ));

        sink->locked_backend()->set_formatter(
            fmt::format("\t<record id=\"%1%\" timestamp=\"%2%\">%3%</record>")
                % fmt::attr< unsigned int >("Line #")
                % fmt::date_time< boost::posix_time::ptime >("TimeStamp")
                % fmt::xml_dec[ fmt::message() ]                // The log message has to be decorated, if it contains special characters
            );

        // Set header and footer writing functors
        sink->locked_backend()->open_handler(_1 << "<?xml version=\"1.0\"?>\n<log>\n");
        sink->locked_backend()->close_handler(_1 << "</log>\n");

        // Add the sink to the core
        logging::core::get()->add_sink(sink);

        // Add some attributes too
        shared_ptr< logging::attribute > attr(new attrs::local_clock);
        logging::core::get()->add_global_attribute("TimeStamp", attr);
        attr.reset(new attrs::counter< unsigned int >);
        logging::core::get()->add_global_attribute("Line #", attr);

        // Do some logging
        src::logger lg;
        for (unsigned int i = 0; i < LOG_RECORDS_TO_WRITE; ++i)
        {
            BOOST_LOG(lg) << "XML log record " << i;
        }

        // Test that XML character decoration works
        BOOST_LOG(lg) << "Special XML characters: &, <, >, '";

        return 0;
    }
    catch (std::exception& e)
    {
        std::cout << "FAILURE: " << e.what() << std::endl;
        return 1;
    }
}
