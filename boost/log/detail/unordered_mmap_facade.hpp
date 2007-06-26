/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   unordered_mmap_facade.hpp
 * \author Andrey Semashev
 * \date   09.05.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_UNORDERED_MMAP_FACADE_HPP_INCLUDED_
#define BOOST_LOG_UNORDERED_MMAP_FACADE_HPP_INCLUDED_

#include <vector>
#include <utility>
#include <boost/mpl/if.hpp>
#include <boost/iterator/iterator_traits.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/log/detail/prologue.hpp>

#ifdef _MSC_VER
#pragma warning(push)
    // 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
    // non dll-interface class 'A' used as base for dll-interface class 'B'
#pragma warning(disable: 4275)
#endif // _MSC_VER

namespace boost {

namespace log {

namespace aux {

    //! A helper traits to check if an iterator is const
    template< typename ItT >
    struct is_const_iterator :
        public is_const<
            typename remove_reference<
                typename iterator_reference< ItT >::type
            >::type
        >
    {
    };

    //! A base class that implements a subset of unordered_multimap functionality
    template< typename DescriptorT >
    class unordered_multimap_facade
    {
    protected:
        //! Value descriptor type that provides facilities to operate on contained data
        typedef DescriptorT descriptor;

    public:
        //! Value type
        typedef typename descriptor::value_type value_type;
        //! Reference type
        typedef value_type& reference;
        //! Const reference type
        typedef value_type const& const_reference;
        //! Pointer type
        typedef value_type* pointer;
        //! Const pointer type
        typedef value_type const* const_pointer;
        //! Size type
        typedef std::size_t size_type;
        //! Difference type
        typedef std::ptrdiff_t difference_type;

        //! Key type
        typedef typename descriptor::key_type key_type;
        //! Mapped type
        typedef typename descriptor::mapped_type mapped_type;

        //! Character type
        typedef typename key_type::value_type char_type;

    protected:
        //! Container node
        struct node :
            public descriptor::internal_value_type
        {
        private:
            //! Base type
            typedef typename descriptor::internal_value_type base_type;

        public:
            //! Its hash table index
            unsigned char m_HTIndex;

            //! Constructor
            node(key_type const& k, mapped_type const& m, unsigned char h)
                : base_type(k, m), m_HTIndex(h) {}
        };
        //! The elements container type
        typedef typename descriptor::BOOST_NESTED_TEMPLATE make_node_container< node >::type node_container;
        //! The hash table type
        typedef std::vector<
            std::pair<
                typename node_container::iterator,
                typename node_container::iterator
            >
        > hash_table;

    public:
        //! Forward iterator type
        typedef typename descriptor::BOOST_NESTED_TEMPLATE iterator<
            typename node_container::iterator
        > iterator;
        //! Forward const iterator type
        typedef typename descriptor::BOOST_NESTED_TEMPLATE iterator<
            typename node_container::const_iterator
        > const_iterator;

    private:
        //! The container of elements
        node_container m_Nodes;
        //! Hash table
        hash_table m_HashTable;

    public:
        //! Default constructor
        BOOST_LOG_EXPORT unordered_multimap_facade();
        //! Copy constructor
        BOOST_LOG_EXPORT unordered_multimap_facade(unordered_multimap_facade const& that);
        //! Destructor
        BOOST_LOG_EXPORT ~unordered_multimap_facade();

        //  Iterator generators
        const_iterator begin() const { return const_iterator(m_Nodes.begin()); }
        const_iterator end() const { return const_iterator(m_Nodes.end()); }

        //! The method returns number of elements in the container
        size_type size() const { return m_Nodes.size(); }
        //! The method checks if the container is empty
        bool empty() const { return m_Nodes.empty(); }

        //! The method finds the attribute by name
        BOOST_LOG_EXPORT const_iterator find(key_type const& key) const;

        //! The method returns the number of the same named attributes in the container
        BOOST_LOG_EXPORT size_type count(key_type const& key) const;

        //! The method returns a range of the same named attributes
        BOOST_LOG_EXPORT std::pair< const_iterator, const_iterator > equal_range(key_type const& key) const;

        //! The method clears the container
        BOOST_LOG_EXPORT void clear();

    protected:
        //! Accessor to node container
        node_container& nodes() { return m_Nodes; }
        //! Accessor to node container
        node_container const& nodes() const { return m_Nodes; }

        //! Accessor to bucket container
        hash_table& buckets() { return m_HashTable; }
        //! Accessor to bucket container
        hash_table const& buckets() const { return m_HashTable; }

        //! The method rebuilds hash table
        void rehash();

        //! Find method implementation
        template< typename ResultT, typename ThisT >
        static ResultT find_impl(ThisT* pthis, key_type const& key);
        //! Equal_range method implementation
        template< typename ResultT, typename ThisT >
        static ResultT equal_range_impl(ThisT* pthis, key_type const& key);
    };

} // namespace aux

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_UNORDERED_MMAP_FACADE_HPP_INCLUDED_
