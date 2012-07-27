/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   debug_output_backend.hpp
 * \author Andrey Semashev
 * \date   07.11.2008
 *
 * The header contains a logging sink backend that outputs log records to the debugger.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_SINKS_DEBUG_OUTPUT_BACKEND_HPP_INCLUDED_
#define BOOST_LOG_SINKS_DEBUG_OUTPUT_BACKEND_HPP_INCLUDED_

#include <string>
#include <boost/log/detail/prologue.hpp>

#ifndef BOOST_LOG_WITHOUT_DEBUG_OUTPUT

#include <boost/log/sinks/basic_sink_backend.hpp>
#include <boost/log/sinks/frontend_requirements.hpp>
#include <boost/log/attributes/attribute_values_view.hpp>
#include <boost/log/core/record.hpp>

#ifdef _MSC_VER
#pragma warning(push)
// 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
// non dll-interface class 'A' used as base for dll-interface class 'B'
#pragma warning(disable: 4275)
#endif // _MSC_VER

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace sinks {

/*!
 * \brief An implementation of a logging sink backend that outputs to the debugger
 *
 * The sink uses Windows API in order to write log records as debug messages, if the
 * application process is run under debugger. The sink backend also provides a specific
 * filter that allows to check whether the debugger is available and thus elide unnecessary
 * formatting.
 */
template< typename CharT >
class basic_debug_output_backend :
    public basic_formatted_sink_backend< CharT, concurrent_feeding >
{
    //! Base type
    typedef basic_formatted_sink_backend< CharT, concurrent_feeding > base_type;

public:
    //! Character type
    typedef typename base_type::char_type char_type;
    //! String type to be used as a message text holder
    typedef typename base_type::string_type string_type;

public:
#ifndef BOOST_LOG_DOXYGEN_PASS

    //! A filter that checks whether the debugger is available
    class debugger_presence_filter
    {
    public:
        BOOST_LOG_API bool operator() (attribute_values_view const& values) const;
    };

#endif // BOOST_LOG_DOXYGEN_PASS

public:
    /*!
     * Constructor. Initializes the sink backend.
     */
    BOOST_LOG_API basic_debug_output_backend();
    /*!
     * Destructor
     */
    BOOST_LOG_API ~basic_debug_output_backend();

    /*!
     * \return A filter that checks whether the debugger is available
     */
    BOOST_LOG_API debugger_presence_filter get_debugger_presence_filter() const;

    /*!
     * The method passes the formatted message to debugger
     */
    BOOST_LOG_API void consume(record const& rec, string_type const& formatted_message);
};

#ifdef BOOST_LOG_USE_CHAR
typedef basic_debug_output_backend< char > debug_output_backend;      //!< Convenience typedef for narrow-character logging
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
typedef basic_debug_output_backend< wchar_t > wdebug_output_backend;  //!< Convenience typedef for wide-character logging
#endif

} // namespace sinks

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_WITHOUT_DEBUG_OUTPUT

#endif // BOOST_LOG_SINKS_DEBUG_OUTPUT_BACKEND_HPP_INCLUDED_
