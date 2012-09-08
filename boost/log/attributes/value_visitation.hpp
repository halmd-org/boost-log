/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   value_visitation.hpp
 * \author Andrey Semashev
 * \date   01.03.2008
 *
 * The header contains implementation of convenience tools to apply visitors to an attribute value
 * in the view.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_ATTRIBUTES_VALUE_VISITATION_HPP_INCLUDED_
#define BOOST_LOG_ATTRIBUTES_VALUE_VISITATION_HPP_INCLUDED_

#include <utility>
#include <boost/type_traits/is_void.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/mpl/set.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/quote.hpp>
#include <boost/mpl/is_sequence.hpp>
#include <boost/mpl/insert.hpp>
#include <boost/mpl/inserter.hpp>
#include <boost/mpl/end.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/apply.hpp>
#include <boost/mpl/equal_to.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/variant/variant_fwd.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/utility/result_of.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/exceptions.hpp>
#include <boost/log/core/record.hpp>
#include <boost/log/attributes/attribute_name.hpp>
#include <boost/log/attributes/attribute_value.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/attributes/attribute_values_view.hpp>
#include <boost/log/attributes/value_visitation_fwd.hpp>
#include <boost/log/attributes/fallback_policy.hpp>
#include <boost/log/utility/explicit_operator_bool.hpp>
#include <boost/log/utility/type_dispatch/static_type_dispatcher.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

/*!
 * \brief The class represents attribute value visitation result
 *
 * The main purpose of this class is to provide a convenient interface for checking
 * whether the attribute value visitation succeeded or not. It also allows to discover
 * the actual cause of failure, should the operation fail.
 */
class visitation_result
{
public:
    //! Error codes for attribute value visitation
    enum error_code
    {
        ok,                     //!< The attribute value has been visited successfully
        value_not_found,        //!< The attribute value is not present in the view
        value_has_invalid_type  //!< The attribute value is present in the vew, but has an unexpected type
    };

private:
    error_code m_Code;

public:
    /*!
     * Initializing constructor. Creates the result that is equivalent to the
     * specified error code.
     */
    visitation_result(error_code code = ok) BOOST_NOEXCEPT : m_Code(code) {}

    /*!
     * Checks if the visitation was successful.
     *
     * \return \c true if the value was visited successfully, \c false otherwise.
     */
    BOOST_LOG_EXPLICIT_OPERATOR_BOOL()
    /*!
     * Checks if the visitation was unsuccessful.
     *
     * \return \c false if the value was visited successfully, \c true otherwise.
     */
    bool operator! () const BOOST_NOEXCEPT { return (m_Code != ok); }

    /*!
     * \return The actual result code of value visitation
     */
    error_code code() const BOOST_NOEXCEPT { return m_Code; }
};

namespace result_of {

/*!
 * The metafunction defines the result type of an attribute visitation. The result can be in one of the following forms:
 *
 * \list
 * \li \c visitation_result, if the visitor always returns \c void
 * \li <tt>std::pair<T, visitation_result></tt>, if the visitor always returns a value of type \c T
 * \li <tt>std::pair<variant< T1, T2... >, visitation_result></tt>, if the visitor may return different types \c Tn depending on the attribute value type
 * \endlist
 *
 * The \c visitation_result component describes whether visitation succeeded or not, see result codes within the class definition.
 */
template< typename ValueT, typename VisitorT >
struct visit
{
    //! Visitor type
    typedef typename remove_reference< VisitorT >::type visitor_type;
    //! Attribute value types
    typedef ValueT value_type;

    template< typename ArgT >
    struct get_visitor_result :
        public boost::result_of< visitor_type(ArgT) >
    {
    };

    // Get all operator() results of the visitor
    typedef typename mpl::eval_if<
        mpl::is_sequence< value_type >,
        mpl::transform1<
            value_type,
            mpl::quote1< get_visitor_result >,
            mpl::inserter< mpl::set<>, mpl::insert< mpl::_1, mpl::end< mpl::_1 >, mpl::_2 > >
        >,
        mpl::apply< mpl::quote1< get_visitor_result >, value_type >
    >::type result_types;

    // Convert to a variant, if needed
    typedef typename mpl::eval_if<
        mpl::is_sequence< result_types >,
        mpl::eval_if<
            mpl::equal_to< mpl::size< result_types >, mpl::int_< 1 > >,
            mpl::front< result_types >,
            make_variant_over< result_types >
        >,
        mpl::identity< result_types >
    >::type result_value_type;

    // Compose the final result type
    typedef typename mpl::if_<
        is_void< result_value_type >,
        visitation_result,
        std::pair< result_value_type, visitation_result >
    >::type type;
};

} // namespace result_of

namespace aux {

//! The function object wrapper saves the visitor result
template< typename ResultT, typename VisitorT >
struct forward_result
{
    typedef void result_type;

    forward_result(ResultT& result, VisitorT& visitor) : m_result(result), m_visitor(visitor)
    {
    }

