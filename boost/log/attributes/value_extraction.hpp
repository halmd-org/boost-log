/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   value_extraction.hpp
 * \author Andrey Semashev
 * \date   01.03.2008
 *
 * The header contains implementation of convenience tools to extract an attribute value
 * from the view.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_ATTRIBUTES_VALUE_EXTRACTION_HPP_INCLUDED_
#define BOOST_LOG_ATTRIBUTES_VALUE_EXTRACTION_HPP_INCLUDED_

#include <boost/type_traits/is_same.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/core/record.hpp>
#include <boost/log/attributes/attribute_name.hpp>
#include <boost/log/attributes/attribute_value.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/attributes/attribute_values_view.hpp>
#include <boost/log/utility/explicit_operator_bool.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

/*!
 * \brief Attribute value extraction policy that returns an empty value if no value can be extracted
 *
 * This class is a function object which can also be used as a policy for the \c value_extractor
 * class template. The function object accepts an attribute value object and extracts the
 * contained value. If the extraction succeeds, the returned value is a filled \c optional
 * with the extracted value. Otherwise, an empty \c optional is returned.
 *
 * The extractor can be specialized on one or several attribute value types. The extraction
 * succeeds if the provided attribute value contains a value of type matching either one of the
 * specified types.
 */
template< typename T >
class extract_value_or_none
{
public:
    //! Attribute value types
    typedef T value_types;
    //! Function object result type
    typedef typename attribute_value::result_of_extract< value_types >::type result_type;

public:
    /*!
     * Extraction operator. Returns an optional value which is present if the provided
     * attribute value contains the value of the specified type and an empty value otherwise.
     */
    result_type operator() (attribute_value const& value) const
    {
        if (!!value)
            return value.BOOST_NESTED_TEMPLATE extract< value_types >();
        else
            return result_type();
    }
};

/*!
 * \brief Attribute value extraction policy that returns an default value if no value can be extracted
 *
 * This class is a function object which can also be used as a policy for the \c value_extractor
 * class template. The function object accepts an attribute value object and extracts the
 * contained value. If the extraction succeeds, the extracted value is returned.
 * Otherwise, the default value is returned.
 *
 * The extractor can be specialized on one or several attribute value types. The extraction
 * succeeds if the provided attribute value contains a value of type matching either one of the
 * specified types.
 */
template< typename T >
class extract_value_or_default
{
public:
    //! Attribute value types
    typedef T value_types;
    //! Function object result type
    typedef typename attribute_value::result_of_extract< value_types >::extracted_type result_type;

private:
    //! Default value
    result_type m_default;

public:
    /*!
     * Default constructor. The default value is default-constructed.
     */
    extract_value_or_default() : m_default() {}
    /*!
     * Initializing constructor. The default value is initialized with the provided value.
     */
    template< typename U >
    explicit extract_value_or_default(U const& val) : m_default(val) {}

    /*!
     * Extraction operator. Returns the value in the provided attribute value if
     * the attribute value contains the value of the specified type and the default value otherwise.
     */
    result_type operator() (attribute_value const& value) const
    {
        if (!!value)
            return value.BOOST_NESTED_TEMPLATE extract< value_types >().get_value_or(m_default);
        else
            return m_default;
    }
};

/*!
 * \brief Generic attribute value extractor
 *
 * Attribute value extractor is a functional object that attempts to find and extract the stored
 * attribute value from the attribute values view or a log record. The extracted value is returned
 * from the extractor.
 */
