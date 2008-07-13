/*
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * This header is the Boost.Log library implementation, see the library documentation
 * at http://www.boost.org/libs/log/doc/log.html.
 */
/*!
 * \file   attribute_set.hpp
 * \author Andrey Semashev
 * \date   08.03.2007
 * 
 * This header contains definition of the attribute set container.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_ATTRIBUTE_SET_HPP_INCLUDED_
#define BOOST_LOG_ATTRIBUTE_SET_HPP_INCLUDED_

#include <string>
#include <utility>
#include <iterator>
#include <algorithm>
#include <boost/shared_ptr.hpp>
#include <boost/mpl/if.hpp>
#include <boost/utility/addressof.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/utility/slim_string.hpp>
#include <boost/log/attributes/attribute.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

template< typename >
class basic_attribute_values_view;

//! An attribute set class
template< typename CharT >
class basic_attribute_set
{
    friend class basic_attribute_values_view< CharT >;

    //! Self type
    typedef basic_attribute_set< CharT > this_type;

public:
    //! Char type
    typedef CharT char_type;
    //! String type
    typedef std::basic_string< char_type > string_type;
    //! Key type
    typedef basic_slim_string< char_type > key_type;
    //! Mapped attribute type
    typedef shared_ptr< attribute > mapped_type;

    //! Value type
    typedef std::pair< const key_type, mapped_type > value_type;
    //! Allocator type
    typedef std::allocator< value_type > allocator_type;
    //! Reference type
    typedef typename allocator_type::reference reference;
    //! Const reference type
    typedef typename allocator_type::const_reference const_reference;
    //! Pointer type
    typedef typename allocator_type::pointer pointer;
    //! Const pointer type
    typedef typename allocator_type::const_pointer const_pointer;
    //! Size type
    typedef typename allocator_type::size_type size_type;
    //! Difference type
    typedef typename allocator_type::difference_type difference_type;

private:
    //! \cond

    //! Implementation
    struct implementation;
    friend struct implementation;

    //! Reference proxy object to implement operator[]
    class reference_proxy;
    friend class reference_proxy;
    class reference_proxy
    {
        basic_attribute_set* m_pContainer;
        const char_type* m_pKey;
        size_type m_KeyLen;

    public:
        //! Constructor
        explicit reference_proxy(basic_attribute_set* pContainer, const char_type* pKey, size_type KeyLen)
            : m_pContainer(pContainer), m_pKey(pKey), m_KeyLen(KeyLen)
        {
        }

        //! Conversion operator (would be invoked in case of reading from the container)
        operator mapped_type() const
        {
            iterator it = m_pContainer->find_impl(m_pKey, m_KeyLen);
            if (it != m_pContainer->end())
                return it->second;
            else
                return mapped_type();
        }
        //! Assignment operator (would be invoked in case of writing to the container)
        mapped_type& operator= (mapped_type const& val) const
        {
            std::pair< iterator, bool > res =
                m_pContainer->insert(key_type(m_pKey, m_KeyLen), val);
            if (!res.second)
                res.first->second = val;
            return res.first->second;
        }
    };

    //! A base class for the container nodes
    struct node_base
    {
        value_type m_Value;
        node_base* m_pPrev;
        node_base* m_pNext;

        node_base() { /* m_pPrev = m_pNext = 0; -- initialized internally in cpp */ }
        node_base(node_base const& that) : m_Value(that.m_Value) { /* m_pPrev = m_pNext = 0; -- initialized internally in cpp */ }
        node_base(key_type const& key, mapped_type const& data) : m_Value(key, data) { /* m_pPrev = m_pNext = 0; -- initialized internally in cpp */ }

    private:
        node_base& operator= (node_base const&);
    };

    //! Iterator class
    template< bool fConstV > class iter;
    template< bool fConstV > friend class iter;
    template< bool fConstV >
    class iter
    {
        friend class iter< !fConstV >;
        friend class basic_attribute_set< CharT >;

    public:
        //  Standard typedefs
        typedef typename this_type::difference_type difference_type;
        typedef typename this_type::value_type value_type;
        typedef typename mpl::if_c<
            fConstV,
            typename this_type::const_reference,
            typename this_type::reference
        >::type reference;
        typedef typename mpl::if_c<
            fConstV,
            typename this_type::const_pointer,
            typename this_type::pointer
        >::type pointer;
        typedef std::bidirectional_iterator_tag iterator_category;

    public:
        //  Constructors
        iter() : m_pNode(NULL) {}
        explicit iter(node_base* pNode) : m_pNode(pNode) {}
        iter(iter< false > const& that) : m_pNode(that.m_pNode) {}

        //! Assignment
        template< bool f >
        iter& operator= (iter< f > const& that)
        {
            m_pNode = that.m_pNode;
            return *this;
        }

        //  Comparison
        template< bool f >
        bool operator== (iter< f > const& that) const { return (m_pNode == that.m_pNode); }
        template< bool f >
        bool operator!= (iter< f > const& that) const { return (m_pNode != that.m_pNode); }

        //  Modification
        iter& operator++ ()
        {
            m_pNode = m_pNode->m_pNext;
            return *this;
        }
        iter& operator-- ()
        {
            m_pNode = m_pNode->m_pPrev;
            return *this;
        }
        iter operator++ (int)
        {
            iter tmp(*this);
            m_pNode = m_pNode->m_pNext;
            return tmp;
        }
        iter operator-- (int)
        {
            iter tmp(*this);
            m_pNode = m_pNode->m_pPrev;
            return tmp;
        }

        //  Dereferencing
        pointer operator-> () const { return addressof(m_pNode->m_Value); }
        reference operator* () const { return m_pNode->m_Value; }

    private:
        node_base* m_pNode;
    };

    //! \endcond

