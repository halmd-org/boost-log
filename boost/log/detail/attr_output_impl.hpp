/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   attr_output_impl.hpp
 * \author Andrey Semashev
 * \date   12.08.2012
 *
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_DETAIL_ATTR_OUTPUT_IMPL_HPP_INCLUDED_
#define BOOST_LOG_DETAIL_ATTR_OUTPUT_IMPL_HPP_INCLUDED_

#include <boost/mpl/is_sequence.hpp>
#include <boost/phoenix/core/actor.hpp>
#include <boost/phoenix/core/meta_grammar.hpp>
#include <boost/phoenix/core/environment.hpp>
#include <boost/phoenix/core/terminal_fwd.hpp>
#include <boost/phoenix/core/is_nullary.hpp>
#include <boost/fusion/sequence/intrinsic/at.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/attributes/attribute_name.hpp>
#include <boost/log/attributes/value_visitation.hpp>
#include <boost/log/expressions/terminal.hpp>
#include <boost/log/expressions/attr.hpp>
#include <boost/log/utility/functional/bind_output.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace expressions {

//! Attribute stream output expression
template< typename LeftT, typename T, typename FallbackPolicyT, typename TagT = void >
class generic_attribute_output_terminal :
    private value_visitor_invoker< T, FallbackPolicyT >
{
public:
    //! Internal typedef for type categorization
    typedef void _is_boost_log_terminal;

    //! Self type
    typedef generic_attribute_output_terminal this_type;
    //! Base type
    typedef value_visitor_invoker< T, FallbackPolicyT > base_type;

    //! Result type definition
    template< typename >
    struct result;

    template< typename ContextT >
    struct result< this_type(ContextT) >
    {
        typedef typename remove_cv< typename remove_reference< ContextT >::type >::type context_type;
        typedef typename phoenix::evaluator::impl<
            typename LeftT::proto_base_expr&,
            context_type,
            phoenix::unused
        >::result_type type;
    };

    template< typename ContextT >
    struct result< const this_type(ContextT) >
    {
        typedef typename remove_cv< typename remove_reference< ContextT >::type >::type context_type;
        typedef typename phoenix::evaluator::impl<
            typename LeftT::proto_base_expr const&,
            context_type,
            phoenix::unused
        >::result_type type;
    };

private:
    //! Left argument actor
    LeftT m_left;
    //! Attribute name
    const attribute_name m_name;

public:
    //! Initializing constructor
    generic_attribute_output_terminal(LeftT const& left, attribute_name const& name) : base_type(), m_left(left), m_name(name)
    {
    }

    //! Initializing constructor
    template< typename U >
    generic_attribute_output_terminal(LeftT const& left, attribute_name const& name, U const& arg) : base_type(arg), m_left(left), m_name(name)
    {
    }

    //! Invokation operator
    template< typename ContextT >
    typename result< this_type(ContextT) >::type operator() (ContextT const& ctx)
    {
        typedef typename result< this_type(ContextT) >::type result_type;
        result_type strm = phoenix::eval(m_left, ctx);
        base_type::operator() (m_name, fusion::at_c< 0 >(phoenix::env(ctx).args()), boost::log::bind_output(strm));
        return strm;
    }

    //! Invokation operator
    template< typename ContextT >
    typename result< const this_type(ContextT) >::type operator() (ContextT const& ctx) const
    {
        typedef typename result< const this_type(ContextT) >::type result_type;
        result_type strm = phoenix::eval(m_left, ctx);
        base_type::operator() (m_name, fusion::at_c< 0 >(phoenix::env(ctx).args()), boost::log::bind_output(strm));
        return strm;
    }
};

namespace aux {

template< typename LeftT, typename T, typename FallbackPolicyT, typename TagT >
struct make_output_expression
{
    //! Resulting expression
    typedef generic_attribute_output_terminal< LeftT, T, FallbackPolicyT, TagT > type;

    //! Creates the output expression
    template< typename RightT >
    static BOOST_LOG_FORCEINLINE type make(LeftT const& left, RightT const& right)
    {
        return type(left, right.get_name(), right.get_fallback_policy());
    }
};

template< typename LeftT, typename RightT, typename ValueT = typename RightT::value_type, bool IsSequenceV = mpl::is_sequence< ValueT >::value >
struct make_output_actor;

template< template< typename > class ActorT, typename LeftExprT, typename RightT, typename ValueT >
struct make_output_actor< ActorT< LeftExprT >, RightT, ValueT, false >
{
    typedef make_output_expression<
        ActorT< LeftExprT >,
        ValueT,
        typename RightT::fallback_policy,
        typename RightT::tag_type
    > make_expression;

    typedef ActorT< typename make_expression::type > type;

    static BOOST_LOG_FORCEINLINE type make(ActorT< LeftExprT > const& left, RightT const& right)
    {
        type res = { make_expression::make(left, right) };
        return res;
    }
};

template< template< typename > class ActorT, typename LeftExprT, typename RightT, typename ValueT >
struct make_output_actor< ActorT< LeftExprT >, RightT, ValueT, true >
{
    typedef generic_attribute_output_terminal< ActorT< LeftExprT >, ValueT, typename RightT::fallback_policy, typename RightT::tag_type > expression_type;

    typedef ActorT< expression_type > type;

    static BOOST_LOG_FORCEINLINE type make(ActorT< LeftExprT > const& left, RightT const& right)
    {
        type res = { expression_type(left, right.get_name(), right.get_fallback_policy()) };
        return res;
    }
};

} // namespace aux

