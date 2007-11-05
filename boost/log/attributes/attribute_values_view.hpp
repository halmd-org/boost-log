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
#include <boost/shared_ptr.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/detail/slim_string.hpp>
#include <boost/log/detail/unordered_mmap_facade.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/attributes/attribute_set.hpp>

namespace boost {

namespace log {

namespace aux {

    //! A simple vector-like container that does not require value_type to be assignable
    template< typename T >
    class reduced_vector :
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
        reduced_vector();
        reduced_vector(reduced_vector const& that);
        ~reduced_vector();

        //! Assignment
        reduced_vector& operator= (reduced_vector const& that);

        //  Iterator acquirement
        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;

        //  Accessors
        size_type size() const;
        bool empty() const;

        //! Swaps two containers
        void swap(reduced_vector& that);

        //! Storage reservation
        void reserve(size_type n);

        //! Appends a new value to the end of the container
        void push_back(const_reference x);

        //! The method extracts attribute values and arranges them in the container
        template< typename IteratorT >
        void adopt_nodes(IteratorT& it, IteratorT end, unsigned char HTIndex);
    };

    //! A free-standing swap for node container
    template< typename T >
    inline void swap(reduced_vector< T >& left, reduced_vector< T >& right)
    {
        left.swap(right);
    }

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
            typedef reduced_vector< T > type;
        };
    };

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

    //! Copy constructor
	basic_attribute_values_view(basic_attribute_values_view const& that)
        : base_type(static_cast< base_type const& >(that))
    {
    }

	//! Assignment
    BOOST_LOG_EXPORT basic_attribute_values_view& operator= (basic_attribute_values_view const& that);
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