public:
#ifndef BOOST_LOG_DOXYGEN_PASS
    //! Iterator type
    typedef iter< false > iterator;
    //! Const iterator type
    typedef iter< true > const_iterator;
#else
    //! Iterator type
    typedef [implementation defined] iterator;
    //! Const iterator type
    typedef [implementation defined] const_iterator;
#endif // BOOST_LOG_DOXYGEN_PASS

private:
    //! Pointer to implementation
    implementation* m_pImpl;

public:
    //! Default constructor
    BOOST_LOG_EXPORT basic_attribute_set();
    //! Copy constructor
    BOOST_LOG_EXPORT basic_attribute_set(basic_attribute_set const& that);
    //! Destructor
    BOOST_LOG_EXPORT ~basic_attribute_set();

    //! Assignment
    BOOST_LOG_EXPORT basic_attribute_set& operator= (basic_attribute_set const& that);

    //! Swap
    void swap(basic_attribute_set& that) { std::swap(m_pImpl, that.m_pImpl); }

    //  Iterator generators
    BOOST_LOG_EXPORT iterator begin();
    BOOST_LOG_EXPORT iterator end();
    BOOST_LOG_EXPORT const_iterator begin() const;
    BOOST_LOG_EXPORT const_iterator end() const;

    //! The method returns number of elements in the container
    BOOST_LOG_EXPORT size_type size() const;
    //! The method checks if the container is empty
    bool empty() const { return (size() == 0); }

    //! The method finds the attribute by name
    iterator find(key_type const& key)
    {
        return this->find_impl(key.data(), key.size());
    }
    //! The method finds the attribute by name
    iterator find(string_type const& key)
    {
        return this->find_impl(key.data(), key.size());
    }
    //! The method finds the attribute by name
    iterator find(const char_type* key)
    {
        typedef std::char_traits< char_type > traits_type;
        return this->find_impl(key, traits_type::length(key));
    }
    //! The method finds the attribute by name
    const_iterator find(key_type const& key) const
    {
        return this->find_impl(key.data(), key.size());
    }
    //! The method finds the attribute by name
    const_iterator find(string_type const& key) const
    {
        return this->find_impl(key.data(), key.size());
    }
    //! The method finds the attribute by name
    const_iterator find(const char_type* key) const
    {
        typedef std::char_traits< char_type > traits_type;
        return this->find_impl(key, traits_type::length(key));
    }
    //! The method returns the number of the same named attributes in the container
    size_type count(key_type const& key) const { return size_type(find(key) != end()); }
    //! The method returns the number of the same named attributes in the container
    size_type count(string_type const& key) const { return size_type(find(key) != end()); }
    //! The method returns the number of the same named attributes in the container
    size_type count(const char_type* key) const { return size_type(find(key) != end()); }

    //! Combined lookup/insertion operator
    reference_proxy operator[] (key_type const& key)
    {
        return reference_proxy(this, key.data(), key.size());
    }
    //! Combined lookup/insertion operator
    reference_proxy operator[] (string_type const& key)
    {
        return reference_proxy(this, key.data(), key.size());
    }
    //! Combined lookup/insertion operator
    reference_proxy operator[] (const char_type* key)
    {
        typedef std::char_traits< char_type > traits_type;
        return reference_proxy(this, key, traits_type::length(key));
    }
    //! Combined lookup/insertion operator
    mapped_type operator[] (key_type const& key) const
    {
        const_iterator it = find(key);
        if (it != end())
            return it->second;
        else
            return mapped_type();
    }
    //! Combined lookup/insertion operator
    mapped_type operator[] (string_type const& key) const
    {
        const_iterator it = find(key);
        if (it != end())
            return it->second;
        else
            return mapped_type();
    }
    //! Combined lookup/insertion operator
    mapped_type operator[] (const char_type* key) const
    {
        const_iterator it = find(key);
        if (it != end())
            return it->second;
        else
            return mapped_type();
    }

    //! Insertion method
    BOOST_LOG_EXPORT std::pair< iterator, bool > insert(key_type const& key, mapped_type const& data);

    //! Insertion method
    std::pair< iterator, bool > insert(const_reference value)
    {
        return insert(value.first, value.second);
    }

    //! Mass insertion method
    template< typename FwdIteratorT >
    void insert(FwdIteratorT begin, FwdIteratorT end)
    {
        for (; begin != end; ++begin)
            insert(*begin);
    }

    //! Mass insertion method with ability to return iterators to the inserted elements
    template< typename FwdIteratorT, typename OutputIteratorT >
    void insert(FwdIteratorT begin, FwdIteratorT end, OutputIteratorT out)
    {
        for (; begin != end; ++begin, ++out)
            *out = insert(*begin);
    }

    //! The method erases all attributes with the specified name
    BOOST_LOG_EXPORT size_type erase(key_type const& key);
    //! The method erases the specified attribute
    BOOST_LOG_EXPORT void erase(iterator it);
    //! The method erases all attributes within the specified range
    BOOST_LOG_EXPORT void erase(iterator begin, iterator end);

    //! The method clears the container
    BOOST_LOG_EXPORT void clear();

private:
    //! Internal lookup implementation
    BOOST_LOG_EXPORT iterator find_impl(const char_type* key, size_type len);
    //! Internal lookup implementation
    const_iterator find_impl(const char_type* key, size_type len) const
    {
        return const_iterator(const_cast< basic_attribute_set* >(this)->find_impl(key, len));
    }
};

//! Free swap overload
template< typename CharT >
inline void swap(basic_attribute_set< CharT >& left, basic_attribute_set< CharT >& right)
{
    left.swap(right);
}

typedef basic_attribute_set< char > attribute_set;
typedef basic_attribute_set< wchar_t > wattribute_set;

} // namespace log

} // namespace boost

#endif // BOOST_LOG_ATTRIBUTE_SET_HPP_INCLUDED_
