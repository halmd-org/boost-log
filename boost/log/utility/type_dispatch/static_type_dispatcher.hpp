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
#include <boost/array.hpp>
#include <boost/compatibility/cpp_c_headers/cstddef>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/inherit.hpp>
#include <boost/mpl/inherit_linearly.hpp>
#include <boost/mpl/quote.hpp>
#include <boost/mpl/lambda.hpp>
#include <boost/mpl/apply.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/utility/type_info_wrapper.hpp>
#include <boost/log/utility/type_dispatch/type_dispatcher.hpp>
#if !defined(BOOST_LOG_NO_THREADS)
#include <boost/thread/once.hpp>
#else
#include <boost/log/utility/no_unused_warnings.hpp>
#endif

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace aux {

template< typename VisitorGenT >
struct inherit_visitors
{
    template< typename NextBaseT, typename T >
    struct BOOST_LOG_NO_VTABLE apply :
        public NextBaseT,
        public VisitorGenT::BOOST_NESTED_TEMPLATE apply< T >::type
    {
        typedef apply< NextBaseT, T > type;
    };
};

} // namespace aux

//! A static type dispatcher implementation
template<
    typename TypeSequenceT,
    typename VisitorGenT = mpl::quote1< type_visitor >,
    typename RootT = type_dispatcher
>
class static_type_dispatcher :
    public mpl::inherit_linearly<
        TypeSequenceT,
        aux::inherit_visitors< typename mpl::lambda< VisitorGenT >::type >,
//        mpl::inherit< mpl::_1, typename mpl::lambda< VisitorGenT >::type::BOOST_NESTED_TEMPLATE apply< mpl::_2 > >,
        RootT
    >::type
{
    // The static type dispatcher must eventually derive from the type_dispatcher interface class
    BOOST_MPL_ASSERT((is_base_of< type_dispatcher, RootT >));

public:
    //! Type sequence of the supported types
    typedef TypeSequenceT supported_types;

private:
    //! The dispatching map
    typedef array<
        std::pair< type_info_wrapper, std::ptrdiff_t >,
        mpl::size< supported_types >::value
    > dispatching_map;

    //! An ordering predicate for the dispatching map
    struct dispatching_map_order;
    friend struct dispatching_map_order;
    struct dispatching_map_order
    {
        typedef bool result_type;
        typedef typename dispatching_map::value_type first_argument_type, second_argument_type;
        bool operator() (first_argument_type const& left, second_argument_type const& right) const
        {
            return (left.first < right.first);
        }
    };

#if !defined(BOOST_LOG_NO_THREADS)
    //! Function delegate to implement one-time initialization
    struct delegate
    {
        typedef void result_type;
        explicit delegate(static_type_dispatcher* pThis, void (static_type_dispatcher::*pFun)())
            : m_pThis(pThis), m_pFun(pFun)
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
#endif // !defined(BOOST_LOG_NO_THREADS)

    //! The wrapper metafunction is used with mpl::for_each to pass visitor type to the dispatching_map_initializer
    struct visitor_wrapper
    {
        template< typename T >
        struct apply
        {
            typedef typename mpl::apply< VisitorGenT, T >::type* type;
        };
    };
    //! The functor is used to initialize dispatching map of visitors
    struct dispatching_map_initializer;
    friend struct dispatching_map_initializer;
    struct dispatching_map_initializer
    {
        typedef void result_type;

        dispatching_map_initializer(static_type_dispatcher* pThis, typename dispatching_map::iterator& pEntry)
            : m_pThis(pThis), m_pEntry(pEntry)
        {
        }

        template< typename T >
        void operator() (T*) const
        {
            typedef typename T::supported_type supported_type;
            m_pEntry->first = typeid(supported_type);
            BOOST_LOG_ASSUME(m_pThis != NULL);
            // To honor GCC bugs we have to operate on pointers other than void*
            m_pEntry->second = std::distance(
                    reinterpret_cast< char* >(m_pThis),
                    reinterpret_cast< char* >(static_cast< type_visitor< supported_type >* >(m_pThis)));
            ++m_pEntry;
        }

    private:
        static_type_dispatcher* m_pThis;
        typename dispatching_map::iterator& m_pEntry;
    };

public:
    //! Constructor
    static_type_dispatcher()
    {
#if !defined(BOOST_LOG_NO_THREADS)
        static once_flag flag = BOOST_ONCE_INIT;
        boost::call_once(flag, delegate(this, &static_type_dispatcher::init_dispatching_map));
#else
        static bool initialized = (init_dispatching_map(), true);
        BOOST_LOG_NO_UNUSED_WARNINGS(initialized);
#endif
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
                dispatching_map_order()
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

        typename dispatching_map::iterator it = disp_map.begin();
        mpl::for_each< supported_types, visitor_wrapper >(dispatching_map_initializer(this, it));

        std::sort(disp_map.begin(), disp_map.end(), dispatching_map_order());
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
