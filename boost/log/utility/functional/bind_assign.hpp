/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   bind_assign.hpp
 * \author Andrey Semashev
 * \date   30.03.2008
 *
 * This header contains a function object that assigns the received value to the bound object.
 * This is a lightweight alternative to what Boost.Phoenix and Boost.Lambda provides.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_UTILITY_FUNCTIONAL_BIND_ASSIGN_HPP_INCLUDED_
#define BOOST_LOG_UTILITY_FUNCTIONAL_BIND_ASSIGN_HPP_INCLUDED_

#include <boost/log/detail/prologue.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

//! The function object that assigns its operand to the bound value
template< typename AssigneeT >
struct assign_fun
{
    typedef void result_type;

    explicit assign_fun(AssigneeT& assignee) : m_Assignee(assignee)
    {
    }

    template< typename T >
    void operator() (T const& val) const
    {
        m_Assignee = val;
    }

private:
    AssigneeT& m_Assignee;
};

template< typename AssigneeT >
BOOST_LOG_FORCEINLINE assign_fun< AssigneeT > bind_assign(AssigneeT& assignee)
{
    return assign_fun< AssigneeT >(assignee);
}

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_UTILITY_FUNCTIONAL_BIND_ASSIGN_HPP_INCLUDED_
