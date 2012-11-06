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
#include <boost/phoenix/core/terminal_fwd.hpp>
#include <boost/phoenix/core/is_nullary.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/attributes/attribute_name.hpp>
#include <boost/log/expressions/terminal.hpp>
#include <boost/log/expressions/attr_fwd.hpp>
#include <boost/log/attributes/value_extraction.hpp>
#include <boost/log/attributes/fallback_policy.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace expressions {

namespace aux {

template< typename BaseT >
class extractor_adapter :
    public BaseT
{
    //! Base function type
    typedef BaseT base_type;
    //! Self type
    typedef extractor_adapter< base_type > this_type;

private:
    //! Attribute value name
    const attribute_name m_name;

public:
    //! Function result type
    template< typename >
    struct result;

    template< typename ArgsT >
    struct result< this_type(ArgsT) > :
        public boost::result_of< base_type(attribute_name, typename fusion::result_of::at_c< ArgsT, 0 >::type) >
    {
    };

    template< typename ArgsT >
    struct result< const this_type(ArgsT) > :
        public boost::result_of< const base_type(attribute_name, typename fusion::result_of::at_c< ArgsT, 0 >::type) >
    {
    };

public:
    //! Initializing constructor
    explicit extractor_adapter(attribute_name const& name) : m_name(name)
    {
    }
    //! Initializing constructor
    template< typename ArgT1 >
    extractor_adapter(attribute_name const& name, ArgT1 const& arg1) : base_type(arg1), m_name(name)
    {
    }
    //! Initializing constructor
    template< typename ArgT1, typename ArgT2 >
    extractor_adapter(attribute_name const& name, ArgT1 const& arg1, ArgT2 const& arg2) : base_type(arg1, arg2), m_name(name)
    {
    }

    //! Returns attribute value name
    attribute_name get_name() const
    {
        return m_name;
    }

    //! The operator forwards the call to the base function
    template< typename ArgsT >
    typename boost::result_of< base_type(typename fusion::result_of::at_c< ArgsT, 0 >::type) >::type
    operator() (ArgsT const& args)
    {
        return base_type::operator()(m_name, fusion::at_c< 0 >(args));
    }

    //! The operator forwards the call to the base function
    template< typename ArgsT >
    typename boost::result_of< const base_type(typename fusion::result_of::at_c< ArgsT, 0 >::type) >::type
    operator() (ArgsT const& args) const
    {
        return base_type::operator()(m_name, fusion::at_c< 0 >(args));
    }
};

} // namespace aux

/*!
 * An attribute value extraction terminal
 */
template< typename T, typename FallbackPolicyT, typename TagT >
class attribute_terminal :
    public terminal<
        aux::extractor_adapter<
            value_extractor< T, FallbackPolicyT, TagT >
        >
    >
{
public:
    //! Attribute tag type
    typedef TagT tag_type;
    //! Attribute value extractor
    typedef value_extractor< T, FallbackPolicyT, tag_type > extractor_type;
    //! Base terminal type
    typedef terminal< aux::extractor_adapter< extractor_type > > terminal_type;
    //! Attribute value type
    typedef typename extractor_type::value_type value_type;

public:
    //! Initializing constructor
    explicit attribute_terminal(attribute_name const& name) : terminal_type(name)
    {
    }

    //! Initializing constructor
    template< typename U >
    attribute_terminal(attribute_name const& name, U const& arg) : terminal_type(name, arg)
    {
    }
};

/*!
 * An attribute value extraction terminal actor
 */
template< typename T, typename FallbackPolicyT, typename TagT, template< typename > class ActorT >
class attribute_actor :
    public ActorT< attribute_terminal< T, FallbackPolicyT, TagT > >
{
public:
    //! Attribute tag type
    typedef TagT tag_type;
    //! Fallback policy
    typedef FallbackPolicyT fallback_policy;
    //! Base terminal type
    typedef attribute_terminal< T, fallback_policy, tag_type > terminal_type;
    //! Base actor type
    typedef ActorT< terminal_type > base_type;
    //! Attribute value type
    typedef typename terminal_type::value_type value_type;

public:
    //! Initializing constructor
    explicit attribute_actor(base_type const& act) : base_type(act)
    {
    }

    /*!
     * \returns The attribute name
     */
    attribute_name get_name() const
    {
        return this->proto_expr_.child0.get_name();
    }

    /*!
     * \returns Fallback policy
     */
    fallback_policy const& get_fallback_policy() const
    {
        return this->proto_expr_.child0.get_fallback_policy();
    }

    //! Expression with cached attribute name
    typedef attribute_actor< value_type, fallback_to_none, tag_type, ActorT > or_none_result_type;

    //! Generates an expression that extracts the attribute value or a default value
    or_none_result_type or_none() const
    {
        typedef typename or_none_result_type::terminal_type result_terminal;
        base_type act = {{ result_terminal(get_name()) }};
        return or_none_result_type(act);
    }

    //! Expression with cached attribute name
    typedef attribute_actor< value_type, fallback_to_throw, tag_type, ActorT > or_throw_result_type;

    //! Generates an expression that extracts the attribute value or throws an exception
    or_throw_result_type or_throw() const
    {
        typedef typename or_throw_result_type::terminal_type result_terminal;
        base_type act = {{ result_terminal(get_name()) }};
        return or_throw_result_type(act);
    }

    //! Generates an expression that extracts the attribute value or a default value
    template< typename DefaultT >
    attribute_actor< value_type, fallback_to_default< DefaultT >, tag_type, ActorT > or_default(DefaultT const& def_val) const
    {
        typedef attribute_actor< value_type, fallback_to_default< DefaultT >, tag_type, ActorT > or_default_result_type;
        typedef typename or_default_result_type::terminal_type result_terminal;
        base_type act = {{ result_terminal(get_name(), def_val) }};
        return or_default_result_type(act);
    }
};

/*!
 * The function generates a terminal node in a template expression. The node will extract the value of the attribute
 * with the specified name and type.
 */
template< typename AttributeValueT >
BOOST_LOG_FORCEINLINE attribute_actor< AttributeValueT > attr(attribute_name const& name)
{
    typedef attribute_actor< AttributeValueT > result_type;
    typedef typename result_type::terminal_type result_terminal;
    typename result_type::base_type act = {{ result_terminal(name) }};
    return result_type(act);
}

/*!
 * The function generates a terminal node in a template expression. The node will extract the value of the attribute
 * with the specified name and type.
 */
template< typename AttributeValueT, typename TagT >
BOOST_LOG_FORCEINLINE attribute_actor< AttributeValueT, fallback_to_none, TagT > attr(attribute_name const& name)
{
    typedef attribute_actor< AttributeValueT, fallback_to_none, TagT > result_type;
    typedef typename result_type::terminal_type result_terminal;
    typename result_type::base_type act = {{ result_terminal(name) }};
    return result_type(act);
}

} // namespace expressions

BOOST_LOG_CLOSE_NAMESPACE // namespace log

#ifndef BOOST_LOG_DOXYGEN_PASS

namespace phoenix {

namespace result_of {

template< typename T, typename FallbackPolicyT, typename TagT >
struct is_nullary< custom_terminal< boost::log::expressions::attribute_terminal< T, FallbackPolicyT, TagT > > > :
    public mpl::false_
{
};

} // namespace result_of

} // namespace phoenix

#endif

} // namespace boost

#if defined(BOOST_LOG_EXPRESSIONS_STREAM_HPP_INCLUDED_)
#include <boost/log/detail/attr_output_impl.hpp>
#endif

#endif // BOOST_LOG_EXPRESSIONS_ATTR_HPP_INCLUDED_
