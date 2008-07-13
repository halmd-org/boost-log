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
 * \file   type_info_wrapper.hpp
 * \author Andrey Semashev
 * \date   15.04.2007
 * 
 * The header contains implementation of a type information wrapper.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_UTILITY_TYPE_INFO_WRAPPER_HPP_INCLUDED_
#define BOOST_LOG_UTILITY_TYPE_INFO_WRAPPER_HPP_INCLUDED_

#ifdef __GNUC__
#include <cxxabi.h>
#include <memory.h>
#endif // __GNUC__
#include <typeinfo>
#include <string>
#include <algorithm>
#include <boost/operators.hpp>
#include <boost/log/detail/prologue.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

//! A simple type_info wrapper that implements value semantic to the type information
class type_info_wrapper
    //! \cond
    : public partially_ordered< type_info_wrapper,
        equality_comparable< type_info_wrapper >
    >
    //! \endcond
{
private:
    //! An inaccessible type to indicate an uninitialized state of the wrapper
    enum uninitialized {};

#ifndef BOOST_NO_UNSPECIFIED_BOOL
    struct dummy
    {
        int data1;
        int data2;
    };

    typedef int (dummy::*unspecified_bool);
#endif // BOOST_NO_UNSPECIFIED_BOOL

#ifdef __GNUC__
    //! A simple scope guard for automatic memory free
    struct auto_free
    {
        explicit auto_free(void* p) : p_(p) {}
        ~auto_free() { free(p_); }
    private:    
        void* p_;
    };
#endif // __GNUC__

private:
    //! A pointer to the actual type info
    std::type_info const* info;

public:
    //! Default constructor (leaves object in an unusable state)
    type_info_wrapper() : info(&typeid(uninitialized)) {}
    //! Copy constructor
    type_info_wrapper(type_info_wrapper const& that) : info(that.info) {}
    //! Conversion constructor
    type_info_wrapper(std::type_info const& that) : info(&that) {}

#ifdef BOOST_NO_UNSPECIFIED_BOOL
    operator bool () const { return (*info != typeid(uninitialized)); }
#else
    operator unspecified_bool() const
    {
        if (*info != typeid(uninitialized))
            return &dummy::data2;
        else
            return 0;
    }
#endif // BOOST_NO_UNSPECIFIED_BOOL

    //! Type info getter
    std::type_info const& get() const { return *info; }

    //! Swaps two instances of the wrapper
    void swap(type_info_wrapper& that)
    {
        std::swap(info, that.info);
    }

    //! The method returns the contained type name string in a possibly more readable format than get().name()
    std::string pretty_name() const
    {
        if (*info != typeid(uninitialized))
        {
#ifdef __GNUC__
            // GCC returns decorated type name, will need to demangle it using ABI
            int status = 0;
            size_t size = 0;
            const char* name = info->name();
            char* undecorated = abi::__cxa_demangle(name, NULL, &size, &status);
            auto_free _(undecorated);

            if (undecorated)
                return undecorated;
            else
                return name;
#else
            return info->name();
#endif
        }
        else
            return "[uninitialized]";
    }

#ifdef _MSC_VER
#pragma warning(push)
// 'int' : forcing value to bool 'true' or 'false' (performance warning)
#pragma warning(disable: 4800)
#endif

    //! Initialized state checker
    bool operator! () const { return (*info == typeid(uninitialized)); }

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

//! Free swap for type info wrapper
inline void swap(type_info_wrapper& left, type_info_wrapper& right)
{
    left.swap(right);
}

} // namespace log

} // namespace boost

#endif // BOOST_LOG_UTILITY_TYPE_INFO_WRAPPER_HPP_INCLUDED_
