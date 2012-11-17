/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   formatters/named_scope.hpp
 * \author Andrey Semashev
 * \date   11.11.2012
 *
 * The header contains a formatter function for named scope attribute values.
 */

#ifndef BOOST_LOG_EXPRESSIONS_FORMATTERS_NAMED_SCOPE_HPP_INCLUDED_
#define BOOST_LOG_EXPRESSIONS_FORMATTERS_NAMED_SCOPE_HPP_INCLUDED_

#include <string>
#include <iterator>
#include <utility>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/parameter/binding.hpp>
#include <boost/preprocessor/iteration/iterate.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/attributes/attribute_name.hpp>
#include <boost/log/attributes/fallback_policy.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/detail/light_function.hpp>
#include <boost/log/detail/parameter_tools.hpp>
#include <boost/log/expressions/attr_fwd.hpp>
#include <boost/log/expressions/keyword_fwd.hpp>
#include <boost/log/expressions/output_manip_terminal.hpp>
#include <boost/log/utility/formatting_stream_fwd.hpp>
#include <boost/log/utility/string_literal_fwd.hpp>
#include <boost/log/keywords/format.hpp>
#include <boost/log/keywords/delimiter.hpp>
#include <boost/log/keywords/depth.hpp>
#include <boost/log/keywords/iteration.hpp>

