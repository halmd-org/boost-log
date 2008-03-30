/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   init_from_stream.cpp
 * \author Andrey Semashev
 * \date   22.03.2008
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#include <cctype>
#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <locale>
#include <utility>
#include <stdexcept>
#include <algorithm>
#include <functional>

#ifndef BOOST_SPIRIT_THREADSAFE
#define BOOST_SPIRIT_THREADSAFE
#endif // BOOST_SPIRIT_THREADSAFE

#include <boost/ref.hpp>
#include <boost/bind.hpp>
#include <boost/none.hpp>
#include <boost/optional.hpp>
#include <boost/throw_exception.hpp>
#include <boost/variant/get.hpp>
#include <boost/variant/variant.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/utility/in_place_factory.hpp>
#include <boost/spirit/core.hpp>
#include <boost/spirit/utility/confix.hpp>
#include <boost/spirit/utility/escape_char.hpp>
#include <boost/log/logging_core.hpp>
#include <boost/log/init/from_stream.hpp>
#include <boost/log/type_dispatch/standard_types.hpp>
#include <boost/log/filters/basic_filters.hpp>
#include <boost/log/filters/attr.hpp>
#include <boost/log/filters/has_attr.hpp>
#include <boost/log/detail/functional.hpp>

namespace boost {

namespace log {

namespace {

//! Some constants needed for parsing
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

