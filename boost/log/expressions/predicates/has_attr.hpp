/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   has_attr.hpp
 * \author Andrey Semashev
 * \date   23.07.2012
 *
 * The header contains implementation of a generic attribute presence checker in template expressions.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_EXPRESSIONS_PREDICATES_HAS_ATTR_HPP_INCLUDED_
#define BOOST_LOG_EXPRESSIONS_PREDICATES_HAS_ATTR_HPP_INCLUDED_

#include <boost/phoenix/core/actor.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/core/record.hpp>
#include <boost/log/attributes/attribute_name.hpp>
#include <boost/log/attributes/attribute_values_view.hpp>
#include <boost/log/expressions/terminal.hpp>
#include <boost/log/expressions/keyword_fwd.hpp>
#include <boost/log/expressions/unary_adapter.hpp>
#include <boost/log/attributes/value_visitation.hpp>
#include <boost/log/utility/functional/nop.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace expressions {

/*!
 * An attribute value presence checker.
 */
template< typename T >
class has_attribute
{
public:
    //! Function result_type
    typedef bool result_type;
    //! Expected attribute value type
    typedef T value_type;

private:
    //! Attribute value name
    const attribute_name m_name;
    //! Visitor invoker
    value_visitor_invoker< value_type > m_visitor_invoker;

public:
    /*!
     * Initializing constructor
     *
     * \param name Attribute name
     */
    explicit has_attribute(attribute_name const& name) : m_name(name)
    {
    }

    /*!
     * Checking operator
     *
     * \param arg A set of attribute values or a log record
     * \return \c true if the log record contains the sought attribute value, \c false otherwise
     */
    template< typename ArgT >
    result_type operator() (ArgT const& arg) const
    {
        return m_visitor_invoker(m_name, arg, nop()).code() == visitation_result::ok;
    }
};

/*!
 * An attribute value presence checker. This specialization does not check the type of the attribute value.
 */
template< >
class has_attribute< void >
{
public:
    //! Function result_type
    typedef bool result_type;
    //! Expected attribute value type
    typedef void value_type;

private:
    //! Attribute name
    const attribute_name m_name;

public:
    /*!
     * Initializing constructor
     *
     * \param name Attribute name
     */
    explicit has_attribute(attribute_name const& name) : m_name(name)
    {
    }

    /*!
     * Checking operator
     *
     * \param attrs A set of attribute values
     * \return \c true if the log record contains the sought attribute value, \c false otherwise
     */
    result_type operator() (attribute_values_view const& attrs) const
    {
        return attrs.find(m_name) != attrs.end();
    }

    /*!
     * Checking operator
     *
     * \param rec A log record
     * \return \c true if the log record contains the sought attribute value, \c false otherwise
     */
    result_type operator() (record const& rec) const
    {
        return operator()(rec.attribute_values());
    }
};

/*!
 * The function generates a terminal node in a template expression. The node will check for the attribute value
 * presence in a log record. The node will also check that the attribute value has the specified type, if present.
 */
template< typename AttributeValueT >
BOOST_LOG_FORCEINLINE phoenix::actor< terminal< unary_adapter< has_attribute< AttributeValueT > > > > has_attr(attribute_name const& name)
{
    typedef terminal< unary_adapter< has_attribute< AttributeValueT > > > terminal_type;
    phoenix::actor< terminal_type > act = {{ terminal_type(name) }};
    return act;
}

/*!
 * The function generates a terminal node in a template expression. The node will check for the attribute value
 * presence in a log record.
 */
BOOST_LOG_FORCEINLINE phoenix::actor< terminal< unary_adapter< has_attribute< void > > > > has_attr(attribute_name const& name)
{
    typedef terminal< unary_adapter< has_attribute< void > > > terminal_type;
    phoenix::actor< terminal_type > act = {{ terminal_type(name) }};
    return act;
}

/*!
 * The function generates a terminal node in a template expression. The node will check for the attribute value
 * presence in a log record. The node will also check that the attribute value has the specified type, if present.
 */
template< typename DescriptorT, template< typename > class ActorT >
BOOST_LOG_FORCEINLINE phoenix::actor< terminal< unary_adapter< has_attribute< typename DescriptorT::value_type > > > > has_attr(attribute_keyword< DescriptorT, ActorT > const&)
{
    typedef terminal< unary_adapter< has_attribute< typename DescriptorT::value_type > > > terminal_type;
    phoenix::actor< terminal_type > act = {{ terminal_type(DescriptorT::get_name()) }};
    return act;
}

} // namespace expressions

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_EXPRESSIONS_PREDICATES_HAS_ATTR_HPP_INCLUDED_
