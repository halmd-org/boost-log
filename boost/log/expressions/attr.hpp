/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   attr.hpp
 * \author Andrey Semashev
 * \date   21.07.2012
 *
 * The header contains implementation of a generic attribute placeholder in template expressions.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_EXPRESSIONS_ATTR_HPP_INCLUDED_
#define BOOST_LOG_EXPRESSIONS_ATTR_HPP_INCLUDED_

#include <boost/phoenix/core/actor.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/detail/parameter_tools.hpp>
#include <boost/log/attributes/attribute_name.hpp>
#include <boost/log/expressions/terminal.hpp>
#include <boost/log/expressions/unary_adapter.hpp>
#include <boost/log/attributes/value_extraction.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace expressions {

/*!
 * An attribute value extraction terminal
 */
template< typename ExtractorT, typename ParamsT = boost::log::aux::empty_arg_list, template< typename > class ActorT = phoenix::actor >
class attribute_terminal :
    public ActorT<
        terminal<
            unary_adapter<
                value_extractor< ExtractorT >
            >
        >
    >
{
public:
    //! Base terminal type
    typedef terminal<
        unary_adapter<
            value_extractor< ExtractorT >
        >
    > terminal_type;
    //! Base actor type
    typedef ActorT< terminal_type > base_type;
    //! Attribute value type
    typedef typename ExtractorT::value_type value_type;

    //! Additional parameters
    typedef ParamsT parameters_type;

private:
    //! Additional parameters
    parameters_type m_params;

public:
    //! Initializing constructor
    explicit attribute_terminal(base_type const& act, parameters_type const& params = parameters_type()) : base_type(act), m_params(params)
    {
    }

    //! Returns the attribute name
    attribute_name get_name() const
    {
        return this->proto_expr_.get_name();
    }

    //! Returns additional parameters
    parameters_type const& get_params() const
    {
        return m_params;
    }

    //! Expression with cached attribute name
    typedef attribute_terminal< extract_value_or_none< value_type >, parameters_type, ActorT > or_none_result_type;

    //! Generates an expression that extracts the attribute value or a default value
    or_none_result_type or_none() const
    {
        typedef typename or_none_result_type::terminal_type result_terminal;
        base_type act = { result_terminal(this->proto_expr_.get_name()) };
        return or_none_result_type(act);
    }

    //! Expression with cached attribute name
    typedef attribute_terminal< extract_value_or_throw< value_type >, parameters_type, ActorT > or_throw_result_type;

    //! Generates an expression that extracts the attribute value or throws an exception
    or_throw_result_type or_throw() const
    {
        typedef typename or_throw_result_type::terminal_type result_terminal;
        base_type act = { result_terminal(this->proto_expr_.get_name()) };
        return or_throw_result_type(act);
    }

    //! Generates an expression that extracts the attribute value or a default value
    template< typename T >
    attribute_terminal< extract_value_or_default< value_type, T >, parameters_type, ActorT > or_default(T const& def_val) const
    {
        typedef attribute_terminal< extract_value_or_default< value_type, T >, parameters_type, ActorT > or_default_result_type;
        typedef typename or_default_result_type::terminal_type result_terminal;
        base_type act = { result_terminal(get_name(), def_val) };
        return or_default_result_type(act);
    }
};

/*!
 * The function generates a terminal node in a template expression. The node will extract the value of the attribute
 * with the specified name and type.
 */
template< typename AttributeValueT >
BOOST_LOG_FORCEINLINE attribute_terminal< extract_value_or_none< AttributeValueT > > attr(attribute_name const& name)
{
    typedef attribute_terminal< extract_value_or_none< AttributeValueT > > result_type;
    typedef typename result_type::terminal_type result_terminal;
    typename result_type::base_type act = { result_terminal(name) };
    return result_type(act);
}

} // namespace expressions

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_EXPRESSIONS_ATTR_HPP_INCLUDED_
