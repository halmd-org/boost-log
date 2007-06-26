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
#include <boost/functional/hash/hash.hpp>

namespace boost {

namespace log {

namespace aux {

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
template< typename ResultT, typename ThisT >
BOOST_LOG_FORCEINLINE ResultT
unordered_multimap_facade< DescriptorT >::find_impl(ThisT* pthis, key_type const& key)
{
    typedef typename mpl::if_<
        is_const< ThisT >,
        typename node_container::const_iterator,
        typename node_container::iterator
    >::type nodes_iterator;
    nodes_iterator Result = pthis->m_Nodes.end();
    unsigned char HTIndex = static_cast< unsigned char >(::boost::hash_value(key));
    typename hash_table::const_reference HTEntry = pthis->m_HashTable[HTIndex];

    if (HTEntry.first != Result)
    {
        // The bucket is not empty, we'll have to look through elements to find what we need
        nodes_iterator it = HTEntry.first;
        nodes_iterator end = HTEntry.second;

        // First roughly find an element with equal sized key
        const std::size_t key_size = key.size();
        for (++end; it != end && descriptor::get_key(*it).size() < key_size; ++it);

        // Then find the one that has equal key
        for (; it != end && descriptor::get_key(*it).size() == key_size; ++it)
        {
            register int ComparisonResult;
            if ((ComparisonResult = key.compare(descriptor::get_key(*it))) >= 0)
            {
                if (ComparisonResult == 0)
                    Result = it;

                break;
            }
        }
    }

    return ResultT(Result);
}

//! The method finds the attribute by name
template< typename DescriptorT >
typename unordered_multimap_facade< DescriptorT >::const_iterator
unordered_multimap_facade< DescriptorT >::find(key_type const& key) const
{
    return find_impl< const_iterator >(this, key);
}

//! The method returns the number of the same named attributes in the container
template< typename DescriptorT >
typename unordered_multimap_facade< DescriptorT >::size_type
unordered_multimap_facade< DescriptorT >::count(key_type const& key) const
{
    const_iterator it = this->find(key);
    size_type Result = size_type(it != this->end());
    if (Result)
    {
        for (++it; it != this->end() && descriptor::get_key(*it) == key; ++it, ++Result);
    }

    return Result;
}

//! Equal_range method implementation
template< typename DescriptorT >
template< typename ResultT, typename ThisT >
BOOST_LOG_FORCEINLINE ResultT
unordered_multimap_facade< DescriptorT >::equal_range_impl(ThisT* pthis, key_type const& key)
{
    ResultT Result(pthis->find(key), pthis->end());

    if (Result.first != Result.second)
    {
        Result.second = Result.first;
        for (++Result.second;
            Result.second != pthis->end() && descriptor::get_key(*Result.second) == key;
            ++Result.second);
    }

    return Result;
}

//! The method returns a range of the same named attributes
template< typename DescriptorT >
std::pair<
    typename unordered_multimap_facade< DescriptorT >::const_iterator,
    typename unordered_multimap_facade< DescriptorT >::const_iterator
> unordered_multimap_facade< DescriptorT >::equal_range(key_type const& key) const
{
    typedef std::pair< const_iterator, const_iterator > result_type;
    return equal_range_impl< result_type >(this, key);
}

//! The method clears the container
template< typename DescriptorT >
void unordered_multimap_facade< DescriptorT >::clear()
{
    if (!m_Nodes.empty())
    {
        m_Nodes.clear();
        std::fill_n(m_HashTable.begin(), m_HashTable.size(),
            std::make_pair(m_Nodes.end(), m_Nodes.end()));
    }
}

} // namespace aux

} // namespace log

} // namespace boost
