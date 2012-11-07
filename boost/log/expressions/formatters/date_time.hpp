/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   formatters/date_time.hpp
 * \author Andrey Semashev
 * \date   16.09.2012
 *
 * The header contains a formatter function for date and time attribute values.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_EXPRESSIONS_FORMATTERS_DATE_TIME_HPP_INCLUDED_
#define BOOST_LOG_EXPRESSIONS_FORMATTERS_DATE_TIME_HPP_INCLUDED_

#include <string>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/attributes/attribute_name.hpp>
#include <boost/log/attributes/fallback_policy.hpp>
#include <boost/log/detail/light_function.hpp>
#include <boost/log/expressions/attr_fwd.hpp>
#include <boost/log/expressions/keyword_fwd.hpp>
#include <boost/log/expressions/output_manip_terminal.hpp>
#include <boost/log/utility/formatting_stream_fwd.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace expressions {

namespace aux {

template< typename T, typename CharT, typename VoidT = void >
struct date_time_formatter_generator_traits;

template< typename T, typename FallbackPolicyT, typename CharT >
class fmt_date_time_gen :
    private FallbackPolicyT
{
public:
    //! Attribute value type
    typedef T value_type;
    //! Fallback policy
    typedef FallbackPolicyT fallback_policy;
    //! Character type
    typedef CharT char_type;
    //! String type
    typedef std::basic_string< char_type > string_type;
    //! Formatting stream type
    typedef basic_formatting_stream< char_type > stream_type;

    //! Formatter function
    typedef boost::log::aux::light_function2< void, stream_type&, value_type const& > formatter_function_type;
    //! Formatter generator traits
    typedef date_time_formatter_generator_traits< value_type, char_type > formatter_generator;

private:
    //! Attribute name
    attribute_name m_name;
    //! Formattr function
    formatter_function_type m_formatter;

public:
    //! Initializing constructor
    fmt_date_time_gen(attribute_name const& name, fallback_policy const& fallback, string_type const& format) :
        fallback_policy(fallback), m_name(name), m_formatter(formatter_generator::parse(format))
    {
    }
    //! Copy constructor
    fmt_date_time_gen(fmt_date_time_gen const& that) : fallback_policy(static_cast< fallback_policy const& >(that)), m_name(that.m_name), m_formatter(that.m_formatter)
    {
    }

    //! Returns attribute name
    attribute_name get_name() const
    {
        return m_name;
    }

    //! Returns fallback policy
    fallback_policy const& get_fallback_policy() const
    {
        return static_cast< fallback_policy const& >(*this);
    }

    //! Retruns formatter function
    formatter_function_type const& get_formatter_function() const
    {
        return m_formatter;
    }
};

#define BOOST_LOG_AUX_OVERLOAD(left_ref, right_ref)\
    template< typename LeftExprT, typename T, typename FallbackPolicyT, typename CharT >\
    BOOST_LOG_FORCEINLINE phoenix::actor< output_manip_terminal< phoenix::actor< LeftExprT >, T, FallbackPolicyT, typename fmt_date_time_gen< T, FallbackPolicyT, CharT >::formatter_function_type > >\
    operator<< (phoenix::actor< LeftExprT > left_ref left, fmt_date_time_gen< T, FallbackPolicyT, CharT > right_ref right)\
    {\
        typedef output_manip_terminal< phoenix::actor< LeftExprT >, T, FallbackPolicyT, typename fmt_date_time_gen< T, FallbackPolicyT, CharT >::formatter_function_type > terminal_type;\
        phoenix::actor< terminal_type > actor = {{ terminal_type(left, right.get_name(), right.get_formatter_function(), right.get_fallback_policy()) }};\
        return actor;\
    }

#include <boost/log/detail/generate_overloads.hpp>

#undef BOOST_LOG_AUX_OVERLOAD

} // namespace aux

/*!
 * The function generates a manipulator node in a template expression. The manipulator must participate in a formatting
 * expression (stream output or \c format placeholder filler).
 *
 * \param name Attribute name
 * \param format Format string
 */
