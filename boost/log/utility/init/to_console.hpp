/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   to_console.hpp
 * \author Andrey Semashev
 * \date   16.05.2008
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_UTILITY_INIT_TO_CONSOLE_HPP_INCLUDED_
#define BOOST_LOG_UTILITY_INIT_TO_CONSOLE_HPP_INCLUDED_

#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/logging_core.hpp>
#include <boost/log/sinks/sink.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/utility/empty_deleter.hpp>

namespace boost {

namespace log {

//! The function constructs the sink and adds it to the core
template< typename CharT >
shared_ptr<
    sinks::synchronous_sink<
        sinks::basic_text_ostream_backend< CharT >
    >
> init_log_to_console(std::basic_ostream< CharT >& strm)
{
    shared_ptr< std::basic_ostream< CharT > > pStream(&strm, empty_deleter());

    typedef sinks::basic_text_ostream_backend< CharT > backend_t;
    shared_ptr< backend_t > pBackend = boost::make_shared< backend_t >();
    pBackend->add_stream(pStream);

    shared_ptr< sinks::synchronous_sink< backend_t > > pSink =
        boost::make_shared< sinks::synchronous_sink< backend_t > >(pBackend);
    basic_logging_core< CharT >::get()->add_sink(pSink);

    return pSink;
}

//! The function initializes the logging library to write logs to console
inline shared_ptr<
    sinks::synchronous_sink<
        sinks::text_ostream_backend
    >
> init_log_to_console()
{
    return init_log_to_console(std::clog);
}

//! The function initializes the logging library to write logs to wide console
inline shared_ptr<
    sinks::synchronous_sink<
        sinks::wtext_ostream_backend
    >
> winit_log_to_console()
{
    return init_log_to_console(std::wclog);
}

} // namespace log

} // namespace boost

#endif // BOOST_LOG_UTILITY_INIT_TO_CONSOLE_HPP_INCLUDED_
