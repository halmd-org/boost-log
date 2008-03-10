/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   named_scope.hpp
 * \author Andrey Semashev
 * \date   26.11.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_FORMATTERS_NAMED_SCOPE_HPP_INCLUDED_
#define BOOST_LOG_FORMATTERS_NAMED_SCOPE_HPP_INCLUDED_

#include <limits>
#include <string>
#include <ostream>
#include <iterator>
#include <algorithm>
#include <boost/parameter/keyword.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/detail/attachable_sstream_buf.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/formatters/basic_formatters.hpp>
#include <boost/log/type_dispatch/type_dispatcher.hpp>

namespace boost {

namespace log {

namespace formatters {

namespace keywords {

    BOOST_PARAMETER_KEYWORD(tag, scope_delimiter)
    BOOST_PARAMETER_KEYWORD(tag, scope_depth)
    BOOST_PARAMETER_KEYWORD(tag, scope_iteration)

    //! Scope iteration directions
    enum scope_iteration_values
    {
        forward,
        reverse
    };

} // namespace keywords

//! Named scope attribute formatter
template< typename CharT >
class fmt_named_scope :
    public basic_formatter< CharT, fmt_named_scope< CharT > >
{
private:
    //! Base type
    typedef basic_formatter< CharT, fmt_named_scope< CharT > > base_type;

public:
    //! Char type
    typedef typename base_type::char_type char_type;
    //! String type
    typedef typename base_type::string_type string_type;
    //! Stream type
    typedef typename base_type::ostream_type ostream_type;
    //! Boost.Format object type
    typedef typename base_type::format_type format_type;
    //! Attribute values set type
    typedef typename base_type::attribute_values_view attribute_values_view;

private:
    //! Scope stack container type
    typedef typename attributes::basic_named_scope< char_type >::scope_stack scope_stack;

private:
    //! Attribute name
    const string_type m_AttributeName;
    //! Scope delimiter
    const string_type m_ScopeDelimiter;
    //! Number of scopes to output
    const typename scope_stack::size_type m_MaxScopes;
    //! Scope iteration direction
    const keywords::scope_iteration_values m_IterationDirection;

public:
    //! Constructor
    template< typename T1, typename T2 >
    fmt_named_scope(T1 const& name, T2 const& delimiter, typename scope_stack::size_type max_scopes, keywords::scope_iteration_values direction)
        : m_AttributeName(name), m_ScopeDelimiter(delimiter), m_MaxScopes(max_scopes), m_IterationDirection(direction)
    {
    }

    //! Output operator
    void operator() (ostream_type& strm, attribute_values_view const& attrs, string_type const&) const
    {
        typename attribute_values_view::const_iterator it = attrs.find(m_AttributeName);
        if (it != attrs.end())
        {
            optional< scope_stack const& > maybe_scopes = it->second->get< scope_stack >();
            if (!!maybe_scopes)
            {
                // Found the attribute value
                scope_stack const& scopes = maybe_scopes.get();
                typename scope_stack::size_type const scopes_to_iterate = (std::min)(m_MaxScopes, scopes.size());
                if (m_IterationDirection == keywords::forward)
                {
                    // Iterating through scopes in forward direction
                    typename scope_stack::const_iterator it = scopes.end(), end = it;
                    std::advance(it, -static_cast< typename scope_stack::difference_type >(scopes_to_iterate));

                    if (it != end)
                    {
                        if (it != scopes.begin())
                            strm << "..." << m_ScopeDelimiter;

                        strm << it->scope_name;
                        for (++it; it != end; ++it)
                            strm << m_ScopeDelimiter << it->scope_name;
                    }
                }
                else
                {
                    // Iterating through scopes in reverse direction
                    typename scope_stack::const_reverse_iterator it = scopes.rbegin(), end = it;
                    std::advance(end, static_cast< typename scope_stack::difference_type >(scopes_to_iterate));

                    if (it != end)
                    {
                        strm << it->scope_name;
                        for (++it; it != end; ++it)
                            strm << m_ScopeDelimiter << it->scope_name;

                        if (it != scopes.rend())
                            strm << m_ScopeDelimiter << "...";
                    }
                }
            }
        }
    }
    //! Format operator
    void operator() (format_type& fmt, attribute_values_view const& attrs, string_type const& msg) const
    {
        string_type str;
        boost::log::aux::basic_ostringstreambuf< char_type > buf(str);
        std::basic_ostream< char_type > strm(&buf);
        (*this)(strm, attrs, msg);
        if (strm.good())
        {
            strm.flush();
            fmt % str;
        }
    }
};

namespace aux {

    //! Auxiliary traits to acquire correct default delimiter depending on the character type
    template< typename CharT >
    struct default_scope_delimiter;
    template< >
    struct default_scope_delimiter< char >
    {
        static const char* forward() { return "->"; }
        static const char* reverse() { return "<-"; }
    };
    template< >
    struct default_scope_delimiter< wchar_t >
    {
        static const wchar_t* forward() { return L"->"; }
        static const wchar_t* reverse() { return L"<-"; }
    };

