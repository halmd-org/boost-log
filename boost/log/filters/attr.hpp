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

#ifndef BOOST_LOG_FILTERS_ATTR_HPP_INCLUDED_
#define BOOST_LOG_FILTERS_ATTR_HPP_INCLUDED_

#include <string>
#include <algorithm>
#include <functional>
#include <boost/regex.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/size_t.hpp>
#include <boost/mpl/begin.hpp>
#include <boost/mpl/end.hpp>
#include <boost/mpl/next.hpp>
#include <boost/mpl/deref.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/mpl/is_sequence.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/filters/basic_filters.hpp>
#include <boost/log/type_dispatch/static_type_dispatcher.hpp>
#include <boost/log/attributes/attribute_values_view.hpp>

namespace boost {

namespace log {

namespace filters {

//! A filter that checks that any of the specified attribute values
//! in the view obeys the specified functor (single supported type version)
template<
    typename CharT,
    typename FunT,
    typename AttributeValueT
>
class flt_attr_single :
    public type_dispatcher,
    public type_visitor< AttributeValueT >,
    public basic_filter< CharT, flt_attr_single< CharT, FunT, AttributeValueT > >
{
private:
    //! Base type
    typedef basic_filter< CharT, flt_attr_single< CharT, FunT, AttributeValueT > > base_type;
    //! Attribute values container type
    typedef typename base_type::attribute_values_view attribute_values_view;

public:
    //! Char type
    typedef typename base_type::char_type char_type;
    //! String type
    typedef std::basic_string< char_type > string_type;
    //! An attribute value type
    typedef AttributeValueT attribute_value_type;
    //! Checker functor type
    typedef FunT checker_type;

private:
    //! Attribute name
    string_type m_AttributeName;
    //! Attribute value checker
    checker_type m_Checker;
    //! Filtering result
    mutable bool m_Result;

public:
    flt_attr_single(string_type const& name, checker_type const& checker)
        : m_AttributeName(name), m_Checker(checker)
    {
    }

    bool operator() (attribute_values_view const& values) const
    {
        m_Result = false;

        // Find all values of the attribute
        typedef typename attribute_values_view::const_iterator attribute_values_iterator;
        std::pair< attribute_values_iterator, attribute_values_iterator > Values =
            values.equal_range(m_AttributeName);

        // Find the one of them which complies the checker functor
        for (; Values.first != Values.second && !m_Result; ++Values.first)
            Values.first->second->dispatch(*const_cast< flt_attr_single* >(this));

        return m_Result;
    }

private:
    //! The get_visitor method implementation
    void* get_visitor(std::type_info const& type)
    {
        if (type == typeid(attribute_value_type))
            return static_cast< type_visitor< attribute_value_type >* >(this);
        else
            return 0;
    }

    //! The method invokes the visitor-specific logic with the given value
    void visit(attribute_value_type const& value)
    {
        m_Result = m_Checker(value);
    }
};

namespace aux {

    //! The base class for attr filter generator
    template< typename CharT, typename AttributeValueT >
    class flt_attr_single_gen_base
    {
    protected:
        //! Char type
        typedef CharT char_type;
        //! String type
        typedef std::basic_string< char_type > string_type;
        //! Attribute values container type
        typedef basic_attribute_values_view< char_type > attribute_values_view;
        //! Size type
        typedef typename attribute_values_view::size_type size_type;
        //! Supported attribute value type
        typedef AttributeValueT attribute_value_type;

    protected:
        //! Attribute name
        string_type m_AttributeName;

    public:
        explicit flt_attr_single_gen_base(string_type const& name) : m_AttributeName(name) {}
        explicit flt_attr_single_gen_base(const char_type* name) : m_AttributeName(name) {}

#define BOOST_LOG_FILTER_ATTR_SINGLE_MEMBER(member, fun)\
        flt_attr_single<\
            char_type,\
            std::binder2nd< fun< attribute_value_type > >,\
            attribute_value_type\
        > member (attribute_value_type const& arg) const\
        {\
            typedef flt_attr_single<\
                char_type,\
                std::binder2nd< fun< attribute_value_type > >,\
                attribute_value_type\
            > flt_attr_t;\
            return flt_attr_t(this->m_AttributeName, std::bind2nd(fun< attribute_value_type >(), arg));\
        }