#ifdef BOOST_LOG_HAS_PRAGMA_ONCE
#pragma once
#endif

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace expressions {

//! Scope iteration directions
enum scope_iteration_direction
{
    forward,    //!< Iterate through scopes from outermost to innermost
    reverse     //!< Iterate through scopes from innermost to outermost
};

namespace aux {

//! Parses the named scope format string and constructs the formatter function
template< typename CharT >
BOOST_LOG_API boost::log::aux::light_function2< void, basic_formatting_ostream< CharT >&, attributes::named_scope::value_type::value_type const& >
parse_named_scope_format(const CharT* begin, const CharT* end);

//! Parses the named scope format string and constructs the formatter function
template< typename CharT >
inline boost::log::aux::light_function2< void, basic_formatting_ostream< CharT >&, attributes::named_scope::value_type::value_type const& >
parse_named_scope_format(const CharT* format)
{
    return parse_named_scope_format(format, format + std::char_traits< CharT >::length(format));
}

//! Parses the named scope format string and constructs the formatter function
template< typename CharT, typename TraitsT, typename AllocatorT >
inline boost::log::aux::light_function2< void, basic_formatting_ostream< CharT >&, attributes::named_scope::value_type::value_type const& >
parse_named_scope_format(std::basic_string< CharT, TraitsT, AllocatorT > const& format)
{
    const CharT* p = format.c_str();
    return parse_named_scope_format(p, p + format.size());
}

//! Parses the named scope format string and constructs the formatter function
template< typename CharT, typename TraitsT >
inline boost::log::aux::light_function2< void, basic_formatting_ostream< CharT >&, attributes::named_scope::value_type::value_type const& >
parse_named_scope_format(basic_string_literal< CharT, TraitsT > const& format)
{
    const CharT* p = format.c_str();
    return parse_named_scope_format(p, p + format.size());
}

template< typename CharT >
class fmt_named_scope_impl
{
public:
    //! Function result type
    typedef void result_type;

    //! Character type
    typedef CharT char_type;
    //! String type
    typedef std::basic_string< char_type > string_type;
    //! Formatting stream type
    typedef basic_formatting_ostream< char_type > stream_type;
    //! Attribute value type
    typedef attributes::named_scope::value_type value_type;
    //! Named scope formatter
    typedef boost::log::aux::light_function2< void, stream_type&, value_type::value_type const& > element_formatter_type;

private:
    //! Element formatting function
    element_formatter_type m_element_formatter;
    //! Element delimiter
    string_type m_delimiter;
    //! Maximum number of elements to output
    value_type::size_type m_depth;
    //! Iteration direction
    scope_iteration_direction m_direction;

public:
    //! Initializing constructor
    fmt_named_scope_impl(element_formatter_type const& element_formatter, string_type const& delimiter, value_type::size_type depth, scope_iteration_direction direction) :
        m_element_formatter(element_formatter),
        m_delimiter(delimiter),
        m_depth(depth),
        m_direction(direction)
    {
    }
    //! Copy constructor
    fmt_named_scope_impl(fmt_named_scope_impl const& that) :
        m_element_formatter(that.m_element_formatter),
        m_delimiter(that.m_delimiter),
        m_depth(that.m_depth),
        m_direction(that.m_direction)
    {
    }

    //! Formatting operator
    result_type operator() (stream_type& strm, value_type const& scopes) const
    {
        if (m_direction == expressions::forward)
            format_forward(strm, scopes);
        else
            format_reverse(strm, scopes);
    }

private:
    //! The function performs formatting of the extracted scope stack in forward direction
    void format_forward(stream_type& strm, value_type const& scopes) const
    {
        value_type::const_iterator it, end = scopes.end();
        if (m_depth > 0)
        {
            value_type::size_type const scopes_to_iterate = (std::min)(m_depth, scopes.size());
            it = scopes.end();
            std::advance(it, -static_cast< value_type::difference_type >(scopes_to_iterate));
        }
        else
        {
            it = scopes.begin();
        }

        if (it != end)
        {
            if (it != scopes.begin())
                strm << "..." << m_delimiter;

            m_element_formatter(strm, *it);
            for (++it; it != end; ++it)
            {
                strm << m_delimiter;
                m_element_formatter(strm, *it);
            }
        }
    }
    //! The function performs formatting of the extracted scope stack in reverse direction
    void format_reverse(stream_type& strm, value_type const& scopes) const
    {
        value_type::const_reverse_iterator it = scopes.rbegin(), end;
        if (m_depth > 0)
        {
            value_type::size_type const scopes_to_iterate = (std::min)(m_depth, scopes.size());
            end = it;
            std::advance(end, static_cast< value_type::difference_type >(scopes_to_iterate));
        }
        else
        {
            end = scopes.rend();
        }

        if (it != end)
        {
            m_element_formatter(strm, *it);
            for (++it; it != end; ++it)
            {
                strm << m_delimiter;
                m_element_formatter(strm, *it);
            }

            if (it != scopes.rend())
                strm << m_delimiter << "...";
        }
    }
};

template< typename FallbackPolicyT, typename CharT >
class fmt_named_scope_gen :
    private FallbackPolicyT
{
public:
    //! Attribute value type
    typedef attributes::named_scope::value_type value_type;
    //! Fallback policy
    typedef FallbackPolicyT fallback_policy;
    //! Character type
    typedef CharT char_type;
    //! String type
    typedef std::basic_string< char_type > string_type;
    //! Formatting stream type
    typedef basic_formatting_ostream< char_type > stream_type;

    //! Formatter function
    typedef fmt_named_scope_impl< char_type > formatter_function_type;

private:
    //! Attribute name
    attribute_name m_name;
    //! Formattr function
    formatter_function_type m_formatter;

public:
    //! Initializing constructor
    template< typename FormatT >
    fmt_named_scope_gen(attribute_name const& name, fallback_policy const& fallback, FormatT const& element_format, string_type const& delimiter, value_type::size_type depth, scope_iteration_direction direction) :
        fallback_policy(fallback), m_name(name), m_formatter((parse_named_scope_format)(element_format), delimiter, depth, direction)
    {
    }
    //! Copy constructor
    fmt_named_scope_gen(fmt_named_scope_gen const& that) : fallback_policy(static_cast< fallback_policy const& >(that)), m_name(that.m_name), m_formatter(that.m_formatter)
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
    template< typename LeftExprT, typename FallbackPolicyT, typename CharT >\
    BOOST_LOG_FORCEINLINE phoenix::actor< output_manip_terminal< phoenix::actor< LeftExprT >, attributes::named_scope::value_type, FallbackPolicyT, fmt_named_scope_impl< CharT > > >\
    operator<< (phoenix::actor< LeftExprT > left_ref left, fmt_named_scope_gen< FallbackPolicyT, CharT > right_ref right)\
    {\
        typedef output_manip_terminal< phoenix::actor< LeftExprT >, attributes::named_scope::value_type, FallbackPolicyT, fmt_named_scope_impl< CharT > > terminal_type;\
        phoenix::actor< terminal_type > actor = {{ terminal_type(left, right.get_name(), right.get_formatter_function(), right.get_fallback_policy()) }};\
        return actor;\
    }

#include <boost/log/detail/generate_overloads.hpp>

#undef BOOST_LOG_AUX_OVERLOAD

//! Auxiliary traits to detect character type from a string
template< typename T >
struct deduce_char_type;

template< typename CharT, typename TraitsT, typename AllocatorT >
struct deduce_char_type< std::basic_string< CharT, TraitsT, AllocatorT > >
{
    typedef CharT type;
};

template< typename CharT, typename TraitsT >
struct deduce_char_type< basic_string_literal< CharT, TraitsT > >
{
    typedef CharT type;
};

template< >
struct deduce_char_type< const char* >
{
    typedef char type;
};

template< >
struct deduce_char_type< char* >
{
    typedef char type;
};

template< unsigned int CountV >
struct deduce_char_type< const char(&)[CountV] >
{
    typedef char type;
};

template< unsigned int CountV >
struct deduce_char_type< char(&)[CountV] >
{
    typedef char type;
};

template< >
struct deduce_char_type< const wchar_t* >
{
    typedef wchar_t type;
};

template< >
struct deduce_char_type< wchar_t* >
{
    typedef wchar_t type;
};

template< unsigned int CountV >
struct deduce_char_type< const wchar_t(&)[CountV] >
{
    typedef wchar_t type;
};

template< unsigned int CountV >
struct deduce_char_type< wchar_t(&)[CountV] >
{
    typedef wchar_t type;
};

//! Auxiliary traits to acquire correct default delimiter depending on the character type
template< typename CharT >
struct default_scope_delimiter;

#ifdef BOOST_LOG_USE_CHAR
template< >
struct default_scope_delimiter< char >
{
    static const char* forward() { return "->"; }
    static const char* reverse() { return "<-"; }
};
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
template< >
struct default_scope_delimiter< wchar_t >
{
    static const wchar_t* forward() { return L"->"; }
    static const wchar_t* reverse() { return L"<-"; }
};
#endif

template< typename CharT, typename FallbackPolicyT, typename ArgsT >
BOOST_LOG_FORCEINLINE fmt_named_scope_gen< FallbackPolicyT, CharT > format_named_scope(attribute_name const& name, FallbackPolicyT const& fallback, ArgsT const& args)
{
    typedef fmt_named_scope_gen< FallbackPolicyT, CharT > result_type;
    scope_iteration_direction dir = args[keywords::iteration | expressions::forward];
    const CharT* default_delimiter = (dir == expressions::forward ? default_scope_delimiter< CharT >::forward() : default_scope_delimiter< CharT >::reverse());
    return result_type
    (
        name,
        fallback,
        args[keywords::format],
        args[keywords::delimiter | default_delimiter],
        args[keywords::depth | static_cast< attributes::named_scope::value_type::size_type >(0)],
        dir
    );
}

} // namespace aux

/*!
 * The function generates a manipulator node in a template expression. The manipulator must participate in a formatting
 * expression (stream output or \c format placeholder filler).
 *
 * \param name Attribute name
 * \param element_format Format string for a single named scope
 */
template< typename CharT >
BOOST_LOG_FORCEINLINE aux::fmt_named_scope_gen< fallback_to_none, CharT > format_named_scope(attribute_name const& name, const CharT* element_format)
{
    return aux::fmt_named_scope_gen< fallback_to_none, CharT >(name, fallback_to_none(), element_format);
}

/*!
 * The function generates a manipulator node in a template expression. The manipulator must participate in a formatting
 * expression (stream output or \c format placeholder filler).
 *
 * \param name Attribute name
 * \param element_format Format string for a single named scope
 */
template< typename CharT >
BOOST_LOG_FORCEINLINE aux::fmt_named_scope_gen< fallback_to_none, CharT > format_named_scope(attribute_name const& name, std::basic_string< CharT > const& element_format)
{
    return aux::fmt_named_scope_gen< fallback_to_none, CharT >(name, fallback_to_none(), element_format);
}

/*!
 * The function generates a manipulator node in a template expression. The manipulator must participate in a formatting
 * expression (stream output or \c format placeholder filler).
 *
 * \param keyword Attribute keyword
 * \param element_format Format string for a single named scope
 */
template< typename DescriptorT, template< typename > class ActorT, typename CharT >
BOOST_LOG_FORCEINLINE aux::fmt_named_scope_gen< fallback_to_none, CharT >
format_named_scope(attribute_keyword< DescriptorT, ActorT > const& keyword, const CharT* element_format)
{
    BOOST_STATIC_ASSERT_MSG((is_same< typename DescriptorT::value_type, attributes::named_scope::value_type >::value),\
        "Boost.Log: Named scope formatter only accepts attribute values of type attributes::named_scope::value_type.");
    return aux::fmt_named_scope_gen< fallback_to_none, CharT >(keyword.get_name(), fallback_to_none(), element_format);
}

/*!
 * The function generates a manipulator node in a template expression. The manipulator must participate in a formatting
 * expression (stream output or \c format placeholder filler).
 *
 * \param keyword Attribute keyword
 * \param element_format Format string for a single named scope
 */
template< typename DescriptorT, template< typename > class ActorT, typename CharT >
BOOST_LOG_FORCEINLINE aux::fmt_named_scope_gen< fallback_to_none, CharT >
format_named_scope(attribute_keyword< DescriptorT, ActorT > const& keyword, std::basic_string< CharT > const& element_format)
{
    BOOST_STATIC_ASSERT_MSG((is_same< typename DescriptorT::value_type, attributes::named_scope::value_type >::value),\
        "Boost.Log: Named scope formatter only accepts attribute values of type attributes::named_scope::value_type.");
    return aux::fmt_named_scope_gen< fallback_to_none, CharT >(keyword.get_name(), fallback_to_none(), element_format);
}

/*!
 * The function generates a manipulator node in a template expression. The manipulator must participate in a formatting
 * expression (stream output or \c format placeholder filler).
 *
 * \param placeholder Attribute placeholder
 * \param element_format Format string for a single named scope
 */
template< typename T, typename FallbackPolicyT, typename TagT, template< typename > class ActorT, typename CharT >
BOOST_LOG_FORCEINLINE aux::fmt_named_scope_gen< FallbackPolicyT, CharT >
format_named_scope(attribute_actor< T, FallbackPolicyT, TagT, ActorT > const& placeholder, const CharT* element_format)
{
    BOOST_STATIC_ASSERT_MSG((is_same< T, attributes::named_scope::value_type >::value),\
        "Boost.Log: Named scope formatter only accepts attribute values of type attributes::named_scope::value_type.");
    return aux::fmt_named_scope_gen< FallbackPolicyT, CharT >(placeholder.get_name(), placeholder.get_fallback_policy(), element_format);
}

/*!
 * The function generates a manipulator node in a template expression. The manipulator must participate in a formatting
 * expression (stream output or \c format placeholder filler).
 *
 * \param placeholder Attribute placeholder
 * \param element_format Format string for a single named scope
 */
template< typename T, typename FallbackPolicyT, typename TagT, template< typename > class ActorT, typename CharT >
BOOST_LOG_FORCEINLINE aux::fmt_named_scope_gen< FallbackPolicyT, CharT >
format_named_scope(attribute_actor< T, FallbackPolicyT, TagT, ActorT > const& placeholder, std::basic_string< CharT > const& element_format)
{
    BOOST_STATIC_ASSERT_MSG((is_same< T, attributes::named_scope::value_type >::value),\
        "Boost.Log: Named scope formatter only accepts attribute values of type attributes::named_scope::value_type.");
    return aux::fmt_named_scope_gen< FallbackPolicyT, CharT >(placeholder.get_name(), placeholder.get_fallback_policy(), element_format);
}

#if !defined(BOOST_LOG_DOXYGEN_PASS)

#   define BOOST_PP_FILENAME_1 <boost/log/detail/named_scope_fmt_pp.hpp>
#   define BOOST_PP_ITERATION_LIMITS (1, 4)
#   include BOOST_PP_ITERATE()

#else // BOOST_LOG_DOXYGEN_PASS

/*!
 * Formatter generator. Construct the named scope formatter with the specified formatting parameters.
 *
 * \param name Attribute name
 * \param args An set of named parameters. Supported parameters:
 *             \li \c format - A format string for named scopes. The string can contain "%n", "%f" and "%l" placeholders for the scope name, file and line number, respectively. This parameter is mandatory.
 *             \li \c delimiter - A string that is used to delimit the formatted scope names. Default: "->" or "<-", depending on the iteration direction.
 *             \li \c iteration - Iteration direction, see \c scope_iteration_direction enumeration. Default: forward.
 *             \li \c depth - Iteration depth. Default: unlimited.
 */
template< typename... ArgsT >
unspecified format_named_scope(attribute_name const& name, ArgsT... const& args);

/*! \overload */
template< typename DescriptorT, template< typename > class ActorT, typename... ArgsT >
unspecified format_named_scope(attribute_keyword< DescriptorT, ActorT > const& keyword, ArgsT... const& args);

/*! \overload */
template< typename T, typename FallbackPolicyT, typename TagT, template< typename > class ActorT, typename... ArgsT >
unspecified format_named_scope(attribute_actor< T, FallbackPolicyT, TagT, ActorT > const& placeholder, ArgsT... const& args);

#endif // BOOST_LOG_DOXYGEN_PASS

} // namespace expressions

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_EXPRESSIONS_FORMATTERS_NAMED_SCOPE_HPP_INCLUDED_
