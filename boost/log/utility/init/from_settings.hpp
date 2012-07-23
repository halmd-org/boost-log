/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   from_settings.hpp
 * \author Andrey Semashev
 * \date   11.10.2009
 *
 * The header contains definition of facilities that allows to initialize the library from
 * settings.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_UTILITY_INIT_FROM_SETTINGS_HPP_INCLUDED_
#define BOOST_LOG_UTILITY_INIT_FROM_SETTINGS_HPP_INCLUDED_

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/log/detail/setup_prologue.hpp>
#include <boost/log/detail/light_function.hpp>
#include <boost/log/sinks/sink.hpp>
#include <boost/log/utility/init/settings.hpp>

#ifdef _MSC_VER
#pragma warning(push)
// 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
// non dll-interface class 'A' used as base for dll-interface class 'B'
#pragma warning(disable: 4275)
#endif // _MSC_VER

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

/*!
 * The function initializes the logging library from a settings container
 *
 * \param setts Library settings container
 *
 * \b Throws: An <tt>std::exception</tt>-based exception if the provided settings are not valid.
 */
template< typename CharT >
BOOST_LOG_SETUP_API void init_from_settings(basic_settings_section< CharT > const& setts);

#if !defined(BOOST_NO_TEMPLATE_ALIASES) && !defined(BOOST_NO_CXX11_TEMPLATE_ALIASES)

template< typename CharT >
using basic_sink_factory = boost::log::aux::light_function1< shared_ptr< sinks::sink >, basic_settings_section< CharT > const& >;

typedef basic_sink_factory< char > sink_factory;
typedef basic_sink_factory< wchar_t > wsink_factory;

#else

typedef boost::log::aux::light_function1< shared_ptr< sinks::sink >, basic_settings_section< char > const& > sink_factory;
typedef boost::log::aux::light_function1< shared_ptr< sinks::sink >, basic_settings_section< wchar_t > const& > wsink_factory;

#endif


/*!
 * \brief The function registers a factory for a custom sink
 *
 * The function registers a factory for a sink. The factory will be called to create sink
 * instance when the parser discovers the specified sink type in the settings file. The
 * factory must accept a map of parameters [parameter name -> parameter value] that it
 * may use to initialize the sink. The factory must return a non-NULL pointer to the
 * constructed sink instance.
 *
 * \param sink_name The custom sink name. Must point to a zero-terminated sequence of characters,
 *                  must not be NULL.
 * \param factory Custom sink factory function
 */
template< typename CharT >
BOOST_LOG_SETUP_API void register_sink_factory(
    const char* sink_name,
    boost::log::aux::light_function1< shared_ptr< sinks::sink >, basic_settings_section< CharT > const& > const& factory);

/*!
 * \brief The function registers a factory for a custom sink
 *
 * The function registers a factory for a sink. The factory will be called to create sink
 * instance when the parser discovers the specified sink type in the settings file. The
 * factory must accept a map of parameters [parameter name -> parameter value] that it
 * may use to initialize the sink. The factory must return a non-NULL pointer to the
 * constructed sink instance.
 *
 * \param sink_name The custom sink name
 * \param factory Custom sink factory function
 */
template< typename CharT >
inline void register_sink_factory(
    std::string const& sink_name,
    boost::log::aux::light_function1< shared_ptr< sinks::sink >, basic_settings_section< CharT > const& > const& factory)
{
    register_sink_factory(sink_name.c_str(), factory);
}

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_UTILITY_INIT_FROM_SETTINGS_HPP_INCLUDED_
