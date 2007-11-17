/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   attribute_set.cpp
 * \author Andrey Semashev
 * \date   19.04.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#include "attribute_set_impl.hpp"

namespace boost {

namespace log {

//! Iterator dereferencing implementation
template< typename CharT >
template< bool fConstV >
typename basic_attribute_set< CharT >::BOOST_NESTED_TEMPLATE iter< fConstV >::reference
basic_attribute_set< CharT >::iter< fConstV >::dereference() const
{
    return static_cast< typename implementation::node* >(m_pNode)->m_Value;
}


//! Default constructor
template< typename CharT >
basic_attribute_set< CharT >::basic_attribute_set() : m_pImpl(new implementation())
{
}

//! Copy constructor
template< typename CharT >
basic_attribute_set< CharT >::basic_attribute_set(basic_attribute_set const& that)
    : m_pImpl(new implementation(*that.m_pImpl))
{
}

//! Assignment
template< typename CharT >
basic_attribute_set< CharT >& basic_attribute_set< CharT >::operator= (basic_attribute_set const& that)
{
    if (this != &that)
    {
        basic_attribute_set tmp(that);
        swap(tmp);
    }
    return *this;
}

//  Iterator generators
template< typename CharT >
typename basic_attribute_set< CharT >::iterator basic_attribute_set< CharT >::begin()
{
    return iterator(m_pImpl->Nodes.begin().pointed_node());
}
template< typename CharT >
typename basic_attribute_set< CharT >::iterator basic_attribute_set< CharT >::end()
{
    return iterator(m_pImpl->Nodes.end().pointed_node());
}
template< typename CharT >
typename basic_attribute_set< CharT >::const_iterator basic_attribute_set< CharT >::begin() const
{
    return const_iterator(m_pImpl->Nodes.begin().pointed_node());
}
template< typename CharT >
typename basic_attribute_set< CharT >::const_iterator basic_attribute_set< CharT >::end() const
{
    return const_iterator(m_pImpl->Nodes.end().pointed_node());
}

//! The method returns number of elements in the container
template< typename CharT >
typename basic_attribute_set< CharT >::size_type basic_attribute_set< CharT >::size() const
{
    return m_pImpl->Nodes.size();
}

//! Insertion method
template< typename CharT >
std::pair< typename basic_attribute_set< CharT >::iterator, bool >
basic_attribute_set< CharT >::insert(key_type const& key, mapped_type const& data)
{
    typename implementation::node n(key, data);
    std::pair<
        typename implementation::node_container::iterator,
        bool
    > insertion_result = m_pImpl->Nodes.insert(n);

    return std::make_pair(iterator(insertion_result.first.pointed_node()), insertion_result.second);
}

//! The method erases all attributes with the specified name
template< typename CharT >
typename basic_attribute_set< CharT >::size_type
basic_attribute_set< CharT >::erase(key_type const& key)
{
    return m_pImpl->Nodes.erase(key);
}

//! The method erases the specified attribute
template< typename CharT >
void basic_attribute_set< CharT >::erase(iterator it)
{
    m_pImpl->Nodes.erase(static_cast< typename implementation::node* >(it.m_pNode));
}
//! The method erases all attributes within the specified range
template< typename CharT >
void basic_attribute_set< CharT >::erase(iterator begin, iterator end)
{
    while (begin != end)
        m_pImpl->Nodes.erase(static_cast< typename implementation::node* >((begin++).m_pNode));
}

//! The method clears the container
template< typename CharT >
void basic_attribute_set< CharT >::clear()
{
    m_pImpl->Nodes.clear();
}

//! Internal lookup implementation
template< typename CharT >
typename basic_attribute_set< CharT >::iterator
basic_attribute_set< CharT >::find_impl(const char_type* key, size_type len)
{
    return iterator(m_pImpl->Nodes.find(typename implementation::light_key_type(key, len)).pointed_node());
}

template class basic_attribute_set< char >;
template class basic_attribute_set< wchar_t >;
template class basic_attribute_set< char >::iter< true >;
template class basic_attribute_set< wchar_t >::iter< true >;
template class basic_attribute_set< char >::iter< false >;
template class basic_attribute_set< wchar_t >::iter< false >;

} // namespace log

} // namespace boost
