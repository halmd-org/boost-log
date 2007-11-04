/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   threading_models.hpp
 * \author Andrey Semashev
 * \date   22.04.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_SINKS_THREADING_MODELS_HPP_INCLUDED_
#define BOOST_LOG_SINKS_THREADING_MODELS_HPP_INCLUDED_

#include <boost/type_traits/is_base_of.hpp>
#include <boost/log/detail/prologue.hpp>

namespace boost {

namespace log {

namespace sinks {

//! Base tag for all other tags
struct threading_model_tag {};

//! The sink backend ensures all needed synchronization, it is capable to handle multithreaded calls
struct backend_synchronization_tag : threading_model_tag {};
//! The sink backend requires to be called in a single thread (IOW, no other threads EVER are allowed to write to the backend)
struct single_thread_tag : threading_model_tag {};
//! The sink backend expects pre-synchronized calls, all needed synchronization is implemented in the frontend (IOW, only one thread is writing to the backend concurrently, but is is possible for several threads to write sequentially) 
struct frontend_synchronization_tag : threading_model_tag {};

//! A helper metafunction to check if a therading model is supported
template< typename TestedT, typename RequiredT >
struct is_model_supported :
    public is_base_of< RequiredT, TestedT >
{
};

} // namespace sinks

} // namespace log

} // namespace boost

#endif // BOOST_LOG_SINKS_THREADING_MODELS_HPP_INCLUDED_
