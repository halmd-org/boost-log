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

#include <new>
#include <memory>
#include <cassert>
#include <boost/shared_ptr.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/detail/slim_string.hpp>
#include <boost/log/detail/unordered_mmap_facade.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/attributes/attribute_set.hpp>

namespace boost {

namespace log {

namespace aux {

    //! A value descriptor for unordered_multimap_facade
    template< typename CharT >
    struct attribute_values_view_descr
    {
        //! Char type
        typedef CharT char_type;
        //! Mapped attribute value type
        typedef shared_ptr< attribute_value > mapped_type;

        //! Metafunction to make node container
        template< typename T >
        struct make_node_container
        {
            //! A simple vector-like container that does not require value_type to be assignable
            class type :
                private std::allocator< T >
            {
            public:
                //  Standard typedefs
                typedef std::allocator< T > allocator_type;
                typedef typename allocator_type::value_type value_type;
                typedef typename allocator_type::pointer pointer;
                typedef typename allocator_type::const_pointer const_pointer;
                typedef typename allocator_type::reference reference;
                typedef typename allocator_type::const_reference const_reference;
                typedef typename allocator_type::size_type size_type;
                typedef typename allocator_type::difference_type difference_type;
                typedef pointer iterator;
                typedef const_pointer const_iterator;

            private:
                //! Pointer to the beginning of the storage
                pointer m_pBegin;
                //! Pointer to the after-the-last element
                pointer m_pEnd;
                //! Pointer to the end of storage
                pointer m_pEOS;

            public:
                //  Structors
                type() : m_pBegin(0), m_pEnd(0), m_pEOS(0) {}
                type(type const& that)
                    : m_pBegin(allocator_type::allocate(that.size())), m_pEnd(m_pBegin), m_pEOS(m_pBegin + that.size())
                {
                    for (const_iterator it = that.begin(); it != that.end(); ++it)
                        this->push_back(*it); // won't throw
                }
                ~type()
                {
                    for (pointer p = m_pBegin; p != m_pEnd; ++p)
                        p->~value_type();
                    allocator_type::deallocate(m_pBegin, m_pEOS - m_pBegin);
                }

                //! Assignment
                type& operator= (type const& that)
                {
                    type tmp(that);
                    this->swap(tmp);
                    return *this;
                }

                //  Iterator acquirement
                iterator begin() { return m_pBegin; }
                iterator end() { return m_pEnd; }
                const_iterator begin() const { return m_pBegin; }
                const_iterator end() const { return m_pEnd; }

                //  Accessors
                size_type size() const { return (m_pEnd - m_pBegin); }
                bool empty() const { return (size() == 0); }

                //! Swaps two containers
                void swap(type& that)
                {
                    using std::swap;
                    swap(m_pBegin, that.m_pBegin);
                    swap(m_pEnd, that.m_pEnd);
                    swap(m_pEOS, that.m_pEOS);
                }

                //! Storage reservation
                void reserve(size_type n)
                {
                    // Should be called once, before any insertions
                    assert(m_pBegin == 0);
                    m_pBegin = m_pEnd = allocator_type::allocate(n);
                    m_pEOS = m_pBegin + n;
                }

                //! Appends a new value to the end of the container
                void push_back(const_reference x)
                {
                    // Should be called after reservation
                    assert(m_pBegin != 0);
                    assert(m_pEnd < m_pEOS);
                    new (m_pEnd) value_type(x);
                    ++m_pEnd;
                }

                //! The method extracts attribute values and arranges them in the container
                template< typename IteratorT >
                void adopt_nodes(IteratorT& it, IteratorT end, unsigned char HTIndex)
                {
                    for (; it != end && it->m_HTIndex == HTIndex; ++it, ++m_pEnd)
                    {
                        new (m_pEnd) value_type(it->first, it->second->get_value(), HTIndex);
                    }
                }
            };
        };
    };

    //! A free-standing swap for node container
    template< typename CharT, typename T >
    inline void swap(
        typename attribute_values_view_descr< CharT >::BOOST_NESTED_TEMPLATE make_node_container< T >::type& left,
        typename attribute_values_view_descr< CharT >::BOOST_NESTED_TEMPLATE make_node_container< T >::type& right)
    {
        left.swap(right);
    }

} // namespace aux

//! A generated attribute values view
template< typename CharT >
class basic_attribute_values_view :
    public aux::unordered_multimap_facade< aux::attribute_values_view_descr< CharT > >
{
private:
    //! Base type
    typedef aux::unordered_multimap_facade<
        aux::attribute_values_view_descr< CharT >
    > base_type;

public:
    //! Value type
    typedef typename base_type::value_type value_type;
    //! Reference type
    typedef typename base_type::reference reference;
    //! Const reference type
    typedef typename base_type::const_reference const_reference;
    //! Pointer type
    typedef typename base_type::pointer pointer;
    //! Const pointer type
    typedef typename base_type::const_pointer const_pointer;
    //! Size type
    typedef typename base_type::size_type size_type;
    //! Difference type
    typedef typename base_type::difference_type difference_type;

    //! Key type
    typedef typename base_type::key_type key_type;
    //! Mapped type
    typedef typename base_type::mapped_type mapped_type;

    //! Character type
    typedef typename base_type::char_type char_type;
    //! Attribute set type
    typedef basic_attribute_set< char_type > attribute_set;

    //! Const iterator type
    typedef typename base_type::const_iterator const_iterator;

protected:
    //! Container descriptor
    typedef typename base_type::descriptor descriptor;
    //! Node type
    typedef typename base_type::node node;
    //! Node container type
    typedef typename base_type::node_container node_container;
    //! Bucket container
    typedef typename base_type::hash_table hash_table;

public:
    //! The constructor adopts three attribute sets to the view
    BOOST_LOG_EXPORT basic_attribute_values_view(
        attribute_set const& source_attrs,
        attribute_set const& thread_attrs,
        attribute_set const& global_attrs);

private:
    //! Copy constructor (closed)
    basic_attribute_values_view(basic_attribute_values_view const&);
    //! Assignment (closed)
    basic_attribute_values_view& operator= (basic_attribute_values_view const&);
};

typedef basic_attribute_values_view< char > attribute_values_view;
typedef basic_attribute_values_view< wchar_t > wattribute_values_view;

} // namespace log

} // namespace boost

#endif // BOOST_LOG_ATTRIBUTE_VALUES_VIEW_HPP_INCLUDED_
