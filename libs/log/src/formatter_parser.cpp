/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   formatter_parser.cpp
 * \author Andrey Semashev
 * \date   07.04.2008
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#include <string>
#include <locale> // Ticket #1788.
#include <iterator> // Ticket #1788.
#include <stdexcept>

#undef BOOST_MPL_LIMIT_VECTOR_SIZE
#define BOOST_MPL_LIMIT_VECTOR_SIZE 50

#ifndef BOOST_SPIRIT_THREADSAFE
#define BOOST_SPIRIT_THREADSAFE
#endif // BOOST_SPIRIT_THREADSAFE

#include <boost/ref.hpp>
#include <boost/bind.hpp>
#include <boost/compatibility/cpp_c_headers/ctime>
#include <boost/io/ios_state.hpp> // Ticket #1788.
#include <boost/mpl/vector.hpp>
#include <boost/mpl/copy.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/back_inserter.hpp>
#include <boost/mpl/joint_view.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/spirit/core.hpp>
#include <boost/spirit/utility/confix.hpp>
#include <boost/spirit/utility/escape_char.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/init/formatter_parser.hpp>
#include <boost/log/formatters/attr.hpp>
#include <boost/log/formatters/chain.hpp>
#include <boost/log/formatters/wrappers.hpp>
#include <boost/log/formatters/message.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/type_dispatch/standard_types.hpp>
#include <boost/log/type_dispatch/date_time_types.hpp>
#include "parser_utils.hpp"
#include "singleton.hpp"

namespace boost {

// Workaround for Boost.DateTime bug. Ticket #642.
// http://svn.boost.org/trac/boost/ticket/642
namespace date_time {
using boost::gregorian::operator<<;
} // namespace date_time

// Workaround for absence of local_time::local_time_period output operator in Boost.DateTime
// posix_time::operator<< implementation taken as a starting point. Author: Jeff Garland, Bart Garst.
// Ticket #1788.
// http://svn.boost.org/trac/boost/ticket/1788
namespace local_time {

template <class CharT, class TraitsT>
inline std::basic_ostream<CharT, TraitsT>& operator<<(
    std::basic_ostream<CharT, TraitsT>& os, const boost::local_time::local_time_period& p)
{
    boost::io::ios_flags_saver iflags(os);
    typedef boost::date_time::time_facet<local_date_time, CharT> facet_t;
    typedef std::time_put<CharT> std_time_facet;
    std::ostreambuf_iterator<CharT> oitr(os);
    if (std::has_facet<facet_t>(os.getloc()))
    {
        std::use_facet<facet_t>(os.getloc()).put(oitr, os, os.fill(), p);
    }
    else
    {
        std::ostreambuf_iterator<CharT> oitr(os);
        facet_t* f = new facet_t();
        std::locale l = std::locale(os.getloc(), f);
        os.imbue(l);
        f->put(oitr, os, os.fill(), p);
    }
    return os;
}

} // namespace local_time

namespace log {

namespace {

//! The structure contains formatter factories repository
template< typename CharT >
struct formatters_repository :
    public log::aux::lazy_singleton< formatters_repository< CharT > >
{
    friend class log::aux::lazy_singleton< formatters_repository< CharT > >;

    //! Synchronization mutex
    mutable shared_mutex m_Mutex;
    //! The map of formatter factories
    typename formatter_types< CharT >::factories_map m_Map;

private:
    formatters_repository() {}
};

//! Formatter parsing grammar
template< typename CharT >
struct formatter_grammar :
    public spirit::grammar< formatter_grammar< CharT > >
{
    typedef CharT char_type;
    typedef typename formatter_types< char_type >::string_type string_type;
    typedef typename formatter_types< char_type >::formatter_type formatter_type;
    typedef boost::log::aux::char_constants< char_type > constants;
    typedef formatter_grammar< char_type > formatter_grammar_type;

    template< typename ScannerT >
    struct definition;

private:
    //! The formatter being constructed
    formatter_type& m_Formatter;

    //! Attribute name
    mutable string_type m_AttrName;
    //! Formatter factory arguments
    mutable typename formatter_types< char_type >::formatter_factory_args m_FactoryArgs;

    //! Formatter argument name
    mutable string_type m_ArgName;
    //! Argument value
    mutable string_type m_ArgValue;

public:
    //! Constructor
    explicit formatter_grammar(formatter_type& fmt) : m_Formatter(fmt) {}

    //! The method is called when an argument name is discovered
    void on_arg_name(const char_type* begin, const char_type* end) const
    {
        m_ArgName.assign(begin, end);
    }
    //! The method is called when an argument value is discovered
    void on_arg_value(const char_type* begin, const char_type* end) const
    {
        m_ArgValue.assign(begin, end);
        constants::translate_escape_sequences(m_ArgValue);
    }
    //! The method is called when an argument is filled
    void push_arg(const char_type* begin, const char_type* end) const
    {
        m_FactoryArgs[m_ArgName] = m_ArgValue;
        m_ArgName.clear();
        m_ArgValue.clear();
    }

    //! The method is called when an attribute name is discovered
    void on_attr_name(const char_type* begin, const char_type* end) const
    {
        m_AttrName.assign(begin, end);
    }
    //! The method is called when an attribute is filled
    void push_attr(const char_type* begin, const char_type* end) const
    {
        if (m_AttrName == constants::message_text_keyword())
        {
            // We make a special treatment for the message text formatter
            append_formatter(formatters::fmt_message< char_type >());
        }
        else
        {
            formatters_repository< char_type > const& repo = formatters_repository< char_type >::get();
            shared_lock< shared_mutex > lock(repo.m_Mutex);
    
            typename formatter_types< CharT >::factories_map::const_iterator it = repo.m_Map.find(m_AttrName);
            if (it != repo.m_Map.end())
            {
                // We've found a user-defined factory for this attribute
                append_formatter(it->second(m_AttrName, m_FactoryArgs));
            }
            else
            {
                // No user-defined factory, shall use the most generic formatter we can ever imagine at this point
                lock.unlock();

                typedef mpl::joint_view<
                    // We have to exclude std::time_t since it's an integral type and will conflict with one of the standard types
                    boost_date_time_types,
                    mpl::joint_view<
                        boost_time_duration_types,
                        boost_time_period_types
                    >
                > time_related_types;

                typedef typename mpl::copy<
                    mpl::joint_view<
                        time_related_types,
                        typename make_default_attribute_types< char_type >::type
                    >,
                    mpl::back_inserter<
                        mpl::vector1<
                            attributes::basic_named_scope_list< char_type >
                        >
                    >
                >::type supported_types;

                append_formatter(formatters::attr< supported_types >(m_AttrName));
            }
        }

        // Eventually, clear all the auxiliary data
        m_AttrName.clear();
        m_FactoryArgs.clear();
    }

    //! The method is called when a string literal is discovered
    void push_string(const char_type* begin, const char_type* end) const
    {
        string_type str(begin, end);
        constants::translate_escape_sequences(str);
        append_formatter(formatters::fmt_wrapper< char_type, string_type >(str));
    }

private:
    //  Assignment and copying are prohibited
    formatter_grammar(formatter_grammar const&);
    formatter_grammar& operator= (formatter_grammar const&);

    //! The method appends a formatter part to the final formatter
    template< typename FormatterT >
    void append_formatter(FormatterT const& fmt) const
    {
        if (!m_Formatter.empty())
            m_Formatter = formatters::fmt_chain< char_type, formatter_type, FormatterT >(m_Formatter, fmt);
        else
            m_Formatter = fmt;
    }
};

//! Grammar definition
template< typename CharT >
template< typename ScannerT >
struct formatter_grammar< CharT >::definition
{
    //! Boost.Spirit rule type
    typedef spirit::rule< ScannerT > rule_type;

