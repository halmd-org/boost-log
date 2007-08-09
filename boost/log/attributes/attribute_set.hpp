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
#include <string>
#include <utility>
#include <iterator>
#include <boost/shared_ptr.hpp>
#include <boost/mpl/if.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/log/detail/prologue.hpp>
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
        //! Attribute name type
        typedef std::basic_string< CharT > key_type;
        //! Mapped attribute type
        typedef shared_ptr< attribute > mapped_type;
        //! Value type
        typedef std::pair< const key_type, mapped_type > value_type;
        //! Internal value type used to actually store container value along with some additional data
        typedef value_type internal_value_type;

        //! Metafunction to make node container
        template< typename T >
        struct make_node_container
        {
            typedef std::list< T > type;
        };

        //! Container iterator type
        template< typename NodeIteratorT >
        class iterator :
            public iterator_adaptor<
                iterator< NodeIteratorT >,
                NodeIteratorT,
                value_type,
                typename iterator_category< NodeIteratorT >::type,
                typename mpl::if_<
                    is_const_iterator< NodeIteratorT >,
                    value_type const&,
                    value_type&
                >::type
            >
        {
            friend class ::boost::iterator_core_access;
            friend struct attribute_set_descr< CharT >;

        private:
            //! Base type
            typedef typename iterator::iterator_adaptor_ base_type;

        public:
            //! Constructor
            iterator() {}
            //! Copy constructor
            iterator(iterator const& that) : base_type(static_cast< base_type const& >(that)) {}
            //! Conversion constructor
            template< typename AnotherNodeIteratorT >
            iterator(iterator< AnotherNodeIteratorT > const& that) : base_type(that.base()) {}
            //! Constructor
            explicit iterator(NodeIteratorT const& it) : base_type(it) {}

            //! Assignment
            iterator& operator= (NodeIteratorT const& it)
            {
                this->base_reference() = it;
                return *this;
            }
        };

        //! Node iterator extractor
        template< typename NodeIteratorT >
        static NodeIteratorT get_node_iterator(iterator< NodeIteratorT > const& it)
        {
            return it.base();
        }

        //! Key extractor
        static key_type const& get_key(value_type const& value)
        {
            return value.first;
        }
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
    BOOST_LOG_EXPORT iterator find(key_type const& key);
    //! The method returns a range of the same named attributes
    BOOST_LOG_EXPORT std::pair< iterator, iterator > equal_range(key_type const& key);

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
        for (; begin != end; ++begin)
        {
            *out = insert(*begin);
            ++out;
        }
    }

    //! The method erases all attributes with the specified name
    BOOST_LOG_EXPORT size_type erase(key_type const& key);
    //! The method erases the specified attribute
    BOOST_LOG_EXPORT void erase(iterator it);
    //! The method erases all attributes within the specified range
    BOOST_LOG_EXPORT void erase(iterator begin, iterator end);
};

typedef basic_attribute_set< char > attribute_set;
typedef basic_attribute_set< wchar_t > wattribute_set;

} // namespace log

} // namespace boost

#endif // BOOST_LOG_ATTRIBUTE_SET_HPP_INCLUDED_
