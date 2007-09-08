/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   attribute_set.hpp
 * \author Andrey Semashev
 * \date   08.03.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_ATTRIBUTE_SET_HPP_INCLUDED_
#define BOOST_LOG_ATTRIBUTE_SET_HPP_INCLUDED_

#include <list>
#include <boost/shared_ptr.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/detail/slim_string.hpp>
#include <boost/log/detail/unordered_mmap_facade.hpp>
#include <boost/log/attributes/attribute.hpp>

namespace boost {

namespace log {

template< typename >
class basic_attribute_values_view;

namespace aux {

    //! A value descriptor for unordered_multimap_facade
    template< typename CharT >
    struct attribute_set_descr
    {
        //! Char type
        typedef CharT char_type;
        //! Mapped attribute type
        typedef shared_ptr< attribute > mapped_type;

        //! Metafunction to make node container
        template< typename T >
        struct make_node_container
        {
            //! A wrapper around std::list to cache the size of the container
            class type :
                private std::list< T >
            {
                //! Base type
                typedef std::list< T > base_type;

            public:
                //  Standard typedefs
                typedef typename base_type::value_type value_type;
                typedef typename base_type::pointer pointer;
                typedef typename base_type::const_pointer const_pointer;
                typedef typename base_type::reference reference;
                typedef typename base_type::const_reference const_reference;
                typedef typename base_type::size_type size_type;
                typedef typename base_type::difference_type difference_type;
                typedef typename base_type::allocator_type allocator_type;
                typedef typename base_type::iterator iterator;
                typedef typename base_type::const_iterator const_iterator;
                typedef typename base_type::reverse_iterator reverse_iterator;
                typedef typename base_type::const_reverse_iterator const_reverse_iterator;

            private:
                //! Container size
                size_type m_Size;

            public:
                //! Default constructor
                type() : m_Size(0) {}
                //! Copy constructor
                type(type const& that) : base_type(static_cast< base_type const& >(that)), m_Size(that.m_Size) {}
                //! Ñonstructor from the range of values
                template< typename IteratorT >
                type(IteratorT first, IteratorT last) : m_Size(0)
                {
                    this->insert(this->end(), first, last);
                }

                //! Assignment
                type& operator= (type const& that)
                {
                    base_type::operator= (static_cast< base_type const& >(that));
                    m_Size = that.m_Size;
                    return *this;
                }

                using base_type::begin;
                using base_type::end;
                using base_type::rbegin;
                using base_type::rend;
                using base_type::front;
                using base_type::back;
                using base_type::max_size;

                //! Size accessor
                size_type size() const { return m_Size; }
                //! Empty checker
                bool empty() const { return (m_Size == 0); }

                //! Clears the container
                void clear()
                {
                    base_type::clear();
                    m_Size = 0;
                }

                //! Swaps two containers
                void swap(type& that)
                {
                    using std::swap;
                    base_type::swap(static_cast< base_type& >(that));
                    swap(m_Size, that.m_Size);
                }

                //  Insertion
                iterator insert(iterator pos, const_reference x)
                {
                    iterator it = base_type::insert(pos, x);
                    ++m_Size;
                    return it;
                }
                template< typename IteratorT >
                void insert(iterator pos, IteratorT first, IteratorT last)
                {
                    for (; first != last; ++first)
                        this->insert(pos, *first);
                }

                //  Erasure
                iterator erase(iterator pos)
                {
                    iterator it = base_type::erase(pos);
                    --m_Size;
                    return it;
                }
                iterator erase(iterator first, iterator last)
                {
                    while (first != last)
                        this->erase(first++);
                    return last;
                }

                //  Push/pop front/back
                void push_front(const_reference x)
                {
                    base_type::push_front(x);
                    ++m_Size;
                }
                void push_back(const_reference x)
                {
                    base_type::push_back(x);
                    ++m_Size;
                }
                void pop_front()
                {
                    base_type::pop_front();
                    --m_Size;
                }
                void pop_back()
                {
                    base_type::pop_back();
                    --m_Size;
                }

                //  Assign, resize, etc.
                void resize(size_type new_size, const_reference x)
                {
                    base_type::resize(new_size, x);
                    m_Size = new_size;
                }
                void assign(size_type n, const_reference val)
                {
                    base_type::assign(n, val);
                    m_Size = n;
                }
                template< typename IteratorT >
                void assign(IteratorT first, IteratorT last)
                {
                    type tmp(first, last);
                    this->swap(tmp);
                }
            };
        };
    };

    //! A free-standing swap for node container
    template< typename CharT, typename T >
    inline void swap(
        typename attribute_set_descr< CharT >::BOOST_NESTED_TEMPLATE make_node_container< T >::type& left,
        typename attribute_set_descr< CharT >::BOOST_NESTED_TEMPLATE make_node_container< T >::type& right)
    {
        left.swap(right);
    }

} // namespace aux

//! An attribute set class
template< typename CharT >
class basic_attribute_set :
    public aux::unordered_multimap_facade< aux::attribute_set_descr< CharT > >
{
    friend class basic_attribute_values_view< CharT >;

private:
    //! Base type
    typedef aux::unordered_multimap_facade<
        aux::attribute_set_descr< CharT >
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

    //! String type
    typedef typename base_type::string_type string_type;
    //! Key type
    typedef typename base_type::key_type key_type;
    //! Mapped type
    typedef typename base_type::mapped_type mapped_type;

    //! Character type
    typedef typename base_type::char_type char_type;

    //! Iterator type
    typedef typename base_type::iterator iterator;
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
    //! Default constructor
    basic_attribute_set() : base_type() {}
    //! Copy constructor
    basic_attribute_set(basic_attribute_set const& that)
        : base_type(static_cast< base_type const& >(that))
    {
    }

    //! Constructor with contents initialization
    template< typename FwdItT >
    basic_attribute_set(FwdItT begin, FwdItT end) : base_type()
    {
        for (; begin != end; ++begin)
            this->insert(*begin);
    }

    //! Assignment
    BOOST_LOG_EXPORT basic_attribute_set& operator= (basic_attribute_set const& that);

    //  Iterator generators
    iterator begin() { return iterator(this->nodes().begin()); }
    iterator end() { return iterator(this->nodes().end()); }

    //! The method swaps two containers
    void swap(basic_attribute_set& that)
    {
        this->nodes().swap(that.nodes());
        this->buckets().swap(that.buckets());
    }

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

    //! The method returns a range of the same named attributes
    std::pair< iterator, iterator > equal_range(key_type const& key)
    {
        return this->equal_range_impl(key.data(), key.size());
    }
    //! The method returns a range of the same named attributes
    std::pair< iterator, iterator > equal_range(string_type const& key)
    {
        return this->equal_range_impl(key.data(), key.size());
    }
    //! The method returns a range of the same named attributes
    std::pair< iterator, iterator > equal_range(const char_type* key)
    {
        typedef std::char_traits< char_type > traits_type;
        return this->equal_range_impl(key, traits_type::length(key));
    }

    //! Insertion method
    BOOST_LOG_EXPORT iterator insert(key_type const& key, mapped_type const& data);

    //! Insertion method
    iterator insert(const_reference value)
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
};

typedef basic_attribute_set< char > attribute_set;
typedef basic_attribute_set< wchar_t > wattribute_set;

} // namespace log

} // namespace boost

#endif // BOOST_LOG_ATTRIBUTE_SET_HPP_INCLUDED_