        BOOST_LOG_FILTER_ATTR_SINGLE_MEMBER(operator ==, std::equal_to)
        BOOST_LOG_FILTER_ATTR_SINGLE_MEMBER(operator !=, std::not_equal_to)
        BOOST_LOG_FILTER_ATTR_SINGLE_MEMBER(operator >, std::greater)
        BOOST_LOG_FILTER_ATTR_SINGLE_MEMBER(operator <, std::less)
        BOOST_LOG_FILTER_ATTR_SINGLE_MEMBER(operator >=, std::greater_equal)
        BOOST_LOG_FILTER_ATTR_SINGLE_MEMBER(operator <=, std::less_equal)
    };


    //! An attribute type dispatcher visitor
    template<
        template< typename > class FunT,
        typename AttributeValueTypesT,
        typename ItT = typename mpl::begin< AttributeValueTypesT >::type,
        typename EndT = typename mpl::end< AttributeValueTypesT >::type
    >
    class flt_attr_multiple_type_dispatcher :
        public flt_attr_multiple_type_dispatcher<
            FunT,
            AttributeValueTypesT,
            typename mpl::next< ItT >::type,
            EndT
        >
    {
        //! Base type
        typedef flt_attr_multiple_type_dispatcher<
            FunT,
            AttributeValueTypesT,
            typename mpl::next< ItT >::type,
            EndT
        > base_type;

    public:
        //! An attribute value type
        typedef typename mpl::deref< ItT >::type attribute_value_type;
        //! Attribute checker type
        typedef FunT< attribute_value_type > checker_type;

    private:
        //! Checker functor
        checker_type m_Checker;
        //! Bound argument
        attribute_value_type m_Arg;

    public:
        //! Constructor, binding the argument to the checker
        template< typename T >
        flt_attr_multiple_type_dispatcher(T const& arg)
            : base_type(arg), m_Arg(arg)
        {
        }

        //! The method invokes the visitor-specific logic with the given value
        void visit(attribute_value_type const& value)
        {
            this->m_Result = m_Checker(value, m_Arg);
        }
    };

    //! Attribute type dispatcher base type
    template<
        template< typename > class FunT,
        typename AttributeValueTypesT,
        typename EndT
    >
    class flt_attr_multiple_type_dispatcher< FunT, AttributeValueTypesT, EndT, EndT > :
        public static_type_dispatcher< AttributeValueTypesT >
    {
        mutable bool m_Result;

        template< typename T >
        flt_attr_multiple_type_dispatcher(T const&) {}
    };

} // namespace aux

//! A filter that checks that any of the specified attribute values
//! in the view obeys the specified functor (multiple supported types version)
template<
    typename CharT,
    template< typename > class FunT,
    typename AttributeValueTypesT
>
class flt_attr_multiple :
    public basic_filter< CharT, flt_attr_multiple< CharT, FunT, AttributeValueTypesT > >
{
private:
    //! Base type
    typedef basic_filter< CharT, flt_attr_multiple< CharT, FunT, AttributeValueTypesT > > base_type;
    //! Attribute values container type
    typedef typename base_type::attribute_values_view attribute_values_view;

public:
    //! Char type
    typedef typename base_type::char_type char_type;
    //! String type
    typedef std::basic_string< char_type > string_type;

private:
    //! An attribute type dispatcher type
    typedef aux::flt_attr_multiple_type_dispatcher<
        FunT,
        AttributeValueTypesT
    > dispatcher_type;

private:
    //! Attribute name
    string_type m_AttributeName;
    //! Attribute value types dispatcher
    dispatcher_type m_Dispatcher;

public:
    template< typename T >
    flt_attr_multiple(string_type const& name, T const& arg)
        : m_AttributeName(name), m_Dispatcher(arg)
    {
    }

    bool operator() (attribute_values_view const& values) const
    {
        bool& Result = m_Dispatcher.m_Result;
        Result = false;

        // Find all values of the attribute
        typedef typename attribute_values_view::const_iterator attribute_values_iterator;
        std::pair< attribute_values_iterator, attribute_values_iterator > Values =
            values.equal_range(m_AttributeName);

        // Find the one of them which complies the checker functor
        for (; Values.first != Values.second && !Result; ++Values.first)
            Values.first->second->dispatch(m_Dispatcher);

        return Result;
    }
};

namespace aux {

