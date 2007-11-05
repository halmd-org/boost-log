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
#include <algorithm>
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

    //! A wrapper around std::list to cache the size of the container
    template< typename T >
    class reduced_list
    {
        //! Container type
        typedef std::list< T > container_type;

    public:
        //  Standard typedefs
        typedef typename container_type::value_type value_type;
        typedef typename container_type::pointer pointer;
        typedef typename container_type::const_pointer const_pointer;
        typedef typename container_type::reference reference;
        typedef typename container_type::const_reference const_reference;
        typedef typename container_type::size_type size_type;
        typedef typename container_type::difference_type difference_type;
        typedef typename container_type::allocator_type allocator_type;
        typedef typename container_type::iterator iterator;
        typedef typename container_type::const_iterator const_iterator;

    private:
        //! Underlying container
        container_type m_Container;
        //! Container size
        size_type m_Size;

    public:
        //! Default constructor
        reduced_list();
        //! Copy constructor
        reduced_list(reduced_list const& that);

        //! Assignment
        reduced_list& operator= (reduced_list const& that);

        //  Iterator acquirement
        iterator begin() { return m_Container.begin(); }
        iterator end() { return m_Container.end(); }
        const_iterator begin() const { return m_Container.begin(); }
        const_iterator end() const { return m_Container.end(); }

        //! Size accessor
        size_type size() const { return m_Size; }
        //! Empty checker
        bool empty() const { return (m_Size == 0); }

        //! Clears the container
        void clear();

        //! Swaps two containers
        void swap(reduced_list& that)
        {
            using std::swap;
            m_Container.swap(that.m_Container);
            swap(m_Size, that.m_Size);
        }

        //  Insertion
        iterator insert(iterator pos, const_reference x);

        //  Erasure
        iterator erase(iterator pos);
        iterator erase(iterator first, iterator last);

        //  Push/pop front/back
        void push_front(const_reference x);
        void push_back(const_reference x);
        void pop_front();
        void pop_back();

        //! Resize
        void resize(size_type new_size, const_reference x);
    };

    //! A free-standing swap for node container
    template< typename T >
    inline void swap(reduced_list< T >& left, reduced_list< T >& right)
    {
        left.swap(right);
    }

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
            typedef reduced_list< T > type;
        };
    };

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
