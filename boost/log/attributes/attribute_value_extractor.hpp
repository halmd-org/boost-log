/*
 *          Copyright Andrey Semashev 2007 - 2010.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
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

#ifndef BOOST_LOG_ATTRIBUTES_ATTRIBUTE_VALUE_EXTRACTOR_HPP_INCLUDED_
#define BOOST_LOG_ATTRIBUTES_ATTRIBUTE_VALUE_EXTRACTOR_HPP_INCLUDED_

#include <boost/log/detail/prologue.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/attributes/attribute_name.hpp>
#include <boost/log/attributes/attribute_value.hpp>
#include <boost/log/attributes/attribute_values_view.hpp>
#include <boost/log/utility/explicit_operator_bool.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

//! Error codes for attribute value extraction
enum extraction_result_code
{
    value_extracted,        //!< The attribute value has been extracted successfully
    value_not_found,        //!< The attribute value is not present in the view
    value_has_invalid_type  //!< The attribute value is present in the vew, but has an unexpected type
};

/*!
 * \brief The class represents attribute value extraction result
 *
 * The main purpose of this class is to provide a convenient interface for checking
 * whether the attribute value extraction succeeded or not. It also allows to discover
 * the actual cause of failure, should the extraction fail.
 */
class extraction_result
{
private:
    extraction_result_code m_Code;

public:
    /*!
     * Initializing constructor. Creates the result that is equivalent to the
     * specified error code.
     */
    extraction_result(extraction_result_code code = value_extracted) : m_Code(code) {}

    /*!
     * Checks if the extraction was successful.
     *
     * \return \c true if the value was extracted successfully, \c false otherwise.
     */
    BOOST_LOG_EXPLICIT_OPERATOR_BOOL()
    /*!
     * Checks if the extraction was unsuccessful.
     *
     * \return \c false if the value was extracted successfully, \c true otherwise.
     */
    bool operator! () const { return (m_Code != value_extracted); }

    /*!
     * \return The actual result code of value extraction
     */
    extraction_result_code code() const { return m_Code; }
};

/*!
 * \brief Generic attribute value extractor
 *
 * Attribute value extractor is a functional object that attempts to extract the stored
 * attribute value from the attribute value wrapper object. The extracted value is passed to
 * an unary functional object (the receiver) provided by user.
 *
 * The extractor can be specialized on one or several attribute value types that should be
 * specified in the second template argument.
 */
template< typename CharT, typename T >
class attribute_value_extractor
{
public:
    //! Function object result type
    typedef extraction_result result_type;

    //! Character type
    typedef CharT char_type;
    //! Attribute name type
    typedef basic_attribute_name< char_type > attribute_name_type;
    //! Attribute values view type
    typedef basic_attribute_values_view< char_type > values_view_type;
    //! Attribute value types
    typedef T value_types;

private:
    //! Attribute name to extract
    attribute_name_type m_Name;

public:
    /*!
     * Constructor
     *
     * \param name Attribute name to be extracted on invokation
     */
    explicit attribute_value_extractor(attribute_name_type const& name) : m_Name(name) {}

    /*!
     * Extraction operator. Looks for an attribute value with the name specified on construction
     * and tries to acquire the stored value of one of the supported types. If extraction succeeds,
     * the extracted value is passed to \a receiver.
     *
     * \param attrs A set of attribute values in which to look for the specified attribute value.
     * \param receiver A receiving functional object to pass the extracted value to.
     * \return \c true if extraction succeeded, \c false otherwise
     */
    template< typename ReceiverT >
    result_type operator() (values_view_type const& attrs, ReceiverT receiver) const
    {
        typename values_view_type::const_iterator it = attrs.find(m_Name);
        if (it != attrs.end())
        {
            if (it->second.extract(receiver))
                return result_type(value_extracted);
            else
                return result_type(value_has_invalid_type);
        }
        return result_type(value_not_found);
    }
};

#ifdef BOOST_LOG_DOXYGEN_PASS

/*!
 * The function extracts an attribute value from the view. The user has to explicitly specify the
 * type or set of possible types of the attribute value to be extracted.
 *
 * \param name An attribute value name to extract.
 * \param attrs A set of attribute values in which to look for the specified attribute value.
 * \param receiver A receiving functional object to pass the extracted value to.
 * \return \c true if extraction succeeded, \c false otherwise
 */
template< typename T, typename CharT, typename ReceiverT >
extraction_result extract(
    basic_attribute_name< CharT > const& name, basic_attribute_values_view< CharT > const& attrs, ReceiverT receiver);

#else // BOOST_LOG_DOXYGEN_PASS

#ifdef BOOST_LOG_USE_CHAR

template< typename T, typename ReceiverT >
inline extraction_result extract(
    basic_attribute_name< char > const& name, basic_attribute_values_view< char > const& attrs, ReceiverT receiver)
{
    attribute_value_extractor< char, T > extractor(name);
    return extractor(attrs, receiver);
}

#endif // BOOST_LOG_USE_CHAR

#ifdef BOOST_LOG_USE_WCHAR_T

template< typename T, typename ReceiverT >
inline extraction_result extract(
    basic_attribute_name< wchar_t > const& name, basic_attribute_values_view< wchar_t > const& attrs, ReceiverT receiver)
{
    attribute_value_extractor< wchar_t, T > extractor(name);
    return extractor(attrs, receiver);
}

#endif // BOOST_LOG_USE_WCHAR_T

#endif // BOOST_LOG_DOXYGEN_PASS

} // namespace log

} // namespace boost

#endif // BOOST_LOG_ATTRIBUTES_ATTRIBUTE_VALUE_EXTRACTOR_HPP_INCLUDED_
