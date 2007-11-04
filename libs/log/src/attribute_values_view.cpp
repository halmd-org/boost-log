/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   attribute_values_view.cpp
 * \author Andrey Semashev
 * \date   19.04.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#include <cassert>
#include <boost/log/attributes/attribute_values_view.hpp>
#include "unordered_mmap_facade.hpp"

namespace boost {

namespace log {

//! Assignment
template< typename CharT >
basic_attribute_values_view< CharT >& basic_attribute_values_view< CharT >::operator= (basic_attribute_values_view const& that)
{
    if (this != &that)
    {
		basic_attribute_values_view tmp(that);
		this->swap(tmp);
    }
	return *this;
}

//! The constructor adopts three attribute sets to the view
template< typename CharT >
basic_attribute_values_view< CharT >::basic_attribute_values_view(
    attribute_set const& source_attrs,
    attribute_set const& thread_attrs,
    attribute_set const& global_attrs)
{
    typedef typename attribute_set::node_container attribute_set_nodes;
    typedef typename attribute_set_nodes::const_iterator attribute_set_nodes_iterator;

    node_container& Nodes = this->nodes();

    // The view should be empty when the method is being called
    assert(Nodes.empty());
    Nodes.reserve(source_attrs.size() + thread_attrs.size() + global_attrs.size());

    attribute_set_nodes const& SourceAttrs = source_attrs.nodes();
    attribute_set_nodes_iterator itSource = SourceAttrs.begin();
    attribute_set_nodes_iterator itSourceEnd = SourceAttrs.end();
    attribute_set_nodes const& ThreadAttrs = thread_attrs.nodes();
    attribute_set_nodes_iterator itThread = ThreadAttrs.begin();
    attribute_set_nodes_iterator itThreadEnd = ThreadAttrs.end();
    attribute_set_nodes const& GlobalAttrs = global_attrs.nodes();
    attribute_set_nodes_iterator itGlobal = GlobalAttrs.begin();
    attribute_set_nodes_iterator itGlobalEnd = GlobalAttrs.end();

    while (itSource != itSourceEnd || itThread != itThreadEnd || itGlobal != itGlobalEnd)
    {
        // Determine the least hash value of the current elements in each container
        register unsigned char HTIndex = (std::numeric_limits< unsigned char >::max)();
        if (itSource != itSourceEnd && itSource->m_HTIndex < HTIndex)
            HTIndex = itSource->m_HTIndex;
        if (itThread != itThreadEnd && itThread->m_HTIndex < HTIndex)
            HTIndex = itThread->m_HTIndex;
        if (itGlobal != itGlobalEnd && itGlobal->m_HTIndex < HTIndex)
            HTIndex = itGlobal->m_HTIndex;

        // Insert nodes to the view that correspond to the selected bucket
        // It is safe since no reallocations will occur because of memory reservation above
        Nodes.adopt_nodes(itSource, itSourceEnd, HTIndex);
        Nodes.adopt_nodes(itThread, itThreadEnd, HTIndex);
        Nodes.adopt_nodes(itGlobal, itGlobalEnd, HTIndex);
    }

    // Rebuild hash table
    this->rehash();
}

//! Explicitly instantiate container implementation
namespace aux {
    template class unordered_multimap_facade< attribute_values_view_descr< char > >;
    template class unordered_multimap_facade< attribute_values_view_descr< wchar_t > >;
} // namespace aux
template class basic_attribute_values_view< char >;
template class basic_attribute_values_view< wchar_t >;

} // namespace log

} // namespace boost