template< typename AttributeValueT, typename CharT >
BOOST_LOG_FORCEINLINE aux::fmt_date_time_gen< AttributeValueT, fallback_to_none, CharT > format_date_time(attribute_name const& name, const CharT* format)
{
    return aux::fmt_date_time_gen< AttributeValueT, fallback_to_none, CharT >(name, fallback_to_none(), format);
}

/*!
 * The function generates a manipulator node in a template expression. The manipulator must participate in a formatting
 * expression (stream output or \c format placeholder filler).
 *
 * \param name Attribute name
 * \param format Format string
 */
template< typename AttributeValueT, typename CharT >
BOOST_LOG_FORCEINLINE aux::fmt_date_time_gen< AttributeValueT, fallback_to_none, CharT > format_date_time(attribute_name const& name, std::basic_string< CharT > const& format)
{
    return aux::fmt_date_time_gen< AttributeValueT, fallback_to_none, CharT >(name, fallback_to_none(), format);
}

/*!
 * The function generates a manipulator node in a template expression. The manipulator must participate in a formatting
 * expression (stream output or \c format placeholder filler).
 *
 * \param keyword Attribute keyword
 * \param format Format string
 */
template< typename DescriptorT, template< typename > class ActorT, typename CharT >
BOOST_LOG_FORCEINLINE aux::fmt_date_time_gen< typename DescriptorT::value_type, fallback_to_none, CharT >
format_date_time(attribute_keyword< DescriptorT, ActorT > const& keyword, const CharT* format)
{
    return aux::fmt_date_time_gen< typename DescriptorT::value_type, fallback_to_none, CharT >(keyword.get_name(), fallback_to_none(), format);
}

/*!
 * The function generates a manipulator node in a template expression. The manipulator must participate in a formatting
 * expression (stream output or \c format placeholder filler).
 *
 * \param keyword Attribute keyword
 * \param format Format string
 */
template< typename DescriptorT, template< typename > class ActorT, typename CharT >
BOOST_LOG_FORCEINLINE aux::fmt_date_time_gen< typename DescriptorT::value_type, fallback_to_none, CharT >
format_date_time(attribute_keyword< DescriptorT, ActorT > const& keyword, std::basic_string< CharT > const& format)
{
    return aux::fmt_date_time_gen< typename DescriptorT::value_type, fallback_to_none, CharT >(keyword.get_name(), fallback_to_none(), format);
}

/*!
 * The function generates a manipulator node in a template expression. The manipulator must participate in a formatting
 * expression (stream output or \c format placeholder filler).
 *
 * \param placeholder Attribute placeholder
 * \param format Format string
 */
template< typename T, typename FallbackPolicyT, typename TagT, template< typename > class ActorT, typename CharT >
BOOST_LOG_FORCEINLINE aux::fmt_date_time_gen< T, FallbackPolicyT, CharT >
format_date_time(attribute_actor< T, FallbackPolicyT, TagT, ActorT > const& placeholder, const CharT* format)
{
    return aux::fmt_date_time_gen< T, FallbackPolicyT, CharT >(placeholder.get_name(), placeholder.get_fallback_policy(), format);
}

/*!
 * The function generates a manipulator node in a template expression. The manipulator must participate in a formatting
 * expression (stream output or \c format placeholder filler).
 *
 * \param placeholder Attribute placeholder
 * \param format Format string
 */
template< typename T, typename FallbackPolicyT, typename TagT, template< typename > class ActorT, typename CharT >
BOOST_LOG_FORCEINLINE aux::fmt_date_time_gen< T, FallbackPolicyT, CharT >
format_date_time(attribute_actor< T, FallbackPolicyT, TagT, ActorT > const& placeholder, std::basic_string< CharT > const& format)
{
    return aux::fmt_date_time_gen< T, FallbackPolicyT, CharT >(placeholder.get_name(), placeholder.get_fallback_policy(), format);
}

} // namespace expressions

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_EXPRESSIONS_FORMATTERS_DATE_TIME_HPP_INCLUDED_
