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
 * \file   to_console.hpp
 * \author Andrey Semashev
 * \date   16.05.2008
 * 
 * The header contains implementation of convenience functions for enabling logging to console.
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

//! \cond
#ifndef BOOST_LOG_NO_THREADS
#define BOOST_LOG_CONSOLE_SINK_FRONTEND sinks::synchronous_sink
#else
#define BOOST_LOG_CONSOLE_SINK_FRONTEND sinks::unlocked_sink
#endif
//! \endcond

namespace boost {

namespace BOOST_LOG_NAMESPACE {

/*!
 * The function constructs sink for the specified console stream and adds it to the core
 * 
 * \param strm One of the standard console streams: <tt>std::cout</tt>, <tt>std::cerr</tt> or <tt>std::clog</tt>
 *             (or the corresponding wide-character analogues).
 * \return Pointer to the constructed sink.
 */
template< typename CharT >
shared_ptr<
    BOOST_LOG_CONSOLE_SINK_FRONTEND<
        sinks::basic_text_ostream_backend< CharT >
    >
> init_log_to_console(std::basic_ostream< CharT >& strm)
{
    shared_ptr< std::basic_ostream< CharT > > pStream(&strm, empty_deleter());

    typedef sinks::basic_text_ostream_backend< CharT > backend_t;
    shared_ptr< backend_t > pBackend = boost::make_shared< backend_t >();
    pBackend->add_stream(pStream);

    shared_ptr< BOOST_LOG_CONSOLE_SINK_FRONTEND< backend_t > > pSink =
        boost::make_shared< BOOST_LOG_CONSOLE_SINK_FRONTEND< backend_t > >(pBackend);
    basic_logging_core< CharT >::get()->add_sink(pSink);

    return pSink;
}

/*!
 * The function constructs sink for the <tt>std::clog</tt> stream and adds it to the core
 * 
 * \return Pointer to the constructed sink.
 */
inline shared_ptr<
    BOOST_LOG_CONSOLE_SINK_FRONTEND<
        sinks::text_ostream_backend
    >
> init_log_to_console()
{
    return init_log_to_console(std::clog);
}

/*!
 * The function constructs sink for the <tt>std::wclog</tt> stream and adds it to the core
 * 
 * \return Pointer to the constructed sink.
 */
inline shared_ptr<
    BOOST_LOG_CONSOLE_SINK_FRONTEND<
        sinks::wtext_ostream_backend
    >
> winit_log_to_console()
{
    return init_log_to_console(std::wclog);
}

} // namespace log

} // namespace boost

#undef BOOST_LOG_CONSOLE_SINK_FRONTEND

#endif // BOOST_LOG_UTILITY_INIT_TO_CONSOLE_HPP_INCLUDED_
