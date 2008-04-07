/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   formatter_parser.hpp
 * \author Andrey Semashev
 * \date   07.04.2008
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_INIT_FORMATTER_PARSER_HPP_INCLUDED_
#define BOOST_LOG_INIT_FORMATTER_PARSER_HPP_INCLUDED_

#include <iosfwd>
#include <map>
#include <string>
#include <boost/function/function2.hpp>
#include <boost/function/function3.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/attributes/attribute_values_view.hpp>

#ifdef _MSC_VER
#pragma warning(push)
// 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
// non dll-interface class 'A' used as base for dll-interface class 'B'
#pragma warning(disable: 4275)
#endif // _MSC_VER

namespace boost {

namespace log {

//! The structure generates commonly used types related to formatters
template< typename CharT >
struct formatter_types
{
    //! Character type
    typedef CharT char_type;
    //! String type
    typedef std::basic_string< char_type > string_type;
    //! Output stream type
    typedef std::basic_ostream< char_type > ostream_type;
    //! Map of attribute values
    typedef basic_attribute_values_view< char_type > attribute_values_view;
    //! The formatter function object
    typedef function3< void, ostream_type&, attribute_values_view const&, string_type const& > formatter_type;

    //! Map of formatter factory arguments [argument name -> argument value]
    typedef std::map< string_type, string_type > formatter_factory_args;
    /*!
     * \brief The function object constructs the formatter
     * \param name Attribute name
     * \param args Formatter arguments
     * \return The constructed formatter. The formatter must not be empty.
     * \throw std::exception-based If an exception is thrown from the method, the exception is propagated to the parse_formatter caller
     */
    typedef function2< formatter_type, string_type const&, formatter_factory_args const& > formatter_factory;
    //! Map of formatter factory function objects
    typedef std::map< string_type, formatter_factory > factories_map;
};

//! The function registers a user-defined formatter factory
template< typename CharT >
BOOST_LOG_EXPORT void
register_formatter_factory(
    const CharT* attr_name,
    typename formatter_types< CharT >::formatter_factory const& factory);

//! The function registers a user-defined formatter factory
template< typename CharT, typename TraitsT, typename AllocatorT >
inline void
register_formatter_factory(
    std::basic_string< CharT, TraitsT, AllocatorT > const& attr_name,
    typename formatter_types< CharT >::formatter_factory const& factory)
{
    register_formatter_factory(attr_name.c_str(), factory);
}

//! The function parses a formatter from the string
template< typename CharT >
BOOST_LOG_EXPORT typename formatter_types< CharT >::formatter_type
parse_formatter(const CharT* begin, const CharT* end);

//! The function parses a formatter from the string
template< typename CharT, typename TraitsT, typename AllocatorT >
inline typename formatter_types< CharT >::formatter_type
parse_formatter(std::basic_string< CharT, TraitsT, AllocatorT > const& str)
{
    const CharT* p = str.c_str();
    return parse_formatter(p, p + str.size());
}

//! The function parses a formatter from the string
template< typename CharT >
inline typename formatter_types< CharT >::formatter_type
parse_formatter(const CharT* str)
{
    return parse_formatter(str, str + std::char_traits< CharT >::length(str));
}

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_INIT_FORMATTER_PARSER_HPP_INCLUDED_
