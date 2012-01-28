/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   keyword.hpp
 * \author Andrey Semashev
 * \date   29.01.2012
 *
 * The header contains attribute keyword declaration.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_EXPRESSIONS_KEYWORD_HPP_INCLUDED_
#define BOOST_LOG_EXPRESSIONS_KEYWORD_HPP_INCLUDED_

#include <boost/proto/extends.hpp>
#include <boost/phoenix/core/actor.hpp>
#include <boost/fusion/sequence/intrinsic/at.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/expressions/domain.hpp>
#include <boost/log/expressions/terminal.hpp>
#include <boost/log/expressions/keyword_fwd.hpp>
#include <boost/log/attributes/value_extraction.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace expressions {

/*!
 * \brief This class implements an expression template keyword
 */
template< typename DescriptorT, template< typename > class ActorT >
struct attribute_keyword
{
    //! Internal typedef for type categorization
    typedef void _is_boost_log_attribute_keyword;

    //! Attribute descriptor type
    typedef DescriptorT descriptor_type;
    //! Attribute value type
    typedef typename descriptor_type::value_type value_type;
    //! Character type
    typedef typename descriptor_type::char_type char_type;
    //! Expression type
    typedef ActorT< attribute_terminal< DescriptorT > > expr_type;

    BOOST_PROTO_EXTENDS(expr_type, attribute_keyword, domain)

    //! Expression with cached attribute name
    typedef ActorT<
        cached_attribute_terminal<
            value_extractor<
                char_type,
                extract_value_or_none< value_type >
            >
        >
    > or_none_result_type;

    //! Generates an expression that extracts the attribute value or a default value
    or_none_result_type or_none() const
    {
        typedef cached_attribute_terminal<
            value_extractor<
                char_type,
                extract_value_or_none< value_type >
            >
        > cached_terminal;
        ActorT< cached_terminal > res = { cached_terminal(descriptor_type::get_name()) };
        return res;
    }

    //! Expression with cached attribute name and default value
    typedef ActorT<
        cached_attribute_terminal<
            value_extractor<
                char_type,
                extract_value_or_default< value_type >
            >
        >
    > or_default_result_type;

    //! Generates an expression that extracts the attribute value or a default value
    template< typename T >
    or_default_result_type or_default(T const& def_val) const
    {
        typedef cached_attribute_terminal<
            value_extractor<
                char_type,
                extract_value_or_default< value_type >
            >
        > cached_terminal;
        ActorT< cached_terminal > res = { cached_terminal(descriptor_type::get_name(), def_val) };
        return res;
    }
};

} // namespace expressions

} // namespace log

} // namespace boost

#define BOOST_LOG_DECLARE_ATTRIBUTE_IMPL(tag_ns_, char_type_, name_, tag_, attr_type_, ctor_arg_)\
    namespace tag_ns_\
    {\
        struct tag_\
        {\
            typedef char_type_ char_type;\
            typedef attr_type_ attribute_type;\
            typedef attribute_type::value_type value_type;\
            typedef ::boost::log::basic_attribute_name< char_type > name_type;\
            static name_type get_name() { return name_type(name_); }\
            static attribute_type create() { return attribute_type(ctor_arg_); }\
        };\
    }\
    const ::boost::log::expressions::attribute_keyword< tag_ns_::tag_ > tag_ = {};

#endif // BOOST_LOG_EXPRESSIONS_KEYWORD_HPP_INCLUDED_