template< typename CharT, typename ExtractionPolicyT >
class value_extractor :
    public ExtractionPolicyT
{
    typedef ExtractionPolicyT base_type;

public:
    //! Function object result type
    typedef typename base_type::result_type result_type;

    //! Character type
    typedef CharT char_type;
    //! Attribute name type
    typedef basic_attribute_name< char_type > attribute_name_type;
    //! Attribute values view type
    typedef basic_attribute_values_view< char_type > values_view_type;
    //! Log record type
    typedef basic_record< char_type > record_type;

private:
    //! The name of the attribute value to extract
    attribute_name_type m_Name;

public:
    /*!
     * Constructor
     *
     * \param name Attribute name to be extracted on invokation
     */
    explicit value_extractor(attribute_name_type const& name) : m_Name(name) {}

    /*!
     * Initializing constructor
     *
     * \param name Attribute name to be extracted on invokation
     * \param arg1 An argument to the value extraction policy
     */
    template< typename ArgT1 >
    value_extractor(attribute_name_type const& name, ArgT1 const& arg1) : base_type(arg1), m_Name(name) {}

    /*!
     * Extraction operator. Looks for an attribute value with the name specified on construction
     * and tries to acquire the stored value of one of the supported types. If extraction succeeds,
     * the extracted value is returned.
     *
     * \param attrs A set of attribute values in which to look for the specified attribute value.
     * \return The extracted value, if extraction succeeded, an empty value otherwise.
     */
    result_type operator() (values_view_type const& attrs) const
    {
        typename values_view_type::const_iterator it = attrs.find(m_Name);
        if (it != attrs.end())
            return base_type::operator() (it->second);
        else
            return base_type::operator() (attribute_value());
    }

    /*!
     * Extraction operator. Looks for an attribute value with the name specified on construction
     * and tries to acquire the stored value of one of the supported types. If extraction succeeds,
     * the extracted value is returned.
     *
     * \param record A log record. The attribute value will be sought among those associated with the record.
     * \return The extracted value, if extraction succeeded, an empty value otherwise.
     */
    result_type operator() (record_type const& record) const
    {
        return operator() (record.attribute_values());
    }
};

#ifdef BOOST_LOG_DOXYGEN_PASS

/*!
 * The function extracts an attribute value from the view. The user has to explicitly specify the
 * type or set of possible types of the attribute value to be extracted.
 *
 * \param name The name of the attribute value to extract.
 * \param attrs A set of attribute values in which to look for the specified attribute value.
 * \return The extracted value, if found. An empty value otherwise.
 */
template< typename T, typename CharT >
typename value_extractor< char, extract_value_or_none< T > >::result_type extract(
    basic_attribute_name< CharT > const& name, basic_attribute_values_view< CharT > const& attrs);

/*!
 * The function extracts an attribute value from the view. The user has to explicitly specify the
 * type or set of possible types of the attribute value to be extracted.
 *
 * \param name The name of the attribute value to extract.
 * \param record A log record. The attribute value will be sought among those associated with the record.
 * \return The extracted value, if found. An empty value otherwise.
 */
template< typename T, typename CharT >
typename value_extractor< char, extract_value_or_none< T > >::result_type extract(
    basic_attribute_name< CharT > const& name, basic_record< CharT > const& record);

/*!
 * The function extracts an attribute value from the view. The user has to explicitly specify the
 * type or set of possible types of the attribute value to be extracted.
 *
 * \param name The name of the attribute value to extract.
 * \param attrs A set of attribute values in which to look for the specified attribute value.
 * \param def_val The default value
 * \return The extracted value, if found. The default value otherwise.
 */
template< typename T, typename CharT, typename DefaultT >
typename value_extractor< char, extract_value_or_default< T > >::result_type extract_or_default(
    basic_attribute_name< CharT > const& name, basic_attribute_values_view< CharT > const& attrs, DefaultT const& def_val);

/*!
 * The function extracts an attribute value from the view. The user has to explicitly specify the
 * type or set of possible types of the attribute value to be visited.
 *
 * \param name The name of the attribute value to extract.
 * \param record A log record. The attribute value will be sought among those associated with the record.
 * \param def_val The default value
 * \return The extracted value, if found. The default value otherwise.
 */
template< typename T, typename CharT, typename DefaultT >
typename value_extractor< char, extract_value_or_default< T > >::result_type extract_or_default(
    basic_attribute_name< CharT > const& name, basic_record< CharT > const& record, DefaultT const& def_val);

#else // BOOST_LOG_DOXYGEN_PASS

#ifdef BOOST_LOG_USE_CHAR

template< typename T >
inline typename value_extractor< char, extract_value_or_none< T > >::result_type extract(
    basic_attribute_name< char > const& name, basic_attribute_values_view< char > const& attrs)
{
    value_extractor< char, extract_value_or_none< T > > extractor(name);
    return extractor(attrs);
}

template< typename T >
inline typename value_extractor< char, extract_value_or_none< T > >::result_type extract(
    basic_attribute_name< char > const& name, basic_record< char > const& record)
{
    value_extractor< char, extract_value_or_none< T > > extractor(name);
    return extractor(record);
}

