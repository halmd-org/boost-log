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
#include <boost/mpl/if.hpp>
#include <boost/mpl/bool.hpp>
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
    fmt_chain(LeftFmtT const& left, RightFmtT const& right) : m_Left(left), m_Right(right) {}

    //! Output operator
    void operator() (ostream_type& strm, attribute_values_view const& attrs, string_type const& msg) const
    {
        m_Left(strm, attrs, msg);
        m_Right(strm, attrs, msg);
    }
};

//! Formatter wrapper to output objects into streams
template< typename CharT, typename T >
class fmt_wrap :
    public basic_formatter< CharT, fmt_wrap< CharT, T > >
{
private:
    //! Base type
    typedef basic_formatter< CharT, fmt_wrap< CharT, T > > base_type;

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
    explicit fmt_wrap(T const& obj) : m_T(obj) {}

    //! Output operator
    void operator() (ostream_type& strm, attribute_values_view const&, string_type const&) const
    {
        strm << m_T;
    }
};

namespace aux {

    template< typename LeftFmtT, typename RightFmtT >
    inline fmt_chain< LeftFmtT, RightFmtT > make_fmt_chain(
        LeftFmtT const& left, RightFmtT const& right, mpl::true_ const&)
    {
        return fmt_chain< LeftFmtT, RightFmtT >(left, right);
    }
    template< typename LeftFmtT, typename RightT >
    inline fmt_chain<
        LeftFmtT,
        fmt_wrap< typename LeftFmtT::char_type, RightT >
    > make_fmt_chain(LeftFmtT const& left, RightT const& right, mpl::false_ const&)
    {
        return fmt_chain<
            LeftFmtT,
            fmt_wrap< typename LeftFmtT::char_type, RightT >
        >(left, fmt_wrap< typename LeftFmtT::char_type, RightT >(right));
    }

} // namespace aux

//! An ADL-reachable operator to generate fmt_chain
template< typename CharT, typename LeftFmtT, typename RightT >
inline typename mpl::if_<
    is_formatter< RightT >,
    fmt_chain< LeftFmtT, RightT >,
    fmt_chain<
        LeftFmtT,
        fmt_wrap< typename LeftFmtT::char_type, RightT >
    >
>::type operator<< (basic_formatter< CharT, LeftFmtT > const& left, RightT const& right)
{
    return aux::make_fmt_chain(
        static_cast< LeftFmtT const& >(left),
        right,
        typename is_formatter< RightT >::type());
}

//! A helper operator to automatically generate fmt_wrap from string literals
template< typename FmtT, typename CharT >
inline fmt_chain<
    FmtT,
    fmt_wrap<
        CharT,
        std::basic_string< CharT >
    >
> operator<< (basic_formatter< CharT, FmtT > const& left, const CharT* str)
{
    return fmt_chain<
        FmtT,
        fmt_wrap<
            CharT,
            std::basic_string< CharT >
        >
    >(static_cast< FmtT const& >(left), fmt_wrap< CharT, std::basic_string< CharT > >(str));
}
//! A helper operator to automatically generate fmt_wrap from STL strings
template< typename FmtT, typename CharT >
inline fmt_chain<
    FmtT,
    fmt_wrap<
        CharT,
        std::basic_string< CharT >
    >
> operator<< (basic_formatter< CharT, FmtT > const& left, std::basic_string< CharT > const& str)
{
    return fmt_chain<
        FmtT,
        fmt_wrap<
            CharT,
            std::basic_string< CharT >
        >
    >(static_cast< FmtT const& >(left), fmt_wrap< CharT, std::basic_string< CharT > >(str));
}

//! Formatter generator
template< typename CharT >
inline fmt_wrap< CharT, std::basic_string< CharT > > string_constant(const CharT* str)
{
    return fmt_wrap< CharT, std::basic_string< CharT > >(str);
}
//! Formatter generator
template< typename CharT >
inline fmt_wrap< CharT, std::basic_string< CharT > > string_constant(std::basic_string< CharT > const& str)
{
    return fmt_wrap< CharT, std::basic_string< CharT > >(str);
}

//! Formatter generator
template< typename CharT, typename T >
inline fmt_wrap< CharT, T > wrap(T const& obj BOOST_APPEND_EXPLICIT_TEMPLATE_TYPE(CharT))
{
    return fmt_wrap< CharT, T >(obj);
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
