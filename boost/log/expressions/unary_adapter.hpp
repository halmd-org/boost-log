/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   unary_adapter.hpp
 * \author Andrey Semashev
 * \date   21.07.2012
 *
 * The header contains attribute value extractor adapter for constructing expression template terminals.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_EXPRESSIONS_UNARY_ADAPTER_HPP_INCLUDED_
#define BOOST_LOG_EXPRESSIONS_UNARY_ADAPTER_HPP_INCLUDED_

#include <boost/utility/result_of.hpp>
#include <boost/fusion/sequence/intrinsic/at.hpp>
#include <boost/log/detail/prologue.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace expressions {

/*!
 * \brief An adapter for an unary function to be used as a terminal in a Boost.Phoenix expression
 *
 * This class is an adapter between Boost.Phoenix expression invokation protocol and
 * an unary function. It forwards the call to the base function, passing only the first argument
 * from the original call. This allows to to embed value extractors in template expressions.
 */
template< typename BaseT >
struct unary_adapter :
    public BaseT
{
    //! Base function type
    typedef BaseT base_type;

    //! Function result type
    template< typename >
    struct result;

    template< typename ArgsT >
    struct result< unary_adapter< base_type >(ArgsT) > :
        public boost::result_of< base_type(typename fusion::result_of::at_c< ArgsT, 0 >::type) >
    {
    };

    template< typename ArgsT >
    struct result< const unary_adapter< base_type >(ArgsT) > :
        public boost::result_of< const base_type(typename fusion::result_of::at_c< ArgsT, 0 >::type) >
    {
    };

    //! Default constructor
    BOOST_LOG_DEFAULTED_FUNCTION(unary_adapter(), {})
    //! Initializing constructor
    template< typename ArgT1 >
    explicit unary_adapter(ArgT1 const& arg1) : base_type(arg1) {}
    //! Initializing constructor
    template< typename ArgT1, typename ArgT2 >
    unary_adapter(ArgT1 const& arg1, ArgT2 const& arg2) : base_type(arg1, arg2) {}

    //! The operator forwards the call to the base function
    template< typename ArgsT >
    typename boost::result_of< base_type(typename fusion::result_of::at_c< ArgsT, 0 >::type) >::type
    operator() (ArgsT const& args)
    {
        return base_type::operator()(fusion::at_c< 0 >(args));
    }

    //! The operator forwards the call to the base function
    template< typename ArgsT >
    typename boost::result_of< const base_type(typename fusion::result_of::at_c< ArgsT, 0 >::type) >::type
    operator() (ArgsT const& args) const
    {
        return base_type::operator()(fusion::at_c< 0 >(args));
    }
};

} // namespace expressions

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_EXPRESSIONS_UNARY_ADAPTER_HPP_INCLUDED_
