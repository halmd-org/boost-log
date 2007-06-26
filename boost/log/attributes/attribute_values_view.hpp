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

#include <string>
#include <vector>
#include <boost/ref.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/log/detail/prologue.hpp>
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
        //! Attribute name type
        typedef std::basic_string< CharT > key_type;
        //! Mapped attribute value type
        typedef shared_ptr< attribute_value > mapped_type;
        //! Value type
        typedef std::pair< reference_wrapper< const key_type >, mapped_type > value_type;
        //! Internal value type used to actually store container value along with some additional data
        struct internal_value_type :
            public value_type
        {
            //! An attribute_set iterator
            typedef typename basic_attribute_set< CharT >::const_iterator attribute_set_iterator;
            attribute_set_iterator m_itAttribute;

            //! Transparent constructor
            internal_value_type(key_type const& key, mapped_type const& data)
                : value_type(cref(key), data) {}
        };

        //! Metafunction to make node container
        template< typename T >
        struct make_node_container
        {
            typedef std::vector< T > type;
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
            template< typename >
            friend class iterator;
            friend struct attribute_values_view_descr< CharT >;

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

        private:
            //! Dereferencing
            typename base_type::reference dereference() const
            {
                if (!this->base()->second)
                {
                    // The attribute value is not generated yet
                    const_cast< mapped_type& >(this->base()->second) =
                        this->base()->m_itAttribute->second->get_value();
                }

                return *this->base();
            }

            //  Hide internals from the user
            using base_type::base;
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
    //! Constructor
    basic_attribute_values_view() {}

    //! The method adopts three attribute sets to the view
    BOOST_LOG_EXPORT void adopt(
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
