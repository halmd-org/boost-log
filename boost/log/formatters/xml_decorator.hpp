/*
 * (C) 2009 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * This header is the Boost.Log library implementation, see the library documentation
 * at http://www.boost.org/libs/log/doc/log.html.
 */
/*!
 * \file   xml_decorator.hpp
 * \author Andrey Semashev
 * \date   07.06.2009
 *
 * The header contains implementation of XML-style decorator.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_FORMATTERS_XML_DECORATOR_HPP_INCLUDED_
#define BOOST_LOG_FORMATTERS_XML_DECORATOR_HPP_INCLUDED_

#include <boost/range/iterator_range.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/formatters/char_decorator.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace formatters {

namespace aux {

    template< typename >
    struct xml_decorator_traits;

#ifdef BOOST_LOG_USE_CHAR
    template< >
    struct xml_decorator_traits< char >
    {
        static const char* const g_From[4];
        static const char* const g_To[4];
    };
    const char* const xml_decorator_traits< char >::g_From[4] = { "&", "<", ">", "'" };
    const char* const xml_decorator_traits< char >::g_To[4] = { "&amp;", "&lt;", "&gt;", "&apos;" };
#endif // BOOST_LOG_USE_CHAR

#ifdef BOOST_LOG_USE_WCHAR_T
    template< >
    struct xml_decorator_traits< wchar_t >
    {
        static const wchar_t* const g_From[4];
        static const wchar_t* const g_To[4];
    };
    const wchar_t* const xml_decorator_traits< wchar_t >::g_From[4] = { L"&", L"<", L">", L"'" };
    const wchar_t* const xml_decorator_traits< wchar_t >::g_To[4] = { L"&amp;", L"&lt;", L"&gt;", L"&apos;" };
#endif // BOOST_LOG_USE_WCHAR_T

    struct fmt_xml_decorator_gen
    {
        template< typename FormatterT >
        fmt_char_decorator< FormatterT > operator[] (FormatterT const& fmt) const
        {
            typedef fmt_char_decorator< FormatterT > decorator;
            typedef xml_decorator_traits< typename decorator::char_type > traits_t;
            return decorator(
                fmt,
                boost::make_iterator_range(traits_t::g_From),
                boost::make_iterator_range(traits_t::g_To));
        }
    };

} // namespace aux

#ifndef BOOST_LOG_DOXYGEN_PASS

const aux::fmt_xml_decorator_gen xml_dec = {};

#else // BOOST_LOG_DOXYGEN_PASS

/*!
 * XML-style decorator generator object. The decorator replaces characters that have special meaning
 * in XML documents with the corresponding decorated counterparts. The generator provides
 * <tt>operator[]</tt> that can be used to construct the actual decorator. For example:
 *
 * <code>
 * xml_dec[ attr< std::string >("MyAttr") ]
 * </code>
 */
implementation_defined xml_dec;

#endif // BOOST_LOG_DOXYGEN_PASS

} // namespace formatters

} // namespace log

} // namespace boost

#endif // BOOST_LOG_FORMATTERS_XML_DECORATOR_HPP_INCLUDED_
