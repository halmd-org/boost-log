/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   keyword.hpp
 * \author Andrey Semashev
 * \date   29.01.2012
 *
 * The header contains attribute keyword declaration.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_EXPRESSIONS_KEYWORD_HPP_INCLUDED_
#define BOOST_LOG_EXPRESSIONS_KEYWORD_HPP_INCLUDED_

#include <boost/proto/extends.hpp>
#include <boost/phoenix/core/actor.hpp>
#include <boost/fusion/sequence/intrinsic/at.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/facilities/empty.hpp>
#include <boost/preprocessor/facilities/identity.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/expressions/domain.hpp>
#include <boost/log/expressions/terminal.hpp>
#include <boost/log/expressions/keyword_fwd.hpp>
#include <boost/log/attributes/value_extraction.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace expressions {

/*!
 * \brief This class implements an expression template keyword
 */
template< typename DescriptorT, template< typename > class ActorT >
struct attribute_keyword
{
    //! Internal typedef for type categorization
    typedef void _is_boost_log_attribute_keyword;

    //! Attribute descriptor type
    typedef DescriptorT descriptor_type;
    //! Attribute value type
    typedef typename descriptor_type::value_type value_type;
    //! Expression type
    typedef ActorT< attribute_terminal< DescriptorT > > expr_type;

    BOOST_PROTO_EXTENDS(expr_type, attribute_keyword, domain)

    //! Expression with cached attribute name
    typedef ActorT<
        cached_attribute_terminal<
            value_extractor<
                extract_value_or_none< value_type >
            >
        >
    > or_none_result_type;

    //! Generates an expression that extracts the attribute value or a default value
    or_none_result_type or_none() const
    {
        typedef cached_attribute_terminal<
            value_extractor<
                extract_value_or_none< value_type >
            >
        > cached_terminal;
        ActorT< cached_terminal > res = { cached_terminal(descriptor_type::get_name()) };
        return res;
    }

    //! Expression with cached attribute name and default value
    typedef ActorT<
        cached_attribute_terminal<
            value_extractor<
                extract_value_or_default< value_type >
            >
        >
    > or_default_result_type;

    //! Generates an expression that extracts the attribute value or a default value
    template< typename T >
    or_default_result_type or_default(T const& def_val) const
    {
        typedef cached_attribute_terminal<
            value_extractor<
                extract_value_or_default< value_type >
            >
        > cached_terminal;
        ActorT< cached_terminal > res = { cached_terminal(descriptor_type::get_name(), def_val) };
        return res;
    }
};

} // namespace expressions

} // namespace log

} // namespace boost

#ifndef BOOST_LOG_DOXYGEN_PASS

#define BOOST_LOG_DECLARE_ATTRIBUTE_KEYWORD_TYPE_IMPL(tag_ns_, name_, keyword_, attr_type_, create_decl_)\
    namespace tag_ns_\
    {\
        struct keyword_\
        {\
            typedef attr_type_ attribute_type;\
            typedef attribute_type::value_type value_type;\
            static ::boost::log::attribute_name get_name() { return ::boost::log::attribute_name(name_); }\
            static create_decl_() attribute_type create();\
        };\
    }\
    typedef ::boost::log::expressions::attribute_keyword< tag_ns_::keyword_ > BOOST_PP_CAT(keyword_, _type);

#define BOOST_LOG_DECLARE_ATTRIBUTE_KEYWORD_IMPL(tag_ns_, name_, keyword_, attr_type_, create_decl_)\
    BOOST_LOG_DECLARE_ATTRIBUTE_KEYWORD_TYPE_IMPL(tag_ns_, name_, keyword_, attr_type_, create_decl_)\
    const BOOST_PP_CAT(keyword_, _type) keyword_ = {};

#define BOOST_LOG_DEFINE_ATTRIBUTE_KEYWORD_CREATE_IMPL(create_decl_, keyword_ns_, keyword_)\
    create_decl_() keyword_ns_::keyword_::attribute_type keyword_ns_::keyword_::create()

#endif // BOOST_LOG_DOXYGEN_PASS

