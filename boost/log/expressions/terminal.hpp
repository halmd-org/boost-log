/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   terminal.hpp
 * \author Andrey Semashev
 * \date   29.01.2012
 *
 * The header contains attribute terminal definition.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_EXPRESSIONS_TERMINAL_HPP_INCLUDED_
#define BOOST_LOG_EXPRESSIONS_TERMINAL_HPP_INCLUDED_

#include <boost/mpl/bool.hpp>
#include <boost/phoenix/core/terminal.hpp>
#include <boost/phoenix/core/environment.hpp>
#include <boost/utility/result_of.hpp>
#include <boost/log/detail/prologue.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace expressions {

/*!
 * \brief A terminal node in the expression template tree
 *
 * This terminal contains a base function object and is used for expression
 * templates evaluation. The function object is expected to receive a Boost.Fusion sequence
 * of arguments upon calling. The result of the function will be returned as the terminal value.
 *
 * The base function can be an attribute value extractor wrapped into the \c extractor_adapter
 * class template, in which case the terminal results in the attribute value upon invokation.
 */
template< typename BaseT >
struct terminal :
    public BaseT
{
    //! Internal typedef for type categorization
    typedef void _is_boost_log_terminal;

    //! Base function type
    typedef BaseT base_type;

    template< typename >
    struct result;

    template< typename EnvT >
    struct result< terminal< base_type >(EnvT) > :
        public result_of< base_type(typename EnvT::args_type) >
    {
    };

    template< typename EnvT >
    struct result< const terminal< base_type >(EnvT) > :
        public result_of< const base_type(typename EnvT::args_type) >
    {
    };

    //! Default constructor
    BOOST_LOG_DEFAULTED_FUNCTION(terminal(), {})
    //! Initializing constructor
    template< typename ArgT1 >
    explicit terminal(ArgT1 const& arg1) : base_type(arg1) {}
    //! Initializing constructor
    template< typename ArgT1, typename ArgT2 >
    terminal(ArgT1 const& arg1, ArgT2 const& arg2) : base_type(arg1, arg2) {}

    //! Invokation operator
    template< typename EnvT >
    typename result_of< base_type(typename EnvT::args_type) >::type
    operator() (EnvT const& env)
    {
        return base_type::operator() (env.args());
    }

    //! Invokation operator
    template< typename EnvT >
    typename result_of< const base_type(typename EnvT::args_type) >::type
    operator() (EnvT const& env) const
    {
        return base_type::operator() (env.args());
    }
};

} // namespace expressions

} // namespace log

#ifndef BOOST_LOG_DOXYGEN_PASS

namespace phoenix {

namespace result_of {

template< typename T >
struct is_nullary< T, typename T::_is_boost_log_terminal > :
    public mpl::false_
{
};

} // namespace result_of

template< typename T >
struct is_custom_terminal< T, typename T::_is_boost_log_terminal > :
    public mpl::true_
{
};

template< typename T >
struct custom_terminal< T, typename T::_is_boost_log_terminal >
{
    typedef typename T::result_type result_type;

    template< typename ContextT >
    result_type operator() (T const& term, ContextT& ctx) const
    {
        return term(phoenix::env(ctx));
    }
};

} // namespace phoenix

#endif // BOOST_LOG_DOXYGEN_PASS

} // namespace boost

#endif // BOOST_LOG_EXPRESSIONS_TERMINAL_HPP_INCLUDED_
