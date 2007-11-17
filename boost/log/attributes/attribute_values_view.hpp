/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   attribute_values_view.hpp
 * \author Andrey Semashev
 * \date   21.04.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_ATTRIBUTE_VALUES_VIEW_HPP_INCLUDED_
#define BOOST_LOG_ATTRIBUTE_VALUES_VIEW_HPP_INCLUDED_

#include <memory>
#include <iterator>
#include <algorithm>
#include <boost/shared_ptr.hpp>
#include <boost/utility/addressof.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/detail/slim_string.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/attributes/attribute_set.hpp>

namespace boost {

namespace log {

//! A generated attribute values view
template< typename CharT >
class basic_attribute_values_view :
    private std::allocator< char >
{
    //! Self type
    typedef basic_attribute_values_view this_type;

public:
    //! Char type
    typedef CharT char_type;
    //! String type
    typedef std::basic_string< char_type > string_type;
    //! Key type
    typedef aux::basic_slim_string< char_type > key_type;
    //! Mapped attribute type
    typedef shared_ptr< attribute_value > mapped_type;
    //! Corresponding attribute set type
    typedef basic_attribute_set< char_type > attribute_set;

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
    typedef std::allocator< char > internal_allocator_type;
    struct implementation;

    //! Contained node type
    struct node
    {
        value_type m_Value;
        attribute* m_pAttribute;

        node(key_type const& key, attribute* attr)
            : m_Value(key, mapped_type()), m_pAttribute(attr)
        {
        }
    };

public:
    //! Const iterator class
    class const_iterator;
    friend class const_iterator;
    class const_iterator
    {
    public:
        //  Standard typedefs
        typedef typename this_type::difference_type difference_type;
        typedef typename this_type::value_type value_type;
        typedef typename this_type::const_reference reference;
        typedef typename this_type::const_pointer pointer;
        typedef std::bidirectional_iterator_tag iterator_category;

    public:
        //  Constructors
        const_iterator() : m_pNode(NULL) {}
        explicit const_iterator(node* pNode) : m_pNode(pNode) {}

        //  Comparison
        bool operator== (const_iterator const& that) const { return (m_pNode == that.m_pNode); }
        bool operator!= (const_iterator const& that) const { return (m_pNode != that.m_pNode); }

        //  Modification
        const_iterator& operator++ ()
        {
            ++m_pNode;
            return *this;
        }
        const_iterator& operator-- ()
        {
            --m_pNode;
            return *this;
        }
        const_iterator operator++ (int)
        {
            const_iterator tmp(*this);
            ++m_pNode;
            return tmp;
        }
        const_iterator operator-- (int)
        {
            const_iterator tmp(*this);
            --m_pNode;
            return tmp;
        }

        //  Dereferencing
        pointer operator-> () const
        {
            freeze_element();
            return addressof(m_pNode->m_Value);
        }
        reference operator* () const
        {
            freeze_element();
            return m_pNode->m_Value;
        }

        //! The method ensures that the pointed element has acquired the attribute value
        void freeze_element() const
        {
            if (!m_pNode->m_Value.second)
                m_pNode->m_Value.second = m_pNode->m_pAttribute->get_value();
        }

    private:
        //! The pointed element of the container
        node* m_pNode;
    };

private:
    //! Pointer to the container implementation
    implementation* m_pImpl;

public:
    //! The constructor adopts three attribute sets to the view
    BOOST_LOG_EXPORT basic_attribute_values_view(
        attribute_set const& source_attrs,
        attribute_set const& thread_attrs,
        attribute_set const& global_attrs);
    //! Copy constructor
    BOOST_LOG_EXPORT basic_attribute_values_view(basic_attribute_values_view const& that);
    //! Destructor
    BOOST_LOG_EXPORT ~basic_attribute_values_view();

    //! Assignment
    BOOST_LOG_EXPORT basic_attribute_values_view& operator= (basic_attribute_values_view const& that);

    //! Swap
    void swap(basic_attribute_values_view& that)
    {
        std::swap(m_pImpl, that.m_pImpl);
    }

    //  Iterator generators
    BOOST_LOG_EXPORT const_iterator begin() const;
    BOOST_LOG_EXPORT const_iterator end() const;

    //! The method returns number of elements in the container
    BOOST_LOG_EXPORT size_type size() const;
    //! The method checks if the container is empty
    bool empty() const { return (size() == 0); }

    //! The method finds the attribute by name
    const_iterator find(key_type const& key) const
    {
        return find_impl(key.data(), key.size());
    }
    //! The method finds the attribute by name
    const_iterator find(string_type const& key) const
    {
        return find_impl(key.data(), key.size());
    }
    //! The method finds the attribute by name
    const_iterator find(const char_type* key) const
    {
        typedef std::char_traits< char_type > traits_type;
        return find_impl(key, traits_type::length(key));
    }

    //! Alternative lookup syntax
    mapped_type operator[] (key_type const& key) const
    {
        const_iterator it = this->find(key);
        if (it != this->end())
            return it->second;
        else
            return mapped_type();
    }
    //! Alternative lookup syntax
    mapped_type operator[] (string_type const& key) const
    {
        const_iterator it = this->find(key);
        if (it != this->end())
            return it->second;
        else
            return mapped_type();
    }
    //! Alternative lookup syntax
    mapped_type operator[] (const char_type* key) const
    {
        const_iterator it = this->find(key);
        if (it != this->end())
            return it->second;
        else
            return mapped_type();
    }

    //! The method returns the number of the same named attributes in the container
    size_type count(key_type const& key) const { return size_type(find(key) != end()); }
    //! The method returns the number of the same named attributes in the container
    size_type count(string_type const& key) const { return size_type(find(key) != end()); }
    //! The method returns the number of the same named attributes in the container
    size_type count(const char_type* key) const { return size_type(find(key) != end()); }

    //! The method acquires values of all adopted attributes. Users don't need to call it, since will always get an already frozen view.
    BOOST_LOG_EXPORT void freeze();

private:
    //! Internal lookup implementation
    BOOST_LOG_EXPORT const_iterator find_impl(const char_type* key, size_type len) const;
};

//! Free swap overload
template< typename CharT >
inline void swap(basic_attribute_values_view< CharT >& left, basic_attribute_values_view< CharT >& right)
{
    left.swap(right);
}

typedef basic_attribute_values_view< char > attribute_values_view;
typedef basic_attribute_values_view< wchar_t > wattribute_values_view;

} // namespace log

} // namespace boost

#endif // BOOST_LOG_ATTRIBUTE_VALUES_VIEW_HPP_INCLUDED_
