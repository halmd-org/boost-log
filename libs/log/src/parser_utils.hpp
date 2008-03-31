/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   parser_utils.hpp
 * \author Andrey Semashev
 * \date   31.03.2008
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_PARSER_UTILS_HPP_INCLUDED_
#define BOOST_LOG_PARSER_UTILS_HPP_INCLUDED_

#include <cctype>
#include <string>
#include <iterator>

namespace boost {

namespace log {

namespace aux {

//! Some constants and algorithms needed for parsing
template< typename > struct char_constants;

template< >
struct char_constants< char >
{
    typedef char char_type;
    static const char_type char_comment = '#';
    static const char_type char_quote = '"';
    static const char_type char_percent = '%';
    static const char_type char_exclamation = '!';
    static const char_type char_and = '&';
    static const char_type char_or = '|';
    static const char_type char_equal = '=';
    static const char_type char_greater = '>';
    static const char_type char_less = '<';
    static const char_type char_section_bracket_left = '[';
    static const char_type char_section_bracket_right = ']';
    static const char_type char_paren_bracket_left = '(';
    static const char_type char_paren_bracket_right = ')';

    static const char_type* not_keyword() { return "not"; }
    static const char_type* and_keyword() { return "and"; }
    static const char_type* or_keyword() { return "or"; }
    static const char_type* not_equal_keyword() { return "!="; }
    static const char_type* greater_or_equal_keyword() { return ">="; }
    static const char_type* less_or_equal_keyword() { return "<="; }
    static const char_type* begins_with_keyword() { return "begins_with"; }
    static const char_type* ends_with_keyword() { return "ends_with"; }
    static const char_type* contains_keyword() { return "contains"; }
    static const char_type* matches_keyword() { return "matches"; }

    static const char_type* core_section_name() { return "Core"; }

    static const char_type* core_disable_logging_param_name() { return "DisableLogging"; }
    static const char_type* filter_param_name() { return "Filter"; }

    static int to_number(char_type c)
    {
        using namespace std; // to make sure we can use C functions unqualified
        int n = 0;
        if (isdigit(c))
            n = c - '0';
        else if (c >= 'a' && c <= 'f')
            n = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F')
            n = c - 'A' + 10;
        return n;
    }

    static void translate_escape_sequences(std::basic_string< char_type >& str)
    {
        using namespace std; // to make sure we can use C functions unqualified

        std::basic_string< char_type >::iterator it = str.begin();
        while (it != str.end())
        {
            it = std::find(it, str.end(), '\\');
            if (std::distance(it, str.end()) >= 2)
            {
                str.erase(it);
                switch (*it)
                {
                    case 'n':
                        *it = '\n'; break;
                    case 'r':
                        *it = '\r'; break;
                    case 'a':
                        *it = '\a'; break;
                    case '\\':
                        ++it; break;
                    case 't':
                        *it = '\t'; break;
                    case 'b':
                        *it = '\b'; break;
                    case 'x':
                    {
                        std::basic_string< char_type >::iterator b = it;
                        if (std::distance(++b, str.end()) >= 2)
                        {
                            char_type c1 = *b++, c2 = *b++;
                            if (isxdigit(c1) && isxdigit(c2))
                            {
                                *it++ = char_type((to_number(c1) << 4) | to_number(c2));
                                str.erase(it, b);
                            }
                        }
                        break;
                    }
                    default:
                    {
                        if (*it >= '0' && *it <= '7')
                        {
                            std::basic_string< char_type >::iterator b = it;
                            int c = (*b++) - '0';
                            if (*b >= '0' && *b <= '7')
                                c = c * 8 + (*b++) - '0';
                            if (*b >= '0' && *b <= '7')
                                c = c * 8 + (*b++) - '0';

                            *it++ = char_type(c);
                            str.erase(it, b);
                        }
                        break;
                    }
                }
            }
        }
    }
};
template< >
struct char_constants< wchar_t >
{
    typedef wchar_t char_type;
    static const char_type char_comment = L'#';
    static const char_type char_quote = L'"';
    static const char_type char_percent = L'%';
    static const char_type char_exclamation = L'!';
    static const char_type char_and = L'&';
    static const char_type char_or = L'|';
    static const char_type char_equal = L'=';
    static const char_type char_greater = L'>';
    static const char_type char_less = L'<';
    static const char_type char_section_bracket_left = L'[';
    static const char_type char_section_bracket_right = L']';
    static const char_type char_paren_bracket_left = L'(';
    static const char_type char_paren_bracket_right = L')';

