/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   count_of.hpp
 * \author Andrey Semashev
 * \date   22.04.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_FILTERS_COUNT_OF_HPP_INCLUDED_
#define BOOST_LOG_FILTERS_COUNT_OF_HPP_INCLUDED_

#include <string>
#include <functional>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/filters/basic_filters.hpp>

namespace boost {

namespace log {

namespace filters {

//! A filter that checks that the number of the specified attribute values in the view obeys the specified functor
template< typename CharT, typename FunT >
class flt_count_of :
    public basic_filter< CharT, flt_count_of< CharT, FunT > >
{
private:
    //! Base type
    typedef basic_filter< CharT, flt_count_of< CharT, FunT > > base_type;
    //! Attribute values container type
    typedef typename base_type::attribute_values_view attribute_values_view;
    //! Char type
    typedef typename base_type::char_type char_type;
    //! String type
    typedef std::basic_string< char_type > string_type;

private:
    //! Attribute name
    string_type m_AttributeName;
    //! Attribute count checker
    FunT m_Checker;

public:
    flt_count_of(string_type const& name, FunT const& checker)
        : m_AttributeName(name), m_Checker(checker)
    {
    }

    bool operator() (attribute_values_view const& values) const
    {
        return m_Checker(values.count(m_AttributeName));
    }
};

namespace aux {

    //! The count_of filter generator
    template< typename CharT >
    class flt_count_of_gen
    {
    private:
        //! Char type
        typedef CharT char_type;
        //! String type
        typedef std::basic_string< char_type > string_type;
        //! Attribute values container type
        typedef typename basic_attribute_values_view< char_type > attribute_values_view;
        //! Size type
        typedef typename attribute_values_view::size_type size_type;

    private:
        //! Attribute name
        string_type m_AttributeName;

    public:
        explicit flt_count_of_gen(string_type const& name) : m_AttributeName(name) {}
        explicit flt_count_of_gen(const char_type* name) : m_AttributeName(name) {}

#define BOOST_LOG_FILTER_COUNT_OF_OPERATOR(op, fun)\
        flt_count_of<\
            char_type,\
            std::binder2nd< std::fun< size_type > >\
        > operator op (size_type size) const\
        {\
            typedef flt_count_of<\
                char_type,\
                std::binder2nd< std::fun< size_type > >\
            > flt_count_of_t;\
            return flt_count_of_t(m_AttributeName, std::bind2nd(std::fun< size_type >(), size));\
        }

        BOOST_LOG_FILTER_COUNT_OF_OPERATOR(==, equal_to)
        BOOST_LOG_FILTER_COUNT_OF_OPERATOR(!=, not_equal_to)
        BOOST_LOG_FILTER_COUNT_OF_OPERATOR(>, greater)
        BOOST_LOG_FILTER_COUNT_OF_OPERATOR(<, less)
        BOOST_LOG_FILTER_COUNT_OF_OPERATOR(>=, greater_equal)
        BOOST_LOG_FILTER_COUNT_OF_OPERATOR(<=, less_equal)

#undef BOOST_LOG_FILTER_COUNT_OF_OPERATOR

    };

} // namespace aux

//! Filter generator
template< typename CharT >
aux::flt_count_of_gen< CharT > count_of(const CharT* name)
{
    return aux::flt_count_of_gen< CharT >(name);
}

//! Filter generator
template< typename CharT >
aux::flt_count_of_gen< CharT > count_of(std::basic_string< CharT > const& name)
{
    return aux::flt_count_of_gen< CharT >(name);
}

} // namespace filters

} // namespace log

} // namespace boost

#endif // BOOST_LOG_FILTERS_COUNT_OF_HPP_INCLUDED_
