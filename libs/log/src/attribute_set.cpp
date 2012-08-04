/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   attribute_set.cpp
 * \author Andrey Semashev
 * \date   19.04.2007
 *
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#include <deque>
#include <boost/assert.hpp>
#include <boost/intrusive/options.hpp>
#include <boost/intrusive/list.hpp>
#include <boost/intrusive/link_mode.hpp>
#include <boost/intrusive/derivation_value_traits.hpp>
#include <boost/log/attributes/attribute_set.hpp>
#include "attribute_set_impl.hpp"

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

inline attribute_set::node_base::node_base() :
    m_pPrev(NULL),
    m_pNext(NULL)
{
}

inline attribute_set::node::node(key_type const& key, mapped_type const& data) :
    node_base(),
    m_Value(key, data)
{
}

//! Default constructor
BOOST_LOG_API attribute_set::attribute_set() :
    m_pImpl(new implementation())
{
}

//! Copy constructor
BOOST_LOG_API attribute_set::attribute_set(attribute_set const& that) :
    m_pImpl(new implementation(*that.m_pImpl))
{
}

//! Destructor
BOOST_LOG_API attribute_set::~attribute_set() BOOST_NOEXCEPT
{
    delete m_pImpl;
}

//  Iterator generators
BOOST_LOG_API attribute_set::iterator attribute_set::begin()
{
    return m_pImpl->begin();
}
BOOST_LOG_API attribute_set::iterator attribute_set::end()
{
    return m_pImpl->end();
}
BOOST_LOG_API attribute_set::const_iterator attribute_set::begin() const
{
    return const_iterator(m_pImpl->begin());
}
BOOST_LOG_API attribute_set::const_iterator attribute_set::end() const
{
    return const_iterator(m_pImpl->end());
}

//! The method returns number of elements in the container
BOOST_LOG_API attribute_set::size_type attribute_set::size() const
{
    return m_pImpl->size();
}

//! Insertion method
BOOST_LOG_API std::pair< attribute_set::iterator, bool >
attribute_set::insert(key_type key, mapped_type const& data)
{
    return m_pImpl->insert(key, data);
}

//! The method erases all attributes with the specified name
BOOST_LOG_API attribute_set::size_type attribute_set::erase(key_type key)
{
    iterator it = m_pImpl->find(key);
    if (it != end())
    {
        m_pImpl->erase(it);
        return 1;
    }
    else
        return 0;
}

//! The method erases the specified attribute
BOOST_LOG_API void attribute_set::erase(iterator it)
{
    m_pImpl->erase(it);
}
//! The method erases all attributes within the specified range
BOOST_LOG_API void attribute_set::erase(iterator begin, iterator end)
{
    while (begin != end)
    {
        m_pImpl->erase(begin++);
    }
}

//! The method clears the container
BOOST_LOG_API void attribute_set::clear()
{
    m_pImpl->clear();
}

//! Internal lookup implementation
BOOST_LOG_API attribute_set::iterator attribute_set::find(key_type key)
{
    return m_pImpl->find(key);
}

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost
