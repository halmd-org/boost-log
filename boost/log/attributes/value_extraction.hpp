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

#include <boost/mpl/vector.hpp>
#include <boost/mpl/joint_view.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/mpl/is_sequence.hpp>
#include <boost/mpl/contains.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/variant/variant.hpp>
#include <boost/optional/optional.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/detail/functional.hpp>
#include <boost/log/exceptions.hpp>
#include <boost/log/core/record.hpp>
#include <boost/log/attributes/attribute_name.hpp>
#include <boost/log/attributes/attribute_value_def.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/attributes/attribute_values_view.hpp>
#include <boost/log/utility/explicit_operator_bool.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace result_of {

/*!
 * \brief A metafunction that allows to acquire the result of the value extraction
 *
 * The metafunction results in a type that is in form of <tt>T</tt>, if \c T is
 * not a MPL type sequence and <tt>DefaultT</tt> is the same as <tt>T</tt>,
 * or <tt>variant< T1, T2, ..., DefaultT ></tt> otherwise, with
 * \c T1, \c T2, etc. being the types comprising the type sequence \c T. If
 * <tt>DefaultT</tt> already exists in <tt>T</tt>, it is skipped.
 */
template< typename T, typename DefaultT >
struct extract_or_default
{
    typedef typename mpl::eval_if<
        mpl::is_sequence< T >,
        mpl::eval_if<
            mpl::contains< T, DefaultT >,
            mpl::identity< T >,
            mpl::push_back< T, DefaultT >
        >,
        mpl::if_<
            is_same< T, DefaultT >,
            T,
            mpl::vector2< T, DefaultT >
        >
    >::type extracted_type;

    typedef typename mpl::eval_if<
        mpl::is_sequence< extracted_type >,
        make_variant_over< extracted_type >,
        mpl::identity< T >
    >::type type;
};

/*!
 * \brief A metafunction that allows to acquire the result of the value extraction
 *
 * The metafunction results in a type that is in form of <tt>T</tt>, if \c T is
 * not a MPL type sequence, or <tt>variant< T1, T2, ... ></tt> otherwise, with
 * \c T1, \c T2, etc. being the types comprising the type sequence \c T.
 */
template< typename T >
struct extract_or_throw
{
    typedef typename mpl::eval_if<
        mpl::is_sequence< T >,
        make_variant_over< T >,
        mpl::identity< T >
    >::type type;
};

/*!
 * \brief A metafunction that allows to acquire the result of the value extraction
 *
 * The metafunction results in a type that is in form of <tt>optional< T ></tt>, if \c T is
 * not a MPL type sequence, or <tt>optional< variant< T1, T2, ... > ></tt> otherwise, with
 * \c T1, \c T2, etc. being the types comprising the type sequence \c T.
 */
template< typename T >
struct extract
{
    typedef typename mpl::eval_if<
        mpl::is_sequence< T >,
        make_variant_over< T >,
        mpl::identity< T >
    >::type extracted_type;

    typedef optional< extracted_type > type;
};

} // namespace result_of

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
    typedef typename result_of::extract< value_types >::type result_type;

public:
    /*!
     * Extraction operator. Returns an optional value which is present if the provided
     * attribute value contains the value of the specified type and an empty value otherwise.
     */
    result_type operator() (attribute_value const& value) const
    {
        if (!!value)
        {
            result_type res;
            value.visit< value_types >(boost::log::aux::bind_assign(res));
            return res;
        }
        else
            return result_type();
    }
};

/*!
 * \brief Attribute value extraction policy that throws an exception if no value can be extracted
 *
 * This class is a function object which can also be used as a policy for the \c value_extractor
 * class template. The function object accepts an attribute value object and extracts the
 * contained value. If the extraction succeeds, the returned value is the extracted value.
 * Otherwise, an exception is thrown.
 *
 * The extractor can be specialized on one or several attribute value types. The extraction
 * succeeds if the provided attribute value contains a value of type matching either one of the
 * specified types.
 */