    //! A parser for an argument value
    rule_type arg_value;
    //! A parser for a named argument
    rule_type arg;
    //! A parser for an optional argument list for a formatter
    rule_type arg_list;
    //! A parser for the complete formatter expression
    rule_type expression;

    //! Constructor
    definition(formatter_grammar_type const& gram)
    {
        reference_wrapper< const formatter_grammar_type > g(gram);

        arg_value = spirit::confix_p(constants::char_quote, *spirit::c_escape_ch_p, constants::char_quote) |
            *(spirit::print_p - spirit::space_p);

        arg = *spirit::space_p >>
            (*spirit::alnum_p)[bind(&formatter_grammar_type::on_arg_name, g, _1, _2)] >>
            *spirit::space_p >>
            constants::char_equal >>
            *spirit::space_p >>
            arg_value[bind(&formatter_grammar_type::on_arg_value, g, _1, _2)] >>
            *spirit::space_p;

        arg_list = spirit::confix_p(
            constants::char_paren_bracket_left,
            arg[bind(&formatter_grammar_type::push_arg, g, _1, _2)] >>
                *(spirit::ch_p(constants::char_comma) >> arg[bind(&formatter_grammar_type::push_arg, g, _1, _2)]),
            constants::char_paren_bracket_right);

        expression =
            (*(spirit::c_escape_ch_p - constants::char_percent))
                [bind(&formatter_grammar_type::push_string, g, _1, _2)] >>
            *(
                spirit::confix_p(
                    constants::char_percent,
                    (
                        spirit::str_p(constants::message_text_keyword())
                            [bind(&formatter_grammar_type::on_attr_name, g, _1, _2)] |
                        (
                            (*(spirit::print_p - constants::char_paren_bracket_left))
                                [bind(&formatter_grammar_type::on_attr_name, g, _1, _2)] >>
                            !arg_list
                        )
                    ),
                    constants::char_percent)[bind(&formatter_grammar_type::push_attr, g, _1, _2)] >>
                (*(spirit::c_escape_ch_p - constants::char_percent))
                    [bind(&formatter_grammar_type::push_string, g, _1, _2)]
            );
    }

    //! Accessor for the formatter rule
    rule_type const& start() const { return expression; }
};

} // namespace

//! The function registers a user-defined formatter factory
template< typename CharT >
void register_formatter_factory(
    const CharT* attr_name,
    typename formatter_types< CharT >::formatter_factory const& factory)
{
    typename formatter_types< CharT >::string_type name(attr_name);
    formatters_repository< CharT >& repo = formatters_repository< CharT >::get();

    unique_lock< shared_mutex > _(repo.m_Mutex);
    repo.m_Map[name] = factory;
}

//! The function parses a formatter from the string
template< typename CharT >
typename formatter_types< CharT >::formatter_type
parse_formatter(const CharT* begin, const CharT* end)
{
    typedef CharT char_type;
    typedef typename formatter_types< char_type >::formatter_type formatter_type;

    formatter_type fmt;
    formatter_grammar< char_type > gram(fmt);
    if (!spirit::parse(begin, end, gram).full)
        boost::throw_exception(std::runtime_error("Could not parse the formatter"));

    return fmt;
}

template
void register_formatter_factory< char >(
    const char* attr_name,
    formatter_types< char >::formatter_factory const& factory);
template
void register_formatter_factory< wchar_t >(
    const wchar_t* attr_name,
    formatter_types< wchar_t >::formatter_factory const& factory);

template
formatter_types< char >::formatter_type parse_formatter< char >(const char* begin, const char* end);
template
formatter_types< wchar_t >::formatter_type parse_formatter< wchar_t >(const wchar_t* begin, const wchar_t* end);

} // namespace log

} // namespace boost
