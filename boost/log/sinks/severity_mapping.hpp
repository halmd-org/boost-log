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
 * \file   severity_mapping.hpp
 * \author Andrey Semashev
 * \date   07.11.2008
 * 
 * The header contains facilities that are used in different sinks to map severity levels
 * used throughout the application to levels used with the specific native logging API.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_SINKS_SEVERITY_MAPPING_HPP_INCLUDED_
#define BOOST_LOG_SINKS_SEVERITY_MAPPING_HPP_INCLUDED_

#include <map>
#include <string>
#include <functional>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/attributes/attribute_values_view.hpp>
#include <boost/log/utility/attribute_value_extractor.hpp>

#ifdef _MSC_VER
#pragma warning(push)
// 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
// non dll-interface class 'A' used as base for dll-interface class 'B'
#pragma warning(disable: 4275)
#endif // _MSC_VER

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace sinks {

//! Base class for level mapping function objects
template< typename CharT, typename MappedT >
struct basic_severity_mapping :
    public std::unary_function< basic_attribute_values_view< CharT >, MappedT >
{
    //! Char type
    typedef CharT char_type;
    //! String type
    typedef std::basic_string< char_type > string_type;
    //! Attribute values view type
    typedef basic_attribute_values_view< char_type > values_view_type;
    //! Mapped severity level type
    typedef MappedT mapped_type;
};

/*!
 * \brief Straightforward severity level mapping
 * 
 * This type of mapping assumes that attribute with a particular name always
 * provides values that map directly onto the native levels. The mapping
 * simply returns the extracted attribute value converted to the native severity level.
 */
template< typename CharT, typename MappedT, typename AttributeValueT = int >
class basic_direct_severity_mapping :
    public basic_severity_mapping< CharT, MappedT >
{
    //! Base type
    typedef basic_severity_mapping< CharT, MappedT > base_type;

public:
    //! Attribute contained value type
    typedef AttributeValueT attribute_value_type;
    //! Char type
    typedef typename base_type::char_type char_type;
    //! String type
    typedef typename base_type::string_type string_type;
    //! Attribute values view type
    typedef typename base_type::values_view_type values_view_type;
    //! Mapped severity level type
    typedef typename base_type::mapped_type mapped_type;

private:
    //! \cond

    //! Attribute value receiver
    struct receiver
    {
        typedef void result_type;

        explicit receiver(mapped_type& extracted) : m_Extracted(extracted) {}
        template< typename T >
        void operator() (T const& val) const
        {
            // TODO: Make this thing work with non-POD mapped_types
            mapped_type v = { val };
            m_Extracted = v;
        }

    private:
        mapped_type& m_Extracted;
    };

    //! \endcond

private:
    //! Attribute value extractor
    attribute_value_extractor< char_type, attribute_value_type > m_Extractor;
    //! Default native severity level
    mapped_type m_DefaultLevel;

public:
    /*!
     * Constructor
     * 
     * \param name Attribute name
     * \param default_level The default native severity level that is returned if the attribute value is not found
     */
    explicit basic_direct_severity_mapping(string_type const& name, mapped_type const& default_level) :
        m_Extractor(name),
        m_DefaultLevel(default_level)
    {
    }

    /*!
     * Extraction operator
     * 
     * \param values A set of attribute values attached to a logging record
     * \return An extracted attribute value
     */
    mapped_type operator() (values_view_type const& values) const
    {
        mapped_type res = m_DefaultLevel;
        receiver rcv(res);
        m_Extractor(values, rcv);
        return res;
    }
};

/*!
 * \brief Customizable severity level mapping
 * 
 * The class allows to setup a custom mapping between an attribute and native severity levels.
 * The mapping should be initialized similarly to the standard \c map container, by using
 * indexing operator and assignment.
 *
 * \note Unlike many other components of the library, exact type of the attribute value
 *       must be specified in the template parameter \c AttributeValueT. Type sequences
 *       are not supported.
 */
template< typename CharT, typename MappedT, typename AttributeValueT = int >
class basic_custom_severity_mapping :
    public basic_severity_mapping< CharT, MappedT >
{
    //! Base type
    typedef basic_severity_mapping< CharT, MappedT > base_type;

public:
    //! Attribute contained value type
    typedef AttributeValueT attribute_value_type;
    //! String type
    typedef typename base_type::string_type string_type;
    //! Attribute values view type
    typedef typename base_type::values_view_type values_view_type;
    //! Mapped severity level type
    typedef typename base_type::mapped_type mapped_type;

private:
    //! \cond

    //! Mapping type
    typedef std::map< attribute_value_type, mapped_type > mapping_type;
    //! Smart reference class for implementing insertion into the map
    class reference_proxy;
    friend class reference_proxy;
    class reference_proxy
    {
        mapping_type& m_Mapping;
        attribute_value_type m_Key;

    public:
        //! Constructor
        reference_proxy(mapping_type& mapping, attribute_value_type const& key) : m_Mapping(mapping), m_Key(key) {}
        //! Insertion
        reference_proxy const& operator= (mapped_type const& val) const
        {
            m_Mapping[m_Key] = val;
            return *this;
        }
    };

    //! Attribute value receiver
    struct receiver;
    friend struct receiver;
    struct receiver
    {
        typedef void result_type;

        receiver(mapping_type const& mapping, mapped_type& extracted) :
            m_Mapping(mapping),
            m_Extracted(extracted)
        {
        }
        template< typename T >
        void operator() (T const& val) const
        {
            typename mapping_type::const_iterator it = m_Mapping.find(val);
            if (it != m_Mapping.end())
                m_Extracted = it->second;
        }

    private:
        mapping_type const& m_Mapping;
        mapped_type& m_Extracted;
    };

    //! \endcond

private:
    //! Attribute value extractor
    attribute_value_extractor< char_type, attribute_value_type > m_Extractor;
    //! Default native severity level
    mapped_type m_DefaultLevel;
    //! Conversion mapping
    mapping_type m_Mapping;

public:
    /*!
     * Constructor
     * 
     * \param name Attribute name
     * \param default_level The default native severity level that is returned if the conversion cannot be performed
     */
    explicit basic_custom_severity_mapping(string_type const& name, mapped_type const& default_level) :
        m_AttributeName(name),
        m_DefaultLevel(default_level)
    {
    }
    /*!
     * Extraction operator. Extracts the attribute value and attempts to map it onto
     * the native severity level.
     * 
     * \param values A set of attribute values attached to a logging record
     * \return A mapped level, if mapping was successfull, or the default level if
     *         mapping did not succeed.
     */
    mapped_type operator() (values_view_type const& values) const
    {
        mapped_type res = m_DefaultLevel;
        receiver rcv(m_Mapping, res);
        m_Extractor(values, rcv);
        return res;
    }
    /*!
     * Insertion operator
     * 
     * \param key Attribute value to be mapped
     * \return An object of unspecified type that allows to insert a new mapping through assignment.
     *         The \a key argument becomes the key attribute value, and the assigned value becomes the
     *         mapped syslog level.
     */
#ifndef BOOST_LOG_DOXYGEN_PASS
    reference_proxy operator[] (attribute_value_type const& key)
#else
    implementation_defined operator[] (attribute_value_type const& key)
#endif
    {
        return reference_proxy(m_Mapping, key);
    }
};

} // namespace sinks

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_SINKS_SEVERITY_MAPPING_HPP_INCLUDED_
