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
#include <iostream>

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
    static const char_type char_comma = ',';
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

    static const char_type* message_text_keyword() { return "_"; }

    static const char_type* default_level_attribute_name() { return "Severity"; }

    static const char_type* core_section_name() { return "Core"; }
    static const char_type* sink_section_name_prefix() { return "Sink:"; }

    static const char_type* core_disable_logging_param_name() { return "DisableLogging"; }
    static const char_type* filter_param_name() { return "Filter"; }

    static const char_type* sink_destination_param_name() { return "Destination"; }
    static const char_type* file_name_param_name() { return "FileName"; }
    static const char_type* rotation_size_param_name() { return "RotationSize"; }
    static const char_type* rotation_interval_param_name() { return "RotationInterval"; }
    static const char_type* auto_flush_param_name() { return "AutoFlush"; }
    static const char_type* asynchronous_param_name() { return "Asynchronous"; }
    static const char_type* format_param_name() { return "Format"; }

    static const char_type* text_file_destination() { return "TextFile"; }
    static const char_type* console_destination() { return "Console"; }
    static const char_type* syslog_destination() { return "Syslog"; }

    static std::ostream& get_console_log_stream() { return std::clog; }

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

    static void translate_escape_sequences(std::basic_string< char_type >& str);
};

template< >
struct char_constants< wchar_t >
{
    typedef wchar_t char_type;
    static const char_type char_comment = L'#';
    static const char_type char_comma = L',';
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

    static const char_type* message_text_keyword() { return L"_"; }

    static const char_type* default_level_attribute_name() { return L"Severity"; }

    static const char_type* core_section_name() { return L"Core"; }
    static const char_type* sink_section_name_prefix() { return L"Sink:"; }

    static const char_type* core_disable_logging_param_name() { return L"DisableLogging"; }
    static const char_type* filter_param_name() { return L"Filter"; }

    static const char_type* sink_destination_param_name() { return L"Destination"; }
    static const char_type* file_name_param_name() { return L"FileName"; }
    static const char_type* rotation_size_param_name() { return L"RotationSize"; }
    static const char_type* rotation_interval_param_name() { return L"RotationInterval"; }
    static const char_type* auto_flush_param_name() { return L"AutoFlush"; }
    static const char_type* asynchronous_param_name() { return L"Asynchronous"; }
    static const char_type* format_param_name() { return L"Format"; }

    static const char_type* text_file_destination() { return L"TextFile"; }
    static const char_type* console_destination() { return L"Console"; }
    static const char_type* syslog_destination() { return L"Syslog"; }

    static std::wostream& get_console_log_stream() { return std::wclog; }

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

    static void translate_escape_sequences(std::basic_string< char_type >& str);
};

} // namespace aux

} // namespace log

} // namespace boost

#endif // BOOST_LOG_PARSER_UTILS_HPP_INCLUDED_