template< typename T, typename DefaultT >
inline typename disable_if<
    is_same< T, DefaultT >,
    typename value_extractor< char, extract_value_or_default< T > >::result_type
>::type extract_or_default(
    basic_attribute_name< char > const& name, basic_attribute_values_view< char > const& attrs, DefaultT const& def_val)
{
    value_extractor< char, extract_value_or_default< T > > extractor(name, def_val);
    return extractor(attrs);
}

template< typename T, typename DefaultT >
inline typename disable_if<
    is_same< T, DefaultT >,
    typename value_extractor< char, extract_value_or_default< T > >::result_type
>::type extract_or_default(
    basic_attribute_name< char > const& name, basic_record< char > const& record, DefaultT const& def_val)
{
    value_extractor< char, extract_value_or_default< T > > extractor(name, def_val);
    return extractor(record);
}

template< typename T >
inline typename value_extractor< char, extract_value_or_default< T > >::result_type extract_or_default(
    basic_attribute_name< char > const& name, basic_attribute_values_view< char > const& attrs, T const& def_val)
{
    value_extractor< char, extract_value_or_default< T > > extractor(name, def_val);
    return extractor(attrs);
}

template< typename T >
inline typename value_extractor< char, extract_value_or_default< T > >::result_type extract_or_default(
    basic_attribute_name< char > const& name, basic_record< char > const& record, T const& def_val)
{
    value_extractor< char, extract_value_or_default< T > > extractor(name, def_val);
    return extractor(record);
}

#endif // BOOST_LOG_USE_CHAR

#ifdef BOOST_LOG_USE_WCHAR_T

template< typename T >
inline typename value_extractor< wchar_t, extract_value_or_none< T > >::result_type extract(
    basic_attribute_name< wchar_t > const& name, basic_attribute_values_view< wchar_t > const& attrs)
{
    value_extractor< wchar_t, extract_value_or_none< T > > extractor(name);
    return extractor(attrs);
}

template< typename T >
inline typename value_extractor< wchar_t, extract_value_or_none< T > >::result_type extract(
    basic_attribute_name< wchar_t > const& name, basic_record< wchar_t > const& record)
{
    value_extractor< wchar_t, extract_value_or_none< T > > extractor(name);
    return extractor(record);
}

template< typename T, typename DefaultT >
inline typename disable_if<
    is_same< T, DefaultT >,
    typename value_extractor< wchar_t, extract_value_or_default< T > >::result_type
>::type extract_or_default(
    basic_attribute_name< wchar_t > const& name, basic_attribute_values_view< wchar_t > const& attrs, DefaultT const& def_val)
{
    value_extractor< wchar_t, extract_value_or_default< T > > extractor(name, def_val);
    return extractor(attrs);
}

template< typename T, typename DefaultT >
inline typename disable_if<
    is_same< T, DefaultT >,
    typename value_extractor< wchar_t, extract_value_or_default< T > >::result_type
>::type extract_or_default(
    basic_attribute_name< wchar_t > const& name, basic_record< wchar_t > const& record, DefaultT const& def_val)
{
    value_extractor< wchar_t, extract_value_or_default< T > > extractor(name, def_val);
    return extractor(record);
}

template< typename T >
inline typename value_extractor< wchar_t, extract_value_or_default< T > >::result_type extract_or_default(
    basic_attribute_name< wchar_t > const& name, basic_attribute_values_view< wchar_t > const& attrs, T const& def_val)
{
    value_extractor< wchar_t, extract_value_or_default< T > > extractor(name, def_val);
    return extractor(attrs);
}

template< typename T >
inline typename value_extractor< wchar_t, extract_value_or_default< T > >::result_type extract_or_default(
    basic_attribute_name< wchar_t > const& name, basic_record< wchar_t > const& record, T const& def_val)
{
    value_extractor< wchar_t, extract_value_or_default< T > > extractor(name, def_val);
    return extractor(record);
}

#endif // BOOST_LOG_USE_WCHAR_T

#endif // BOOST_LOG_DOXYGEN_PASS

} // namespace log

} // namespace boost

#endif // BOOST_LOG_ATTRIBUTES_VALUE_EXTRACTION_HPP_INCLUDED_
