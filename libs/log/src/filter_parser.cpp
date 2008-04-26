/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   filter_parser.cpp
 * \author Andrey Semashev
 * \date   31.03.2008
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#include <stack>
#include <string>
#include <stdexcept>

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
#include <boost/utility/in_place_factory.hpp>
#include <boost/spirit/core.hpp>
#include <boost/spirit/utility/confix.hpp>
#include <boost/spirit/utility/escape_char.hpp>
#include <boost/log/logging_core.hpp>
#include <boost/log/init/filter_parser.hpp>
#include <boost/log/type_dispatch/standard_types.hpp>
#include <boost/log/filters/basic_filters.hpp>
#include <boost/log/filters/attr.hpp>
#include <boost/log/filters/has_attr.hpp>
#include <boost/log/detail/functional.hpp>
#include "parser_utils.hpp"

namespace boost {

namespace log {

namespace {

//! Filter parsing grammar
template< typename CharT >
struct filter_grammar :
    public spirit::grammar< filter_grammar< CharT > >
{
    typedef CharT char_type;
    typedef std::basic_string< char_type > string_type;
    typedef typename basic_logging_core< char_type >::filter_type filter_type;
    typedef boost::log::aux::char_constants< char_type > constants;
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
            m_Grammar.m_Subexpressions.push(
                boost::log::filters::attr<
                    boost::log::integral_types
                >(m_Grammar.m_AttributeName.get()).satisfies(boost::log::aux::bind2nd(RelationT(), val)));
        }
        void operator() (double& val) const
        {
            m_Grammar.m_Subexpressions.push(
                boost::log::filters::attr<
                    boost::log::floating_point_types
                >(m_Grammar.m_AttributeName.get()).satisfies(boost::log::aux::bind2nd(RelationT(), val)));
        }
        void operator() (string_type& val) const
        {
            m_Grammar.m_Subexpressions.push(
                boost::log::filters::attr<
                    string_type
                >(m_Grammar.m_AttributeName.get()).satisfies(boost::log::aux::bind2nd(RelationT(), val)));
        }

    private:
        filter_grammar_type const& m_Grammar;
    };

    //! Parsed attribute name
    mutable optional< string_type > m_AttributeName;
    //! The second operand of a relation
    mutable optional< arg_type > m_Operand;

    //! Filter subexpressions as they are parsed
    mutable std::stack< filter_type > m_Subexpressions;

    //! Reference to the filter being constructed
    filter_type& m_Filter;

    //! Constructor
    explicit filter_grammar(filter_type& f) : m_Filter(f) {}

    //! The method finalizes filter construction by flushing its internal data that may not have been put into the filter
    void flush() const
    {
        if (!m_Subexpressions.empty())
            m_Filter.swap(m_Subexpressions.top());
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
        // In case if previous subexpression consisted only
        // from attribute name, like in "%Attribute1% & %Attribute2% > 1"
        make_has_attr();

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
        if (!m_Subexpressions.empty())
        {
            m_Subexpressions.top() = !boost::log::filters::wrap(m_Subexpressions.top());
        }
        else
        {
            // This would happen if a filter consists of a single '!'
            boost::throw_exception(std::logic_error("Filter parsing error:"
                " a negation operator applied to nothingness"));
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
                m_Subexpressions.push(
                    boost::log::filters::attr<
                        string_type
                    >(m_AttributeName.get()).satisfies(boost::log::aux::bind2nd(RelationT(), *operand)));
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
                m_Subexpressions.push(
                    boost::log::filters::attr<
                        string_type
                    >(m_AttributeName.get()).matches(*operand));
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
        if (!m_Subexpressions.empty())
        {
            filter_type right = m_Subexpressions.top();
            m_Subexpressions.pop();
            if (!m_Subexpressions.empty())
            {
                filter_type const& left = m_Subexpressions.top();
                typedef boost::log::filters::flt_wrap< char_type, filter_type > wrap_t;
                m_Subexpressions.top() = OperationT< wrap_t, wrap_t >(wrap_t(left), wrap_t(right));
                return;
            }
        }

        // This should never happen
        boost::throw_exception(std::logic_error("Filter parser internal error:"
            " the subexpression is not set while trying to construct a filter"));
    }

    //! The function is called when a full expression have finished parsing
    void on_expression_finished(const char_type* begin, const char_type* end) const
    {
        make_has_attr();
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
            m_Subexpressions.push(boost::log::filters::has_attr(m_AttributeName.get()));
            m_AttributeName = none;
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
            spirit::confix_p(constants::char_percent, *(spirit::print_p - constants::char_percent), constants::char_percent)
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

        expression = (term >> *(
            ((spirit::str_p(constants::and_keyword()) | constants::char_and) >> term)
                [bind(&filter_grammar_type::BOOST_NESTED_TEMPLATE on_operation< boost::log::filters::flt_and >, g, _1, _2)] |
            ((spirit::str_p(constants::or_keyword()) | constants::char_or) >> term)
                [bind(&filter_grammar_type::BOOST_NESTED_TEMPLATE on_operation< boost::log::filters::flt_or >, g, _1, _2)]
            ))[bind(&filter_grammar_type::on_expression_finished, g, _1, _2)];
    }

    //! Accessor for the filter rule
    rule_type const& start() const { return expression; }
};

} // namespace

//! The function parses a filter from the string
template< typename CharT >
typename basic_logging_core< CharT >::filter_type parse_filter(const CharT* begin, const CharT* end)
{
    typedef CharT char_type;
    typedef typename basic_logging_core< char_type >::filter_type filter_type;

    filter_type filt;
    filter_grammar< char_type > gram(filt);
    if (!spirit::parse(begin, end, gram, spirit::space_p).full)
        boost::throw_exception(std::runtime_error("Could not parse the filter"));
    gram.flush();

    return filt;
}

template BOOST_LOG_EXPORT
basic_logging_core< char >::filter_type parse_filter< char >(const char* begin, const char* end);
template BOOST_LOG_EXPORT
basic_logging_core< wchar_t >::filter_type parse_filter< wchar_t >(const wchar_t* begin, const wchar_t* end);

} // namespace log

} // namespace boost
