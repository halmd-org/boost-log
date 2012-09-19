/*
 *          Copyright Andrey Semashev 2007 - 2012.
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
#include <memory>
#include <boost/array.hpp>
#include <boost/intrusive/options.hpp>
#include <boost/intrusive/list.hpp>
#include <boost/intrusive/link_mode.hpp>
#include <boost/intrusive/derivation_value_traits.hpp>
#include <boost/log/attributes/attribute_name.hpp>
#include <boost/log/attributes/attribute_value.hpp>
#include <boost/log/attributes/attribute_values_view.hpp>
#include "alignment_gap_between.hpp"
#include "attribute_set_impl.hpp"
#include "stateless_allocator.hpp"

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

BOOST_LOG_FORCEINLINE attribute_values_view::node_base::node_base() :
    m_pPrev(NULL),
    m_pNext(NULL)
{
}

BOOST_LOG_FORCEINLINE attribute_values_view::node::node(key_type const& key, mapped_type& data, bool dynamic) :
    node_base(),
    m_Value(key, mapped_type()),
    m_DynamicallyAllocated(dynamic)
{
    m_Value.second.swap(data);
}

//! Container implementation
struct attribute_values_view::implementation
{
public:
    typedef key_type::id_type id_type;

private:
    typedef attribute_set_type::implementation attribute_set_impl_type;
    typedef boost::log::aux::stateless_allocator< char > stateless_allocator;

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
        intrusive::constant_time_size< false >
    > node_list;

    //! A hash table bucket
    struct bucket
    {
        //! Points to the first element in the bucket
        node* first;
        //! Points to the last element in the bucket (not the one after the last!)
        node* last;

        bucket() : first(NULL), last(NULL) {}
    };

    //! A list of buckets
    typedef boost::array< bucket, 1U << BOOST_LOG_HASH_TABLE_SIZE_LOG > buckets;

    //! Element disposer
    struct disposer
    {
        typedef void result_type;
        void operator() (node* p) const
        {
            if (!p->m_DynamicallyAllocated)
                p->~node();
            else
                delete p;
        }
    };

private:
    //! Pointer to the source-specific attributes
    attribute_set_impl_type* m_pSourceAttributes;
    //! Pointer to the thread-specific attributes
    attribute_set_impl_type* m_pThreadAttributes;
    //! Pointer to the global attributes
    attribute_set_impl_type* m_pGlobalAttributes;

    //! The container with elements
    node_list m_Nodes;
    //! The pointer to the beginning of the storage of the elements
    node* m_pStorage;
    //! The pointer to the end of the allocated elements within the storage
    node* m_pEnd;
    //! The pointer to the end of storage
    node* m_pEOS;

    //! Hash table buckets
    buckets m_Buckets;

private:
    //! Constructor
    implementation(
        node* storage,
        node* eos,
        attribute_set_impl_type* source_attrs,
        attribute_set_impl_type* thread_attrs,
        attribute_set_impl_type* global_attrs
    ) :
        m_pSourceAttributes(source_attrs),
        m_pThreadAttributes(thread_attrs),
        m_pGlobalAttributes(global_attrs),
        m_pStorage(storage),
        m_pEnd(storage),
        m_pEOS(eos)
    {
    }

    //! Destructor
    ~implementation()
    {
        m_Nodes.clear_and_dispose(disposer());
    }

    //! The function allocates memory and creates the object
    static implementation* create(
        size_type element_count,
        attribute_set_impl_type* source_attrs,
        attribute_set_impl_type* thread_attrs,
        attribute_set_impl_type* global_attrs)
    {
        // Calculate the buffer size
        const size_type header_size = sizeof(implementation) +
            aux::alignment_gap_between< implementation, node >::value;
        const size_type buffer_size = header_size + element_count * sizeof(node);

        stateless_allocator alloc;
        implementation* p = reinterpret_cast< implementation* >(alloc.allocate(buffer_size));
        node* const storage = reinterpret_cast< node* >(reinterpret_cast< char* >(p) + header_size);
        new (p) implementation(storage, storage + element_count, source_attrs, thread_attrs, global_attrs);

        return p;
    }

public:
    //! The function allocates memory and creates the object
    static implementation* create(
        attribute_set_type const& source_attrs,
        attribute_set_type const& thread_attrs,
        attribute_set_type const& global_attrs,
        size_type reserve_count)
    {
        return create(
            source_attrs.m_pImpl->size() + thread_attrs.m_pImpl->size() + global_attrs.m_pImpl->size() + reserve_count,
            source_attrs.m_pImpl,
            thread_attrs.m_pImpl,
            global_attrs.m_pImpl);
    }

    //! The function allocates memory and creates the object
    static implementation* create(size_type reserve_count)
    {
        return create(reserve_count, NULL, NULL, NULL);
    }

    //! Creates a copy of the object
    static implementation* copy(implementation* that)
    {
        // Create new object
        implementation* p = create(that->size(), NULL, NULL, NULL);

        // Copy all elements
        node_list::iterator it = that->m_Nodes.begin(), end = that->m_Nodes.end();
        for (; it != end; ++it)
        {
            node* n = p->m_pEnd++;
            mapped_type data = it->m_Value.second;
            new (n) node(it->m_Value.first, data, false);
            p->m_Nodes.push_back(*n);

            // Since nodes within buckets are ordered, we can simply append the node to the end of the bucket
            bucket& b = p->get_bucket(n->m_Value.first.id());
            if (b.first == NULL)
                b.first = b.last = n;
            else
                b.last = n;
        }

        return p;
    }

    //! Destroys the object and releases the memory
    static void destroy(implementation* p)
    {
        const size_type buffer_size = reinterpret_cast< char* >(p->m_pEOS) - reinterpret_cast< char* >(p);
        p->~implementation();
        stateless_allocator alloc;
        alloc.deallocate(reinterpret_cast< stateless_allocator::pointer >(p), buffer_size);
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
    size_type size()
    {
        freeze();
        return (m_pEnd - m_pStorage);
    }

    //! Looks for the element with an equivalent key
    node_base* find(key_type key)
    {
        // First try to find an acquired element
        bucket& b = get_bucket(key.id());
        register node* p = b.first;
        if (p)
        {
            // The bucket is not empty, search among the elements
            p = find_in_bucket(key, b);
            if (p->m_Value.first == key)
                return p;
        }

        // Element not found, try to acquire the value from attribute sets, if not frozen yet
        if (m_pSourceAttributes)
            return freeze_node(key, b, p);
        else
            return m_Nodes.end().pointed_node();
    }

    //! Freezes all elements of the container
    void freeze()
    {
        if (m_pSourceAttributes)
        {
            freeze_nodes_from(m_pSourceAttributes);
            freeze_nodes_from(m_pThreadAttributes);
            freeze_nodes_from(m_pGlobalAttributes);
            m_pSourceAttributes = m_pThreadAttributes = m_pGlobalAttributes = NULL;
        }
    }

    //! Inserts an element
    std::pair< node*, bool > insert(key_type key, mapped_type const& mapped)
    {
        bucket& b = get_bucket(key.id());
        node* p = find_in_bucket(key, b);
        if (!p || p->m_Value.first != key)
        {
            p = insert_node(key, b, p, mapped);
            return std::pair< node*, bool >(p, true);
        }
        else
        {
            return std::pair< node*, bool >(p, false);
        }
    }

private:
    //! The function returns a bucket for the specified element
    bucket& get_bucket(id_type id)
    {
        return m_Buckets[id & (buckets::static_size - 1)];
    }

    //! Attempts to find an element with the specified key in the bucket
    node* find_in_bucket(key_type key, bucket const& b)
    {
        typedef node_list::node_traits node_traits;
        typedef node_list::value_traits value_traits;

        // All elements within the bucket are sorted to speedup the search.
        register node* p = b.first;
        while (p != b.last && p->m_Value.first.id() < key.id())
        {
            p = value_traits::to_value_ptr(node_traits::get_next(p));
        }

        return p;
    }

    //! Acquires the attribute value from the attribute sets
    node_base* freeze_node(key_type key, bucket& b, node* where)
    {
        attribute_set_type::iterator it = m_pSourceAttributes->find(key);
        if (it == m_pSourceAttributes->end())
        {
            it = m_pThreadAttributes->find(key);
            if (it == m_pThreadAttributes->end())
            {
                it = m_pGlobalAttributes->find(key);
                if (it == m_pGlobalAttributes->end())
                {
                    // The attribute is not found
                    return m_Nodes.end().pointed_node();
                }
            }
        }

        // The attribute is found, acquiring the value
        return insert_node(key, b, where, it->second.get_value());
    }

    //! The function inserts a node into the container
    node* insert_node(key_type key, bucket& b, node* where, mapped_type data)
    {
        node* p;
        if (m_pEnd != m_pEOS)
        {
            p = m_pEnd++;
            new (p) node(key, data, false);
        }
        else
        {
            p = new node(key, data, true);
        }

        if (b.first == NULL)
        {
            // The bucket is empty
            b.first = b.last = p;
            m_Nodes.push_back(*p);
        }
        else if (where == b.last && key.id() > where->m_Value.first.id())
        {
            // The new element should become the last element of the bucket
            node_list::iterator it = m_Nodes.iterator_to(*where);
            ++it;
            m_Nodes.insert(it, *p);
            b.last = p;
        }
        else
        {
            // The new element should be within the bucket
            node_list::iterator it = m_Nodes.iterator_to(*where);
            m_Nodes.insert(it, *p);
        }

        return p;
    }

    //! Acquires attribute values from the set of attributes
    void freeze_nodes_from(attribute_set_impl_type* attrs)
    {
        attribute_set_type::const_iterator
            it = attrs->begin(), end = attrs->end();
        for (; it != end; ++it)
        {
            key_type key = it->first;
            bucket& b = get_bucket(key.id());
            register node* p = b.first;
            if (p)
            {
                p = find_in_bucket(key, b);
                if (p->m_Value.first == key)
                    continue; // the element is already frozen
            }

            insert_node(key, b, p, it->second.get_value());
        }
    }
};

#ifdef _MSC_VER
#pragma warning(push)
// 'this' : used in base member initializer list
#pragma warning(disable: 4355)
#endif

//! The constructor creates an empty view
BOOST_LOG_API attribute_values_view::attribute_values_view(
    size_type reserve_count
) :
    m_pImpl(implementation::create(reserve_count))
{
}

//! The constructor adopts three attribute sets to the view
BOOST_LOG_API attribute_values_view::attribute_values_view(
    attribute_set_type const& source_attrs,
    attribute_set_type const& thread_attrs,
    attribute_set_type const& global_attrs,
    size_type reserve_count
) :
    m_pImpl(implementation::create(source_attrs, thread_attrs, global_attrs, reserve_count))
{
}

//! Copy constructor
BOOST_LOG_API attribute_values_view::attribute_values_view(attribute_values_view const& that)
{
    if (that.m_pImpl)
        m_pImpl = implementation::copy(that.m_pImpl);
    else
        m_pImpl = NULL;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

//! Destructor
BOOST_LOG_API attribute_values_view::~attribute_values_view() BOOST_NOEXCEPT
{
    if (m_pImpl)
    {
        implementation::destroy(m_pImpl);
        m_pImpl = NULL;
    }
}

//  Iterator generators
BOOST_LOG_API attribute_values_view::const_iterator
attribute_values_view::begin() const
{
    return const_iterator(m_pImpl->begin(), const_cast< attribute_values_view* >(this));
}

BOOST_LOG_API attribute_values_view::const_iterator
attribute_values_view::end() const
{
    return const_iterator(m_pImpl->end(), const_cast< attribute_values_view* >(this));
}

//! The method returns number of elements in the container
BOOST_LOG_API attribute_values_view::size_type
attribute_values_view::size() const
{
    return m_pImpl->size();
}

//! Internal lookup implementation
BOOST_LOG_API attribute_values_view::const_iterator
attribute_values_view::find(key_type key) const
{
    return const_iterator(m_pImpl->find(key), const_cast< attribute_values_view* >(this));
}

//! The method acquires values of all adopted attributes. Users don't need to call it, since will always get an already frozen view.
BOOST_LOG_API void attribute_values_view::freeze()
{
    m_pImpl->freeze();
}

//! Inserts an element into the view
BOOST_LOG_API std::pair< attribute_values_view::const_iterator, bool >
attribute_values_view::insert(key_type key, mapped_type const& mapped)
{
    std::pair< node*, bool > res = m_pImpl->insert(key, mapped);
    return std::pair< const_iterator, bool >(const_iterator(res.first, this), res.second);
}

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost
