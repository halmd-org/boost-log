/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   wrappers.hpp
 * \author Andrey Semashev
 * \date   22.04.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_FORMATTERS_WRAPPERS_HPP_INCLUDED_
#define BOOST_LOG_FORMATTERS_WRAPPERS_HPP_INCLUDED_

#include <boost/ref.hpp>
#include <boost/mpl/not.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/formatters/basic_formatters.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace formatters {

//! Formatter fmt_wrapper to output objects into streams
template< typename CharT, typename T >
class fmt_wrapper :
    public basic_formatter< CharT, fmt_wrapper< CharT, T > >
{
private:
    //! Base type
    typedef basic_formatter< CharT, fmt_wrapper< CharT, T > > base_type;

public:
    //! String type
    typedef typename base_type::string_type string_type;
    //! Stream type
    typedef typename base_type::ostream_type ostream_type;
    //! Attribute values set type
    typedef typename base_type::values_view_type values_view_type;

private:
    //! Object to be output
    T m_T;

public:
    //! Constructor
    explicit fmt_wrapper(T const& obj) : m_T(obj) {}

    //! Output operator
    void operator() (ostream_type& strm, values_view_type const&, string_type const&) const
    {
        strm << m_T;
    }
};

//! Specialization to put objects into streams by reference
template< typename CharT, typename T >
class fmt_wrapper< CharT, reference_wrapper< T > > :
    public basic_formatter< CharT, fmt_wrapper< CharT, reference_wrapper< T > > >
{
private:
    //! Base type
    typedef basic_formatter< CharT, fmt_wrapper< CharT, reference_wrapper< T > > > base_type;

public:
    //! String type
    typedef typename base_type::string_type string_type;
    //! Stream type
    typedef typename base_type::ostream_type ostream_type;
    //! Attribute values set type
    typedef typename base_type::values_view_type values_view_type;

private:
    //! Reference to object to be output
    T& m_T;

public:
    //! Constructor
    explicit fmt_wrapper(reference_wrapper< T > const& obj) : m_T(obj.get()) {}

    //! Output operator
    void operator() (ostream_type& strm, values_view_type const&, string_type const&) const
    {
        strm << m_T;
    }
};

//! A convenience class that conditionally wraps the type into a formatter
template< typename CharT, typename T, bool >
struct wrap_if_c
{
    fmt_wrapper< CharT, T > type;
};

template< typename CharT, typename T >
struct wrap_if_c< CharT, T, false >
{
    typedef T type;
};

template< typename CharT, typename T, typename PredT >
struct wrap_if :
    public wrap_if_c< CharT, T, PredT::value >
{
};

template< typename CharT, typename T >
struct wrap_if_not_formatter :
    public wrap_if< CharT, T, mpl::not_< is_formatter< T > > >
{
};

} // namespace formatters

} // namespace log

} // namespace boost

#endif // BOOST_LOG_FORMATTERS_WRAPPERS_HPP_INCLUDED_
