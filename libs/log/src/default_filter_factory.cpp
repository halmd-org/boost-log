/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   default_filter_factory.hpp
 * \author Andrey Semashev
 * \date   29.05.2010
 *
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#include <string>
#include <boost/move/move.hpp>
#include <boost/phoenix/core.hpp>
#include <boost/phoenix/bind/bind_function_object.hpp>
#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_eoi.hpp>
#include <boost/spirit/include/qi_as.hpp>
#include <boost/log/exceptions.hpp>
#include <boost/log/expressions/attr.hpp>
#include <boost/log/expressions/has_attr.hpp>
#include <boost/log/utility/type_dispatch/standard_types.hpp>
#include <boost/log/utility/string_literal.hpp>
#include <boost/log/detail/functional.hpp>
#include <boost/log/detail/code_conversion.hpp>
#include <boost/log/support/xpressive.hpp>
#include <boost/xpressive/xpressive_dynamic.hpp>
#if defined(BOOST_LOG_USE_CHAR) && defined(BOOST_LOG_USE_WCHAR_T)
#include <boost/fusion/container/set.hpp>
#include <boost/fusion/algorithm/query/find.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#endif
#include "default_filter_factory.hpp"
#include "parser_utils.hpp"
#include "spirit_encoding.hpp"

namespace qi = boost::spirit::qi;

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace aux {

//! The callback for equality relation filter
template< typename CharT >
filter default_filter_factory< CharT >::on_equality_relation(attribute_name const& name, string_type const& arg)
{
    return parse_argument< log::aux::equal_to >(name, arg);
}

//! The callback for inequality relation filter
template< typename CharT >
filter default_filter_factory< CharT >::on_inequality_relation(attribute_name const& name, string_type const& arg)
{
    return parse_argument< log::aux::not_equal_to >(name, arg);
}

//! The callback for less relation filter
template< typename CharT >
filter default_filter_factory< CharT >::on_less_relation(attribute_name const& name, string_type const& arg)
{
    return parse_argument< log::aux::less >(name, arg);
}

//! The callback for greater relation filter
template< typename CharT >
filter default_filter_factory< CharT >::on_greater_relation(attribute_name const& name, string_type const& arg)
{
    return parse_argument< log::aux::greater >(name, arg);
}

//! The callback for less or equal relation filter
template< typename CharT >
filter default_filter_factory< CharT >::on_less_or_equal_relation(attribute_name const& name, string_type const& arg)
{
    return parse_argument< log::aux::less_equal >(name, arg);
}

//! The callback for greater or equal relation filter
template< typename CharT >
filter default_filter_factory< CharT >::on_greater_or_equal_relation(attribute_name const& name, string_type const& arg)
{
    return parse_argument< log::aux::greater_equal >(name, arg);
}

//! The callback for custom relation filter
template< typename CharT >
filter default_filter_factory< CharT >::on_custom_relation(attribute_name const& name, string_type const& rel, string_type const& arg)
{
    typedef log::aux::char_constants< char_type > constants;
    if (rel == constants::begins_with_keyword())
        return filter(log::filters::attr< string_type >(name).begins_with(arg));
    else if (rel == constants::ends_with_keyword())
        return filter(log::filters::attr< string_type >(name).ends_with(arg));
    else if (rel == constants::contains_keyword())
        return filter(log::filters::attr< string_type >(name).contains(arg));
    else if (rel == constants::matches_keyword())
    {
        typedef xpressive::basic_regex< typename string_type::const_iterator > regex_t;
        regex_t rex = regex_t::compile(arg, regex_t::ECMAScript | regex_t::optimize);
        return filter(log::filters::attr< string_type >(name, std::nothrow).matches(rex));
    }
    else
    {
        BOOST_LOG_THROW_DESCR(parse_error, "The custom attribute relation \"" + log::aux::to_narrow(rel) + "\" is not supported");
    }
}

template< typename CharT >
template< typename RelationT >
struct default_filter_factory< CharT >::on_integral_argument
{
    typedef void result_type;

    on_integral_argument(attribute_name const& name, filter& f) : m_name(name), m_filter(f)
    {
    }

    result_type operator() (long val) const
    {
        m_filter = phoenix::bind(RelationT(), expressions::attr< log::integral_types >(m_name), val);
    }

private:
    attribute_name m_name;
    filter& m_filter;
};

template< typename CharT >
template< typename RelationT >
struct default_filter_factory< CharT >::on_fp_argument
{
    typedef void result_type;

    on_fp_argument(attribute_name const& name, filter& f) : m_name(name), m_filter(f)
    {
    }

    result_type operator() (double val) const
    {
        m_filter = phoenix::bind(RelationT(), expressions::attr< log::floating_point_types >(m_name), val);
    }

private:
    attribute_name m_name;
    filter& m_filter;
};

template< typename CharT >
template< typename RelationT >
struct default_filter_factory< CharT >::on_string_argument
{
    typedef void result_type;

#if defined(BOOST_LOG_USE_CHAR) && defined(BOOST_LOG_USE_WCHAR_T)
    //! A special filtering predicate that adopts the string operand to the attribute value character type
    struct predicate :
        public RelationT
    {
        struct initializer
        {
            typedef void result_type;

            explicit initializer(string_type const& val) : m_initializer(val)
            {
            }

            template< typename T >
            result_type operator() (T& val) const
            {
                try
                {
                    log::aux::code_convert(m_initializer, val);
                }
                catch (...)
                {
                    val.clear();
                }
            }

        private:
            string_type const& m_initializer;
        };

        typedef typename RelationT::result_type result_type;

        explicit predicate(string_type const& operand)
        {
            fusion::for_each(m_operands, initializer(operand));
        }

        template< typename T >
        result_type operator() (T const& val) const
        {
            typedef std::basic_string< typename T::value_type > operand_type;
            return RelationT::operator() (val, fusion::find< operand_type >(m_operands));
        }

    private:
        fusion::set< std::string, std::wstring > m_operands;
    };
#endif

    on_string_argument(attribute_name const& name, filter& f) : m_name(name), m_filter(f)
    {
    }

    result_type operator() (string_type const& val) const
    {
#if defined(BOOST_LOG_USE_CHAR) && defined(BOOST_LOG_USE_WCHAR_T)
        m_filter = phoenix::bind(predicate(val), expressions::attr< log::string_types >(m_name));
#else
        m_filter = phoenix::bind(RelationT(), expressions::attr< log::string_types >(m_name), val);
#endif
    }

private:
    attribute_name m_name;
    filter& m_filter;
};


//! The function parses the argument value for a binary relation and constructs the corresponding filter
template< typename CharT >
template< typename RelationT >
filter default_filter_factory< CharT >::parse_argument(attribute_name const& name, string_type const& arg)
{
    typedef log::aux::encoding_specific< typename log::aux::encoding< char_type >::type > encoding_specific;
    const qi::real_parser< double, qi::strict_real_policies< double > > real_;

    filter f;
    const on_fp_argument< RelationT > on_fp(name, f);
    const on_integral_argument< RelationT > on_int(name, f);
    const on_string_argument< RelationT > on_str(name, f);

    const bool res = qi::parse
    (
        arg.c_str(), arg.c_str() + arg.size(),
        (
            real_[on_fp] |
            qi::long_[on_int] |
            qi::as< string_type >()[ +encoding_specific::print ][on_str]
        ) >> qi::eoi
    );

    if (!res)
        BOOST_LOG_THROW_DESCR(parse_error, "Failed to parse relation operand");

    return boost::move(f);
}

//  Explicitly instantiate factory implementation
#ifdef BOOST_LOG_USE_CHAR
template class default_filter_factory< char >;
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
template class default_filter_factory< wchar_t >;
#endif

} // namespace aux

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost
