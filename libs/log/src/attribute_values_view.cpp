/*
 *          Copyright Andrey Semashev 2007 - 2010.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   attribute_values_view.cpp
 * \author Andrey Semashev
 * \date   19.04.2007
 *
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#include <new>
#include <boost/compatibility/cpp_c_headers/cstring> // memset
#include <boost/assert.hpp>
#include <boost/limits.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_unsigned.hpp>
#include <boost/type_traits/alignment_of.hpp>
#include <boost/intrusive/options.hpp>
#include <boost/intrusive/list.hpp>
#include <boost/intrusive/link_mode.hpp>
#include <boost/intrusive/derivation_value_traits.hpp>
#include <boost/log/attributes/attribute_name.hpp>
#include <boost/log/attributes/attribute_value.hpp>
#include <boost/log/attributes/attribute_values_view.hpp>
#ifndef BOOST_LOG_NO_THREADS
#include <boost/detail/atomic_count.hpp>
#endif

namespace boost {

namespace BOOST_LOG_NAMESPACE {

template< typename CharT >
inline basic_attribute_values_view< CharT >::node_base::node_base() :
    m_pPrev(NULL),
    m_pNext(NULL)
{
}

template< typename CharT >
inline basic_attribute_values_view< CharT >::node::node(key_type const& key, mapped_type const& data) :
    node_base(),
    m_Value(key, data)
{
}

//! Container implementation
template< typename CharT >
struct basic_attribute_values_view< CharT >::implementation
{
public:
    typedef typename key_type::id_type id_type;

private:
    //! Node base class traits for the intrusive list
    struct node_traits
    {
        typedef node_base node;
        typedef node* node_ptr;
        typedef node const* const_node_ptr;
        static node* get_next(const node* n) { return n->m_pNext; }
        static void set_next(node* n, node* next) { n->m_pNext = next; }
        static node* get_previous(const node* n) { return n->m_pPrev; }
        static void set_previous(node* n, node* prev) { n->m_pPrev = prev; }
    };

    //! Contained node traits for the intrusive list
    typedef intrusive::derivation_value_traits<
        node,
        node_traits,
        intrusive::normal_link
    > value_traits;

    //! A container that provides iteration through elements of the container
    typedef intrusive::list<
        node,
        intrusive::value_traits< value_traits >,
        intrusive::constant_time_size< true >
    > node_list;

    //! Element disposer
    struct disposer
    {
        typedef void result_type;
        void operator() (node* p) const
        {
            p->~node();
        }
    };

    //! Node status values - exact values are important
    enum node_status
    {
        uninitialized = 0,
        absent = 1,
        present = 3
    };

    // Algorithms with node statuses require this
    BOOST_STATIC_ASSERT(std::numeric_limits< unsigned char >::digits == 8);
    BOOST_STATIC_ASSERT(is_unsigned< id_type >::value);

private:
    //! Reference count
#ifndef BOOST_LOG_NO_THREADS
    boost::detail::atomic_count m_RefCount;
#else
    unsigned int m_RefCount;
#endif
    //! The container with elements
    node_list m_Nodes;
    //! The base identifier value for attribute names
    id_type m_BaseID;
    //! The max identifier value in the view
    id_type m_MaxID;
    //! The pointer to the beginning of the storage of the elements
    node* m_pStorage;

    //! Pointer to the source-specific attributes
    const attribute_set_type* m_pSourceAttributes;
    //! Pointer to the thread-specific attributes
    const attribute_set_type* m_pThreadAttributes;
    //! Pointer to the global attributes
    const attribute_set_type* m_pGlobalAttributes;

private:
    //! Constructor
    implementation(
        id_type base_id,
        id_type max_id,
        node* storage,
        attribute_set_type const& source_attrs,
        attribute_set_type const& thread_attrs,
        attribute_set_type const& global_attrs
    ) :
        m_RefCount(1),
        m_BaseID(base_id),
        m_MaxID(max_id),
        m_pStorage(storage),
        m_pSourceAttributes(&source_attrs),
        m_pThreadAttributes(&thread_attrs),
        m_pGlobalAttributes(&global_attrs)
    {
    }

    //! Destructor
    ~implementation()
    {
        m_Nodes.clear_and_dispose(disposer());
    }

public:
    //! The function allocates memory and creates the object
    static implementation* create(
        internal_allocator_type& alloc,
        attribute_set_type const& source_attrs,
        attribute_set_type const& thread_attrs,
        attribute_set_type const& global_attrs)
    {
        // Calculate the range of attribute name identifiers
        id_type base_id = (std::numeric_limits< id_type >::max)(), max_id = 0;
        if (!source_attrs.empty())
        {
            id_type id = source_attrs.begin()->first.id();
            if (id < base_id)
                base_id = id;
            id = (--(source_attrs.end()))->first.id();
            if (id > max_id)
                max_id = id;
        }
        if (!thread_attrs.empty())
        {
            id_type id = thread_attrs.begin()->first.id();
            if (id < base_id)
                base_id = id;
            id = (--(thread_attrs.end()))->first.id();
            if (id > max_id)
                max_id = id;
        }
        if (!global_attrs.empty())
        {
            id_type id = global_attrs.begin()->first.id();
            if (id < base_id)
                base_id = id;
            id = (--(global_attrs.end()))->first.id();
            if (id > max_id)
                max_id = id;
        }

        const std::size_t element_count = max_id - base_id + 1;

        // Calculate the buffer size
        const std::size_t statuses_size = ((element_count + 3) >> 2); // the table of node statuses
        const std::size_t size1 = sizeof(implementation) + statuses_size;
        const std::size_t storage_offset = size1 +
            alignment_of< node >::value - (size1 % alignment_of< node >::value); // alignment for nodes

        implementation* p = reinterpret_cast< implementation* >(
            alloc.allocate(storage_offset + element_count * sizeof(node)));
        new (p) implementation(
            base_id,
            max_id,
            reinterpret_cast< node* >(reinterpret_cast< char* >(p) + storage_offset),
            source_attrs,
            thread_attrs,
            global_attrs);

        // Set all node statuses to uninitialized
        std::memset(p + 1, 0, statuses_size);

        return p;
    }

    //! Destroys the object and releases the memory
    static void destroy(internal_allocator_type& alloc, implementation* p)
    {
        const std::size_t element_count = p->m_MaxID - p->m_BaseID + 1;

        p->~implementation();

        // Calculate the buffer size
        const std::size_t statuses_size = ((element_count + 3) >> 2); // the table of node statuses
        const std::size_t size1 = sizeof(implementation) + statuses_size;
        const std::size_t storage_offset = size1 +
            alignment_of< node >::value - (size1 % alignment_of< node >::value); // alignment for nodes

        alloc.deallocate(
            reinterpret_cast< typename internal_allocator_type::pointer >(p),
            storage_offset + element_count * sizeof(node));
    }

    //! Increases reference count for the container
    void add_ref()
    {
        ++m_RefCount;
    }
    //! Decreases reference count for the container
    unsigned int release()
    {
        return --m_RefCount;
    }

    //! Returns the pointer to the first element
    node_base* begin()
    {
        freeze();
        return m_Nodes.begin().pointed_node();
    }
    //! Returns the pointer after the last element
    node_base* end()
    {
        return m_Nodes.end().pointed_node();
    }

    //! Returns the number of elements in the container
    size_type size() const
    {
        const_cast< implementation* >(this)->freeze();
        return m_Nodes.size();
    }

    //! Looks for the element with an equivalent key
    node_base* find(key_type key) const
    {
        id_type index = key.id() - m_BaseID;
        if (index <= m_MaxID && const_cast< implementation* >(this)->freeze_node(key) == present)
            return m_pStorage + index;
        else
            return m_Nodes.end().pointed_node();
    }

    //! Freezes all elements of the container
    void freeze()
    {
        if (m_pSourceAttributes)
        {
            // Set all "frozen" flags so that all values that have not been acquired yet appear as absent
            unsigned char* p = reinterpret_cast< unsigned char* >(this + 1);
            // m_MaxID - m_BaseID + 1 = element count in storage; + 3 is added to ceil the result
            unsigned char* e = p + ((m_MaxID - m_BaseID + 1 + 3) >> 2);
            for (; p != e; ++p)
                *p |= 85; // 01010101b

            freeze_from(m_pSourceAttributes);
            freeze_from(m_pThreadAttributes);
            freeze_from(m_pGlobalAttributes);
            m_pSourceAttributes = m_pThreadAttributes = m_pGlobalAttributes = NULL;
        }
    }

private:
    //! The function returns the node status
    node_status get_status(id_type id) const
    {
        const unsigned char* const p = reinterpret_cast< const unsigned char* >(this + 1) + (id >> 2);
        return (node_status)(((*p) >> ((id << 1) & 7)) & 3);
    }
    //! The function sets the node status
    void set_status(id_type id, node_status status)
    {
        unsigned char* const p = reinterpret_cast< unsigned char* >(this + 1) + (id >> 2);
        *p |= static_cast< unsigned char >(status) << ((id << 1) & 7);
    }

    //! Acquires the attribute value from the attribute sets
    node_status freeze_node(key_type key)
    {
        node_status status = get_status(key.id());
        if (status == uninitialized)
        {
            node* p = m_pStorage + key.id() - m_BaseID;
            typename attribute_set_type::const_iterator it = m_pSourceAttributes->find(key);
            if (it == m_pSourceAttributes->end())
            {
                it = m_pThreadAttributes->find(key);
                if (it == m_pThreadAttributes->end())
                {
                    it = m_pGlobalAttributes->find(key);
                    if (it == m_pGlobalAttributes->end())
                    {
                        // The attribute is not found
                        set_status(key.id(), absent);
                        return absent;
                    }
                }
            }

            new (p) node(key, it->second->get_value());
            m_Nodes.push_back(*p);
            set_status(key.id(), present);
            return present;
        }

        return status;
    }

    //! Acquires attribute values from the set of attributes
    void freeze_from(const attribute_set_type* attrs)
    {
        typename attribute_set_type::const_iterator
            it = attrs->begin(), end = attrs->end();
        for (; it != end; ++it)
        {
            register id_type id = it->first.id();
            if (get_status(id) != present)
            {
                node* p = m_pStorage + id - m_BaseID;
                new (p) node(it->first, it->second->get_value());
                m_Nodes.push_back(*p);
                set_status(id, present);
            }
        }
    }
};

//! The constructor adopts three attribute sets to the view
template< typename CharT >
basic_attribute_values_view< CharT >::basic_attribute_values_view(
    attribute_set_type const& source_attrs,
    attribute_set_type const& thread_attrs,
    attribute_set_type const& global_attrs
) :
    m_pImpl(implementation::create(
        static_cast< internal_allocator_type& >(*this), source_attrs, thread_attrs, global_attrs))
{
}

//! Copy constructor
template< typename CharT >
basic_attribute_values_view< CharT >::basic_attribute_values_view(basic_attribute_values_view const& that) :
    internal_allocator_type(static_cast< internal_allocator_type const& >(that)),
    m_pImpl(that.m_pImpl)
{
    m_pImpl->add_ref();
}

//! Destructor
template< typename CharT >
basic_attribute_values_view< CharT >::~basic_attribute_values_view()
{
    if (m_pImpl->release() == 0)
        implementation::destroy(static_cast< internal_allocator_type& >(*this), m_pImpl);
}

//! Assignment
template< typename CharT >
basic_attribute_values_view< CharT >& basic_attribute_values_view< CharT >::operator= (basic_attribute_values_view that)
{
    swap(that);
    return *this;
}

//  Iterator generators
template< typename CharT >
typename basic_attribute_values_view< CharT >::const_iterator
basic_attribute_values_view< CharT >::begin() const
{
    return const_iterator(m_pImpl->begin(), const_cast< basic_attribute_values_view* >(this));
}

template< typename CharT >
typename basic_attribute_values_view< CharT >::const_iterator
basic_attribute_values_view< CharT >::end() const
{
    return const_iterator(m_pImpl->end(), const_cast< basic_attribute_values_view* >(this));
}

//! The method returns number of elements in the container
template< typename CharT >
typename basic_attribute_values_view< CharT >::size_type basic_attribute_values_view< CharT >::size() const
{
    return m_pImpl->size();
}

//! Internal lookup implementation
template< typename CharT >
typename basic_attribute_values_view< CharT >::const_iterator
basic_attribute_values_view< CharT >::find(key_type key) const
{
    return const_iterator(m_pImpl->find(key), const_cast< basic_attribute_values_view* >(this));
}

//! The method acquires values of all adopted attributes. Users don't need to call it, since will always get an already frozen view.
template< typename CharT >
void basic_attribute_values_view< CharT >::freeze()
{
    m_pImpl->freeze();
}

//! Explicitly instantiate container implementation
#ifdef BOOST_LOG_USE_CHAR
template class basic_attribute_values_view< char >;
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
template class basic_attribute_values_view< wchar_t >;
#endif

} // namespace log

} // namespace boost
