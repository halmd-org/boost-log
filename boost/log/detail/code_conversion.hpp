/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   code_conversion.hpp
 * \author Andrey Semashev
 * \date   08.11.2008
 *
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_DETAIL_CODE_CONVERSION_HPP_INCLUDED_
#define BOOST_LOG_DETAIL_CODE_CONVERSION_HPP_INCLUDED_

#include <locale>
#include <string>
#include <boost/log/detail/prologue.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace aux {

//! The function converts one string to the character type of another
BOOST_LOG_API void code_convert(const wchar_t* str1, std::size_t len, std::string& str2, std::locale const& loc = std::locale());
//! The function converts one string to the character type of another
BOOST_LOG_API void code_convert(const char* str1, std::size_t len, std::wstring& str2, std::locale const& loc = std::locale());
#ifndef BOOST_NO_CHAR16_T
//! The function converts one string to the character type of another
BOOST_LOG_API void code_convert(const char16_t* str1, std::size_t len, std::string& str2, std::locale const& loc = std::locale());
//! The function converts one string to the character type of another
BOOST_LOG_API void code_convert(const char* str1, std::size_t len, std::u16string& str2, std::locale const& loc = std::locale());
#endif
#ifndef BOOST_NO_CHAR32_T
//! The function converts one string to the character type of another
BOOST_LOG_API void code_convert(const char32_t* str1, std::size_t len, std::string& str2, std::locale const& loc = std::locale());
//! The function converts one string to the character type of another
BOOST_LOG_API void code_convert(const char* str1, std::size_t len, std::u32string& str2, std::locale const& loc = std::locale());
#endif

//! The function converts one string to the character type of another
template< typename CharT, typename SourceTraitsT, typename SourceAllocatorT, typename TargetTraitsT, typename TargetAllocatorT >
inline void code_convert(std::basic_string< CharT, SourceTraitsT, SourceAllocatorT > const& str1, std::basic_string< CharT, TargetTraitsT, TargetAllocatorT >& str2, std::locale const& = std::locale())
{
    str2.append(str1.c_str(), str1.size());
}
//! The function converts one string to the character type of another
template< typename CharT, typename TargetTraitsT, typename TargetAllocatorT >
inline void code_convert(const CharT* str1, std::size_t len, std::basic_string< CharT, TargetTraitsT, TargetAllocatorT >& str2, std::locale const& = std::locale())
{
    str2.append(str1, len);
}

//! The function converts one string to the character type of another
template< typename SourceCharT, typename SourceTraitsT, typename SourceAllocatorT, typename TargetCharT, typename TargetTraitsT, typename TargetAllocatorT >
inline void code_convert(std::basic_string< SourceCharT, SourceTraitsT, SourceAllocatorT > const& str1, std::basic_string< TargetCharT, TargetTraitsT, TargetAllocatorT >& str2, std::locale const& loc = std::locale())
{
    aux::code_convert(str1.c_str(), str1.size(), str2, loc);
}

//! The function converts the passed string to the narrow-character encoding
inline std::string const& to_narrow(std::string const& str)
{
    return str;
}

//! The function converts the passed string to the narrow-character encoding
inline std::string to_narrow(std::wstring const& str)
{
    std::string res;
    aux::code_convert(str, res);
    return res;
}

//! The function converts the passed string to the wide-character encoding
inline std::wstring const& to_wide(std::wstring const& str)
{
    return str;
}

//! The function converts the passed string to the wide-character encoding
inline std::wstring to_wide(std::string const& str)
{
    std::wstring res;
    aux::code_convert(str, res);
    return res;
}

} // namespace aux

} // namespace log

} // namespace boost

#endif // BOOST_LOG_DETAIL_CODE_CONVERSION_HPP_INCLUDED_