template< typename T >
class extract_value_or_throw
{
public:
    //! Attribute value types
    typedef T value_types;
    //! Function object result type
    typedef typename result_of::extract_or_throw< value_types >::type result_type;

public:
    /*!
     * Extraction operator. Returns an optional value which is present if the provided
     * attribute value contains the value of the specified type and an empty value otherwise.
     */
    result_type operator() (attribute_value const& value) const
    {
        if (!!value)
        {
            result_type res;
            if (!value.visit< value_types >(boost::log::aux::bind_assign(res)))
                BOOST_LOG_THROW_DESCR_PARAMS(invalid_type, "Attribute value has incompatible type", (value.get_type()));
            return res;
        }
        else
            BOOST_LOG_THROW_DESCR(missing_value, "Attribute value not found");
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
template< typename T, typename DefaultT = T >
class extract_value_or_default
{
public:
    //! Attribute value types
    typedef T value_types;
    //! Default value type
    typedef DefaultT default_type;
    //! Function object result type
    typedef typename result_of::extract_or_default< value_types, default_type >::type result_type;

private:
    //! Default value
    default_type m_default;

public:
    /*!
     * Default constructor. The default value is default-constructed.
     */
    extract_value_or_default() : m_default() {}
    /*!
     * Initializing constructor. The default value is initialized with the provided value.
     */
    explicit extract_value_or_default(default_type const& val) : m_default(val) {}

    /*!
     * Extraction operator. Returns the value in the provided attribute value if
     * the attribute value contains the value of the specified type and the default value otherwise.
     */
    result_type operator() (attribute_value const& value) const
    {
        if (!!value)
        {
            result_type res = m_default;
            value.visit< value_types >(boost::log::aux::bind_assign(res));
            return res;
        }
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
template< typename ExtractionPolicyT >
class value_extractor :
    public ExtractionPolicyT
{
    typedef ExtractionPolicyT base_type;

public:
    //! Function object result type
    typedef typename base_type::result_type result_type;

private:
    //! The name of the attribute value to extract
    attribute_name m_Name;

public:
    /*!
     * Constructor
     *
     * \param name Attribute name to be extracted on invokation
     */
    explicit value_extractor(attribute_name const& name) : m_Name(name) {}

    /*!
     * Initializing constructor
     *
     * \param name Attribute name to be extracted on invokation
     * \param arg1 An argument to the value extraction policy
     */
    template< typename ArgT1 >
    value_extractor(attribute_name const& name, ArgT1 const& arg1) : base_type(arg1), m_Name(name) {}

    /*!
     * Extraction operator. Looks for an attribute value with the name specified on construction
     * and tries to acquire the stored value of one of the supported types. If extraction succeeds,
     * the extracted value is returned.
     *
     * \param attrs A set of attribute values in which to look for the specified attribute value.
     * \return The extracted value, if extraction succeeded, an empty value otherwise.
     */
    result_type operator() (attribute_values_view const& attrs) const
    {
        try
        {
            attribute_values_view::const_iterator it = attrs.find(m_Name);
            if (it != attrs.end())
                return base_type::operator() (it->second);
            else
                return base_type::operator() (attribute_value());
        }
        catch (exception& e)
        {
            // Attach the attribute name to the exception
            boost::log::aux::attach_attribute_name_info(e, m_Name);
            throw;
        }
    }

    /*!
     * Extraction operator. Looks for an attribute value with the name specified on construction
     * and tries to acquire the stored value of one of the supported types. If extraction succeeds,
     * the extracted value is returned.
     *
     * \param record A log record. The attribute value will be sought among those associated with the record.
     * \return The extracted value, if extraction succeeded, an empty value otherwise.
     */
    result_type operator() (record const& rec) const
    {
        return operator() (rec.attribute_values());
    }

    /*!
     * \return The cached attribute name to be extracted.
     */
    attribute_name get_name() const
    {
        return m_Name;
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
template< typename T >
typename result_of::extract< T >::type extract(attribute_name const& name, attribute_values_view const& attrs);

/*!
 * The function extracts an attribute value from the view. The user has to explicitly specify the
 * type or set of possible types of the attribute value to be extracted.
 *
 * \param name The name of the attribute value to extract.
 * \param record A log record. The attribute value will be sought among those associated with the record.
 * \return The extracted value, if found. An empty value otherwise.
 */
template< typename T >
typename result_of::extract< T >::type extract(attribute_name const& name, record const& record);

/*!
 * The function extracts an attribute value from the view. The user has to explicitly specify the
 * type or set of possible types of the attribute value to be extracted.
 *
 * \param name The name of the attribute value to extract.
 * \param attrs A set of attribute values in which to look for the specified attribute value.
 * \return The extracted value.
 * \throws An exception is thrown if the requested value cannot be extracted.
 */
template< typename T >
typename result_of::extract_or_throw< T >::type extract_or_throw(attribute_name const& name, attribute_values_view const& attrs);

/*!
 * The function extracts an attribute value from the view. The user has to explicitly specify the
 * type or set of possible types of the attribute value to be extracted.
 *
 * \param name The name of the attribute value to extract.
 * \param record A log record. The attribute value will be sought among those associated with the record.
 * \return The extracted value.
 * \throws An exception is thrown if the requested value cannot be extracted.
 */
template< typename T >
typename result_of::extract_or_throw< T >::type extract_or_throw(attribute_name const& name, record const& record);

/*!
 * The function extracts an attribute value from the view. The user has to explicitly specify the
 * type or set of possible types of the attribute value to be extracted.
 *
 * \param name The name of the attribute value to extract.
 * \param attrs A set of attribute values in which to look for the specified attribute value.
 * \param def_val The default value
 * \return The extracted value, if found. The default value otherwise.
 */
template< typename T, typename DefaultT >
typename result_of::extract_or_default< T, DefaultT >::type extract_or_default(
    attribute_name const& name, attribute_values_view const& attrs, DefaultT const& def_val);

/*!
 * The function extracts an attribute value from the view. The user has to explicitly specify the
 * type or set of possible types of the attribute value to be visited.
 *
 * \param name The name of the attribute value to extract.
 * \param record A log record. The attribute value will be sought among those associated with the record.
 * \param def_val The default value
 * \return The extracted value, if found. The default value otherwise.
 */
template< typename T, typename DefaultT >
typename result_of::extract_or_default< T, DefaultT >::type extract_or_default(
    attribute_name const& name, record const& record, DefaultT const& def_val);

#else // BOOST_LOG_DOXYGEN_PASS

template< typename T >
inline typename value_extractor< extract_value_or_none< T > >::result_type extract(
    attribute_name const& name, attribute_values_view const& attrs)
{
    value_extractor< extract_value_or_none< T > > extractor(name);
    return extractor(attrs);
}

template< typename T >
inline typename value_extractor< extract_value_or_none< T > >::result_type extract(
    attribute_name const& name, record const& record)
{
    value_extractor< extract_value_or_none< T > > extractor(name);
    return extractor(record);
}

template< typename T >
inline typename value_extractor< extract_value_or_throw< T > >::result_type extract_or_throw(
    attribute_name const& name, attribute_values_view const& attrs)
{
    value_extractor< extract_value_or_throw< T > > extractor(name);
    return extractor(attrs);
}

template< typename T >
inline typename value_extractor< extract_value_or_throw< T > >::result_type extract_or_throw(
    attribute_name const& name, record const& record)
{
    value_extractor< extract_value_or_throw< T > > extractor(name);
    return extractor(record);
}

template< typename T, typename DefaultT >
inline typename disable_if<
    is_same< T, DefaultT >,
    typename value_extractor< extract_value_or_default< T, DefaultT > >::result_type
>::type extract_or_default(attribute_name const& name, attribute_values_view const& attrs, DefaultT const& def_val)
{
    value_extractor< extract_value_or_default< T, DefaultT > > extractor(name, def_val);
    return extractor(attrs);
}

template< typename T, typename DefaultT >
inline typename disable_if<
    is_same< T, DefaultT >,
    typename value_extractor< extract_value_or_default< T, DefaultT > >::result_type
>::type extract_or_default(attribute_name const& name, record const& record, DefaultT const& def_val)
{
    value_extractor< extract_value_or_default< T, DefaultT > > extractor(name, def_val);
    return extractor(record);
}

template< typename T >
inline typename value_extractor< extract_value_or_default< T, T > >::result_type extract_or_default(
    attribute_name const& name, attribute_values_view const& attrs, T const& def_val)
{
    value_extractor< extract_value_or_default< T, T > > extractor(name, def_val);
    return extractor(attrs);
}

template< typename T >
inline typename value_extractor< extract_value_or_default< T, T > >::result_type extract_or_default(
    attribute_name const& name, record const& record, T const& def_val)
{
    value_extractor< extract_value_or_default< T, T > > extractor(name, def_val);
    return extractor(record);
}

#endif // BOOST_LOG_DOXYGEN_PASS

} // namespace log

} // namespace boost

// This include has to be here to resolve dependencies between this header and the attribute_value methods implementation
#include <boost/log/attributes/attribute_value.hpp>

#endif // BOOST_LOG_ATTRIBUTES_VALUE_EXTRACTION_HPP_INCLUDED_
