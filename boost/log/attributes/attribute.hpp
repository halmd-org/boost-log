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
#include <boost/log/utility/type_dispatch/type_dispatcher.hpp>

namespace boost {

namespace log {

//! A base class for an attribute value
struct BOOST_LOG_NO_VTABLE attribute_value
{
private:
    //! A simple type dispatcher to support the get method
    template< typename T >
    struct extractor :
        public type_dispatcher,
        public type_visitor< T >
    {
        //! Constructor
        explicit extractor(optional< T const& >& res) : res_(res) {}
        //! Returns itself if the value type matches the requested type
        void* get_visitor(std::type_info const& type)
        {
            BOOST_LOG_ASSUME(this != 0);
            if (type == typeid(T))
                return static_cast< type_visitor< T >* >(this);
            else
                return 0;
        }
        //! Extracts the value
        void visit(T const& value)
        {
            res_ = value;
        }

    private:
        //! The reference to the extracted value
        optional< T const& >& res_;
    };

public:
    virtual ~attribute_value() {}

    //! The method dispatches the value to the given object. It returns true if the
    //! object was capable to consume the real attribute value type and false otherwise.
    virtual bool dispatch(type_dispatcher& dispatcher) = 0;

    //! The method is called when the attribute value is passed to another thread (e.g.
    //! in case of asynchronous logging). The value should ensure it properly owns all thread-specific data.
    virtual shared_ptr< attribute_value > detach_from_thread() = 0;

    //! A simpler way to get attribute value in case if one knows its exact type
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

        result_type res;
        extractor< requested_type > disp(res);
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
