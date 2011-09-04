/*
 *          Copyright Andrey Semashev 2007 - 2011.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   sinks/frontend_requirements.hpp
 * \author Andrey Semashev
 * \date   22.04.2007
 *
 * The header contains definition of requirement tags that sink backend may declare
 * with regard to frontends. These requirements ensure that backend will not
 * be combined with an incompatible frontend.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_SINKS_FRONTEND_REQUIREMENTS_HPP_INCLUDED_
#define BOOST_LOG_SINKS_FRONTEND_REQUIREMENTS_HPP_INCLUDED_

#include <boost/type_traits/is_base_of.hpp>
#include <boost/log/detail/prologue.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace sinks {

#if !defined(BOOST_LOG_NO_THREADS)

/*!
 * The sink backend expects pre-synchronized calls, all needed synchronization is implemented
 * in the frontend (IOW, only one thread is feeding records to the backend concurrently, but
 * is is possible for several threads to write sequentially).
 */
struct syncronized_feeding {};
/*!
 * The sink backend ensures all needed synchronization, it is capable to handle multithreaded calls
 */
struct concurrent_feeding : syncronized_feeding {};

#else // !defined(BOOST_LOG_NO_THREADS)

//  If multithreading is disabled, threading models become redundant
struct syncronized_feeding {};
typedef syncronized_feeding concurrent_feeding;

#endif // !defined(BOOST_LOG_NO_THREADS)

/*!
 * The sink backend requires the frontend to perform log record formatting before feeding
 */
struct formatted_records {};

//! A helper metafunction to check if a requirement is satisfied
template< typename TestedT, typename RequiredT >
struct is_requirement_satisfied :
    public is_base_of< RequiredT, TestedT >
{
};

} // namespace sinks

} // namespace log

} // namespace boost

#endif // BOOST_LOG_SINKS_FRONTEND_REQUIREMENTS_HPP_INCLUDED_