    static void translate_escape_sequences(std::basic_string< char_type >& str)
    {
        using namespace std; // to make sure we can use C functions unqualified
        struct local
        {
            static int to_number(char_type c)
            {
                int n = 0;
                if (isdigit(c))
                    n = c - '0';
                else if (c >= 'a' && c <= 'f')
                    n = c - 'a' + 10;
                else if (c >= 'A' && c <= 'F')
                    n = c - 'A' + 10;
                return n;
            }
        };

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
                                *it++ = char_type((local::to_number(c1) << 4) | local::to_number(c2));
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

    static void translate_escape_sequences(std::basic_string< char_type >& str)
    {
        struct local
        {
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
        };

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
                            if (local::iswxdigit(c1) && local::iswxdigit(c2))
                            {
                                *it++ = char_type((local::to_number(c1) << 4) | local::to_number(c2));
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
                            if (local::iswxdigit(c1) && local::iswxdigit(c2) && local::iswxdigit(c3) && local::iswxdigit(c4))
                            {
                                *it++ = char_type(
                                    (local::to_number(c1) << 12) |
                                    (local::to_number(c2) << 8) |
                                    (local::to_number(c3) << 4) |
                                    local::to_number(c4));
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
                            if (local::iswxdigit(c1) && local::iswxdigit(c2) && local::iswxdigit(c3) && local::iswxdigit(c4) &&
                                local::iswxdigit(c5) && local::iswxdigit(c6) && local::iswxdigit(c7) && local::iswxdigit(c8))
                            {
                                *it++ = char_type(
                                    (local::to_number(c1) << 28) |
                                    (local::to_number(c2) << 24) |
                                    (local::to_number(c3) << 20) |
                                    (local::to_number(c4) << 16) |
                                    (local::to_number(c5) << 12) |
                                    (local::to_number(c6) << 8) |
                                    (local::to_number(c7) << 4) |
                                    local::to_number(c8));
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

//! The class represents parsed logging settings
template< typename CharT >
class settings
{
public:
    //! Character type
    typedef CharT char_type;
    //! String type
    typedef std::basic_string< char_type > string_type;
    //! The type of the map of parameters and their names
    typedef std::map< string_type, string_type > params_t;
    //! The type of the map of sections
    typedef std::map< string_type, params_t > sections_t;
    //! Structure with character constants
    typedef char_constants< char_type > constants;

private:
    //! Parameters
    sections_t m_Sections;

public:
    //! The constructor reads parameters from the stream
    explicit settings(std::basic_istream< char_type >& strm)
    {
        typedef typename string_type::iterator str_iterator;
        std::locale loc = strm.getloc();
        typename sections_t::iterator current_section = m_Sections.end();
        string_type line;
        for (unsigned int line_counter = 1; strm.good(); ++line_counter)
        {
            line.clear();
            std::getline(strm, line);
            boost::algorithm::trim(line, loc);

            // Skipping empty lines and comments
            // NOTE: The comments are only allwed to be the whole line.
            //       Comments beginning in the middle of the line are not supported.
            if (!line.empty() && line[0] != constants::char_comment)
            {
                // Check if the line is a section starter
                if (line[0] == constants::char_section_bracket_left)
                {
                    str_iterator it = std::find(line.begin() + 1, line.end(), constants::char_section_bracket_right);
                    string_type section_name(line.begin() + 1, it);
                    boost::algorithm::trim(section_name, loc);
                    if (it != line.end() && !section_name.empty())
                    {
                        // Creating a new section
                        current_section = m_Sections.insert(std::make_pair(section_name, params_t())).first;
                    }
                    else
                    {
                        // The section starter is broken
                        std::ostringstream descr;
                        descr << "At line " << line_counter << ". The section header is invalid.";
                        boost::throw_exception(std::runtime_error(descr.str()));
                    }
                }
                else
                {
                    // Check that we've already started a section
                    if (current_section != m_Sections.end())
                    {
                        // Find the '=' between the parameter name and value
                        str_iterator it = std::find(line.begin(), line.end(), constants::char_equal);
                        string_type param_name(line.begin(), it);
                        boost::algorithm::trim_right(param_name, loc);
                        if (it != line.end() && !param_name.empty())
                        {
                            // Put the parameter value into the map
                            string_type param_value(++it, line.end());
                            boost::algorithm::trim_left(param_value, loc);
                            if (param_value.size() >= 2
                                && param_value[0] == constants::char_quote && *param_value.rbegin() == constants::char_quote)
                            {
                                param_value = param_value.substr(1, param_value.size() - 2);
                            }

                            current_section->second[param_name] = param_value;
                        }
                        else
                        {
                            // The parameter name is not valid
                            std::ostringstream descr;
                            descr << "At line " << line_counter << ". Parameter description is not valid.";
                            boost::throw_exception(std::runtime_error(descr.str()));
                        }
                    }
                    else
                    {
                        // The parameter encountered before any section starter
                        std::ostringstream descr;
                        descr << "At line " << line_counter << ". Parameters are only allowed in sections.";
                        boost::throw_exception(std::runtime_error(descr.str()));
                    }
                }
            }
        }
    }

    //! Accessor for the map of sections
    sections_t const& sections() const { return m_Sections; }
};

//! Filter parsing grammar
template< typename CharT >
struct filter_grammar :
    public spirit::grammar< filter_grammar< CharT > >
{
    typedef CharT char_type;
    typedef std::basic_string< char_type > string_type;
    typedef typename basic_logging_core< char_type >::filter_type filter_type;
    typedef char_constants< char_type > constants;
    typedef filter_grammar< char_type > filter_grammar_type;

    template< typename ScannerT >
    struct definition;

    //! Relation operand type
    typedef variant<
        long int,
        double,
        string_type
    > arg_type;

    //! A visitor for the arg_type actual type detection
    template< typename RelationT >
    struct binary_relation_visitor :
        public boost::static_visitor< >
    {
        explicit binary_relation_visitor(filter_grammar_type const& gram) : m_Grammar(gram) {}

        void operator() (long int& val) const
        {
            m_Grammar.m_Subexpression =
                boost::log::filters::attr<
                    boost::log::integral_types
                >(m_Grammar.m_AttributeName.get()).satisfies(boost::log::aux::bind2nd(RelationT(), val));
        }
        void operator() (double& val) const
        {
            m_Grammar.m_Subexpression =
                boost::log::filters::attr<
                    boost::log::floating_point_types
                >(m_Grammar.m_AttributeName.get()).satisfies(boost::log::aux::bind2nd(RelationT(), val));
        }
        void operator() (string_type& val) const
        {
            m_Grammar.m_Subexpression =
                boost::log::filters::attr<
                    string_type
                >(m_Grammar.m_AttributeName.get()).satisfies(boost::log::aux::bind2nd(RelationT(), val));
        }

    private:
        filter_grammar_type const& m_Grammar;
    };

    //! Parsed attribute name
    mutable optional< string_type > m_AttributeName;
    //! The second operand of a relation
    mutable optional< arg_type > m_Operand;

    //! Intermediate filter subexpression
    mutable filter_type m_Subexpression;

    //! Reference to the filter being constructed
    filter_type& m_Filter;

    //! Constructor
    explicit filter_grammar(filter_type& f) : m_Filter(f) {}

    //! The method finalizes filter construction by flushing its internal data that may not have been put into the filter
    void flush() const
    {
        make_has_attr();

        // In case if the whole filter is a single condition, like "%Attribute%" or "%Attribute% > 5"
        if (!m_Subexpression.empty())
            m_Filter.swap(m_Subexpression);
    }

    //! The quoted string handler
    void on_quoted_string(const char_type* begin, const char_type* end) const
    {
        // An attribute name should have been parsed at this time
        if (!m_AttributeName)
            boost::throw_exception(std::runtime_error("Invalid filter definition: quoted string is not expected"));

        // Cut off the quotes
        string_type str(begin + 1, end - 1);

        // Translate escape sequences
        constants::translate_escape_sequences(str);
        m_Operand = str;
    }
    //! The attribute name handler
    void on_attribute(const char_type* begin, const char_type* end) const
    {
        // Cut off the '%'
        m_AttributeName = boost::in_place(++begin, --end);
    }
    //! The floating point constant handler
    void on_fp_constant(double val) const
    {
        // An attribute name should have been parsed at this time
        if (!m_AttributeName)
            boost::throw_exception(std::runtime_error("Invalid filter definition: floating point constant is not expected"));
        m_Operand = val;
    }
    //! The integer constant handler
    void on_integer_constant(long int val) const
    {
        // An attribute name should have been parsed at this time
        if (!m_AttributeName)
            boost::throw_exception(std::runtime_error("Invalid filter definition: integer constant is not expected"));
        m_Operand = val;
    }

    //! The negation operation handler
    void on_negation(const char_type* begin, const char_type* end) const
    {
        make_has_attr();
        if (!m_Subexpression.empty())
        {
            // In case if negation is applied to a single subexpression, like !%Attribute%
            m_Subexpression = !boost::log::filters::wrap(m_Subexpression);
        }
        else if (!m_Filter.empty())
        {
            // In case if negation is applied to an expression, like !(%Attribute1% > 2 & %Attribute2% < 3)
            m_Filter = !boost::log::filters::wrap(m_Filter);
        }
        else
        {
            // This should never happen
            boost::throw_exception(std::logic_error("Filter parser internal error:"
                " neither the filter tor the subexpression is set while trying to apply a negation"));
        }
    }
    //! The binary relation handler
    template< typename RelationT >
    void on_binary_relation(const char_type* begin, const char_type* end) const
    {
        if (!!m_AttributeName && !!m_Operand)
        {
            binary_relation_visitor< RelationT > vis(*this);
            m_Operand->apply_visitor(vis);
            m_AttributeName = none;
            m_Operand = none;
        }
        else
        {
            // This should never happen
            boost::throw_exception(std::logic_error("Filter parser internal error:"
                " the attribute name or subexpression operand is not set while trying to construct a subexpression"));
        }
    }

    //! The binary relation handler for string operands
    template< typename RelationT >
    void on_binary_string_relation(const char_type* begin, const char_type* end) const
    {
        if (!!m_AttributeName && !!m_Operand)
        {
            if (string_type* operand = boost::get< string_type >(m_Operand.get_ptr()))
            {
                m_Subexpression =
                    boost::log::filters::attr<
                        string_type
                    >(m_AttributeName.get()).satisfies(boost::log::aux::bind2nd(RelationT(), *operand));
                m_AttributeName = none;
                m_Operand = none;
            }
            else
            {
                // This can happen if one tries to apply a string-specific relation to a non-string operand
                boost::throw_exception(std::runtime_error("Filter parser error:"
                    " the operand type in the subexpression should be string"));
            }
        }
        else
        {
            // This should never happen
            boost::throw_exception(std::logic_error("Filter parser internal error:"
                " the attribute name or subexpression operand is not set while trying to construct a subexpression"));
        }
    }

    //! The Boost.RegEx match handler for string operands
    void on_match_relation(const char_type* begin, const char_type* end) const
    {
        if (!!m_AttributeName && !!m_Operand)
        {
            if (string_type* operand = boost::get< string_type >(m_Operand.get_ptr()))
            {
                m_Subexpression =
                    boost::log::filters::attr<
                        string_type
                    >(m_AttributeName.get()).matches(*operand);
                m_AttributeName = none;
                m_Operand = none;
            }
            else
            {
                // This can happen if one tries to apply a string-specific relation to a non-string operand
                boost::throw_exception(std::runtime_error("Filter parser error:"
                    " the operand type in the subexpression should be string"));
            }
        }
        else
        {
            // This should never happen
            boost::throw_exception(std::logic_error("Filter parser internal error:"
                " the attribute name or subexpression operand is not set while trying to construct a subexpression"));
        }
    }

    //! The boolean operation handler
    template< template< typename, typename > class OperationT >
    void on_operation(const char_type* begin, const char_type* end) const
    {
        if (!m_Subexpression.empty() && !m_Filter.empty())
        {
            typedef boost::log::filters::flt_wrap< char_type, filter_type > wrap_t;
            m_Filter = OperationT< wrap_t, wrap_t >(wrap_t(m_Filter), wrap_t(m_Subexpression));
            m_Subexpression.clear();
        }
        else
        {
            // This should never happen
            boost::throw_exception(std::logic_error("Filter parser internal error:"
                " the subexpression is not set while trying to construct a filter"));
        }
    }

private:
    //  Assignment and copying are prohibited
    filter_grammar(filter_grammar const&);
    filter_grammar& operator= (filter_grammar const&);

    //! The function converts the parsed attribute name into a has_attr filter
    void make_has_attr() const
    {
        if (!!m_AttributeName)
        {
            if (!m_Subexpression)
            {
                m_Subexpression = boost::log::filters::has_attr(m_AttributeName.get());
                m_AttributeName = none;
            }
            else
            {
                // This should never happen
                boost::throw_exception(std::logic_error("Filter parser internal error:"
                    " the subexpression is already set while trying to make another subexpression of has_attr filter"));
            }
        }
    }
};

//! Grammar definition
template< typename CharT >
template< typename ScannerT >
struct filter_grammar< CharT >::definition
{
    //! Boost.Spirit rule type
    typedef spirit::rule< ScannerT > rule_type;

    //! A parser for a single node of the filter expression
    rule_type node;
    //! A parser for a possibly negated node or parenthesized subexpression
    rule_type factor;
    //! A parser for a single condition that consists of two operands and an operation between them
    rule_type term;
    //! A parser for the complete filter expression that consists of one or several terms with boolean operations between them
    rule_type expression;

    //! Constructor
    definition(filter_grammar_type const& gram)
    {
        reference_wrapper< const filter_grammar_type > g(gram);
        node =
            // A quoted string with C-style escape sequences support
            spirit::confix_p(constants::char_quote, *spirit::c_escape_ch_p, constants::char_quote)
                [bind(&filter_grammar_type::on_quoted_string, g, _1, _2)] |
            // An attribute name in form %name%
            spirit::confix_p(constants::char_percent, *spirit::print_p, constants::char_percent)
                [bind(&filter_grammar_type::on_attribute, g, _1, _2)] |
            spirit::strict_real_p[bind(&filter_grammar_type::on_fp_constant, g, _1)] |
            spirit::int_p[bind(&filter_grammar_type::on_integer_constant, g, _1)];

        factor = node |
            (constants::char_paren_bracket_left >> expression >> constants::char_paren_bracket_right) |
            ((spirit::str_p(constants::not_keyword()) | constants::char_exclamation) >> factor)
                [bind(&filter_grammar_type::on_negation, g, _1, _2)];

        term = factor >> *(
            (constants::char_equal >> factor)
                [bind(&filter_grammar_type::BOOST_NESTED_TEMPLATE on_binary_relation< boost::log::aux::equal_to >, g, _1, _2)] |
            (constants::char_greater >> factor)
                [bind(&filter_grammar_type::BOOST_NESTED_TEMPLATE on_binary_relation< boost::log::aux::greater >, g, _1, _2)] |
            (constants::char_less >> factor)
                [bind(&filter_grammar_type::BOOST_NESTED_TEMPLATE on_binary_relation< boost::log::aux::less >, g, _1, _2)] |
            (spirit::str_p(constants::not_equal_keyword()) >> factor)
                [bind(&filter_grammar_type::BOOST_NESTED_TEMPLATE on_binary_relation< boost::log::aux::not_equal_to >, g, _1, _2)] |
            (spirit::str_p(constants::greater_or_equal_keyword()) >> factor)
                [bind(&filter_grammar_type::BOOST_NESTED_TEMPLATE on_binary_relation< boost::log::aux::greater_equal >, g, _1, _2)] |
            (spirit::str_p(constants::less_or_equal_keyword()) >> factor)
                [bind(&filter_grammar_type::BOOST_NESTED_TEMPLATE on_binary_relation< boost::log::aux::less_equal >, g, _1, _2)] |
            (spirit::str_p(constants::begins_with_keyword()) >> factor)
                [bind(&filter_grammar_type::BOOST_NESTED_TEMPLATE on_binary_string_relation< boost::log::aux::begins_with_fun >, g, _1, _2)] |
            (spirit::str_p(constants::ends_with_keyword()) >> factor)
                [bind(&filter_grammar_type::BOOST_NESTED_TEMPLATE on_binary_string_relation< boost::log::aux::ends_with_fun >, g, _1, _2)] |
            (spirit::str_p(constants::contains_keyword()) >> factor)
                [bind(&filter_grammar_type::BOOST_NESTED_TEMPLATE on_binary_string_relation< boost::log::aux::contains_fun >, g, _1, _2)] |
            (spirit::str_p(constants::matches_keyword()) >> factor)
                [bind(&filter_grammar_type::on_match_relation, g, _1, _2)]
            );

        expression = term >> *(
            ((spirit::str_p(constants::and_keyword()) | constants::char_and) >> term)
                [bind(&filter_grammar_type::BOOST_NESTED_TEMPLATE on_operation< boost::log::filters::flt_and >, g, _1, _2)] |
            ((spirit::str_p(constants::or_keyword()) | constants::char_or) >> term)
                [bind(&filter_grammar_type::BOOST_NESTED_TEMPLATE on_operation< boost::log::filters::flt_or >, g, _1, _2)]
            );
    }

    //! Accessor for the filter rule
    rule_type const& start() const { return expression; }
};

//! The function parses a filter from the string
template< typename CharT >
typename basic_logging_core< CharT >::filter_type parse_filter(const CharT* begin, const CharT* end)
{
    typedef CharT char_type;
    typedef typename basic_logging_core< char_type >::filter_type filter_type;

    filter_type filt;
    filter_grammar< char_type > gram(filt);
    spirit::parse(begin, end, gram);
    gram.flush();

    return filt;
}

//! The function applies the settings to the logging core
template< typename CharT >
void apply_core_settings(std::map< std::basic_string< CharT >, std::basic_string< CharT > > const& params)
{
    typedef CharT char_type;
    typedef std::basic_string< char_type > string_type;
    typedef std::map< string_type, string_type > params_t;
    typedef char_constants< char_type > constants;
    typedef std::basic_istringstream< char_type > isstream;
    typedef basic_logging_core< char_type > core_t;
    shared_ptr< core_t > core = core_t::get();

    // Filter
    typename params_t::const_iterator it =
        params.find(constants::filter_param_name());
    if (it != params.end())
    {
        const char_type* p = it->second.c_str();
        core->set_filter(parse_filter(p, p + it->second.size()));
    }
    else
        core->reset_filter();

    // DisableLogging
    it = params.find(constants::core_disable_logging_param_name());
    if (it != params.end())
    {
        isstream strm(it->second);
        strm.setf(std::ios_base::boolalpha);
        bool f = false;
        strm >> f;
        core->set_logging_enabled(!f);
    }
    else
        core->set_logging_enabled(true);
}

} // namespace

//! The function initializes the logging library from a stream containing logging settings
template< typename CharT >
void init_from_stream(std::basic_istream< CharT >& strm)
{
    // Parse the settings
    typedef settings< CharT > settings_t;
    typedef typename settings_t::constants constants;
    settings_t setts(strm);

    // Apply core settings
    typename settings_t::sections_t const& sections = setts.sections();
    typename settings_t::sections_t::const_iterator core_params =
        sections.find(constants::core_section_name());
    if (core_params != sections.end())
        apply_core_settings(core_params->second);
}

template void init_from_stream< char >(std::basic_istream< char >& strm);
template void init_from_stream< wchar_t >(std::basic_istream< wchar_t >& strm);

} // namespace log

} // namespace boost
