/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   to_file.hpp
 * \author Andrey Semashev
 * \date   16.05.2008
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_UTILITY_INIT_TO_FILE_HPP_INCLUDED_
#define BOOST_LOG_UTILITY_INIT_TO_FILE_HPP_INCLUDED_

#include <iostream>
#include <fstream>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/logging_core.hpp>
#include <boost/log/sinks/sink.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/utility/empty_deleter.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace aux {

//! The function constructs the sink and adds it to the core
template< typename CharT >
shared_ptr<
    sinks::synchronous_sink<
        sinks::basic_text_ostream_backend< CharT >
    >
> init_log_to_file(shared_ptr< std::basic_ostream< CharT > > const& strm)
{
    typedef sinks::basic_text_ostream_backend< CharT > backend_t;
    shared_ptr< backend_t > pBackend = boost::make_shared< backend_t >();
    pBackend->add_stream(strm);

    shared_ptr< sinks::synchronous_sink< backend_t > > pSink =
        boost::make_shared< sinks::synchronous_sink< backend_t > >(pBackend);
    basic_logging_core< CharT >::get()->add_sink(pSink);

    return pSink;
}

} // namespace aux

//! The function initializes the logging library to write logs to a file stream
inline shared_ptr<
    sinks::synchronous_sink<
        sinks::text_ostream_backend
    >
> init_log_to_file(const char* file_name)
{
    shared_ptr< std::ostream > p = boost::make_shared< std::ofstream >(
        file_name, std::ios_base::out | std::ios_base::trunc);
    return aux::init_log_to_file(p);
}

//! The function initializes the logging library to write logs to a file stream
inline shared_ptr<
    sinks::synchronous_sink<
        sinks::text_ostream_backend
    >
> init_log_to_file(std::string const& file_name)
{
    return init_log_to_file(file_name.c_str());
}

//! The function initializes the logging library to write logs to a file stream
inline shared_ptr<
    sinks::synchronous_sink<
        sinks::text_ostream_backend
    >
> init_log_to_file(boost::filesystem::path const& file_name)
{
    shared_ptr< std::ostream > p = boost::make_shared< boost::filesystem::ofstream >(
        file_name, std::ios_base::out | std::ios_base::trunc);
    return aux::init_log_to_file(p);
}

#ifndef BOOST_FILESYSTEM_NARROW_ONLY

//! The function initializes the logging library to write logs to a file stream
inline shared_ptr<
    sinks::synchronous_sink<
        sinks::text_ostream_backend
    >
> init_log_to_file(boost::filesystem::wpath const& file_name)
{
    shared_ptr< std::ostream > p = boost::make_shared< boost::filesystem::ofstream >(
        file_name, std::ios_base::out | std::ios_base::trunc);
    return aux::init_log_to_file(p);
}

//! The function initializes the logging library to write logs to a file stream
inline shared_ptr<
    sinks::synchronous_sink<
        sinks::text_ostream_backend
    >
> init_log_to_file(const wchar_t* file_name)
{
    boost::filesystem::wpath fname(file_name);
    return init_log_to_file(fname);
}

//! The function initializes the logging library to write logs to a file stream
inline shared_ptr<
    sinks::synchronous_sink<
        sinks::text_ostream_backend
    >
> init_log_to_file(std::wstring const& file_name)
{
    return init_log_to_file(file_name.c_str());
}

#endif // BOOST_FILESYSTEM_NARROW_ONLY

//! The function initializes the logging library to write logs to a wide file stream
inline shared_ptr<
    sinks::synchronous_sink<
        sinks::wtext_ostream_backend
    >
> winit_log_to_file(const char* file_name)
{
    shared_ptr< std::wostream > p = boost::make_shared< std::wofstream >(
        file_name, std::ios_base::out | std::ios_base::trunc);
    return aux::init_log_to_file(p);
}

//! The function initializes the logging library to write logs to a wide file stream
inline shared_ptr<
    sinks::synchronous_sink<
        sinks::wtext_ostream_backend
    >
> winit_log_to_file(std::string const& file_name)
{
    return winit_log_to_file(file_name.c_str());
}

//! The function initializes the logging library to write logs to a wide file stream
inline shared_ptr<
    sinks::synchronous_sink<
        sinks::wtext_ostream_backend
    >
> winit_log_to_file(boost::filesystem::path const& file_name)
{
    shared_ptr< std::wostream > p = boost::make_shared< boost::filesystem::wofstream >(
        file_name, std::ios_base::out | std::ios_base::trunc);
    return aux::init_log_to_file(p);
}

#ifndef BOOST_FILESYSTEM_NARROW_ONLY

//! The function initializes the logging library to write logs to a wide file stream
inline shared_ptr<
    sinks::synchronous_sink<
        sinks::wtext_ostream_backend
    >
> winit_log_to_file(boost::filesystem::wpath const& file_name)
{
    shared_ptr< std::wostream > p = boost::make_shared< boost::filesystem::wofstream >(
        file_name, std::ios_base::out | std::ios_base::trunc);
    return aux::init_log_to_file(p);
}

//! The function initializes the logging library to write logs to a wide file stream
inline shared_ptr<
    sinks::synchronous_sink<
        sinks::wtext_ostream_backend
    >
> winit_log_to_file(const wchar_t* file_name)
{
    boost::filesystem::wpath fname(file_name);
    return winit_log_to_file(fname);
}

//! The function initializes the logging library to write logs to a wide file stream
inline shared_ptr<
    sinks::synchronous_sink<
        sinks::wtext_ostream_backend
    >
> winit_log_to_file(std::wstring const& file_name)
{
    return winit_log_to_file(file_name.c_str());
}

#endif // BOOST_FILESYSTEM_NARROW_ONLY

} // namespace log

} // namespace boost

#endif // BOOST_LOG_UTILITY_INIT_TO_FILE_HPP_INCLUDED_