template< typename LeftExprT, typename T, typename FallbackPolicyT, typename TagT >
BOOST_LOG_FORCEINLINE typename aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::type
operator<< (phoenix::actor< LeftExprT > const& left, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > const& right)
{
    return aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::make(left, right);
}

template< typename LeftExprT, typename T, typename FallbackPolicyT, typename TagT >
BOOST_LOG_FORCEINLINE typename aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::type
operator<< (phoenix::actor< LeftExprT >& left, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > const& right)
{
    return aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::make(left, right);
}

template< typename LeftExprT, typename T, typename FallbackPolicyT, typename TagT >
BOOST_LOG_FORCEINLINE typename aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::type
operator<< (phoenix::actor< LeftExprT > const& left, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor >& right)
{
    return aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::make(left, right);
}

template< typename LeftExprT, typename T, typename FallbackPolicyT, typename TagT >
BOOST_LOG_FORCEINLINE typename aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::type
operator<< (phoenix::actor< LeftExprT >& left, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor >& right)
{
    return aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::make(left, right);
}

#if !defined(BOOST_NO_RVALUE_REFERENCES) && !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)

template< typename LeftExprT, typename T, typename FallbackPolicyT, typename TagT >
BOOST_LOG_FORCEINLINE typename aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::type
operator<< (phoenix::actor< LeftExprT >&& left, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor >& right)
{
    return aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::make(left, right);
}

template< typename LeftExprT, typename T, typename FallbackPolicyT, typename TagT >
BOOST_LOG_FORCEINLINE typename aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::type
operator<< (phoenix::actor< LeftExprT >&& left, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > const& right)
{
    return aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::make(left, right);
}

template< typename LeftExprT, typename T, typename FallbackPolicyT, typename TagT >
BOOST_LOG_FORCEINLINE typename aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::type
operator<< (phoenix::actor< LeftExprT >& left, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor >&& right)
{
    return aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::make(left, right);
}

template< typename LeftExprT, typename T, typename FallbackPolicyT, typename TagT >
BOOST_LOG_FORCEINLINE typename aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::type
operator<< (phoenix::actor< LeftExprT > const& left, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor >&& right)
{
    return aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::make(left, right);
}

template< typename LeftExprT, typename T, typename FallbackPolicyT, typename TagT >
BOOST_LOG_FORCEINLINE typename aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::type
operator<< (phoenix::actor< LeftExprT > const&& left, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor >& right)
{
    return aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::make(left, right);
}

template< typename LeftExprT, typename T, typename FallbackPolicyT, typename TagT >
BOOST_LOG_FORCEINLINE typename aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::type
operator<< (phoenix::actor< LeftExprT > const&& left, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > const& right)
{
    return aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::make(left, right);
}

template< typename LeftExprT, typename T, typename FallbackPolicyT, typename TagT >
BOOST_LOG_FORCEINLINE typename aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::type
operator<< (phoenix::actor< LeftExprT >& left, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > const&& right)
{
    return aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::make(left, right);
}

template< typename LeftExprT, typename T, typename FallbackPolicyT, typename TagT >
BOOST_LOG_FORCEINLINE typename aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::type
operator<< (phoenix::actor< LeftExprT > const& left, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > const&& right)
{
    return aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::make(left, right);
}

template< typename LeftExprT, typename T, typename FallbackPolicyT, typename TagT >
BOOST_LOG_FORCEINLINE typename aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::type
operator<< (phoenix::actor< LeftExprT >&& left, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor >&& right)
{
    return aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::make(left, right);
}

template< typename LeftExprT, typename T, typename FallbackPolicyT, typename TagT >
BOOST_LOG_FORCEINLINE typename aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::type
operator<< (phoenix::actor< LeftExprT > const&& left, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor >&& right)
{
    return aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::make(left, right);
}

template< typename LeftExprT, typename T, typename FallbackPolicyT, typename TagT >
BOOST_LOG_FORCEINLINE typename aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::type
operator<< (phoenix::actor< LeftExprT >&& left, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > const&& right)
{
    return aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::make(left, right);
}

template< typename LeftExprT, typename T, typename FallbackPolicyT, typename TagT >
BOOST_LOG_FORCEINLINE typename aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::type
operator<< (phoenix::actor< LeftExprT > const&& left, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > const&& right)
{
    return aux::make_output_actor< phoenix::actor< LeftExprT >, attribute_actor< T, FallbackPolicyT, TagT, phoenix::actor > >::make(left, right);
}

#endif // !defined(BOOST_NO_RVALUE_REFERENCES) && !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)

} // namespace expressions

BOOST_LOG_CLOSE_NAMESPACE // namespace log

#ifndef BOOST_LOG_DOXYGEN_PASS

namespace phoenix {

namespace result_of {

template< typename LeftT, typename T, typename FallbackPolicyT, typename TagT >
struct is_nullary< custom_terminal< boost::log::expressions::generic_attribute_output_terminal< LeftT, T, FallbackPolicyT, TagT > > > :
    public mpl::false_
{
};

} // namespace result_of

} // namespace phoenix

#endif // !defined(BOOST_LOG_DOXYGEN_PASS)

} // namespace boost

#endif // BOOST_LOG_DETAIL_ATTR_OUTPUT_IMPL_HPP_INCLUDED_
