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
 * \file   common_attributes.hpp
 * \author Andrey Semashev
 * \date   16.05.2008
 * 
 * The header contains implementation of convenience functions for registering commonly used attributes.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_UTILITY_INIT_COMMON_ATTRIBUTES_HPP_INCLUDED_
#define BOOST_LOG_UTILITY_INIT_COMMON_ATTRIBUTES_HPP_INCLUDED_

#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/logging_core.hpp>
#include <boost/log/attributes/clock.hpp>
#include <boost/log/attributes/counter.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace aux {

template< typename > struct add_common_attributes_constants;
template< >
struct add_common_attributes_constants< char >
{
    static const char* line_id_attr_name() { return "LineID"; }
    static const char* time_stamp_attr_name() { return "TimeStamp"; }
};
template< >
struct add_common_attributes_constants< wchar_t >
{
    static const wchar_t* line_id_attr_name() { return L"LineID"; }
    static const wchar_t* time_stamp_attr_name() { return L"TimeStamp"; }
};

//! The function adds commonly used attributes to the logging system
template< typename CharT >
void add_common_attributes()
{
    typedef add_common_attributes_constants< CharT > traits_t;
    shared_ptr< basic_logging_core< CharT > > pCore = basic_logging_core< CharT >::get();
    pCore->add_global_attribute(
        traits_t::line_id_attr_name(),
        boost::make_shared< attributes::counter< unsigned int > >(1));
    pCore->add_global_attribute(
        traits_t::time_stamp_attr_name(),
        boost::make_shared< attributes::local_clock >());
}

} // namespace aux

//! The function adds commonly used attributes to the logging system
inline void add_common_attributes()
{
    aux::add_common_attributes< char >();
}

//! The function adds commonly used attributes to the logging system
inline void wadd_common_attributes()
{
    aux::add_common_attributes< wchar_t >();
}

} // namespace log

} // namespace boost

#endif // BOOST_LOG_UTILITY_INIT_COMMON_ATTRIBUTES_HPP_INCLUDED_
