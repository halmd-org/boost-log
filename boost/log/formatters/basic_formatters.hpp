/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   basic_formatters.hpp
 * \author Andrey Semashev
 * \date   22.04.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_FORMATTERS_BASIC_FORMATTERS_HPP_INCLUDED_
#define BOOST_LOG_FORMATTERS_BASIC_FORMATTERS_HPP_INCLUDED_

#include <iosfwd>
#include <string>
#include <boost/static_assert.hpp>
#include <boost/mpl/not.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/is_base_and_derived.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/attributes/attribute_values_view.hpp>

namespace boost {

namespace log {

namespace formatters {

//! A base class for every formatter
struct formatter_base {};
//! A template metafunction to detect formatters
template< typename T >
struct is_formatter : public is_base_and_derived< formatter_base, T > {};

//! A base class for all formatters
template< typename CharT, typename DerivedT >
struct basic_formatter : public formatter_base
{
    //! Char type
    typedef CharT char_type;
    //! String type
    typedef std::basic_string< char_type > string_type;
    //! Stream type
    typedef std::basic_ostream< char_type > ostream_type;
    //! Attribute values set type
    typedef basic_attribute_values_view< char_type > attribute_values_view;

    //! Functor result type
    typedef void result_type;
};

//! Formatter fmt_wrapper to output objects into streams
template< typename CharT, typename T >
class fmt_wrapper :
    public basic_formatter< CharT, fmt_wrapper< CharT, T > >
{
private:
    //! Base type
    typedef basic_formatter< CharT, fmt_wrapper< CharT, T > > base_type;

public:
    //! String type
    typedef typename base_type::string_type string_type;
    //! Stream type
    typedef typename base_type::ostream_type ostream_type;
    //! Attribute values set type
    typedef typename base_type::attribute_values_view attribute_values_view;

private:
    //! Object to be output
    T m_T;

public:
    //! Constructor
    explicit fmt_wrapper(T const& obj) : m_T(obj) {}

    //! Output operator
    void operator() (ostream_type& strm, attribute_values_view const&, string_type const&) const
    {
        strm << m_T;
    }
};

//! A convenience class that conditionally wrapst the type into a formatter
template< typename CharT, typename T, bool >
struct wrap_if_c
{
    typedef fmt_wrapper< CharT, T > type;
};

template< typename CharT, typename T >
struct wrap_if_c< CharT, T, false >
{
    typedef T type;
};

template< typename CharT, typename T, typename PredT >
struct wrap_if : public wrap_if_c< CharT, T, PredT::value >
{
};

template< typename CharT, typename T >
struct wrap_if_not_formatter : public wrap_if< CharT, T, mpl::not_< is_formatter< T > > >
{
};

//! A placeholder class to represent a stream in formatters lambda expressions
template< typename CharT >
struct stream_placeholder
{
    //! Trap operator to begin building the lambda expression
    template< typename T >
    typename wrap_if_not_formatter< CharT, T >::type operator<< (T const& fmt) const
    {
        typedef typename wrap_if_not_formatter< CharT, T >::type result_type;
        return result_type(fmt);
    }

    //! C-style strings need a special treatment
    fmt_wrapper< CharT, std::basic_string< CharT > > operator<< (const CharT* s) const
    {
        return fmt_wrapper< CharT, std::basic_string< CharT > >(s);
    }
};

//  Placeholders to begin lambda expresions
const stream_placeholder< char > ostrm = {};
const stream_placeholder< wchar_t > wostrm = {};


//! A formatter compound that encapsulates two other formatters
template< typename LeftFmtT, typename RightFmtT >
class fmt_chain :
    public basic_formatter< typename LeftFmtT::char_type, fmt_chain< LeftFmtT, RightFmtT > >
{
private:
    //! Base type
    typedef basic_formatter<
        typename LeftFmtT::char_type,
        fmt_chain< LeftFmtT, RightFmtT >
    > base_type;

public:
    //! Char type
    typedef typename base_type::char_type char_type;
    BOOST_STATIC_ASSERT((is_same< char_type, typename RightFmtT::char_type >::value));
    //! String type
    typedef typename base_type::string_type string_type;
    //! Stream type
    typedef typename base_type::ostream_type ostream_type;
    //! Attribute values set type
    typedef typename base_type::attribute_values_view attribute_values_view;

private:
    //! Left formatter
    LeftFmtT m_Left;
    //! Right formatter
    RightFmtT m_Right;

public:
    //! Constructor
    template< typename RightT >
    fmt_chain(LeftFmtT const& left, RightT const& right) : m_Left(left), m_Right(right) {}

    //! Output operator
    void operator() (ostream_type& strm, attribute_values_view const& attrs, string_type const& msg) const
    {
        m_Left(strm, attrs, msg);
        m_Right(strm, attrs, msg);
    }
};

//! An ADL-reachable operator to generate fmt_chain
template< typename CharT, typename LeftFmtT, typename RightT >
inline fmt_chain<
    LeftFmtT,
    typename wrap_if_not_formatter< CharT, RightT >::type
> operator<< (basic_formatter< CharT, LeftFmtT > const& left, RightT const& right)
{
    typedef fmt_chain<
        LeftFmtT,
        typename wrap_if_not_formatter< CharT, RightT >::type
    > result_type;
    return result_type(static_cast< LeftFmtT const& >(left), right);
}

//! A helper operator to automatically generate fmt_wrapper from C-strings
template< typename FmtT, typename CharT >
inline fmt_chain<
    FmtT,
    fmt_wrapper<
        CharT,
        std::basic_string< CharT >
    >
> operator<< (basic_formatter< CharT, FmtT > const& left, const CharT* str)
{
    return fmt_chain<
        FmtT,
        fmt_wrapper<
            CharT,
            std::basic_string< CharT >
        >
    >(static_cast< FmtT const& >(left), fmt_wrapper< CharT, std::basic_string< CharT > >(str));
}

//! Message formatter class
template< typename CharT >
class fmt_message :
    public basic_formatter< CharT, fmt_message< CharT > >
{
private:
    //! Base type
    typedef basic_formatter< CharT, fmt_message< CharT > > base_type;

public:
    //! String type
    typedef typename base_type::string_type string_type;
    //! Stream type
    typedef typename base_type::ostream_type ostream_type;
    //! Attribute values set type
    typedef typename base_type::attribute_values_view attribute_values_view;

public:
    //! Output operator
    void operator() (ostream_type& strm, attribute_values_view const&, string_type const& msg) const
    {
        strm << msg;
    }
};

//! Formatter generator
inline fmt_message< char > message()
{
    return fmt_message< char >();
}
//! Formatter generator
inline fmt_message< wchar_t > wmessage()
{
    return fmt_message< wchar_t >();
}

} // namespace formatters

} // namespace log

} // namespace boost

#endif // BOOST_LOG_FORMATTERS_BASIC_FORMATTERS_HPP_INCLUDED_
