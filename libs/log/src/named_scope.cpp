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
#include <boost/thread/tss.hpp>
#include <boost/thread/once.hpp>
#include <boost/log/attributes/named_scope.hpp>

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
        call_once(&implementation::get_instance, flag);
        return get_instance().get();
    }
    //! The reference holder function
    static shared_ptr< implementation >& get_instance()
    {
        static shared_ptr< implementation > p(new implementation);
        return p;
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
    return this->shared_from_this();
}

//! The method dispatches the value to the given object
template< typename CharT >
bool basic_named_scope< CharT >::dispatch(type_dispatcher& dispatcher)
{
    scope_stack& s = pImpl->get_scope_stack();

    register type_visitor< scope_stack >* visitor =
        dispatcher.get_visitor< scope_stack >();
    if (visitor)
    {
        visitor->visit(s);
        return true;
    }
    else
        return false;
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

} // namespace attributes

} // namespace log

} // namespace boost

//! Explicitly instantiate named_scope implementation
template class BOOST_LOG_EXPORT boost::log::attributes::basic_named_scope< char >;
template class BOOST_LOG_EXPORT boost::log::attributes::basic_named_scope< wchar_t >;
