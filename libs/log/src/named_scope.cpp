/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   named_scope.cpp
 * \author Andrey Semashev
 * \date   24.06.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#include <memory>
#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread/tss.hpp>
#include <boost/thread/once.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/attributes/basic_attribute_value.hpp>

namespace boost {

namespace log {

namespace attributes {

//! Named scope attribute implementation
template< typename CharT >
struct basic_named_scope< CharT >::implementation :
    public enable_shared_from_this< implementation >
{
    //! Pointer to the thread-specific scope stack
    thread_specific_ptr< scope_stack > pStacks;

    //! Pointer to implementation instance
    static implementation* const pInstance;

    //! The method returns current thread scope stack
    scope_stack& get_scope_stack()
    {
        register scope_stack* p = pStacks.get();
        if (!p)
        {
            std::auto_ptr< scope_stack > pNew(new scope_stack);
            pStacks.reset(pNew.get());
            p = pNew.release();
        }

        return *p;
    }

private:
    implementation() {}
    implementation(implementation const&);
    implementation& operator= (implementation const&);

    //! The function initializes named scope attribute internals
    static implementation* init_named_scope()
    {
        static once_flag flag = BOOST_ONCE_INIT;
        call_once(&implementation::init_instance, flag);
        return get_instance().get();
    }
    //! The reference holder function
    static shared_ptr< implementation >& get_instance()
    {
        static shared_ptr< implementation > p(new implementation);
        return p;
    }
    //! Instance initializer
    static void init_instance()
    {
        get_instance();
    }
};

template< typename CharT >
typename basic_named_scope< CharT >::implementation* const
basic_named_scope< CharT >::implementation::pInstance =
    basic_named_scope< CharT >::implementation::init_named_scope();


//! Constructor
template< typename CharT >
basic_named_scope< CharT >::basic_named_scope()
    : pImpl(implementation::pInstance->shared_from_this())
{
}

//! The method returns the actual attribute value. It must not return NULL.
template< typename CharT >
shared_ptr< attribute_value > basic_named_scope< CharT >::get_value()
{
    // We need the copy since the attribute value may get passed to another thread
    // in case of asynchronous logging, and while being logged the thread may
    // eventually change its scope and even finish. The copy is supposed to be
    // cheap, since there are only pointers to string literals in the scope stack.
    return shared_ptr< attribute_value >(
        new basic_attribute_value< scope_stack >(pImpl->get_scope_stack()));
}

//! The method pushes the scope to the stack
template< typename CharT >
void basic_named_scope< CharT >::push_scope(scope_name const& name)
{
    scope_stack& s = implementation::pInstance->get_scope_stack();
    s.push_back(name);
}

//! The method pops the top scope
template< typename CharT >
void basic_named_scope< CharT >::pop_scope()
{
    scope_stack& s = implementation::pInstance->get_scope_stack();
    s.pop_back();
}

//! Explicitly instantiate named_scope implementation
template class BOOST_LOG_EXPORT basic_named_scope< char >;
template class BOOST_LOG_EXPORT basic_named_scope< wchar_t >;

} // namespace attributes

} // namespace log

} // namespace boost
