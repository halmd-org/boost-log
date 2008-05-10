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
#include <iterator>
#include <algorithm>
#include <functional>
#include <boost/array.hpp>
#include <boost/compatibility/cpp_c_headers/cstddef>
#include <boost/mpl/begin.hpp>
#include <boost/mpl/end.hpp>
#include <boost/mpl/next.hpp>
#include <boost/mpl/deref.hpp>
#include <boost/mpl/size.hpp>
#include <boost/thread/once.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/utility/type_info_wrapper.hpp>
#include <boost/log/utility/type_dispatch/type_dispatcher.hpp>

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
        void init(void* pthis, std::pair< type_info_wrapper, std::ptrdiff_t >* p)
        {
            p->first = typeid(supported_type);
            BOOST_LOG_ASSUME(this != NULL);
            // To honor GCC bugs we have to operate on pointers other than void*
            p->second = std::distance(
                    reinterpret_cast< char* >(pthis),
                    reinterpret_cast< char* >(static_cast< type_visitor< supported_type >* >(this)));

            base_type::init(pthis, ++p);
        }
    };

    //! A specialization to end the recursion
    template< typename EndT >
    class static_type_dispatcher_visitor< EndT, EndT >
    {
    protected:
        //! Visitor registrar
        void init(void*, std::pair< type_info_wrapper, std::ptrdiff_t >*)
        {
        }
    };

    //! An ordering predicate for the dispatching map
    struct dispatching_map_order :
        public std::binary_function<
            std::pair< type_info_wrapper, std::ptrdiff_t >,
            std::pair< type_info_wrapper, std::ptrdiff_t >,
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
        std::pair< type_info_wrapper, std::ptrdiff_t >,
        mpl::size< supported_types >::value
    > dispatching_map;

    //! Dunction delegate to implement one-time initialization
    struct delegate
    {
        typedef void result_type;
        explicit delegate(static_type_dispatcher* pthis, void (static_type_dispatcher::*pfun)())
            : m_pThis(pthis), m_pFun(pfun)
        {
        }
        result_type operator()() const
        {
            (m_pThis->*m_pFun)();
        }

    private:
        static_type_dispatcher* m_pThis;
        void (static_type_dispatcher::*m_pFun)();
    };

public:
    //! Constructor
    static_type_dispatcher()
    {
        static once_flag flag = BOOST_ONCE_INIT;
        boost::call_once(flag, delegate(this, &static_type_dispatcher::init_dispatching_map));
    }

private:
    //! The get_visitor method implementation
    void* get_visitor(std::type_info const& type)
    {
        dispatching_map const& disp_map = get_dispatching_map();
        type_info_wrapper wrapper(type);
        typename dispatching_map::const_iterator it =
            std::lower_bound(
                disp_map.begin(),
                disp_map.end(),
                std::make_pair(wrapper, std::ptrdiff_t(0)),
                aux::dispatching_map_order()
            );

        if (it != disp_map.end() && it->first == wrapper)
        {
            // To honor GCC bugs we have to operate on pointers other than void*
            char* pthis = reinterpret_cast< char* >(this);
            std::advance(pthis, it->second);
            return pthis;
        }
        else
            return NULL;
    }
    //! The method initializes the dispatching map
    void init_dispatching_map()
    {
        dispatching_map& disp_map = get_dispatching_map();
        base_type::init(this, disp_map.c_array());
        std::sort(disp_map.begin(), disp_map.end(), aux::dispatching_map_order());
    }
    //! The method returns the dispatching map instance
    static dispatching_map& get_dispatching_map()
    {
        static dispatching_map instance;
        return instance;
    }
};

} // namespace log

} // namespace boost

#endif // BOOST_LOG_STATIC_TYPE_DISPATCHER_HPP_INCLUDED_
