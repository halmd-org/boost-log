/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */

#include <string>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/unlocked_frontend.hpp>
#include <boost/log/sinks/basic_sink_backend.hpp>
#include <boost/log/sinks/frontend_requirements.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

//[ example_sinks_unlocked
// A trivial sink backend that requires no thread synchronization
class my_backend :
    public sinks::basic_sink_backend< sinks::concurrent_feeding >
{
public:
    // The function is called for every log record to be written to log
    void consume(logging::record const& rec)
    {
        // We skip the actual synchronization code for brevity
        std::cout << rec[expr::smessage] << std::endl;
    }
};

// Complete sink type
typedef sinks::unlocked_sink< my_backend > sink_t;

void init_logging()
{
    boost::shared_ptr< logging::core > core = logging::core::get();

    // The simplest way, the backend is default-constructed
    boost::shared_ptr< sink_t > sink1(new sink_t());
    core->add_sink(sink1);

    // One can construct backend separately and pass it to the frontend
    boost::shared_ptr< my_backend > backend(new my_backend());
    boost::shared_ptr< sink_t > sink2(new sink_t(backend));
    core->add_sink(sink2);

    // You can manage filtering through the sink interface
    sink1->set_filter(expr::attr< int >("Severity") >= 2);
    sink2->set_filter(expr::attr< std::string >("Channel") == "net");
}
//]

int main(int, char*[])
{
    init_logging();

    src::severity_channel_logger< > lg(keywords::channel = "net");
    BOOST_LOG_SEV(lg, 0) << "Hello world!";

    return 0;
}