    //! The base class for attr filter generator
    template< typename CharT, typename AttributeValueTypesT >
    class flt_attr_multiple_gen_base
    {
    protected:
        //! Char type
        typedef CharT char_type;
        //! String type
        typedef std::basic_string< char_type > string_type;
        //! Attribute values container type
        typedef basic_attribute_values_view< char_type > attribute_values_view;
        //! Size type
        typedef typename attribute_values_view::size_type size_type;
        //! Supported attribute value types
        typedef AttributeValueTypesT attribute_value_types;

    protected:
        //! Attribute name
        string_type m_AttributeName;

    public:
        explicit flt_attr_multiple_gen_base(string_type const& name) : m_AttributeName(name) {}
        explicit flt_attr_multiple_gen_base(const char_type* name) : m_AttributeName(name) {}

#define BOOST_LOG_FILTER_ATTR_MULTIPLE_MEMBER(member, fun)\
        template< typename T >\
        flt_attr_multiple<\
            char_type,\
            fun,\
            attribute_value_types\
        > member (T const& arg) const\
        {\
            typedef flt_attr_multiple<\
                char_type,\
                fun,\
                attribute_value_types\
            > flt_attr_t;\
            return flt_attr_t(this->m_AttributeName, arg);\
        }

        BOOST_LOG_FILTER_ATTR_MULTIPLE_MEMBER(operator ==, std::equal_to)
        BOOST_LOG_FILTER_ATTR_MULTIPLE_MEMBER(operator !=, std::not_equal_to)
        BOOST_LOG_FILTER_ATTR_MULTIPLE_MEMBER(operator >, std::greater)
        BOOST_LOG_FILTER_ATTR_MULTIPLE_MEMBER(operator <, std::less)
        BOOST_LOG_FILTER_ATTR_MULTIPLE_MEMBER(operator >=, std::greater_equal)
        BOOST_LOG_FILTER_ATTR_MULTIPLE_MEMBER(operator <=, std::less_equal)

#undef BOOST_LOG_FILTER_ATTR_MULTIPLE_MEMBER

    };


    //! A helper base class selector to optimize away MPL sequences of size = 1
    template< typename CharT, typename AttributeValueTypesT >
    struct make_flt_attr_gen_base :
        public mpl::if_c<
            mpl::size< AttributeValueTypesT >::value == 1,
            mpl::identity<
                flt_attr_single_gen_base<
                    CharT,
                    typename mpl::front< AttributeValueTypesT >::type
                >
            >,
            mpl::identity<
                flt_attr_multiple_gen_base<
                    CharT,
                    AttributeValueTypesT
                >
            >
        >::type
    {
    };


    //! The attr filter generator (multiple attribute value types supported)
    template<
        typename CharT,
        typename AttributeValueTypesT,
        bool IsSequenceV = mpl::is_sequence< AttributeValueTypesT >::value
    >
    class flt_attr_gen :
        public make_flt_attr_gen_base< CharT, AttributeValueTypesT >::type
    {
        //! Base type
        typedef typename make_flt_attr_gen_base<
            CharT,
            AttributeValueTypesT
        >::type base_type;
        //! Char type
        typedef typename base_type::char_type char_type;
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        explicit flt_attr_gen(string_type const& name) : base_type(name) {}
        explicit flt_attr_gen(const char_type* name) : base_type(name) {}
    };

    //! The attr filter generator (single attribute value type supported)
    template<
        typename CharT,
        typename AttributeValueTypesT
    >
    class flt_attr_gen< CharT, AttributeValueTypesT, false > :
        public flt_attr_single_gen_base< CharT, AttributeValueTypesT >
    {
        //! Base type
        typedef flt_attr_single_gen_base< CharT, AttributeValueTypesT > base_type;
        //! Char type
        typedef typename base_type::char_type char_type;
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        explicit flt_attr_gen(string_type const& name) : base_type(name) {}
        explicit flt_attr_gen(const char_type* name) : base_type(name) {}
    };


