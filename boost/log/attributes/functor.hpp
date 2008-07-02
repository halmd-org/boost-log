/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   functor.hpp
 * \author Andrey Semashev
 * \date   24.06.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_ATTRIBUTES_FUNCTOR_HPP_INCLUDED_
#define BOOST_LOG_ATTRIBUTES_FUNCTOR_HPP_INCLUDED_

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/static_assert.hpp>
#include <boost/utility/result_of.hpp>
#include <boost/type_traits/is_void.hpp>
#include <boost/type_traits/remove_cv.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/attributes/basic_attribute_value.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace attributes {

#ifndef BOOST_LOG_DOXYGEN_PASS

//! A class of an attribute that acquires its value from a third-party functor
template< typename R, typename T >
class functor :
    public attribute
{
public:
    //! A held functor type
    typedef T held_type;

private:
    //! Attribute value type
    typedef basic_attribute_value< R > functor_result_value;

private:
    //! Functor that returns attribute values
    const held_type m_Functor;

public:
    //! Constructor
    explicit functor(held_type const& fun) : m_Functor(fun) {}

    //! The method returns the actual attribute value. It must not return NULL.
    shared_ptr< attribute_value > get_value()
    {
        return boost::make_shared< functor_result_value >(m_Functor());
    }
};

#endif // BOOST_LOG_DOXYGEN_PASS

#ifndef BOOST_NO_RESULT_OF

//  Generator functions
template< typename T >
inline shared_ptr< attribute > make_functor_attr(T const& fun)
{
    typedef typename remove_cv<
        typename remove_reference<
            typename result_of< T() >::type
        >::type
    >::type result_type;
    BOOST_STATIC_ASSERT(!is_void< result_type >::value);

    typedef functor< result_type, T > functor_t;
    return boost::make_shared< functor_t >(fun);
}

#endif // BOOST_NO_RESULT_OF

template< typename R, typename T >
inline shared_ptr< attribute > make_functor_attr(T const& fun)
{
    typedef typename remove_cv<
        typename remove_reference< R >::type
    >::type result_type;
    BOOST_STATIC_ASSERT(!is_void< result_type >::value);

    typedef functor< result_type, T > functor_t;
    return boost::make_shared< functor_t >(fun);
}

} // namespace attributes

} // namespace log

} // namespace boost

#endif // BOOST_LOG_ATTRIBUTES_FUNCTOR_HPP_INCLUDED_