/*!
 * \brief The macro declares an attribute keyword type
 *
 * The macro should be used at a namespace scope. It expands into an attribute keyword type definition, including the
 * \c tag namespace and the keyword tag type within which has the following layout:
 *
 * \code
 * namespace tag
 * {
 *   struct keyword_
 *   {
 *     typedef attr_type_ attribute_type;
 *     typedef attribute_type::value_type value_type;
 *     static boost::log::attribute_name get_name();
 *     static create_decl_() attribute_type create();
 *   };
 * }
 *
 * typedef boost::log::expressions::attribute_keyword< tag::keyword_ > keyword_type;
 * \endcode
 *
 * The \c get_name method returns the attribute name and \c create constructs the attribute object. The \c create
 * method is only declared by this macro and must be defined elsewhere. See the \c BOOST_LOG_DEFINE_ATTRIBUTE_KEYWORD_CREATE
 * macro documentation.
 *
 * \note This macro only defines a type of the keyword. To also define the keyword object, use
 *       the \c BOOST_LOG_DECLARE_ATTRIBUTE_KEYWORD macro instead.
 *
 * \param name_ Attribute name string
 * \param keyword_ Keyword name
 * \param attr_type_ Attribute type
 * \param create_decl_ A preprocessor metafunction that expands to the attribute creation method declaration prefix.
 *                     Specify \c BOOST_PP_EMPTY if not needed.
 */
#define BOOST_LOG_DECLARE_ATTRIBUTE_KEYWORD_TYPE(name_, keyword_, attr_type_, create_decl_)\
    BOOST_LOG_DECLARE_ATTRIBUTE_KEYWORD_TYPE_IMPL(tag, name_, keyword_, attr_type_, create_decl_)

/*!
 * \brief The macro declares an attribute keyword
 *
 * The macro provides definitions similar to \c BOOST_LOG_DECLARE_ATTRIBUTE_KEYWORD_TYPE and addidionally
 * defines the keyword object.
 *
 * \param name_ Attribute name string
 * \param keyword_ Keyword name
 * \param attr_type_ Attribute type
 * \param create_decl_ A preprocessor metafunction that expands to the attribute creation method declaration prefix.
 *                     Specify \c BOOST_PP_EMPTY if not needed.
 */
#define BOOST_LOG_DECLARE_ATTRIBUTE_KEYWORD(name_, keyword_, attr_type_, create_decl_)\
    BOOST_LOG_DECLARE_ATTRIBUTE_KEYWORD_IMPL(tag, name_, keyword_, attr_type_, create_decl_)

/*!
 * \brief The macro expands into the attribute creation method definition header
 *
 * The macro should be used to define the attribute creation method declared by \c BOOST_LOG_DECLARE_ATTRIBUTE_KEYWORD_TYPE
 * or \c BOOST_LOG_DECLARE_ATTRIBUTE_KEYWORD macro. The macro should be used in the same namespace where the keyword
 * was declared. The attribute creation method body must immediately follow this macro. Usage example:
 *
 * \code
 * BOOST_LOG_DEFINE_ATTRIBUTE_KEYWORD_CREATE(BOOST_PP_EMPTY, my_timer)
 * {
 *   return attribute_type();
 * }
 * \endcode
 *
 * \param create_decl_ A preprocessor metafunction that expands to the attribute creation method definition prefix.
 *                     Specify \c BOOST_PP_EMPTY if not needed.
 * \param keyword_ Keyword name
 */
#define BOOST_LOG_DEFINE_ATTRIBUTE_KEYWORD_CREATE(create_decl_, keyword_)\
    BOOST_LOG_DEFINE_ATTRIBUTE_KEYWORD_CREATE_IMPL(create_decl_, tag, keyword_)

/*!
 * \brief The macro declares an attribute keyword type and defines an inline attribute creation method
 *
 * The attribute creation method body must immediately follow this macro, see the \c BOOST_LOG_DEFINE_ATTRIBUTE_KEYWORD_CREATE
 * macro documentation.
 *
 * \param name_ Attribute name string
 * \param keyword_ Keyword name
 * \param attr_type_ Attribute type
 */
