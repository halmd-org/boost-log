/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   record_ordering.hpp
 * \author Andrey Semashev
 * \date   23.08.2009
 *
 * This header contains ordering predicates for logging records.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_UTILITY_RECORD_ORDERING_HPP_INCLUDED_
#define BOOST_LOG_UTILITY_RECORD_ORDERING_HPP_INCLUDED_

#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/detail/function_traits.hpp>
#include <boost/log/core/record.hpp>
#include <boost/log/attributes/attribute_name.hpp>
#include <boost/log/attributes/attribute_value.hpp>
#include <boost/log/attributes/value_visitation.hpp>
#include <boost/log/functional/logical.hpp>
#include <boost/log/functional/nop.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

/*!
 * \brief Ordering predicate, based on log record handle comparison
 *
 * This predicate offers a quick log record ordering based on the log record handles.
 * It is not specified whether one record is less than another, in terms of the
 * predicate, until the actual comparison is performed. Moreover, the ordering may
 * change every time the application runs.
 *
 * This kind of ordering may be useful if log records are to be stored in an associative
 * container with as least performance overhead as possible.
 */
template< typename FunT = less >
class handle_ordering :
    private FunT
{
public:
    //! Result type
    typedef bool result_type;

public:
    /*!
     * Default constructor. Requires \c FunT to be default constructible.
     */
    handle_ordering() : FunT()
    {
    }
    /*!
     * Initializing constructor. Constructs \c FunT instance as a copy of the \a fun argument.
     */
    explicit handle_ordering(FunT const& fun) : FunT(fun)
    {
    }

    /*!
     * Ordering operator
     */
    result_type operator() (record const& left, record const& right) const
    {
        // We rely on the fact that the attribute_values() method returns a reference to the object in the record implementation,
        // so we can comare pointers.
        return FunT::operator() (static_cast< const void* >(&left.attribute_values()), static_cast< const void* >(right.attribute_values()));
    }
};

/*!
 * \brief Ordering predicate, based on attribute values associated with records
 *
 * This predicate allows to order log records based on values of a specificly named attribute
 * associated with them. Two given log records being compared should both have the specified
 * attribute value of the specified type to be able to be ordered properly. As a special case,
 * if neither of the records have the value, these records are considered equivalent. Otherwise,
 * the ordering results are unspecified.
 */
template< typename ValueT, typename FunT = less >
class attribute_value_ordering :
    private FunT
{
public:
    //! Result type
    typedef bool result_type;
    //! Compared attribute value type
    typedef ValueT value_type;

private:
    template< typename LeftT >
    struct l2_visitor
    {
        typedef void result_type;

        l2_visitor(FunT const& fun, LeftT const& left, bool& result) :
            m_fun(fun), m_left(left), m_result(result)
        {
        }

        template< typename RightT >
        result_type operator() (RightT const& right) const
        {
            m_result = m_fun(m_left, right);
        }

    private:
        FunT const& m_fun;
        LeftT const& m_left;
        bool& m_result;
    };

    struct l1_visitor;
    friend struct l1_visitor;
    struct l1_visitor
    {
        typedef void result_type;

        l1_visitor(attribute_value_ordering const& owner, record const& right, bool& result) :
            m_owner(owner), m_right(right), m_result(result)
        {
        }

        template< typename LeftT >
        result_type operator() (LeftT const& left) const
        {
            boost::log::visit< value_type >(m_owner.m_name, m_right, l2_visitor< LeftT >(static_cast< FunT const& >(m_owner), left, m_result));
        }

    private:
        attribute_value_ordering const& m_owner;
        record const& m_right;
        bool& m_result;
    };

private:
    //! Attribute value name
    const attribute_name m_name;

public:
    /*!
     * Initializing constructor.
     *
     * \param name The attribute value name to be compared
     * \param fun The ordering functor
     */
    explicit attribute_value_ordering(attribute_name const& name, FunT const& fun = FunT()) :
        FunT(fun),
        m_name(name)
    {
    }

    /*!
     * Ordering operator
     */
    result_type operator() (record const& left, record const& right) const
    {
        bool result = false;
        if (!boost::log::visit< value_type >(m_name, left, l1_visitor(*this, right, result)))
        {
            return !boost::log::visit< value_type >(m_name, right, nop());
        }
        return result;
    }
};

/*!
 * The function constructs a log record ordering predicate
 */
template< typename ValueT, typename FunT >
inline attribute_value_ordering< ValueT, FunT > make_attr_ordering(attribute_name const& name, FunT const& fun)
{
    typedef attribute_value_ordering< ValueT, FunT > ordering_t;
    return ordering_t(name, fun);
}

#if !defined(BOOST_LOG_NO_FUNCTION_TRAITS)

namespace aux {

    //! An ordering predicate constructor that uses SFINAE to disable invalid instantiations
    template<
        typename FunT,
        typename ArityCheckT = typename enable_if_c< aux::arity_of< FunT >::value == 2 >::type,
        typename Arg1T = typename aux::first_argument_type_of< FunT >::type,
        typename Arg2T = typename aux::second_argument_type_of< FunT >::type,
        typename ArgsCheckT = typename enable_if< is_same< Arg1T, Arg2T > >::type
    >
    struct make_attr_ordering_type
    {
        typedef attribute_value_ordering< Arg1T, FunT > type;
    };

} // namespace aux

/*!
 * The function constructs a log record ordering predicate
 */
template< typename FunT >
inline typename aux::make_attr_ordering_type< FunT >::type make_attr_ordering(attribute_name const& name, FunT const& fun)
{
    typedef typename aux::make_attr_ordering_type< FunT >::type ordering_t;
    return ordering_t(name, fun);
}

#endif // BOOST_LOG_NO_FUNCTION_TRAITS

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_UTILITY_RECORD_ORDERING_HPP_INCLUDED_
