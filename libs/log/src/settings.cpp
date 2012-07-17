/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   settings.cpp
 * \author Andrey Semashev
 * \date   11.10.2009
 *
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#include <boost/assert.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/log/utility/init/settings.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

//! Checks if the container is empty (i.e. contains no sections and parameters).
template< typename CharT >
BOOST_LOG_SETUP_API bool basic_settings_section< CharT >::empty() const
{
    return m_ptree == NULL || m_ptree->empty();
}

//! Checks if the specified section is present in the container.
template< typename CharT >
BOOST_LOG_SETUP_API bool basic_settings_section< CharT >::has_section(string_type const& section_name) const
{
    return m_ptree != NULL && !!m_ptree->get_child_optional(section_name);
}

//! Checks if the specified parameter is present in the container.
template< typename CharT >
BOOST_LOG_SETUP_API bool basic_settings_section< CharT >::has_parameter(string_type const& section_name, string_type const& param_name) const
{
    if (m_ptree)
    {
        optional< property_tree_type const& > section = m_ptree->get_child_optional(section_name);
        if (!!section)
            return (section->find(param_name) != section->not_found());
    }

    return false;
}

template< typename CharT >
BOOST_LOG_SETUP_API basic_settings_section< CharT > basic_settings_section< CharT >::get_section(std::string const& name) const
{
    BOOST_ASSERT(m_ptree != NULL);
    return basic_settings_section(m_ptree->get_child_optional(name).get_ptr());
}

template< typename CharT >
BOOST_LOG_SETUP_API optional< typename basic_settings_section< CharT >::string_type >
basic_settings_section< CharT >::get_parameter(std::string const& name) const
{
    BOOST_ASSERT(m_ptree != NULL);
    return m_ptree->get_optional(name);
}

template< typename CharT >
BOOST_LOG_SETUP_API void basic_settings_section< CharT >::set_parameter(std::string const& name, string_type const& value)
{
    BOOST_ASSERT(m_ptree != NULL);
    m_ptree->put(name, value);
}

template< typename CharT >
BOOST_LOG_SETUP_API void basic_settings_section< CharT >::set_parameter(std::string const& name, optional< string_type > const& value)
{
    BOOST_ASSERT(m_ptree != NULL);

    if (!!value)
    {
        m_ptree->put(name, value);
    }
    else if (optional< property_tree_type& > node = m_ptree->get_child_optional(name))
    {
        node.put_value(string_type());
    }
}


//! Default constructor. Creates an empty settings container.
template< typename CharT >
BOOST_LOG_SETUP_API basic_settings< CharT >::basic_settings() : section(new property_tree_type())
{
}

//! Copy constructor.
template< typename CharT >
BOOST_LOG_SETUP_API basic_settings< CharT >::basic_settings(basic_settings const& that) :
    section(that.m_ptree ? new property_tree_type(*that.m_ptree) : static_cast< property_tree_type* >(NULL))
{
}

//! Initializing constructor
template< typename CharT >
BOOST_LOG_SETUP_API basic_settings< CharT >::basic_settings(property_tree_type const& tree) : section(new property_tree_type(tree))
{
}

//! Destructor
template< typename CharT >
BOOST_LOG_SETUP_API basic_settings< CharT >::~basic_settings()
{
    delete this->m_ptree;
}

#ifdef BOOST_LOG_USE_CHAR
template class basic_settings_section< char >;
template class basic_settings< char >;
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
template class basic_settings_section< wchar_t >;
template class basic_settings< wchar_t >;
#endif

} // namespace log

} // namespace boost
