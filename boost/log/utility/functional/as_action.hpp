/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   as_action.hpp
 * \author Andrey Semashev
 * \date   30.03.2008
 *
 * This header contains function object adapter for compatibility with Boost.Spirit actions interface requirements.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_UTILITY_FUNCTIONAL_AS_ACTION_HPP_INCLUDED_
#define BOOST_LOG_UTILITY_FUNCTIONAL_AS_ACTION_HPP_INCLUDED_

#include <boost/log/detail/prologue.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

//! Function object adapter for Boost.Spirit actions
template< typename FunT >
struct action_adapter :
    public FunT
{
    typedef typename FunT::result_type result_type;

    BOOST_LOG_DEFAULTED_FUNCTION(action_adapter(), {})
    explicit action_adapter(FunT const& fun) : FunT(fun) {}

    template< typename AttributeT, typename ContextT >
    result_type operator() (AttributeT const& attr, ContextT const& ctx, bool& pass) const
    {
        return FunT::operator() (attr);
    }
};

template< typename FunT >
BOOST_LOG_FORCEINLINE action_adapter< FunT > as_action(FunT const& fun)
{
    return action_adapter< FunT >(fun);
}

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_UTILITY_FUNCTIONAL_AS_ACTION_HPP_INCLUDED_