    static const char_type* not_keyword() { return L"not"; }
    static const char_type* and_keyword() { return L"and"; }
    static const char_type* or_keyword() { return L"or"; }
    static const char_type* not_equal_keyword() { return L"!="; }
    static const char_type* greater_or_equal_keyword() { return L">="; }
    static const char_type* less_or_equal_keyword() { return L"<="; }
    static const char_type* begins_with_keyword() { return L"begins_with"; }
    static const char_type* ends_with_keyword() { return L"ends_with"; }
    static const char_type* contains_keyword() { return L"contains"; }
    static const char_type* matches_keyword() { return L"matches"; }

    static const char_type* core_section_name() { return L"Core"; }

    static const char_type* core_disable_logging_param_name() { return L"DisableLogging"; }
    static const char_type* filter_param_name() { return L"Filter"; }

    static int to_number(char_type c)
    {
        int n = 0;
        if (c >= L'0' && c <= L'9')
            n = c - L'0';
        else if (c >= L'a' && c <= L'f')
            n = c - 'a' + 10;
        else if (c >= L'A' && c <= L'F')
            n = c - L'A' + 10;
        return n;
    }

    static bool iswxdigit(char_type c)
    {
        return (c >= L'0' && c <= L'9') || (c >= L'a' && c <= L'f') || (c >= L'A' && c <= L'F');
    }

    static void translate_escape_sequences(std::basic_string< char_type >& str)
    {
        std::basic_string< char_type >::iterator it = str.begin();
        while (it != str.end())
        {
            it = std::find(it, str.end(), L'\\');
            if (std::distance(it, str.end()) >= 2)
            {
                str.erase(it);
                switch (*it)
                {
                    case L'n':
                        *it = L'\n'; break;
                    case L'r':
                        *it = L'\r'; break;
                    case L'a':
                        *it = L'\a'; break;
                    case L'\\':
                        ++it; break;
                    case L't':
                        *it = L'\t'; break;
                    case L'b':
                        *it = L'\b'; break;
                    case L'x':
                    {
                        std::basic_string< char_type >::iterator b = it;
                        if (std::distance(++b, str.end()) >= 2)
                        {
                            char_type c1 = *b++, c2 = *b++;
                            if (iswxdigit(c1) && iswxdigit(c2))
                            {
                                *it++ = char_type((to_number(c1) << 4) | to_number(c2));
                                str.erase(it, b);
                            }
                        }
                        break;
                    }
                    case L'u':
                    {
                        std::basic_string< char_type >::iterator b = it;
                        if (std::distance(++b, str.end()) >= 4)
                        {
                            char_type c1 = *b++, c2 = *b++, c3 = *b++, c4 = *b++;
                            if (iswxdigit(c1) && iswxdigit(c2) && iswxdigit(c3) && iswxdigit(c4))
                            {
                                *it++ = char_type(
                                    (to_number(c1) << 12) |
                                    (to_number(c2) << 8) |
                                    (to_number(c3) << 4) |
                                    to_number(c4));
                                str.erase(it, b);
                            }
                        }
                        break;
                    }
                    case L'U':
                    {
                        std::basic_string< char_type >::iterator b = it;
                        if (std::distance(++b, str.end()) >= 8)
                        {
                            char_type c1 = *b++, c2 = *b++, c3 = *b++, c4 = *b++;
                            char_type c5 = *b++, c6 = *b++, c7 = *b++, c8 = *b++;
                            if (iswxdigit(c1) && iswxdigit(c2) && iswxdigit(c3) && iswxdigit(c4) &&
                                iswxdigit(c5) && iswxdigit(c6) && iswxdigit(c7) && iswxdigit(c8))
                            {
                                *it++ = char_type(
                                    (to_number(c1) << 28) |
                                    (to_number(c2) << 24) |
                                    (to_number(c3) << 20) |
                                    (to_number(c4) << 16) |
                                    (to_number(c5) << 12) |
                                    (to_number(c6) << 8) |
                                    (to_number(c7) << 4) |
                                    to_number(c8));
                                str.erase(it, b);
                            }
                        }
                        break;
                    }
                    default:
                    {
                        if (*it >= L'0' && *it <= L'7')
                        {
                            std::basic_string< char_type >::iterator b = it;
                            int c = (*b++) - L'0';
                            if (*b >= L'0' && *b <= L'7')
                                c = c * 8 + (*b++) - L'0';
                            if (*b >= L'0' && *b <= L'7')
                                c = c * 8 + (*b++) - L'0';

                            *it++ = char_type(c);
                            str.erase(it, b);
                        }
                        break;
                    }
                }
            }
        }
    }
};

} // namespace aux

} // namespace log

} // namespace boost

#endif // BOOST_LOG_PARSER_UTILS_HPP_INCLUDED_
