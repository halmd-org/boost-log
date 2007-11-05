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

#include <boost/log/attributes/attribute_set.hpp>
#include "unordered_mmap_facade.hpp"

namespace boost {

namespace log {

namespace aux {

    //! Default constructor
    template< typename T >
    inline size_caching_list< T >::size_caching_list() : m_Size(0) {}
    //! Copy constructor
    template< typename T >
    inline size_caching_list< T >::size_caching_list(size_caching_list const& that)
        : base_type(static_cast< base_type const& >(that)), m_Size(that.m_Size)
    {
    }
    //! Constructor from the range of values
    template< typename T >
    template< typename IteratorT >
    inline size_caching_list< T >::size_caching_list(IteratorT first, IteratorT last) : m_Size(0)
    {
        this->insert(this->end(), first, last);
    }

    //! Assignment
    template< typename T >
    inline size_caching_list< T >& size_caching_list< T >::operator= (size_caching_list const& that)
    {
        base_type::operator= (static_cast< base_type const& >(that));
        m_Size = that.m_Size;
        return *this;
    }

    //! Size accessor
    template< typename T >
    inline typename size_caching_list< T >::size_type size_caching_list< T >::size() const
    {
        return m_Size;
    }
    //! Empty checker
    template< typename T >
    inline bool size_caching_list< T >::empty() const { return (m_Size == 0); }

    //! Clears the container
    template< typename T >
    inline void size_caching_list< T >::clear()
    {
        base_type::clear();
        m_Size = 0;
    }

    //! Swaps two containers
    template< typename T >
    inline void size_caching_list< T >::swap(size_caching_list& that)
    {
        using std::swap;
        base_type::swap(static_cast< base_type& >(that));
        swap(m_Size, that.m_Size);
    }

    //  Insertion
    template< typename T >
    inline typename size_caching_list< T >::iterator size_caching_list< T >::insert(iterator pos, const_reference x)
    {
        iterator it = base_type::insert(pos, x);
        ++m_Size;
        return it;
    }
    template< typename T >
    template< typename IteratorT >
    inline void size_caching_list< T >::insert(iterator pos, IteratorT first, IteratorT last)
    {
        for (; first != last; ++first)
            this->insert(pos, *first);
    }

    //  Erasure
    template< typename T >
    inline typename size_caching_list< T >::iterator size_caching_list< T >::erase(iterator pos)
    {
        iterator it = base_type::erase(pos);
        --m_Size;
        return it;
    }
    template< typename T >
    inline typename size_caching_list< T >::iterator size_caching_list< T >::erase(iterator first, iterator last)
    {
        while (first != last)
            this->erase(first++);
        return last;
    }

    //  Push/pop front/back
    template< typename T >
    inline void size_caching_list< T >::push_front(const_reference x)
    {
        base_type::push_front(x);
        ++m_Size;
    }
    template< typename T >
    inline void size_caching_list< T >::push_back(const_reference x)
    {
        base_type::push_back(x);
        ++m_Size;
    }
    template< typename T >
    inline void size_caching_list< T >::pop_front()
    {
        base_type::pop_front();
        --m_Size;
    }
    template< typename T >
    inline void size_caching_list< T >::pop_back()
    {
        base_type::pop_back();
        --m_Size;
    }

