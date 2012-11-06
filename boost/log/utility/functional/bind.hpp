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

#include <boost/type_traits/remove_cv.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/log/detail/prologue.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace aux {

template< typename T >
struct make_arg_type
{
    typedef T const& type;
};

template< typename T >
struct make_arg_type< T& >
{
    typedef T& type;
};

} // namespace aux

//! First argument binder
template< typename FunT, typename FirstArgT >
struct binder1st
{
    typedef typename remove_cv< typename remove_reference< FunT >::type >::type::result_type result_type;

    binder1st(FunT fun, typename aux::make_arg_type< FirstArgT >::type arg) : m_Fun(fun), m_Arg(arg) {}

    result_type operator() () const
    {
        return m_Fun(m_Arg);
    }

    template< typename T0 >
    result_type operator() (T0 const& arg0) const
    {
        return m_Fun(m_Arg, arg0);
    }

    template< typename T0, typename T1 >
    result_type operator() (T0 const& arg0, T1 const& arg1) const
    {
        return m_Fun(m_Arg, arg0, arg1);
    }

private:
    FunT m_Fun;
    FirstArgT m_Arg;
};

template< typename FunT, typename FirstArgT >
BOOST_LOG_FORCEINLINE binder1st< FunT, FirstArgT > bind1st(FunT fun, FirstArgT const& arg)
{
    return binder1st< FunT, FirstArgT >(fun, arg);
}

template< typename FunT, typename FirstArgT >
BOOST_LOG_FORCEINLINE binder1st< FunT, FirstArgT > bind1st(FunT fun, FirstArgT& arg)
{
    return binder1st< FunT, FirstArgT >(fun, arg);
}

//! Second argument binder
template< typename FunT, typename SecondArgT >
struct binder2nd
{
    typedef typename remove_cv< typename remove_reference< FunT >::type >::type::result_type result_type;

    binder2nd(FunT fun, typename aux::make_arg_type< SecondArgT >::type arg) : m_Fun(fun), m_Arg(arg) {}

    template< typename T >
    result_type operator() (T const& arg) const
    {
        return m_Fun(arg, m_Arg);
    }

    template< typename T0, typename T1 >
    result_type operator() (T0 const& arg0, T1 const& arg1) const
    {
        return m_Fun(arg0, m_Arg, arg1);
    }

private:
    FunT m_Fun;
    SecondArgT m_Arg;
};

template< typename FunT, typename SecondArgT >
BOOST_LOG_FORCEINLINE binder2nd< FunT, SecondArgT > bind2nd(FunT fun, SecondArgT const& arg)
{
    return binder2nd< FunT, SecondArgT >(fun, arg);
}

template< typename FunT, typename SecondArgT >
BOOST_LOG_FORCEINLINE binder2nd< FunT, SecondArgT > bind2nd(FunT fun, SecondArgT& arg)
{
    return binder2nd< FunT, SecondArgT >(fun, arg);
}

//! Third argument binder
template< typename FunT, typename ThirdArgT >
struct binder3rd
{
    typedef typename remove_cv< typename remove_reference< FunT >::type >::type::result_type result_type;

    binder3rd(FunT fun, typename aux::make_arg_type< ThirdArgT >::type arg) : m_Fun(fun), m_Arg(arg) {}

    template< typename T0, typename T1 >
    result_type operator() (T0 const& arg0, T1 const& arg1) const
    {
        return m_Fun(arg0, arg1, m_Arg);
    }

private:
    FunT m_Fun;
    ThirdArgT m_Arg;
};

template< typename FunT, typename ThirdArgT >
BOOST_LOG_FORCEINLINE binder3rd< FunT, ThirdArgT > bind3rd(FunT fun, ThirdArgT const& arg)
{
    return binder3rd< FunT, ThirdArgT >(fun, arg);
}

template< typename FunT, typename ThirdArgT >
BOOST_LOG_FORCEINLINE binder3rd< FunT, ThirdArgT > bind3rd(FunT fun, ThirdArgT& arg)
{
    return binder3rd< FunT, ThirdArgT >(fun, arg);
}

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_UTILITY_FUNCTIONAL_BIND_HPP_INCLUDED_
