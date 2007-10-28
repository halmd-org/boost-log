/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   attribute.hpp
 * \author Andrey Semashev
 * \date   15.04.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_ATTRIBUTE_HPP_INCLUDED_
#define BOOST_LOG_ATTRIBUTE_HPP_INCLUDED_

#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits/add_const.hpp>
#include <boost/type_traits/add_reference.hpp>
#include <boost/type_traits/remove_cv.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/type_dispatch/type_dispatcher.hpp>

namespace boost {

namespace log {

//! A base class for an attribute value
struct BOOST_LOG_NO_VTABLE attribute_value
{
    virtual ~attribute_value() {}

    //! The method dispatches the value to the given object. It returns true if the
    //! object was capable to consume the real attribute value type and false otherwise.
    virtual bool dispatch(type_dispatcher& dispatcher) = 0;

    //! A simplier way to get attribute value in case if one knows its exact type
    template< typename T >
    optional<
        typename add_reference<
            typename add_const<
                typename remove_cv<
                    typename remove_reference< T >::type
                >::type
            >::type
        >::type
    > get()
    {
        typedef typename remove_cv<
            typename remove_reference< T >::type
        >::type requested_type;
        typedef optional<
            typename add_reference<
                typename add_const< requested_type >::type
            >::type
        > result_type;

        struct local_type_dispatcher :
            public type_dispatcher,
            public type_visitor< requested_type >
        {
            explicit local_type_dispatcher(result_type& res) : res_(res) {}
            void* get_visitor(std::type_info const& type)
            {
                if (type == typeid(requested_type))
                    return static_cast< type_visitor< requested_type >* >(this);
                else
                    return 0;
            }
            void visit(requested_type const& value)
            {
                res_ = value;
            }

        private:
            result_type& res_;
        };

        result_type res;
        local_type_dispatcher disp(res);
        this->dispatch(disp);
        return res;
    }
};

//! A base class for an attribute
struct BOOST_LOG_NO_VTABLE attribute
{
    virtual ~attribute() {}

    //! The method returns the actual attribute value. It must not return NULL.
    virtual shared_ptr< attribute_value > get_value() = 0;
};

} // namespace log

} // namespace boost

#endif // BOOST_LOG_ATTRIBUTE_HPP_INCLUDED_
