/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   attr.hpp
 * \author Andrey Semashev
 * \date   22.04.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_FORMATTERS_ATTR_HPP_INCLUDED_
#define BOOST_LOG_FORMATTERS_ATTR_HPP_INCLUDED_

#include <string>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/begin.hpp>
#include <boost/mpl/end.hpp>
#include <boost/mpl/next.hpp>
#include <boost/mpl/is_sequence.hpp>
#include <boost/mpl/vector/vector10.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/formatters/basic_formatters.hpp>
#include <boost/log/type_dispatch/standard_types.hpp>
#include <boost/log/type_dispatch/static_type_dispatcher.hpp>

namespace boost {

namespace log {

namespace formatters {

namespace aux {

    //! An attribute type dispatcher visitor
    template<
        typename OstreamT,
        typename AttributeValueTypesT,
        typename ItT = typename mpl::begin< AttributeValueTypesT >::type,
        typename EndT = typename mpl::end< AttributeValueTypesT >::type
    >
    struct fmt_attribute_simple_type_dispatcher :
        public fmt_attribute_simple_type_dispatcher<
            OstreamT,
            AttributeValueTypesT,
            typename mpl::next< ItT >::type,
            EndT
        >
    {
        //! An attribute value type
        typedef typename mpl::deref< ItT >::type attribute_value_type;

        //! The method invokes the visitor-specific logic with the given value
        void visit(attribute_value_type const& value)
        {
            (*(this->m_pStream)) << value;
        }
    };

    //! Attribute type dispatcher base type
    template<
        typename OstreamT,
        typename AttributeValueTypesT,
        typename EndT
    >
    struct fmt_attribute_simple_type_dispatcher< OstreamT, AttributeValueTypesT, EndT, EndT > :
        public static_type_dispatcher< AttributeValueTypesT >
    {
        OstreamT* m_pStream;

        fmt_attribute_simple_type_dispatcher() : m_pStream(0) {}
    };

} // namespace aux

//! Simple attribute formatter
template<
    typename CharT,
    typename AttributeValueTypesT = typename make_default_attribute_types< CharT >::type
>
class fmt_attribute_simple :
    public basic_formatter< CharT, fmt_attribute_simple< CharT, AttributeValueTypesT > >
{
private:
    //! Base type
    typedef basic_formatter< CharT, fmt_attribute_simple< CharT, AttributeValueTypesT > > base_type;

public:
    //! Char type
    typedef typename base_type::char_type char_type;
    //! String type
    typedef typename base_type::string_type string_type;
    //! Stream type
    typedef typename base_type::ostream_type ostream_type;
    //! Attribute values set type
    typedef typename base_type::attribute_values_view attribute_values_view;

private:
    //! An attribute type dispatcher type
    typedef aux::fmt_attribute_simple_type_dispatcher<
        ostream_type,
        AttributeValueTypesT
    > dispatcher_type;

private:
    //! Attribute name
    string_type m_AttributeName;
    //! Attribute type dispatcher
    mutable dispatcher_type m_Dispatcher;

public:
    //! Constructor
    explicit fmt_attribute_simple(const char_type* name) : m_AttributeName(name) {}
    //! Constructor
    explicit fmt_attribute_simple(string_type const& name) : m_AttributeName(name) {}

    //! Output operator
    void operator() (ostream_type& strm, attribute_values_view const& attrs, string_type const&) const
    {
        std::pair<
            typename attribute_values_view::const_iterator,
            typename attribute_values_view::const_iterator
        > values = attrs.equal_range(m_AttributeName);
        if (values.first != values.second)
        {
            m_Dispatcher.m_pStream = &strm;
            values.first->second->dispatch(m_Dispatcher);

            for (++values.first; values.first != values.second; ++values.first)
            {
                strm << ", ";
                values.first->second->dispatch(m_Dispatcher);
            }
        }
    }
};

//! Formatter generator
inline fmt_attribute_simple< char > attr(const char* name)
{
    return fmt_attribute_simple< char >(name);
}
//! Formatter generator
inline fmt_attribute_simple< char > attr(std::basic_string< char > const& name)
{
    return fmt_attribute_simple< char >(name);
}
//! Formatter generator
inline fmt_attribute_simple< wchar_t > attr(const wchar_t* name)
{
    return fmt_attribute_simple< wchar_t >(name);
}
//! Formatter generator
inline fmt_attribute_simple< wchar_t > attr(std::basic_string< wchar_t > const& name)
{
    return fmt_attribute_simple< wchar_t >(name);
}

//! Formatter generator with ability to specify an exact attribute value type(s)
template< typename AttributeValueTypesT, typename CharT >
inline fmt_attribute_simple<
    CharT,
    typename mpl::eval_if<
        mpl::is_sequence< AttributeValueTypesT >,
        AttributeValueTypesT,
        mpl::vector1< AttributeValueTypesT >
    >::type
> attr(const CharT* name)
{
    typedef typename mpl::eval_if<
        mpl::is_sequence< AttributeValueTypesT >,
        AttributeValueTypesT,
        mpl::vector1< AttributeValueTypesT >
    >::type types;

    return fmt_attribute_simple< CharT, types >(name);
}
//! Formatter generator with ability to specify an exact attribute value type(s)
template< typename AttributeValueTypesT, typename CharT >
inline fmt_attribute_simple<
    CharT,
    typename mpl::eval_if<
        mpl::is_sequence< AttributeValueTypesT >,
        AttributeValueTypesT,
        mpl::vector1< AttributeValueTypesT >
    >::type
> attr(std::basic_string< CharT > const& name)
{
    typedef typename mpl::eval_if<
        mpl::is_sequence< AttributeValueTypesT >,
        AttributeValueTypesT,
        mpl::vector1< AttributeValueTypesT >
    >::type types;

    return fmt_attribute_simple< CharT, types >(name);
}

} // namespace formatters

} // namespace log

} // namespace boost

#endif // BOOST_LOG_FORMATTERS_ATTR_HPP_INCLUDED_