    template< typename ArgT >
    typename enable_if< is_void< typename boost::result_of< VisitorT(ArgT const&) >::type >, result_type >::type
    operator() (ArgT const& arg) const
    {
        m_visitor(arg);
    }

    template< typename ArgT >
    typename disable_if< is_void< typename boost::result_of< VisitorT(ArgT const&) >::type >, result_type >::type
    operator() (ArgT const& arg) const
    {
        m_result = m_visitor(arg);
    }

private:
    ResultT& m_result;
    VisitorT& m_visitor;
};

} // namespace aux

/*!
 * \brief Generic attribute value visitor invoker
 *
 * Attribute value invoker is a functional object that attempts to find and extract the stored
 * attribute value from the attribute value view or a log record. The extracted value is passed to
 * an unary function object (the visitor) provided by user.
 *
 * The invoker can be specialized on one or several attribute value types that should be
 * specified in the second template argument.
 */
template< typename T, typename FallbackPolicyT >
class value_visitor_invoker :
    private FallbackPolicyT
{
    typedef value_visitor_invoker< T, FallbackPolicyT > this_type;

public:
    //! Attribute value types
    typedef T value_type;

    //! Fallback policy
    typedef FallbackPolicyT fallback_policy;

    //! Function object result type
    template< typename >
    struct result;

    template< typename ThisT, typename AttributeNameT, typename ContainerT, typename VisitorT >
    struct result< ThisT(AttributeNameT, ContainerT, VisitorT) > :
        public result_of::visit< value_type, VisitorT >
    {
    };

    template< typename ThisT, typename AttributeValueT, typename VisitorT >
    struct result< ThisT(AttributeValueT, VisitorT) > :
        public result_of::visit< value_type, VisitorT >
    {
    };

public:
    /*!
     * Default constructor
     */
    BOOST_LOG_DEFAULTED_FUNCTION(value_visitor_invoker(), {})

    /*!
     * Copy constructor
     */
    value_visitor_invoker(value_visitor_invoker const& that) : fallback_policy(static_cast< fallback_policy const& >(that))
    {
    }

    /*!
     * Initializing constructor
     *
     * \param arg Fallback policy argument
     */
    template< typename U >
    explicit value_visitor_invoker(U const& arg) : fallback_policy(arg) {}

    /*!
     * Visitation operator. Attempts to acquire the stored value of one of the supported types. If acquisition succeeds,
     * the value is passed to \a visitor.
     *
     * \param attr An attribute value to apply the visitor to.
     * \param visitor A receiving function object to pass the attribute value to.
     * \return The result of visitation (<tt>result_of::visit</tt> metafunction description).
     */
    template< typename VisitorT >
    typename enable_if< is_same< typename result_of::visit< value_type, VisitorT >::type, visitation_result >, typename result_of::visit< value_type, VisitorT >::type >::type
    operator() (attribute_value const& attr, VisitorT visitor) const
    {
        if (!!attr)
        {
            static_type_dispatcher< value_type > disp(visitor);
            if (attr.dispatch(disp) || fallback_policy::apply_default(visitor))
            {
                return visitation_result::ok;
            }
            else
            {
                fallback_policy::on_invalid_type(attr.get_type());
                return visitation_result::value_has_invalid_type;
            }
        }

        if (fallback_policy::apply_default(visitor))
            return visitation_result::ok;

        fallback_policy::on_missing_value();
        return visitation_result::value_not_found;
    }

    /*!
     * Visitation operator. Looks for an attribute value with the specified name
     * and tries to acquire the stored value of one of the supported types. If acquisition succeeds,
     * the value is passed to \a visitor.
     *
     * \param attr An attribute value to apply the visitor to.
     * \param visitor A receiving function object to pass the attribute value to.
     * \return The result of visitation (<tt>result_of::visit</tt> metafunction description).
     */
    template< typename VisitorT >
    typename disable_if< is_same< typename result_of::visit< value_type, VisitorT >::type, visitation_result >, typename result_of::visit< value_type, VisitorT >::type >::type
    operator() (attribute_value const& attr, VisitorT visitor) const
    {
        typedef typename result_of::visit< value_type, VisitorT >::type result_type;
        result_type result;
        aux::forward_result< typename result_type::first_type, VisitorT > forwarder(result.first, visitor);

        if (!!attr)
        {
            static_type_dispatcher< value_type > disp(forwarder);
            if (attr.dispatch(disp) || fallback_policy::apply_default(forwarder))
            {
                result.second = visitation_result::ok;
            }
            else
            {
                fallback_policy::on_invalid_type(attr.get_type());
                result.second = visitation_result::value_has_invalid_type;
            }
        }
        else if (fallback_policy::apply_default(forwarder))
        {
            result.second = visitation_result::ok;
        }
        else
        {
            fallback_policy::on_missing_value();
            result.second = visitation_result::value_not_found;
        }

        return result;
    }

    /*!
     * Visitation operator. Looks for an attribute value with the specified name
     * and tries to acquire the stored value of one of the supported types. If acquisition succeeds,
     * the value is passed to \a visitor.
     *
     * \param name Attribute value name.
     * \param attrs A set of attribute values in which to look for the specified attribute value.
     * \param visitor A receiving function object to pass the attribute value to.
     * \return The result of visitation (<tt>result_of::visit</tt> metafunction description).
     */
    template< typename VisitorT >
    typename result_of::visit< value_type, VisitorT >::type
    operator() (attribute_name const& name, attribute_values_view const& attrs, VisitorT visitor) const
    {
        try
        {
            attribute_values_view::const_iterator it = attrs.find(name);
            if (it != attrs.end())
                return operator() (it->second, visitor);
            else
                return operator() (attribute_value(), visitor);
        }
        catch (exception& e)
        {
            // Attach the attribute name to the exception
            boost::log::aux::attach_attribute_name_info(e, name);
            throw;
        }
    }

    /*!
     * Visitation operator. Looks for an attribute value with the specified name
     * and tries to acquire the stored value of one of the supported types. If acquisition succeeds,
     * the value is passed to \a visitor.
     *
     * \param name Attribute value name.
     * \param record A log record. The attribute value will be sought among those associated with the record.
     * \param visitor A receiving function object to pass the attribute value to.
     * \return The result of visitation (<tt>result_of::visit</tt> metafunction description).
     */
    template< typename VisitorT >
    typename result_of::visit< value_type, VisitorT >::type
    operator() (attribute_name const& name, record const& rec, VisitorT visitor) const
    {
        return operator() (name, rec.attribute_values(), visitor);
    }

    /*!
     * \returns Fallback policy
     */
    fallback_policy const& get_fallback_policy() const
    {
        return *static_cast< fallback_policy const* >(this);
    }
};

