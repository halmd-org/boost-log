/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   matches.hpp
 * \author Andrey Semashev
 * \date   02.09.2012
 *
 * The header contains implementation of a \c matches predicate in template expressions.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_EXPRESSIONS_PREDICATES_MATCHES_HPP_INCLUDED_
#define BOOST_LOG_EXPRESSIONS_PREDICATES_MATCHES_HPP_INCLUDED_

#include <boost/phoenix/core/actor.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/attributes/attribute_name.hpp>
#include <boost/log/expressions/terminal.hpp>
#include <boost/log/expressions/unary_adapter.hpp>
#include <boost/log/expressions/attr_fwd.hpp>
#include <boost/log/expressions/keyword_fwd.hpp>
#include <boost/log/expressions/predicates/predicate.hpp>
#include <boost/log/attributes/fallback_policy.hpp>
#include <boost/log/utility/functional/matches.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace expressions {

/*!
 * The predicate checks if the attribute value matches a regular expression. The attribute value is assumed to be of a string type.
 */
#if !defined(BOOST_NO_TEMPLATE_ALIASES) && !defined(BOOST_NO_CXX11_TEMPLATE_ALIASES)

template< typename T, typename RegexT, typename FallbackPolicyT = fallback_to_none >
using attribute_matches = attribute_predicate< T, RegexT, matches_fun, FallbackPolicyT >;

#else // !defined(BOOST_NO_TEMPLATE_ALIASES) && !defined(BOOST_NO_CXX11_TEMPLATE_ALIASES)

template< typename T, typename RegexT, typename FallbackPolicyT = fallback_to_none >
class attribute_matches :
    public attribute_predicate< T, RegexT, matches_fun, FallbackPolicyT >
{
    typedef attribute_predicate< T, RegexT, matches_fun, FallbackPolicyT > base_type;

public:
    /*!
     * Initializing constructor
     *
     * \param name Attribute name
     * \param rex The regular expression to match the attribute value against
     */
    attribute_matches(attribute_name const& name, RegexT const& rex) : base_type(name, rex)
    {
    }

    /*!
     * Initializing constructor
     *
     * \param name Attribute name
     * \param rex The regular expression to match the attribute value against
     * \param arg Additional parameter for the fallback policy
     */
    template< typename U >
    attribute_matches(attribute_name const& name, RegexT const& rex, U const& arg) : base_type(name, rex, arg)
    {
    }
};

#endif // !defined(BOOST_NO_TEMPLATE_ALIASES) && !defined(BOOST_NO_CXX11_TEMPLATE_ALIASES)

/*!
 * The function generates a terminal node in a template expression. The node will check if the attribute value,
 * which is assumed to be a string, matches the specified regular expression.
 */
template< typename T, typename FallbackPolicyT, typename TagT, template< typename > class ActorT, typename RegexT >
BOOST_LOG_FORCEINLINE ActorT< terminal< unary_adapter< attribute_matches< T, RegexT, FallbackPolicyT > > > >
matches(attribute_actor< T, FallbackPolicyT, TagT, ActorT > const& attr, RegexT const& rex)
{
    typedef terminal< unary_adapter< attribute_matches< T, RegexT, FallbackPolicyT > > > terminal_type;
    ActorT< terminal_type > act = { terminal_type(attr.get_name(), rex, attr.get_fallback_policy()) };
    return act;
}

/*!
 * The function generates a terminal node in a template expression. The node will check if the attribute value,
 * which is assumed to be a string, matches the specified regular expression.
 */
template< typename DescriptorT, template< typename > class ActorT, typename RegexT >
BOOST_LOG_FORCEINLINE ActorT< terminal< unary_adapter< attribute_matches< typename DescriptorT::value_type, RegexT > > > >
matches(attribute_keyword< DescriptorT, ActorT > const&, RegexT const& rex)
{
    typedef terminal< unary_adapter< attribute_matches< typename DescriptorT::value_type, RegexT > > > terminal_type;
    ActorT< terminal_type > act = { terminal_type(DescriptorT::get_name(), rex) };
    return act;
}

/*!
 * The function generates a terminal node in a template expression. The node will check if the attribute value,
 * which is assumed to be a string, matches the specified regular expression.
 */
template< typename T, typename RegexT >
BOOST_LOG_FORCEINLINE phoenix::actor< terminal< unary_adapter< attribute_matches< T, RegexT > > > >
matches(attribute_name const& name, RegexT const& rex)
{
    typedef terminal< unary_adapter< attribute_matches< T, RegexT > > > terminal_type;
    phoenix::actor< terminal_type > act = { terminal_type(name, rex) };
    return act;
}

} // namespace expressions

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_EXPRESSIONS_PREDICATES_MATCHES_HPP_INCLUDED_
