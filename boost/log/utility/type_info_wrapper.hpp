/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   type_info_wrapper.hpp
 * \author Andrey Semashev
 * \date   15.04.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_UTILITY_TYPE_INFO_WRAPPER_HPP_INCLUDED_
#define BOOST_LOG_UTILITY_TYPE_INFO_WRAPPER_HPP_INCLUDED_

#include <typeinfo>
#include <boost/operators.hpp>
#include <boost/log/detail/prologue.hpp>

namespace boost {

namespace log {

#if (defined __SUNPRO_CC) && (__SUNPRO_CC <= 0x530) && !(defined BOOST_NO_COMPILER_CONFIG)
    // Sun C++ 5.3 can't handle the safe_bool idiom, so don't use it
#    define BOOST_NO_UNSPECIFIED_BOOL
#endif // (defined __SUNPRO_CC) && (__SUNPRO_CC <= 0x530) && !(defined BOOST_NO_COMPILER_CONFIG)

//! A simple type_info wrapper that implements value semantic to the type information
class type_info_wrapper :
    public partially_ordered< type_info_wrapper,
        equality_comparable< type_info_wrapper >
    >
{
private:
    //! An inaccessible type to indicate an uninitialized state of the wrapper
    enum uninitialized_state {};

#ifndef BOOST_NO_UNSPECIFIED_BOOL
    struct dummy
    {
        int data1;
        int data2;
    };

    typedef int (dummy::*unspecified_bool);
#endif // BOOST_NO_UNSPECIFIED_BOOL

private:
    //! A pointer to the actual type info
    std::type_info const* info;

public:
    //! Default constructor (leaves object in an unusable state)
    type_info_wrapper() : info(&typeid(uninitialized_state)) {}
    //! Copy constructor
    type_info_wrapper(type_info_wrapper const& that) : info(that.info) {}
    //! Conversion constructor
    type_info_wrapper(std::type_info const& that) : info(&that) {}

#ifdef BOOST_NO_UNSPECIFIED_BOOL
    operator bool () const { return (*info != typeid(uninitialized_state)); }
#else
    operator unspecified_bool() const
    {
        if (*info != typeid(uninitialized_state))
            return &dummy::data2;
        else
            return 0;
    }
#endif // BOOST_NO_UNSPECIFIED_BOOL

    //! Type info getter
    std::type_info const& get() const { return *info; }

#ifdef _MSC_VER
#pragma warning(push)
// 'int' : forcing value to bool 'true' or 'false' (performance warning)
#pragma warning(disable: 4800)
#endif

    //! Initialized state checker
    bool operator! () const { return (*info == typeid(uninitialized_state)); }

    //! Equality comparison
    bool operator== (type_info_wrapper const& that) const
    {
        return (*info == *that.info);
    }
    //! Ordering operator
    bool operator< (type_info_wrapper const& that) const
    {
        return static_cast< bool >(info->before(*that.info));
    }

#ifdef _MSC_VER
#pragma warning(pop)
#endif

};

#undef BOOST_NO_UNSPECIFIED_BOOL

} // namespace log

} // namespace boost

#endif // BOOST_LOG_UTILITY_TYPE_INFO_WRAPPER_HPP_INCLUDED_
