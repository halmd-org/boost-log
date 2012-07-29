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
 * The header contains implementation of tools for extracting an attribute value
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
#include <boost/type_traits/remove_cv.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/exceptions.hpp>
#include <boost/log/core/record.hpp>
#include <boost/log/attributes/attribute_name.hpp>
#include <boost/log/attributes/attribute_value_def.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/attributes/attribute_values_view.hpp>
#include <boost/log/attributes/value_ref.hpp>
#include <boost/log/utility/explicit_operator_bool.hpp>
#include <boost/log/attributes/value_extraction_fwd.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace result_of {

/*!
 * \brief A metafunction that allows to acquire the result of the value extraction
 *
 * The metafunction results in a type that is in form of <tt>T const&</tt>, if \c T is
 * not a MPL type sequence and <tt>DefaultT</tt> is the same as <tt>T</tt>,
 * or <tt>value_ref< TypesT, TagT ></tt> otherwise, with
 * \c TypesT being a type sequence comprising the types from sequence \c T and \c DefaultT,
 * if it is not present in \c T already.
 */
template< typename T, typename DefaultT, typename TagT >
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

    typedef typename mpl::if_<
        mpl::is_sequence< extracted_type >,
        value_ref< extracted_type, TagT >,
        extracted_type const&
    >::type type;
};

/*!
 * \brief A metafunction that allows to acquire the result of the value extraction
 *
 * The metafunction results in a type that is in form of <tt>T const&</tt>, if \c T is
 * not a MPL type sequence, or <tt>value_ref< T, TagT ></tt> otherwise. In the latter
 * case the value reference shall never be empty.
 */
template< typename T, typename TagT >
struct extract_or_throw
{
    typedef typename mpl::if_<
        mpl::is_sequence< T >,
        value_ref< T, TagT >,
        T const&
    >::type type;
};

/*!
 * \brief A metafunction that allows to acquire the result of the value extraction
 *
 * The metafunction results in a type that is in form of <tt>value_ref< T, TagT ></tt>.
 */
template< typename T, typename TagT >
struct extract
{
    typedef value_ref< T, TagT > type;
};

} // namespace result_of

namespace aux {

//! The function object initializes the value reference
template< typename RefT >
struct value_ref_initializer
{
    typedef void result_type;

    value_ref_initializer(RefT& ref) : m_ref(ref)
    {
    }

    template< typename ArgT >
    result_type operator() (ArgT const& arg) const
    {
        m_ref = RefT(arg);
    }

private:
    RefT& m_ref;
};

//! The function unwraps \c value_ref, if possible
template< typename T, typename TagT >
BOOST_LOG_FORCEINLINE typename enable_if< mpl::is_sequence< T >, value_ref< T, TagT > >::type
unwrap_value_ref(value_ref< T, TagT > const& r)
{
    return r;
}

template< typename T, typename TagT >
BOOST_LOG_FORCEINLINE typename disable_if< mpl::is_sequence< T >, T const& >::type
unwrap_value_ref(value_ref< T, TagT > const& r)
{
    return r.get();
}

} // namespace aux

/*!
 * \brief Attribute value extraction policy that returns an empty value if no value can be extracted
 *
 * This class is a function object which can also be used as a policy for the \c value_extractor
 * class template. The function object accepts an attribute value object and extracts the
 * contained value. If the extraction succeeds, the returned value is a filled \c value_ref
 * with the extracted value. Otherwise, an empty \c value_ref is returned.
 *
 * The extractor can be specialized on one or several attribute value types. The extraction
 * succeeds if the provided attribute value contains a value of type matching either one of the
 * specified types.
 */
template< typename T, typename TagT >
class extract_value_or_none
{
public:
    //! Attribute value types
    typedef T value_type;
    //! Function object result type
    typedef value_ref< value_type, TagT > result_type;

public:
    /*!
     * Extraction operator. Returns an optional value which is present if the provided
     * attribute value contains the value of the specified type and an empty value otherwise.
     */
    result_type operator() (attribute_value const& value) const
    {
        result_type res;
        if (!!value)
            value.visit< value_type >(aux::value_ref_initializer< result_type >(res));
        return res;
    }
};