    //! The begins_with functor
    template< typename T >
    struct begins_with_fun :
        public std::binary_function< T, T, bool >
    {
        bool operator() (T const& left, T const& right) const
        {
            if (left.size() >= right.size())
                return std::equal(right.begin(), right.end(), left.begin());
            else
                return false;
        }
    };

    //! The ends_with functor
    template< typename T >
    struct ends_with_fun :
        public std::binary_function< T, T, bool >
    {
        bool operator() (T const& left, T const& right) const
        {
            if (left.size() >= right.size())
                return std::equal(right.begin(), right.end(), left.end() - right.size());
            else
                return false;
        }
    };

    //! The contains functor
    template< typename T >
    struct contains_fun :
        public std::binary_function< T, T, bool >
    {
        bool operator() (T const& left, T const& right) const
        {
            if (left.size() >= right.size())
            {
                bool Result = false;
                typename T::const_iterator search_end = left.end() - right.size();
                for (typename T::const_iterator it = left.begin(); it != search_end && !Result; ++it)
                    Result = std::equal(right.begin(), right.end(), it);

                return Result;
            }
            else
                return false;
        }
    };

    //! The matches functor
    template< typename CharT, typename TraitsT >
    struct matches_fun :
        public std::unary_function< std::basic_string< CharT >, bool >
    {
    private:
        basic_regex< CharT, TraitsT > m_RegEx;
        match_flag_type m_Flags;

    public:
        matches_fun(basic_regex< CharT, TraitsT > const& expr, match_flag_type flags)
            : m_RegEx(expr), m_Flags(flags)
        {
        }

        bool operator() (std::basic_string< CharT > const& str) const
        {
            return regex_match(str, m_RegEx, m_Flags);
        }
    };

    //! The attr filter generator specialization for case when the attribute is required to be a string
    template< typename CharT >
    class flt_attr_gen< CharT, std::basic_string< CharT >, false > :
        public flt_attr_single_gen_base< CharT, std::basic_string< CharT > >
    {
        //! Base type
        typedef flt_attr_single_gen_base< CharT, std::basic_string< CharT > > base_type;

    private:
        //! Char type
        typedef typename base_type::char_type char_type;
        //! String type
        typedef typename base_type::string_type string_type;
        //! Attribute value types
        typedef typename base_type::attribute_value_type attribute_value_type;

    public:
        explicit flt_attr_gen(string_type const& name) : base_type(name) {}
        explicit flt_attr_gen(const char_type* name) : base_type(name) {}

        BOOST_LOG_FILTER_ATTR_SINGLE_MEMBER(begins_with, begins_with_fun)
        BOOST_LOG_FILTER_ATTR_SINGLE_MEMBER(ends_with, ends_with_fun)
        BOOST_LOG_FILTER_ATTR_SINGLE_MEMBER(contains, contains_fun)

        //! The method allows to apply RegEx to a string attribute value
        template< typename TraitsT >
        flt_attr_single<
            char_type,
            matches_fun< char_type, TraitsT >,
            attribute_value_type
        > matches(basic_regex< char_type, TraitsT > const& expr, match_flag_type flags = match_default)
        {
            typedef flt_attr_single<
                char_type,
                matches_fun< char_type, TraitsT >,
                attribute_value_type
            > flt_attr_t;
            return flt_attr_t(this->m_AttributeName, matches_fun< char_type, TraitsT >(expr, flags));
        }
    };

#undef BOOST_LOG_FILTER_ATTR_SINGLE_MEMBER

} // namespace aux

//! Filter generator
template< typename AttributeValueT, typename CharT >
aux::flt_attr_gen< CharT, AttributeValueT > attr(
    const CharT* name BOOST_APPEND_EXPLICIT_TEMPLATE_TYPE(AttributeValueT))
{
    return aux::flt_attr_gen< CharT, AttributeValueT >(name);
}

//! Filter generator
template< typename AttributeValueT, typename CharT >
aux::flt_attr_gen< CharT, AttributeValueT > attr(
    std::basic_string< CharT > const& name BOOST_APPEND_EXPLICIT_TEMPLATE_TYPE(AttributeValueT))
{
    return aux::flt_attr_gen< CharT, AttributeValueT >(name);
}

} // namespace filters

} // namespace log

} // namespace boost

#endif // BOOST_LOG_FILTERS_ATTR_HPP_INCLUDED_
