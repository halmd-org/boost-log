/*
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * This header is the Boost.Log library implementation, see the library documentation
 * at http://www.boost.org/libs/log/doc/log.html.
 */
/*!
 * \file   attribute_value_extractor.hpp
 * \author Andrey Semashev
 * \date   01.03.2008
 * 
 * The header contains implementation of convenience tools to extract an attribute value
 * into a user-defined functor.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_UTILITY_ATTRIBUTE_VALUE_EXTRACTOR_HPP_INCLUDED_
#define BOOST_LOG_UTILITY_ATTRIBUTE_VALUE_EXTRACTOR_HPP_INCLUDED_

#include <string>
#include <boost/optional.hpp>
#include <boost/mpl/is_sequence.hpp>
#include <boost/utility/addressof.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/attributes/attribute_values_view.hpp>
#include <boost/log/utility/type_dispatch/static_type_dispatcher.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace aux {

//! Fixed type attribute value extractor
template< typename CharT, typename T >
class fixed_type_value_extractor
{
public:
    //! Function object result type
    typedef bool result_type;

    //! Character type
    typedef CharT char_type;
    //! String type
    typedef std::basic_string< char_type > string_type;
    //! Attribute values view type
    typedef basic_attribute_values_view< char_type > values_view_type;
    //! Attribute value type
    typedef T value_type;

private:
    //! Attribute name to extract
    string_type m_Name;

public:
    //! Constructor
    explicit fixed_type_value_extractor(string_type const& name) : m_Name(name) {}

    //! Extraction operator
    template< typename ReceiverT >
    result_type operator() (values_view_type const& attrs, ReceiverT& receiver) const
    {
        return extract(attrs, receiver);
    }
    //! Extraction operator
    template< typename ReceiverT >
    result_type operator() (values_view_type const& attrs, ReceiverT const& receiver) const
    {
        return extract(attrs, receiver);
    }

private:
    //! Implementation of the attribute value extraction
    template< typename ReceiverT >
    result_type extract(values_view_type const& attrs, ReceiverT& receiver) const
    {
        typename values_view_type::const_iterator it = attrs.find(m_Name);
        if (it != attrs.end())
        {
            optional< value_type const& > val = it->second->get< value_type >();
            if (!!val)
            {
                receiver(val.get());
                return true;
            }
        }
        return false;
    }
};

//! Attribute value extractor with type list support
template< typename CharT, typename TypeSequenceT >
class type_list_value_extractor
{
public:
    //! Function object result type
    typedef bool result_type;

    //! Character type
    typedef CharT char_type;
    //! String type
    typedef std::basic_string< char_type > string_type;
    //! Attribute values view type
    typedef basic_attribute_values_view< char_type > values_view_type;
    //! Attribute value types
    typedef TypeSequenceT value_types;

private:
    template< typename ReceiverT >
    struct visitor;
    template< typename ReceiverT >
    friend struct visitor;

    template< typename ReceiverT >
    class dispatcher;
    template< typename ReceiverT >
    friend class dispatcher;

    //! Visitor class definition
    template< typename ReceiverT >
    struct visitor
    {
        template< typename T >
        struct BOOST_LOG_NO_VTABLE apply :
            public type_visitor< T >
        {
            typedef apply< T > type;
            typedef typename type_visitor< T >::supported_type supported_type;

            //! Visitation (virtual)
            void visit(supported_type const& value)
            {
                BOOST_LOG_ASSUME(this != NULL);
                static_cast< dispatcher< ReceiverT >* >(this)->m_Receiver(value);
            }
        };
    };

    //! Type dispatcher class definition
    template< typename ReceiverT >
    class dispatcher :
        public static_type_dispatcher< value_types, visitor< ReceiverT > >
    {
    public:
        ReceiverT& m_Receiver;

        //! Dispatcher constructor
        explicit dispatcher(ReceiverT& receiver) : m_Receiver(receiver)
        {
        }
    };

private:
    //! Attribute name to extract
    string_type m_Name;

public:
    //! Constructor
    explicit type_list_value_extractor(string_type const& name) : m_Name(name) {}

    //! Extraction operator
    template< typename ReceiverT >
    result_type operator() (values_view_type const& attrs, ReceiverT& receiver) const
    {
        return extract(attrs, receiver);
    }
    //! Extraction operator
    template< typename ReceiverT >
    result_type operator() (values_view_type const& attrs, ReceiverT const& receiver) const
    {
        return extract(attrs, receiver);
    }

private:
    //! Implementation of the attribute value extraction
    template< typename ReceiverT >
    bool extract(values_view_type const& attrs, ReceiverT& receiver) const
    {
        typename values_view_type::const_iterator it = attrs.find(m_Name);
        if (it != attrs.end())
        {
            dispatcher< ReceiverT > disp(receiver);
            return it->second->dispatch(disp);
        }
        return false;
    }
};

} // namespace aux

//! Generic attribute value extractor
template< typename CharT, typename T >
class attribute_value_extractor :
    public mpl::if_<
        mpl::is_sequence< T >,
        log::aux::type_list_value_extractor< CharT, T >,
        log::aux::fixed_type_value_extractor< CharT, T >
    >::type
{
    //! Base type
    typedef typename mpl::if_<
        mpl::is_sequence< T >,
        log::aux::type_list_value_extractor< CharT, T >,
        log::aux::fixed_type_value_extractor< CharT, T >
    >::type base_type;

public:
    //! String type
    typedef typename base_type::string_type string_type;

public:
    //! Forwarding constructor
    explicit attribute_value_extractor(string_type const& name) : base_type(name) {}

#ifdef BOOST_LOG_DOXYGEN_PASS
    //! Function object result type
    typedef bool result_type;

    //! Character type
    typedef CharT char_type;
    //! String type
    typedef std::basic_string< char_type > string_type;
    //! Attribute values view type
    typedef basic_attribute_values_view< char_type > values_view_type;

    //! Extraction operator
    template< typename ReceiverT >
    result_type operator() (values_view_type const& attrs, ReceiverT& receiver) const;
#endif // BOOST_LOG_DOXYGEN_PASS
};

} // namespace log

} // namespace boost

#endif // BOOST_LOG_UTILITY_ATTRIBUTE_VALUE_EXTRACTOR_HPP_INCLUDED_
