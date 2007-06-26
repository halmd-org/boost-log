/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   static_type_dispatcher.hpp
 * \author Andrey Semashev
 * \date   15.04.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_STATIC_TYPE_DISPATCHER_HPP_INCLUDED_
#define BOOST_LOG_STATIC_TYPE_DISPATCHER_HPP_INCLUDED_

#include <utility>
#include <algorithm>
#include <functional>
#include <boost/array.hpp>
#include <boost/mpl/begin.hpp>
#include <boost/mpl/end.hpp>
#include <boost/mpl/next.hpp>
#include <boost/mpl/deref.hpp>
#include <boost/mpl/size.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/type_dispatch/type_dispatcher.hpp>
#include <boost/log/detail/type_info_wrapper.hpp>

namespace boost {

namespace log {

namespace aux {

    //! A single type visitor implementation
    template< typename ItT, typename EndT >
    class BOOST_LOG_NO_VTABLE static_type_dispatcher_visitor :
        public type_visitor< typename mpl::deref< ItT >::type >,
        public static_type_dispatcher_visitor<
            typename mpl::next< ItT >::type,
            EndT
        >
    {
        //! Base type
        typedef static_type_dispatcher_visitor<
            typename mpl::next< ItT >::type,
            EndT
        > base_type;

    public:
        //! The single supported type
        typedef typename mpl::deref< ItT >::type supported_type;

    protected:
        //! Visitor registrar
        void init(std::pair< type_info_wrapper, void* >* p)
        {
            p->first = typeid(supported_type);
            BOOST_LOG_ASSUME(this != NULL);
            p->second = static_cast< type_visitor< supported_type >* >(this);

            base_type::init(++p);
        }
    };

    //! A specialization to end the recursion
    template< typename EndT >
    class static_type_dispatcher_visitor< EndT, EndT >
    {
    protected:
        //! Visitor registrar
        void init(std::pair< type_info_wrapper, void* >*)
        {
        }
    };

    //! An ordering predicate for the dispatching map
    struct dispatching_map_order :
        public std::binary_function<
            std::pair< type_info_wrapper, void* >,
            std::pair< type_info_wrapper, void* >,
            bool
        >
    {
        bool operator() (first_argument_type const& left, second_argument_type const& right) const
        {
            return (left.first < right.first);
        }
    };

} // namespace aux

//! A static type dispatcher implementation
template< typename TypeSequenceT >
class BOOST_LOG_NO_VTABLE static_type_dispatcher :
    public type_dispatcher,
    private aux::static_type_dispatcher_visitor<
        typename mpl::begin< TypeSequenceT >::type,
        typename mpl::end< TypeSequenceT >::type
    >
{
    //! Base type
    typedef aux::static_type_dispatcher_visitor<
        typename mpl::begin< TypeSequenceT >::type,
        typename mpl::end< TypeSequenceT >::type
    > base_type;

public:
    //! Type sequence of the supported types
    typedef TypeSequenceT supported_types;

private:
    //! The dispatching map
    typedef array<
        std::pair< aux::type_info_wrapper, void* >,
        mpl::size< supported_types >::value
    > dispatching_map;
    dispatching_map m_DispatchingMap;

public:
    //! Constructor
    static_type_dispatcher()
    {
        base_type::init(m_DispatchingMap.c_array());
        std::sort(m_DispatchingMap.begin(), m_DispatchingMap.end(), aux::dispatching_map_order());
    }
    //! Copying constructor
    static_type_dispatcher(static_type_dispatcher const&)
    {
        base_type::init(m_DispatchingMap.c_array());
        std::sort(m_DispatchingMap.begin(), m_DispatchingMap.end(), aux::dispatching_map_order());
    }

    //! Assignment
    static_type_dispatcher& operator= (static_type_dispatcher const&)
    {
        return *this;
    }

private:
    //! The get_visitor method implementation
    void* get_visitor(std::type_info const& type)
    {
        aux::type_info_wrapper wrapper(type);
        typename dispatching_map::iterator it =
            std::lower_bound(
                m_DispatchingMap.begin(),
                m_DispatchingMap.end(),
                std::make_pair(wrapper, (void*)NULL),
                aux::dispatching_map_order()
            );

        if (it != m_DispatchingMap.end() && it->first == wrapper)
            return it->second;
        else
            return NULL;
    }
};

} // namespace log

} // namespace boost

#endif // BOOST_LOG_STATIC_TYPE_DISPATCHER_HPP_INCLUDED_
