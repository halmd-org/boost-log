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

namespace {

template< typename IteratorT, typename NodeContainerT, typename MappedTypeT >
void import_nodes(IteratorT& it, IteratorT const& end, NodeContainerT& Nodes, unsigned char HTIndex, MappedTypeT const& pNULL)
{
    typedef typename NodeContainerT::value_type node;
    for (; it != end && it->m_HTIndex == HTIndex; ++it)
    {
        Nodes.push_back(node(it->first, pNULL, HTIndex));
        Nodes.back().m_itAttribute = it;
    }
}

} // namespace

//! The method adopts three attribute sets to the view
template< typename CharT >
void basic_attribute_values_view< CharT >::adopt(
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

    const mapped_type pNULL;
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
        // It is safe since no reallocations will occur because memory reservation above
        import_nodes(itSource, itSourceEnd, Nodes, HTIndex, pNULL);
        import_nodes(itThread, itThreadEnd, Nodes, HTIndex, pNULL);
        import_nodes(itGlobal, itGlobalEnd, Nodes, HTIndex, pNULL);
    }

    // Rebuild hash table
    this->rehash();
}

} // namespace log

} // namespace boost

//! Explicitly instantiate container implementation
template class boost::log::aux::unordered_multimap_facade< boost::log::aux::attribute_values_view_descr< char > >;
template class boost::log::aux::unordered_multimap_facade< boost::log::aux::attribute_values_view_descr< wchar_t > >;
template class boost::log::basic_attribute_values_view< char >;
template class boost::log::basic_attribute_values_view< wchar_t >;
