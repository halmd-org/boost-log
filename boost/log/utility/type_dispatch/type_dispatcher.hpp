/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   type_dispatcher.hpp
 * \author Andrey Semashev
 * \date   15.04.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_TYPE_DISPATCHER_HPP_INCLUDED_
#define BOOST_LOG_TYPE_DISPATCHER_HPP_INCLUDED_

#include <typeinfo>
#include <boost/mpl/aux_/lambda_support.hpp>
#include <boost/log/detail/prologue.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

//! An interface to the concrete type visitor
template< typename T >
struct BOOST_LOG_NO_VTABLE type_visitor
{
    //! The type, which the visitor is able to consume
    typedef T supported_type;

    //! The method invokes the visitor-specific logic with the given value
    virtual void visit(T const& value) = 0;

    BOOST_MPL_AUX_LAMBDA_SUPPORT(1, type_visitor, (T))
};

//! A type dispatcher interface
struct BOOST_LOG_NO_VTABLE type_dispatcher
{
public:
    //! The method returns the type-specific visitor or NULL, if the type is not supported
    template< typename T >
    type_visitor< T >* get_visitor(BOOST_EXPLICIT_TEMPLATE_TYPE(T))
    {
        return reinterpret_cast< type_visitor< T >* >(
            this->get_visitor(typeid(T)));
    }

private:
    //! The get_visitor method implementation
    virtual void* get_visitor(std::type_info const& type) = 0;
};

} // namespace log

} // namespace boost

#endif // BOOST_LOG_TYPE_DISPATCHER_HPP_INCLUDED_
