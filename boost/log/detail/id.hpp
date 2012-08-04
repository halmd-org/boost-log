/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   id.hpp
 * \author Andrey Semashev
 * \date   08.01.2012
 *
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_DETAIL_ID_HPP_INCLUDED_
#define BOOST_LOG_DETAIL_ID_HPP_INCLUDED_

#include <boost/log/detail/prologue.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace aux {

//! Generic identifier class
template< typename DescriptorT >
class id
{
public:
    //! Native type of the process id
    typedef typename DescriptorT::native_type native_type;

private:
    native_type m_NativeID;

public:
    BOOST_CONSTEXPR id() BOOST_NOEXCEPT : m_NativeID(0) {}

    explicit id(native_type native) BOOST_NOEXCEPT : m_NativeID(native) {}

    native_type native_id() const BOOST_NOEXCEPT { return m_NativeID; }

    bool operator== (id const& that) const BOOST_NOEXCEPT
    {
        return (m_NativeID == that.m_NativeID);
    }
    bool operator!= (id const& that) const BOOST_NOEXCEPT
    {
        return (m_NativeID != that.m_NativeID);
    }
    bool operator< (id const& that) const BOOST_NOEXCEPT
    {
        return (m_NativeID < that.m_NativeID);
    }
    bool operator> (id const& that) const BOOST_NOEXCEPT
    {
        return (m_NativeID > that.m_NativeID);
    }
    bool operator<= (id const& that) const BOOST_NOEXCEPT
    {
        return (m_NativeID <= that.m_NativeID);
    }
    bool operator>= (id const& that) const BOOST_NOEXCEPT
    {
        return (m_NativeID >= that.m_NativeID);
    }
};

} // namespace aux

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_DETAIL_ID_HPP_INCLUDED_
