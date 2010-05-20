/*
 *          Copyright Andrey Semashev 2007 - 2010.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   static_type_dispatcher.hpp
 * \author Andrey Semashev
 * \date   15.04.2007
 *
 * The header contains implementation of a compile-time type dispatcher.
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
#include <boost/static_assert.hpp>
#include <boost/compatibility/cpp_c_headers/cstddef>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/quote.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/utility/addressof.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/detail/visible_type.hpp>
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

//! Ordering predicate for type dispatching map
struct dispatching_map_order
{
    typedef bool result_type;
    typedef std::pair< type_info_wrapper, void* > first_argument_type, second_argument_type;
    bool operator() (first_argument_type const& left, second_argument_type const& right) const
    {
        return (left.first < right.first);
    }
};

//! Dispatching map filler
template< typename ReceiverT >
struct dispatching_map_initializer
{
    typedef void result_type;

    explicit dispatching_map_initializer(std::pair< type_info_wrapper, void* >*& p) : m_p(p)
    {
    }

    template< typename T >
    void operator() (visible_type< T >) const
    {
        m_p->first = typeid(visible_type< T >);

        typedef void (*trampoline_t)(void*, T const&);
        BOOST_STATIC_ASSERT(sizeof(trampoline_t) == sizeof(void*));
        union
        {
            void* as_pvoid;
            trampoline_t as_trampoline;
        }
        caster;
        caster.as_trampoline = &type_visitor_base::trampoline< ReceiverT, T >;
        m_p->second = caster.as_pvoid;

        ++m_p;
    }

private:
    std::pair< type_info_wrapper, void* >*& m_p;
};

} // namespace aux

/*!
 * \brief A static type dispatcher class
 *
 * The type dispatcher can be used to pass objects of arbitrary types from one
 * component to another. With regard to the library, the type dispatcher
 * can be used to extract attribute values.
 *
 * Static type dispatchers allow to specify set of supported types at compile
 * time. The class is parametrized with the following template parameters:
 *
 * \li \c TypeSequenceT - an MPL type sequence of types that need to be supported
 *     by the dispatcher
 * \li \c VisitorGenT - an MPL unary metafunction class that will be used to
 *     generate concrete visitors. The metafunction will be applied to each
 *     type in the \c TypeSequenceT type sequence.
 * \li \c RootT - the ultimate base class of the dispatcher
 *
 * Users can either derive their classes from the dispatcher and override
 * \c visit methods for all the supported types, or specify in the \c VisitorGenT
 * template parameter a custom unary metafunction class that will generate
 * visitors for all supported types (each generated visitor must derive from
 * the appropriate \c type_visitor instance in this case). Users can also
 * specify a custom base class for the dispatcher in the \c RootT template
 * parameter (the base class, however, must derive from the \c type_dispatcher
 * interface).
 */
template< typename TypeSequenceT >
class static_type_dispatcher :
    public type_dispatcher
{
public:
    //! Type sequence of the supported types
    typedef TypeSequenceT supported_types;

private:
#ifndef BOOST_LOG_DOXYGEN_PASS

    //! The dispatching map
    typedef array<
        std::pair< type_info_wrapper, void* >,
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

private:
    //! Pointer to the receiver function
    void* m_pReceiver;

#endif // BOOST_LOG_DOXYGEN_PASS

public:
    /*!
     * Constructor. Initializes the dispatcher internals.
     */
    template< typename ReceiverT >
    explicit static_type_dispatcher(ReceiverT& receiver) :
        m_pReceiver((void*)boost::addressof(receiver))
    {
#if !defined(BOOST_LOG_NO_THREADS)
        static once_flag flag = BOOST_ONCE_INIT;
        boost::call_once(flag, delegate(this, &static_type_dispatcher::init_dispatching_map< ReceiverT >));
#else
        static bool initialized = (init_dispatching_map< ReceiverT >(), true);
        BOOST_LOG_NO_UNUSED_WARNINGS(initialized);
#endif
    }

private:
#ifndef BOOST_LOG_DOXYGEN_PASS

    //  Copying prohibited
    static_type_dispatcher(static_type_dispatcher const&);
    static_type_dispatcher& operator= (static_type_dispatcher const&);

    //! The get_visitor method implementation
    boost::log::aux::type_visitor_base get_visitor(std::type_info const& type)
    {
        dispatching_map const& disp_map = get_dispatching_map();
        type_info_wrapper wrapper(type);
        typename dispatching_map::value_type const* begin = &*disp_map.begin();
        typename dispatching_map::value_type const* end = begin + disp_map.size();
        typename dispatching_map::value_type const* it =
            std::lower_bound(
                begin,
                end,
                std::make_pair(wrapper, (void*)0),
                aux::dispatching_map_order()
            );

        if (it != end && it->first == wrapper)
            return boost::log::aux::type_visitor_base(m_pReceiver, it->second);
        else
            return boost::log::aux::type_visitor_base();
    }

    //! The method initializes the dispatching map
    template< typename ReceiverT >
    void init_dispatching_map()
    {
        dispatching_map& disp_map = get_dispatching_map();
        typename dispatching_map::value_type* p = &*disp_map.begin();

        mpl::for_each< supported_types, mpl::quote1< aux::visible_type > >(
            boost::log::aux::dispatching_map_initializer< ReceiverT >(p));

        std::sort(disp_map.begin(), disp_map.end(), aux::dispatching_map_order());
    }
    //! The method returns the dispatching map instance
    static dispatching_map& get_dispatching_map()
    {
        static dispatching_map instance;
        return instance;
    }

#endif // BOOST_LOG_DOXYGEN_PASS
};

} // namespace log

} // namespace boost

#endif // BOOST_LOG_STATIC_TYPE_DISPATCHER_HPP_INCLUDED_