    //! Auxiliary function to construct formatter from the complete set of arguments
    template< typename CharT, typename ArgsT >
    fmt_named_scope< CharT > named_scope(const CharT* name, ArgsT const& args)
    {
        typedef fmt_named_scope< CharT > fmt_named_scope_t;

        keywords::scope_iteration_values direction = args[keywords::scope_iteration | keywords::forward];
        const CharT* default_delimiter = (direction == keywords::forward ? default_scope_delimiter< CharT >::forward() : default_scope_delimiter< CharT >::reverse());

        return fmt_named_scope_t(
            name,
            args[keywords::scope_delimiter | default_delimiter],
            args[keywords::scope_depth | (std::numeric_limits< std::size_t >::max)()],
            direction);
    }
    //! Auxiliary function to construct formatter from the complete set of arguments
    template< typename CharT, typename ArgsT >
    fmt_named_scope< CharT > named_scope(std::basic_string< CharT > const& name, ArgsT const& args)
    {
        typedef fmt_named_scope< CharT > fmt_named_scope_t;

        keywords::scope_iteration_values direction = args[keywords::scope_iteration | keywords::forward];
        const CharT* default_delimiter = (direction == keywords::forward ? default_scope_delimiter< CharT >::forward() : default_scope_delimiter< CharT >::reverse());

        return fmt_named_scope_t(
            name,
            args[keywords::scope_delimiter | default_delimiter],
            args[keywords::scope_depth | (std::numeric_limits< std::size_t >::max)()],
            direction);
    }

} // namespace aux

//! Formatter generator
inline fmt_named_scope< char > named_scope(const char* name)
{
    return fmt_named_scope< char >(name, "->", (std::numeric_limits< std::size_t >::max)(), keywords::forward);
}
//! Formatter generator
inline fmt_named_scope< char > named_scope(std::basic_string< char > const& name)
{
    return fmt_named_scope< char >(name, "->", (std::numeric_limits< std::size_t >::max)(), keywords::forward);
}
//! Formatter generator
inline fmt_named_scope< wchar_t > named_scope(const wchar_t* name)
{
    return fmt_named_scope< wchar_t >(name, L"->", (std::numeric_limits< std::size_t >::max)(), keywords::forward);
}
//! Formatter generator
inline fmt_named_scope< wchar_t > named_scope(std::basic_string< wchar_t > const& name)
{
    return fmt_named_scope< wchar_t >(name, L"->", (std::numeric_limits< std::size_t >::max)(), keywords::forward);
}


//! Formatter generator
template< typename T1 >
inline fmt_named_scope< char > named_scope(const char* name, T1 const& arg1)
{
    return aux::named_scope(name, arg1);
}
//! Formatter generator
template< typename T1 >
inline fmt_named_scope< char > named_scope(std::basic_string< char > const& name, T1 const& arg1)
{
    return aux::named_scope(name, arg1);
}
//! Formatter generator
template< typename T1 >
inline fmt_named_scope< wchar_t > named_scope(const wchar_t* name, T1 const& arg1)
{
    return aux::named_scope(name, arg1);
}
//! Formatter generator
template< typename T1 >
inline fmt_named_scope< wchar_t > named_scope(std::basic_string< wchar_t > const& name, T1 const& arg1)
{
    return aux::named_scope(name, arg1);
}

//! Formatter generator
template< typename T1, typename T2 >
inline fmt_named_scope< char > named_scope(const char* name, T1 const& arg1, T2 const& arg2)
{
    return aux::named_scope(name, (arg1, arg2));
}
//! Formatter generator
template< typename T1, typename T2 >
inline fmt_named_scope< char > named_scope(std::basic_string< char > const& name, T1 const& arg1, T2 const& arg2)
{
    return aux::named_scope(name, (arg1, arg2));
}
//! Formatter generator
template< typename T1, typename T2 >
inline fmt_named_scope< wchar_t > named_scope(const wchar_t* name, T1 const& arg1, T2 const& arg2)
{
    return aux::named_scope(name, (arg1, arg2));
}
//! Formatter generator
template< typename T1, typename T2 >
inline fmt_named_scope< wchar_t > named_scope(std::basic_string< wchar_t > const& name, T1 const& arg1, T2 const& arg2)
{
    return aux::named_scope(name, (arg1, arg2));
}

//! Formatter generator
template< typename T1, typename T2, typename T3 >
inline fmt_named_scope< char > named_scope(const char* name, T1 const& arg1, T2 const& arg2, T3 const& arg3)
{
    return aux::named_scope(name, (arg1, arg2, arg3));
}
//! Formatter generator
template< typename T1, typename T2, typename T3 >
inline fmt_named_scope< char > named_scope(std::basic_string< char > const& name, T1 const& arg1, T2 const& arg2, T3 const& arg3)
{
    return aux::named_scope(name, (arg1, arg2, arg3));
}
//! Formatter generator
template< typename T1, typename T2, typename T3 >
inline fmt_named_scope< wchar_t > named_scope(const wchar_t* name, T1 const& arg1, T2 const& arg2, T3 const& arg3)
{
    return aux::named_scope(name, (arg1, arg2, arg3));
}
//! Formatter generator
template< typename T1, typename T2, typename T3 >
inline fmt_named_scope< wchar_t > named_scope(std::basic_string< wchar_t > const& name, T1 const& arg1, T2 const& arg2, T3 const& arg3)
{
    return aux::named_scope(name, (arg1, arg2, arg3));
}

} // namespace formatters

} // namespace log

} // namespace boost

#endif // BOOST_LOG_FORMATTERS_NAMED_SCOPE_HPP_INCLUDED_