    //  Assign, resize, etc.
    template< typename T >
    inline void size_caching_list< T >::resize(size_type new_size, const_reference x)
    {
        base_type::resize(new_size, x);
        m_Size = new_size;
    }
    template< typename T >
    inline void size_caching_list< T >::assign(size_type n, const_reference val)
    {
        base_type::assign(n, val);
        m_Size = n;
    }
    template< typename T >
    template< typename IteratorT >
    inline void size_caching_list< T >::assign(IteratorT first, IteratorT last)
    {
        size_caching_list tmp(first, last);
        this->swap(tmp);
    }

} // namespace aux

//! Assignment
template< typename CharT >
basic_attribute_set< CharT >&
basic_attribute_set< CharT >::operator= (basic_attribute_set const& that)
{
    if (this != &that)
    {
        basic_attribute_set NewValue(that);
        this->swap(NewValue);
    }

    return *this;
}

//! Insertion method
template< typename CharT >
typename basic_attribute_set< CharT >::iterator
basic_attribute_set< CharT >::insert(key_type const& key, mapped_type const& data)
{
    typename node_container::iterator Result;
    unsigned char HTIndex = static_cast< unsigned char >(aux::hash_string(key.data(), key.size()));
    typename hash_table::reference HTEntry = this->buckets()[HTIndex];

    if (HTEntry.first != this->nodes().end())
    {
        // There are some elements in the bucket
        typename node_container::iterator it = HTEntry.first;
        typename node_container::iterator end = HTEntry.second;

        // First roughly find an element with equal sized key
        const std::size_t key_size = key.size();
        for (++end; it != end && it->first.size() < key_size; ++it);

        // Then find the one that has equal or greater key
        for (;
            it != end &&
                it->first.size() == key_size &&
                key.compare(it->first) < 0;
            ++it);

        // We found the place where to insert new node
        Result = this->nodes().insert(it, node(key, data, HTIndex));
        if (it == end)
            ++HTEntry.second; // insertion took place at the end of the bucket
    }
    else
    {
        // The bucket is empty
        if (HTIndex == 0)
        {
            // The insertion is in the first bucket
            this->nodes().push_front(node(key, data, HTIndex));
            Result = HTEntry.first = HTEntry.second = this->nodes().begin();
        }
        else if (HTIndex == (std::numeric_limits< unsigned char >::max)() - 1)
        {
            // The insertion is in the last bucket
            this->nodes().push_back(node(key, data, HTIndex));
            Result = --HTEntry.first;
            --HTEntry.second;
        }
        else
        {
            // The insertion is in some bucket in the middle
            // Find where to insert the node
            for (std::size_t i = HTIndex + 1; i < (std::numeric_limits< unsigned char >::max)(); ++i)
            {
                if (this->buckets()[i].first != this->nodes().end())
                {
                    HTEntry.first = this->buckets()[i].first;
                    break;
                }
            }

            Result = HTEntry.first = HTEntry.second =
                this->nodes().insert(HTEntry.first, node(key, data, HTIndex));
        }
    }

    return iterator(Result);
}

//! The method erases all attributes with the specified name
template< typename CharT >
typename basic_attribute_set< CharT >::size_type
basic_attribute_set< CharT >::erase(key_type const& key)
{
    size_type Result = 0;
    iterator it = this->find(key);
    if (it != this->end())
    {
        do
        {
            this->erase(it++);
            ++Result;
        }
        while (it != this->end() && it->first == key);
    }

    return Result;
}

//! The method erases the specified attribute
template< typename CharT >
void basic_attribute_set< CharT >::erase(iterator it)
{
    typename node_container::iterator itNode = it.base();
    typename hash_table::reference HTEntry = this->buckets()[itNode->m_HTIndex];
    if (HTEntry.first == HTEntry.second)
    {
        // We're deleting the last element from the bucket
        HTEntry.first = HTEntry.second = this->nodes().end();
    }
    else if (HTEntry.first == itNode)
    {
        // We're deleting the first element of the bucket
        ++HTEntry.first;
    }
    else if (HTEntry.second == itNode)
    {
        // We're deleting the last element of the bucket
        --HTEntry.second;
    }

    this->nodes().erase(itNode);
}

//! The method erases all attributes within the specified range
template< typename CharT >
void basic_attribute_set< CharT >::erase(iterator begin, iterator end)
{
    if (begin != end)
    {
        typename node_container::iterator itFirstNode = begin.base();

        iterator last = end;
        --last;
        typename node_container::iterator itLastNode = last.base();

        if (itFirstNode->m_HTIndex != itLastNode->m_HTIndex)
        {
            // Adjust the first and the last buckets
            typename hash_table::reference FirstHTEntry = this->buckets()[itFirstNode->m_HTIndex];
            if (FirstHTEntry.first == itFirstNode)
            {
                // The first bucket is cleared completely
                FirstHTEntry.first = FirstHTEntry.second = this->nodes().end();
            }
            else
            {
                // Some elements are to be left in the first bucket
                FirstHTEntry.second = itFirstNode;
                --FirstHTEntry.second;
            }

            typename hash_table::reference LastHTEntry = this->buckets()[itLastNode->m_HTIndex];
            if (LastHTEntry.second == itLastNode)
            {
                // The last bucket is cleared completely
                LastHTEntry.first = LastHTEntry.second = this->nodes().end();
            }
            else
            {
                // Some elements are to be left in the last bucket
                LastHTEntry.first = itLastNode;
                ++LastHTEntry.first;
            }

            // All buckets between them are cleared out
            std::fill_n(
                this->buckets().begin() + itFirstNode->m_HTIndex + 1,
                std::ptrdiff_t(itLastNode->m_HTIndex - itFirstNode->m_HTIndex - 1),
                std::make_pair(this->nodes().end(), this->nodes().end()));
        }
        else
        {
            // Some elements of a single bucket are to be deleted
            typename hash_table::reference HTEntry = this->buckets()[itFirstNode->m_HTIndex];
            unsigned int const Condition =
                static_cast< unsigned int >(HTEntry.first == itFirstNode) |
                (static_cast< unsigned int >(HTEntry.second == itLastNode) << 1);

            switch (Condition)
            {
            case 1:
                // Some elements are deleted from the bucket beginning
                HTEntry.first = itLastNode;
                ++HTEntry.first;
                break;

            case 2:
                // Some elements are deleted from the bucket end
                HTEntry.second = itFirstNode;
                --HTEntry.second;
                break;

            case 3:
                // The bucket is to be cleared completely
                HTEntry.first =
                    HTEntry.second = this->nodes().end();
                break;

            default:
                // Nothing to do here
                break;
            }
        }

        // Erase the nodes
        this->nodes().erase(itFirstNode, end.base());
    }
}

//! The method clears the container
template< typename CharT >
void basic_attribute_set< CharT >::clear()
{
    if (!this->nodes().empty())
    {
        this->nodes().clear();
        std::fill_n(this->buckets().begin(), this->buckets().size(),
            std::make_pair(this->nodes().end(), this->nodes().end()));
    }
}

//! Explicitly instantiate container implementation
namespace aux {
    template class unordered_multimap_facade< attribute_set_descr< char > >;
    template class unordered_multimap_facade< attribute_set_descr< wchar_t > >;
} // namespace aux
template class basic_attribute_set< char >;
template class basic_attribute_set< wchar_t >;

} // namespace log

} // namespace boost
