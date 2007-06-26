/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   has_attr.hpp
 * \author Andrey Semashev
 * \date   22.04.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_FILTERS_HAS_ATTR_HPP_INCLUDED_
#define BOOST_LOG_FILTERS_HAS_ATTR_HPP_INCLUDED_

#include <string>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/filters/basic_filters.hpp>

namespace boost {

namespace log {

namespace filters {

//! A filter that detects if there is an attribute with given name in the complete attribute view
template< typename CharT >
class flt_has_attr :
    public basic_filter< CharT, flt_has_attr< CharT > >
{
private:
    //! Base type
    typedef basic_filter< CharT, flt_has_attr< CharT > > base_type;
    //! Attribute values container type
    typedef typename base_type::attribute_values_view attribute_values_view;
    //! Char type
    typedef typename base_type::char_type char_type;
    //! String type
    typedef std::basic_string< char_type > string_type;

private:
    //! Attribute name
    string_type m_AttributeName;

public:
    explicit flt_has_attr(string_type const& name) : m_AttributeName(name) {}
    explicit flt_has_attr(const char_type* name) : m_AttributeName(name) {}

    bool operator() (attribute_values_view const& values) const
    {
        return (values.find(m_AttributeName) != values.end());
    }
};

//! Filter generator
template< typename CharT >
flt_has_attr< CharT > has_attr(const CharT* name)
{
    return flt_has_attr< CharT >(name);
}

//! Filter generator
template< typename CharT >
flt_has_attr< CharT > has_attr(std::basic_string< CharT > const& name)
{
    return flt_has_attr< CharT >(name);
}

} // namespace filters

} // namespace log

} // namespace boost

#endif // BOOST_LOG_FILTERS_HAS_ATTR_HPP_INCLUDED_