#ifdef BOOST_LOG_DOXYGEN_PASS

/*!
 * The function applies a visitor to an attribute value from the view. The user has to explicitly specify the
 * type or set of possible types of the attribute value to be visited.
 *
 * \param name The name of the attribute value to visit.
 * \param attrs A set of attribute values in which to look for the specified attribute value.
 * \param visitor A receiving function object to pass the attribute value to.
 * \return The result of visitation (<tt>result_of::visit</tt> metafunction description).
 */
template< typename T, typename VisitorT >
typename result_of::visit< T, VisitorT >::type
visit(attribute_name const& name, attribute_values_view const& attrs, VisitorT visitor);

/*!
 * The function applies a visitor to an attribute value from the view. The user has to explicitly specify the
 * type or set of possible types of the attribute value to be visited.
 *
 * \param name The name of the attribute value to visit.
 * \param rec A log record. The attribute value will be sought among those associated with the record.
 * \param visitor A receiving function object to pass the attribute value to.
 * \return The result of visitation (<tt>result_of::visit</tt> metafunction description).
 */
template< typename T, typename VisitorT >
typename result_of::visit< T, VisitorT >::type
visit(attribute_name const& name, record const& rec, VisitorT visitor);

/*!
 * The function applies a visitor to an attribute value. The user has to explicitly specify the
 * type or set of possible types of the attribute value to be visited.
 *
 * \param value The attribute value to visit.
 * \param visitor A receiving function object to pass the attribute value to.
 * \return The result of visitation (<tt>result_of::visit</tt> metafunction description).
 */
template< typename T, typename VisitorT >
typename result_of::visit< T, VisitorT >::type
visit(attribute_value const& value, VisitorT visitor);

#else // BOOST_LOG_DOXYGEN_PASS

template< typename T, typename VisitorT >
inline typename result_of::visit< T, VisitorT >::type
visit(attribute_name const& name, attribute_values_view const& attrs, VisitorT visitor)
{
    value_visitor_invoker< T > invoker;
    return invoker(name, attrs, visitor);
}

template< typename T, typename VisitorT >
inline typename result_of::visit< T, VisitorT >::type
visit(attribute_name const& name, record const& rec, VisitorT visitor)
{
    value_visitor_invoker< T > invoker;
    return invoker(name, rec, visitor);
}

template< typename T, typename VisitorT >
inline typename result_of::visit< T, VisitorT >::type
visit(attribute_value const& value, VisitorT visitor)
{
    value_visitor_invoker< T > invoker;
    return invoker(value, visitor);
}

#endif // BOOST_LOG_DOXYGEN_PASS

#if !defined(BOOST_LOG_DOXYGEN_PASS)

template< typename T, typename VisitorT >
inline typename result_of::visit< T, VisitorT >::type attribute_value::visit(VisitorT visitor) const
{
    return boost::log::visit< T >(*this, visitor);
}

#endif // !defined(BOOST_LOG_DOXYGEN_PASS)

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_ATTRIBUTES_VALUE_VISITATION_HPP_INCLUDED_
