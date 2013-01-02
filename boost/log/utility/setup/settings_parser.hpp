/*
 *          Copyright Andrey Semashev 2007 - 2013.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   settings_parser.hpp
 * \author Andrey Semashev
 * \date   20.07.2012
 *
 * The header contains definition of a settings parser function.
 */

#ifndef BOOST_LOG_UTILITY_SETUP_SETTINGS_PARSER_HPP_INCLUDED_
#define BOOST_LOG_UTILITY_SETUP_SETTINGS_PARSER_HPP_INCLUDED_

#include <iosfwd>
#include <boost/log/detail/setup_prologue.hpp>
#include <boost/log/utility/setup/settings.hpp>

#ifdef BOOST_LOG_HAS_PRAGMA_ONCE
#pragma once
#endif

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
 * The function parses library settings from an input stream
 *
 * \param strm Stream, that provides library settings
 *
 * \b Throws: An <tt>std::exception</tt>-based exception if the read data cannot be interpreted as the library settings
 */
template< typename CharT >
BOOST_LOG_SETUP_API basic_settings< CharT > parse_settings(std::basic_istream< CharT >& strm);

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_UTILITY_SETUP_SETTINGS_PARSER_HPP_INCLUDED_
