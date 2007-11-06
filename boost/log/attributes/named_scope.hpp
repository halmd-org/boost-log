/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   named_scope.hpp
 * \author Andrey Semashev
 * \date   24.06.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_ATTRIBUTES_NAMED_SCOPE_HPP_INCLUDED_
#define BOOST_LOG_ATTRIBUTES_NAMED_SCOPE_HPP_INCLUDED_

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/current_function.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/detail/string_literal.hpp>

#ifdef _MSC_VER
#pragma warning(push)
// non dll-interface class 'A' used as base for dll-interface class 'B'
#pragma warning(disable: 4275)
// 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
#endif // _MSC_VER

namespace boost {

namespace log {

namespace attributes {

//! A class of an attribute that holds stack of named scopes of the current thread
template< typename CharT >
class BOOST_LOG_EXPORT basic_named_scope :
    public attribute
{
public:
    //! Character type
    typedef CharT char_type;
    //! Scope name type
    typedef basic_string_literal< char_type > scope_name;
    //! Scope names stack
    typedef std::vector< scope_name > scope_stack;

    //! Sentry object class to automatically push and pop scopes
    struct sentry
    {
        //! Attribute type
        typedef basic_named_scope< char_type > named_scope_type;

        //! Constructor
        template< typename T, std::size_t LenV >
        explicit sentry(T(&name)[LenV])
        {
            named_scope_type::push_scope(typename named_scope_type::scope_name(name));
        }

        //! Destructor
        ~sentry()
        {
            named_scope_type::pop_scope();
        }
    };

private:
    //! Attribute implementation class
    struct implementation;

private:
    //! Pointer to the implementation
    shared_ptr< implementation > pImpl;

public:
    //! Constructor
    basic_named_scope();

    //! The method returns the actual attribute value. It must not return NULL.
    shared_ptr< attribute_value > get_value();

    //! The method pushes the scope to the stack
    static void push_scope(scope_name const& name);
    //! The method pops the top scope
    static void pop_scope();
};

typedef basic_named_scope< char > named_scope;
typedef basic_named_scope< wchar_t > wnamed_scope;

} // namespace attributes

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#define BOOST_LOG_UNIQUE_IDENTIFIER_NAME_INTERNAL_(prefix, postfix)\
    BOOST_PP_CAT(prefix, postfix)
#define BOOST_LOG_UNIQUE_IDENTIFIER_NAME_INTERNAL(prefix, postfix)\
    BOOST_LOG_UNIQUE_IDENTIFIER_NAME_INTERNAL_(prefix, postfix)

// In VC 7.0 and later when compiling with /ZI option __LINE__ macro is corrupted
#if BOOST_WORKAROUND(BOOST_MSVC, >=1300)
#  define BOOST_LOG_UNIQUE_IDENTIFIER_NAME(prefix)\
    BOOST_LOG_UNIQUE_IDENTIFIER_NAME_INTERNAL(prefix, __COUNTER__)
#else
#  define BOOST_LOG_UNIQUE_IDENTIFIER_NAME(prefix)\
    BOOST_LOG_UNIQUE_IDENTIFIER_NAME_INTERNAL(prefix, __LINE__)
#endif // BOOST_WORKAROUND(BOOST_MSVC, >= 1300)

//! Macro for scope markup
#define BOOST_LOG_NAMED_SCOPE(name)\
    ::boost::log::attributes::named_scope::sentry BOOST_LOG_UNIQUE_IDENTIFIER_NAME(_boost_log_named_scope_sentry_)(name)

//! Macro for scope markup
#define BOOST_LOG_WNAMED_SCOPE(name)\
    ::boost::log::attributes::wnamed_scope::sentry BOOST_LOG_UNIQUE_IDENTIFIER_NAME(_boost_log_named_scope_sentry_)(name)

//! Macro for function scope markup
#define BOOST_LOG_FUNCTION() BOOST_LOG_NAMED_SCOPE(BOOST_CURRENT_FUNCTION)

//! Macro for function scope markup
#define BOOST_LOG_WFUNCTION() BOOST_LOG_WNAMED_SCOPE(BOOST_PP_CAT(L, BOOST_CURRENT_FUNCTION))

#endif // BOOST_LOG_ATTRIBUTES_NAMED_SCOPE_HPP_INCLUDED_
