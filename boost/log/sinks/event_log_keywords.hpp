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
 * \file   event_log_keywords.hpp
 * \author Andrey Semashev
 * \date   16.11.2008
 *
 * The header contains definition of keywords that are used with Windows event log sink backends
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_SINKS_EVENT_LOG_KEYWORDS_HPP_INCLUDED_
#define BOOST_LOG_SINKS_EVENT_LOG_KEYWORDS_HPP_INCLUDED_

#include <boost/parameter/keyword.hpp>
#include <boost/log/detail/prologue.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace sinks {

namespace keywords {

#ifndef BOOST_LOG_DOXYGEN_PASS

    BOOST_PARAMETER_KEYWORD(tag, log_name)
    BOOST_PARAMETER_KEYWORD(tag, log_source)
    BOOST_PARAMETER_KEYWORD(tag, force)

#else // BOOST_LOG_DOXYGEN_PASS

    //! The keyword is used to pass event log name to the backend constructor
    implementation_defined log_name;
    //! The keyword is used to pass event log source name to the backend constructor
    implementation_defined log_source;
    //! The keyword is used to pass the flag to override log source registration, if there already is one
    implementation_defined force;

#endif // BOOST_LOG_DOXYGEN_PASS

} // namespace keywords

} // namespace sinks

} // namespace log

} // namespace boost

#endif // BOOST_LOG_SINKS_EVENT_LOG_KEYWORDS_HPP_INCLUDED_
