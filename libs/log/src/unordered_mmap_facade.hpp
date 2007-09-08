/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   unordered_mmap_facade.hpp
 * \author Andrey Semashev
 * \date   19.04.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#include <limits>
#include <algorithm>

namespace boost {

namespace log {

namespace aux {

//! Hash function
template< typename CharT >
inline std::size_t hash_string(const CharT* str, std::size_t len)
{
    // Algorithm taken from Bob Jenkins' article:
    // http://www.burtleburtle.net/bob/hash/doobs.html
    register std::size_t hash = 0;
    for (register std::size_t i = 0; i < len; ++i)
    {
        hash += str[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}


//! Default constructor
template< typename DescriptorT >
unordered_multimap_facade< DescriptorT >::unordered_multimap_facade()
    : m_HashTable((std::numeric_limits< unsigned char >::max)(), std::make_pair(m_Nodes.end(), m_Nodes.end()))
{
}

//! Copy constructor
template< typename DescriptorT >
unordered_multimap_facade< DescriptorT >::unordered_multimap_facade(unordered_multimap_facade const& that) :
    m_Nodes(that.m_Nodes),
    m_HashTable((std::numeric_limits< unsigned char >::max)())
{
    // Rebuild hash table
    this->rehash();
}

//! Destructor (just to make it link from the library)
template< typename DescriptorT >
unordered_multimap_facade< DescriptorT >::~unordered_multimap_facade() {}

//! The method rebuilds hash table
template< typename DescriptorT >
void unordered_multimap_facade< DescriptorT >::rehash()
{
    std::fill_n(m_HashTable.begin(),
        (std::numeric_limits< unsigned char >::max)(),
        std::make_pair(m_Nodes.end(), m_Nodes.end()));

    typename node_container::iterator it = m_Nodes.begin();
    for (; it != m_Nodes.end(); ++it)
    {
        if (m_HashTable[it->m_HTIndex].first == m_Nodes.end())
            m_HashTable[it->m_HTIndex].first = it;
        m_HashTable[it->m_HTIndex].second = it;
    }
}

//! Find method implementation
template< typename DescriptorT >
typename unordered_multimap_facade< DescriptorT >::iterator
unordered_multimap_facade< DescriptorT >::find_impl(const char_type* key, size_type key_len)
{
    typedef typename node_container::iterator nodes_iterator;
    nodes_iterator Result = this->m_Nodes.end();
    unsigned char HTIndex = static_cast< unsigned char >(aux::hash_string(key, key_len));
    typename hash_table::const_reference HTEntry = this->m_HashTable[HTIndex];

    if (HTEntry.first != Result)
    {
        // The bucket is not empty, we'll have to look through elements to find what we need
        nodes_iterator it = HTEntry.first;
        nodes_iterator end = HTEntry.second;

        // First roughly find an element with equal sized key
        for (++end; it != end && it->first.size() < key_len; ++it);

        // Then find the one that has equal key
        for (; it != end && it->first.size() == key_len; ++it)
        {
            register int ComparisonResult;
            if ((ComparisonResult = it->first.compare(key, key_len)) <= 0)
            {
                if (ComparisonResult == 0)
                    Result = it;

                break;
            }
        }
    }

    return iterator(Result);
}

//! Count method implementation
template< typename DescriptorT >
typename unordered_multimap_facade< DescriptorT >::size_type
unordered_multimap_facade< DescriptorT >::count_impl(const char_type* key, size_type key_len) const
{
    const_iterator it = const_cast< unordered_multimap_facade* >(this)->find_impl(key, key_len);
    size_type Result = size_type(it != this->end());
    if (Result)
    {
        for (++it; it != this->end() && it->first.compare(key, key_len) == 0; ++it, ++Result);
    }

    return Result;
}

//! Equal_range method implementation
template< typename DescriptorT >
std::pair<
    typename unordered_multimap_facade< DescriptorT >::iterator,
    typename unordered_multimap_facade< DescriptorT >::iterator
> unordered_multimap_facade< DescriptorT >::equal_range_impl(const char_type* key, size_type key_len)
{
    std::pair< iterator, iterator > Result(this->find_impl(key, key_len), iterator(m_Nodes.end()));

    if (Result.first != Result.second)
    {
        Result.second = Result.first;
        for (++Result.second;
            Result.second != this->end() && Result.second->first.compare(key, key_len) == 0;
            ++Result.second);
    }

    return Result;
}

} // namespace aux

} // namespace log

} // namespace boost
