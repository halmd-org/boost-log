/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   terminal.hpp
 * \author Andrey Semashev
 * \date   29.01.2012
 *
 * The header contains attribute terminal definition.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_EXPRESSIONS_TERMINAL_HPP_INCLUDED_
#define BOOST_LOG_EXPRESSIONS_TERMINAL_HPP_INCLUDED_

#include <boost/mpl/bool.hpp>
#include <boost/phoenix/core/terminal.hpp>
#include <boost/phoenix/core/environment.hpp>
#include <boost/fusion/sequence/intrinsic/at.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/attributes/value_extraction.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace expressions {

/*!
 * \brief A terminal node in the expression template tree
 *
 * This terminal is mostly used to declare attribute keywords and not for actual
 * evaluation of expression templates. See \c cached_attribute_terminal.
 */
template< typename DescriptorT >
struct attribute_terminal
{
    //! Internal typedef for type categorization
    typedef void _is_boost_log_attribute_terminal;

    //! Attribute descriptor type
    typedef DescriptorT descriptor_type;

    //! Attribute value type
    typedef typename descriptor_type::value_type value_type;
    //! Character type
    typedef typename descriptor_type::char_type char_type;
    //! Attribute value extractor type
    typedef value_extractor<
        char_type,
        extract_value_or_none< value_type >
    > extractor_type;

    //! Result type
    typedef typename extractor_type::result_type result_type;

    //! The operator extracts the value
    template< typename EnvT >
    result_type operator() (EnvT const& env) const
    {
        return extractor_type(descriptor_type::get_name())(fusion::at_c< 0 >(env.args()));
    }
};

/*!
 * \brief A terminal node in the expression template tree
 *
 * This terminal contains an attribute value extractor and is used for expression
 * templates evaluation. Since the extractor stores attribute name and other data
 * releavant to the attribute value extraction, this terminal cannot be used for
 * keyword declaration.
 */
template< typename ExtractorT >
struct cached_attribute_terminal :
    public ExtractorT
{
    //! Internal typedef for type categorization
    typedef void _is_boost_log_attribute_terminal;

    //! Attribute value extractor
    typedef ExtractorT extractor_type;
    //! Result type
    typedef typename extractor_type::result_type result_type;

    //! Initializing constructor
    template< typename ArgT1 >
    explicit cached_attribute_terminal(ArgT1 const& arg1) : extractor_type(arg1) {}
    //! Initializing constructor
    template< typename ArgT1, typename ArgT2 >
    cached_attribute_terminal(ArgT1 const& arg1, ArgT2 const& arg2) : extractor_type(arg1, arg2) {}

    //! The operator extracts the value
    template< typename EnvT >
    result_type operator() (EnvT const& env) const
    {
        return extractor_type::operator()(fusion::at_c< 0 >(env.args()));
    }
};

} // namespace expressions

} // namespace log

#ifndef BOOST_LOG_DOXYGEN_PASS

namespace phoenix {

namespace result_of {

template< typename T >
struct is_nullary< T, typename T::_is_boost_log_attribute_terminal > :
    public mpl::false_
{
};

} // namespace result_of

template< typename T >
struct is_custom_terminal< T, typename T::_is_boost_log_attribute_terminal > :
    public mpl::true_
{
};

template< typename T >
struct custom_terminal< T, typename T::_is_boost_log_attribute_terminal >
{
    typedef typename T::result_type result_type;

    template< typename ContextT >
    result_type operator() (T const& term, ContextT& ctx) const
    {
        return term(phoenix::env(ctx));
    }
};

} // namespace phoenix

#endif // BOOST_LOG_DOXYGEN_PASS

} // namespace boost

#endif // BOOST_LOG_EXPRESSIONS_TERMINAL_HPP_INCLUDED_
