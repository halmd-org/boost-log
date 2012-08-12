/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   bind.hpp
 * \author Andrey Semashev
 * \date   30.03.2008
 *
 * This header contains function object adapters.
 * This is a lightweight alternative to what Boost.Phoenix and Boost.Bind provides.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_UTILITY_FUNCTIONAL_BIND_HPP_INCLUDED_
#define BOOST_LOG_UTILITY_FUNCTIONAL_BIND_HPP_INCLUDED_

#include <boost/log/detail/prologue.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

//! First argument binder
template< typename FunT, typename FirstArgT >
struct binder1st :
    private FunT
{
    typedef typename FunT::result_type result_type;

    binder1st(FunT const& fun, FirstArgT const& arg) : FunT(fun), m_Arg(arg) {}

    template< typename T >
    result_type operator() (T const& arg) const
    {
        return FunT::operator() (m_Arg, arg);
    }

private:
    FirstArgT m_Arg;
};

template< typename FunT, typename FirstArgT >
BOOST_LOG_FORCEINLINE binder1st< FunT, FirstArgT > bind1st(FunT const& fun, FirstArgT const& arg)
{
    return binder1st< FunT, FirstArgT >(fun, arg);
}

//! Second argument binder
template< typename FunT, typename SecondArgT >
struct binder2nd :
    private FunT
{
    typedef typename FunT::result_type result_type;

    binder2nd(FunT const& fun, SecondArgT const& arg) : FunT(fun), m_Arg(arg) {}

    template< typename T >
    result_type operator() (T const& arg) const
    {
        return FunT::operator() (arg, m_Arg);
    }

private:
    SecondArgT m_Arg;
};

template< typename FunT, typename SecondArgT >
BOOST_LOG_FORCEINLINE binder2nd< FunT, SecondArgT > bind2nd(FunT const& fun, SecondArgT const& arg)
{
    return binder2nd< FunT, SecondArgT >(fun, arg);
}

//! Third argument binder
template< typename FunT, typename ThirdArgT >
struct binder3rd :
    private FunT
{
    typedef typename FunT::result_type result_type;

    binder3rd(FunT const& fun, ThirdArgT const& arg) : FunT(fun), m_Arg(arg) {}

    template< typename T1, typename T2 >
    result_type operator() (T1 const& arg1, T2 const& arg2) const
    {
        return FunT::operator() (arg1, arg2, m_Arg);
    }

private:
    ThirdArgT m_Arg;
};

template< typename FunT, typename ThirdArgT >
BOOST_LOG_FORCEINLINE binder3rd< FunT, ThirdArgT > bind3rd(FunT const& fun, ThirdArgT const& arg)
{
    return binder3rd< FunT, ThirdArgT >(fun, arg);
}

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_UTILITY_FUNCTIONAL_BIND_HPP_INCLUDED_
