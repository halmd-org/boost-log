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
#include <boost/mpl/inherit_linearly.hpp>
#include <boost/mpl/quote.hpp>
#include <boost/mpl/lambda.hpp>
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

//! A simple metafunction class to inherit visitors in a static dispatcher
template< typename VisitorGenT >
struct inherit_visitors
{
    template< typename NextBaseT, typename T >
    struct BOOST_LOG_NO_VTABLE apply :
        public NextBaseT,
        public VisitorGenT::BOOST_NESTED_TEMPLATE apply< T >::type
    {
        typedef apply< NextBaseT, T > type;

    protected:
        //! The function is used to initialize dispatching map of supported types
        void init_visitor(void* pThis, std::pair< type_info_wrapper, std::ptrdiff_t >* pEntry)
        {
            typedef T supported_type;
            pEntry->first = typeid(supported_type);
            BOOST_LOG_ASSUME(this != NULL);
            // To honor GCC bugs we have to operate on pointers other than void*
            pEntry->second = std::distance(
                    reinterpret_cast< char* >(pThis),
                    reinterpret_cast< char* >(static_cast< type_visitor< supported_type >* >(this)));
            NextBaseT::init_visitor(pThis, ++pEntry);
        }
    };
};

//! The class ends recursion of the visitors inheritance hierarchy
template< typename BaseT >
class BOOST_LOG_NO_VTABLE static_type_dispatcher_base :
    public BaseT
{
protected:
    void init_visitor(void*, std::pair< type_info_wrapper, std::ptrdiff_t >*) {}
};

//! Ordering predicate for type dispatching map
struct dispatching_map_order
{
    typedef bool result_type;
    typedef std::pair< type_info_wrapper, std::ptrdiff_t > first_argument_type, second_argument_type;
    bool operator() (first_argument_type const& left, second_argument_type const& right) const
    {
        return (left.first < right.first);
    }
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
        // Usage of this class instead of mpl::inherit provides substantial improvement in compilation time,
        // binary size and compiler memory footprint on some compilers (for instance, ICL)
        aux::inherit_visitors< typename mpl::lambda< VisitorGenT >::type >,
        aux::static_type_dispatcher_base< RootT >
    >::type
{
    // The static type dispatcher must eventually derive from the type_dispatcher interface class
    BOOST_MPL_ASSERT((is_base_of< type_dispatcher, RootT >));

    typedef typename mpl::inherit_linearly<
        TypeSequenceT,
        aux::inherit_visitors< typename mpl::lambda< VisitorGenT >::type >,
        aux::static_type_dispatcher_base< RootT >
    >::type base_type;

public:
    //! Type sequence of the supported types
    typedef TypeSequenceT supported_types;

private:
    //! The dispatching map
    typedef array<
        std::pair< type_info_wrapper, std::ptrdiff_t >,
        mpl::size< supported_types >::value
    > dispatching_map;

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
    using base_type::init_visitor;

    //! The get_visitor method implementation
    void* get_visitor(std::type_info const& type)
    {
        dispatching_map const& disp_map = get_dispatching_map();
        type_info_wrapper wrapper(type);
        typename dispatching_map::value_type const* begin = &*disp_map.begin();
        typename dispatching_map::value_type const* end = begin + disp_map.size();
        typename dispatching_map::value_type const* it =
            std::lower_bound(
                begin,
                end,
                std::make_pair(wrapper, std::ptrdiff_t(0)),
                aux::dispatching_map_order()
            );

        if (it != end && it->first == wrapper)
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
        typename dispatching_map::value_type* begin = &*disp_map.begin();
        typename dispatching_map::value_type* end = begin + disp_map.size();

        base_type::init_visitor(static_cast< void* >(this), begin);
        std::sort(begin, end, aux::dispatching_map_order());
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