#define BOOST_LOG_INLINE_ATTRIBUTE_KEYWORD_TYPE(name_, keyword_, attr_type_)\
    BOOST_LOG_DECLARE_ATTRIBUTE_KEYWORD_TYPE_IMPL(tag, name_, keyword_, attr_type_, BOOST_PP_EMPTY)\
    BOOST_LOG_DEFINE_ATTRIBUTE_KEYWORD_CREATE_IMPL(BOOST_PP_IDENTITY(inline), tag, keyword_)

/*!
 * \brief The macro declares an attribute keyword and defines an inline attribute creation method
 *
 * The attribute creation method body must immediately follow this macro, see the \c BOOST_LOG_DEFINE_ATTRIBUTE_KEYWORD_CREATE
 * macro documentation.
 *
 * \param name_ Attribute name string
 * \param keyword_ Keyword name
 * \param attr_type_ Attribute type
 */
#define BOOST_LOG_INLINE_ATTRIBUTE_KEYWORD(name_, keyword_, attr_type_)\
    BOOST_LOG_DECLARE_ATTRIBUTE_KEYWORD_IMPL(tag, name_, keyword_, attr_type_, BOOST_PP_EMPTY)\
    BOOST_LOG_DEFINE_ATTRIBUTE_KEYWORD_CREATE_IMPL(BOOST_PP_IDENTITY(inline), tag, keyword_)

/*!
 * \brief The macro declares an attribute keyword type and defines an inline attribute creation method
 *
 * The generated attribute creation method will default-construct the attribute.
 *
 * \param name_ Attribute name string
 * \param keyword_ Keyword name
 * \param attr_type_ Attribute type
 */
#define BOOST_LOG_INLINE_ATTRIBUTE_KEYWORD_TYPE_DEFAULT(name_, keyword_, attr_type_)\
    BOOST_LOG_INLINE_ATTRIBUTE_KEYWORD_TYPE(name_, keyword_, attr_type_)\
    {\
        return attribute_type();\
    }

/*!
 * \brief The macro declares an attribute keyword and defines an inline attribute creation method
 *
 * The generated attribute creation method will default-construct the attribute.
 *
 * \param name_ Attribute name string
 * \param keyword_ Keyword name
 * \param attr_type_ Attribute type
 */
#define BOOST_LOG_INLINE_ATTRIBUTE_KEYWORD_DEFAULT(name_, keyword_, attr_type_)\
    BOOST_LOG_INLINE_ATTRIBUTE_KEYWORD(name_, keyword_, attr_type_)\
    {\
        return attribute_type();\
    }

/*!
 * \brief The macro declares an attribute keyword type and defines an inline attribute creation method
 *
 * The generated attribute creation method will construct the attribute with the specified arguments.
 *
 * \param name_ Attribute name string
 * \param keyword_ Keyword name
 * \param attr_type_ Attribute type
 * \param args_ A preprocessor sequence of constructor arguments for the attribute
 */
#define BOOST_LOG_INLINE_ATTRIBUTE_KEYWORD_TYPE_CTOR_ARGS(name_, keyword_, attr_type_, args_)\
    BOOST_LOG_INLINE_ATTRIBUTE_KEYWORD_TYPE(name_, keyword_, attr_type_)\
    {\
        return attribute_type(BOOST_PP_SEQ_ENUM(args_));\
    }

/*!
 * \brief The macro declares an attribute keyword and defines an inline attribute creation method
 *
 * The generated attribute creation method will construct the attribute with the specified arguments.
 *
 * \param name_ Attribute name string
 * \param keyword_ Keyword name
 * \param attr_type_ Attribute type
 * \param args_ A preprocessor sequence of constructor arguments for the attribute
 */
#define BOOST_LOG_INLINE_ATTRIBUTE_KEYWORD_CTOR_ARGS(name_, keyword_, attr_type_, args_)\
    BOOST_LOG_INLINE_ATTRIBUTE_KEYWORD(name_, keyword_, attr_type_)\
    {\
        return attribute_type(BOOST_PP_SEQ_ENUM(args_));\
    }

#endif // BOOST_LOG_EXPRESSIONS_KEYWORD_HPP_INCLUDED_