/*!
 * \brief Attribute value extraction policy that throws an exception if no value can be extracted
 *
 * This class is a function object which can also be used as a policy for the \c value_extractor
 * class template. The function object accepts an attribute value object and extracts the
 * contained value. If the extraction succeeds, the returned value is a \c value_ref that refers
 * to the extracted value. Otherwise, an exception is thrown.
 *
 * The extractor can be specialized on one or several attribute value types. The extraction
 * succeeds if the provided attribute value contains a value of type matching either one of the
 * specified types.
 */
template< typename T, typename TagT >
class extract_value_or_throw
{
public:
    //! Attribute value types
    typedef T value_type;
    //! Function object result type
    typedef value_ref< value_type, TagT > result_type;

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
            if (!value.visit< value_type >(aux::value_ref_initializer< result_type >(res)))
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
 * contained value. If the extraction succeeds, the returned value is a \c value_ref that refers to
 * the extracted value. Otherwise, the \c value_ref refers to the default value.
 *
 * The extractor can be specialized on one or several attribute value types. The extraction
 * succeeds if the provided attribute value contains a value of type matching either one of the
 * specified types.
 */
template< typename T, typename DefaultT, typename TagT >
class extract_value_or_default
{
public:
    //! Attribute value types
    typedef T value_type;
    //! Default value type
    typedef typename remove_cv< typename remove_reference< DefaultT >::type >::type default_type;

private:
    //! Final set of result types
    typedef typename result_of::extract_or_default< value_type, default_type >::extracted_type extracted_type;

public:
    //! Function object result type
    typedef value_ref< extracted_type, TagT > result_type;

private:
    //! Default value
    DefaultT m_default;

public:
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
        result_type res(m_default);
        if (!!value)
            value.visit< value_type >(aux::value_ref_initializer< result_type >(res));
        return res;
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

#if defined(BOOST_LOG_DOXYGEN_PASS) || !(defined(BOOST_NO_FUNCTION_TEMPLATE_DEFAULT_ARGS) || defined(BOOST_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS))
#define BOOST_LOG_AUX_VOID_DEFAULT = void
#else
#define BOOST_LOG_AUX_VOID_DEFAULT
#endif

/*!
 * The function extracts an attribute value from the view. The user has to explicitly specify the
 * type or set of possible types of the attribute value to be extracted.
 *
 * \param name The name of the attribute value to extract.
 * \param attrs A set of attribute values in which to look for the specified attribute value.
 * \return A \c value_ref that refers to the extracted value, if found. An empty value otherwise.
 */
template< typename T, typename TagT BOOST_LOG_AUX_VOID_DEFAULT >
inline typename result_of::extract< T, TagT >::type extract(attribute_name const& name, attribute_values_view const& attrs)
{
    value_extractor< extract_value_or_none< T, TagT > > extractor(name);
    return extractor(attrs);
}

/*!
 * The function extracts an attribute value from the view. The user has to explicitly specify the
 * type or set of possible types of the attribute value to be extracted.
 *
 * \param name The name of the attribute value to extract.
 * \param rec A log record. The attribute value will be sought among those associated with the record.
 * \return A \c value_ref that refers to the extracted value, if found. An empty value otherwise.
 */
template< typename T, typename TagT BOOST_LOG_AUX_VOID_DEFAULT >
inline typename result_of::extract< T, TagT >::type extract(attribute_name const& name, record const& rec)
{
    value_extractor< extract_value_or_none< T, TagT > > extractor(name);
    return extractor(rec);
}

/*!
 * The function extracts an attribute value from the view. The user has to explicitly specify the
 * type or set of possible types of the attribute value to be extracted.
 *
 * \param value Attribute value.
 * \return A \c value_ref that refers to the extracted value, if found. An empty value otherwise.
 */
template< typename T, typename TagT BOOST_LOG_AUX_VOID_DEFAULT >
inline typename result_of::extract< T, TagT >::type extract(attribute_value const& value)
{
    extract_value_or_none< T, TagT > extractor;
    return extractor(value);
}

/*!
 * The function extracts an attribute value from the view. The user has to explicitly specify the
 * type or set of possible types of the attribute value to be extracted.
 *
 * \param name The name of the attribute value to extract.
 * \param attrs A set of attribute values in which to look for the specified attribute value.
 * \return The extracted value or a non-empty \c value_ref that refers to the value.
 * \throws An exception is thrown if the requested value cannot be extracted.
 */
template< typename T, typename TagT BOOST_LOG_AUX_VOID_DEFAULT >
inline typename result_of::extract_or_throw< T, TagT >::type extract_or_throw(attribute_name const& name, attribute_values_view const& attrs)
{
    value_extractor< extract_value_or_throw< T, TagT > > extractor(name);
    return aux::unwrap_value_ref(extractor(attrs));
}

/*!
 * The function extracts an attribute value from the view. The user has to explicitly specify the
 * type or set of possible types of the attribute value to be extracted.
 *
 * \param name The name of the attribute value to extract.
 * \param rec A log record. The attribute value will be sought among those associated with the record.
 * \return The extracted value or a non-empty \c value_ref that refers to the value.
 * \throws An exception is thrown if the requested value cannot be extracted.
 */
template< typename T, typename TagT BOOST_LOG_AUX_VOID_DEFAULT >
inline typename result_of::extract_or_throw< T, TagT >::type extract_or_throw(attribute_name const& name, record const& rec)
{
    value_extractor< extract_value_or_throw< T, TagT > > extractor(name);
    return aux::unwrap_value_ref(extractor(rec));
}

/*!
 * The function extracts an attribute value from the view. The user has to explicitly specify the
 * type or set of possible types of the attribute value to be extracted.
 *
 * \param value Attribute value.
 * \return The extracted value or a non-empty \c value_ref that refers to the value.
 * \throws An exception is thrown if the requested value cannot be extracted.
 */
template< typename T, typename TagT BOOST_LOG_AUX_VOID_DEFAULT >
inline typename result_of::extract_or_throw< T, TagT >::type extract_or_throw(attribute_value const& value)
{
    extract_value_or_throw< T, TagT > extractor;
    return aux::unwrap_value_ref(extractor(value));
}

/*!
 * The function extracts an attribute value from the view. The user has to explicitly specify the
 * type or set of possible types of the attribute value to be extracted.
 *
 * \note Caution must be excercised if the default value is a temporary object. Because the function returns
 *       a reference, if the temporary object is destroued, the reference may become dangling.
 *
 * \param name The name of the attribute value to extract.
 * \param attrs A set of attribute values in which to look for the specified attribute value.
 * \param def_val The default value
 * \return The extracted value, if found. The default value otherwise.
 */
template< typename T, typename TagT BOOST_LOG_AUX_VOID_DEFAULT, typename DefaultT >
inline typename result_of::extract_or_default< T, DefaultT, TagT >::type extract_or_default(
    attribute_name const& name, attribute_values_view const& attrs, DefaultT const& def_val)
{
    value_extractor< extract_value_or_default< T, DefaultT const&, TagT > > extractor(name, def_val);
    return aux::unwrap_value_ref(extractor(attrs));
}

/*!
 * The function extracts an attribute value from the view. The user has to explicitly specify the
 * type or set of possible types of the attribute value to be visited.
 *
 * \note Caution must be excercised if the default value is a temporary object. Because the function returns
 *       a reference, if the temporary object is destroued, the reference may become dangling.
 *
 * \param name The name of the attribute value to extract.
 * \param rec A log record. The attribute value will be sought among those associated with the record.
 * \param def_val The default value
 * \return The extracted value, if found. The default value otherwise.
 */
template< typename T, typename TagT BOOST_LOG_AUX_VOID_DEFAULT, typename DefaultT >
inline typename result_of::extract_or_default< T, DefaultT, TagT >::type extract_or_default(
    attribute_name const& name, record const& rec, DefaultT const& def_val)
{
    value_extractor< extract_value_or_default< T, DefaultT const&, TagT > > extractor(name, def_val);
    return aux::unwrap_value_ref(extractor(rec));
}

/*!
 * The function extracts an attribute value from the view. The user has to explicitly specify the
 * type or set of possible types of the attribute value to be visited.
 *
 * \note Caution must be excercised if the default value is a temporary object. Because the function returns
 *       a reference, if the temporary object is destroued, the reference may become dangling.
 *
 * \param value Attribute value.
 * \param def_val The default value
 * \return The extracted value, if found. The default value otherwise.
 */
template< typename T, typename TagT BOOST_LOG_AUX_VOID_DEFAULT, typename DefaultT >
inline typename result_of::extract_or_default< T, DefaultT, TagT >::type extract_or_default(attribute_value const& value, DefaultT const& def_val)
{
    extract_value_or_default< T, DefaultT const&, TagT > extractor(def_val);
    return aux::unwrap_value_ref(extractor(value));
}

#undef BOOST_LOG_AUX_VOID_DEFAULT

#if defined(BOOST_NO_FUNCTION_TEMPLATE_DEFAULT_ARGS) || defined(BOOST_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS)

template< typename T >
inline typename result_of::extract< T >::type extract(attribute_name const& name, attribute_values_view const& attrs)
{
    value_extractor< extract_value_or_none< T > > extractor(name);
    return extractor(attrs);
}

template< typename T >
inline typename result_of::extract< T >::type extract(attribute_name const& name, record const& rec)
{
    value_extractor< extract_value_or_none< T > > extractor(name);
    return extractor(rec);
}

template< typename T >
inline typename result_of::extract< T >::type extract(attribute_value const& value)
{
    extract_value_or_none< T > extractor;
    return extractor(value);
}

template< typename T >
inline typename result_of::extract_or_throw< T >::type extract_or_throw(attribute_name const& name, attribute_values_view const& attrs)
{
    value_extractor< extract_value_or_throw< T > > extractor(name);
    return aux::unwrap_value_ref(extractor(attrs));
}

template< typename T >
inline typename result_of::extract_or_throw< T >::type extract_or_throw(attribute_name const& name, record const& rec)
{
    value_extractor< extract_value_or_throw< T > > extractor(name);
    return aux::unwrap_value_ref(extractor(rec));
}

template< typename T >
inline typename result_of::extract_or_throw< T >::type extract_or_throw(attribute_value const& value)
{
    extract_value_or_throw< T > extractor;
    return aux::unwrap_value_ref(extractor(value));
}

template< typename T, typename DefaultT >
inline typename result_of::extract_or_default< T, DefaultT >::type extract_or_default(
    attribute_name const& name, attribute_values_view const& attrs, DefaultT const& def_val)
{
    value_extractor< extract_value_or_default< T, DefaultT const& > > extractor(name, def_val);
    return aux::unwrap_value_ref(extractor(attrs));
}

template< typename T, typename DefaultT >
inline typename result_of::extract_or_default< T, DefaultT >::type extract_or_default(
    attribute_name const& name, record const& rec, DefaultT const& def_val)
{
    value_extractor< extract_value_or_default< T, DefaultT const& > > extractor(name, def_val);
    return aux::unwrap_value_ref(extractor(rec));
}

template< typename T, typename DefaultT >
inline typename result_of::extract_or_default< T, DefaultT >::type extract_or_default(attribute_value const& value, DefaultT const& def_val)
{
    extract_value_or_default< T, DefaultT const& > extractor(def_val);
    return aux::unwrap_value_ref(extractor(value));
}

#endif // defined(BOOST_NO_FUNCTION_TEMPLATE_DEFAULT_ARGS) || defined(BOOST_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS)

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

// This include has to be here to resolve dependencies between this header and the attribute_value methods implementation
#include <boost/log/attributes/attribute_value.hpp>

#endif // BOOST_LOG_ATTRIBUTES_VALUE_EXTRACTION_HPP_INCLUDED_
