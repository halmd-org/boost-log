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
#include <boost/optional.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread/tss.hpp>
#include <boost/thread/once.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/attributes/named_scope.hpp>

namespace boost {

namespace log {

namespace attributes {

namespace {

    //! Actual implementation of the named scope list
    template< typename CharT >
    class basic_writeable_named_scope_list :
        public basic_named_scope_list< CharT >
    {
        //! Base type
        typedef basic_named_scope_list< CharT > base_type;
    
    public:
        //! Const reference type
        typedef typename base_type::const_reference const_reference;
    
    public:
        //! The method pushes the scope to the back of the list
        void push_back(const_reference entry)
        {
            const_reference top = this->back();
            entry._m_pPrev = const_cast< aux::named_scope_list_node* >(static_cast< const aux::named_scope_list_node* >(&top));
            top._m_pNext = const_cast< aux::named_scope_list_node* >(static_cast< const aux::named_scope_list_node* >(&entry));
            entry._m_pNext = &this->m_RootNode;
            this->m_RootNode._m_pPrev = const_cast< aux::named_scope_list_node* >(static_cast< const aux::named_scope_list_node* >(&entry));
            ++this->m_Size;
        }
        //! The method removes the top scope entry from the list
        void pop_back()
        {
            const_reference top = this->back();
            top._m_pPrev->_m_pNext = top._m_pNext;
            top._m_pNext->_m_pPrev = top._m_pPrev;
            --this->m_Size;
        }
    };
    
    //! Named scope attribute value
    template< typename CharT >
    class basic_named_scope_value :
        public attribute_value,
        public enable_shared_from_this< basic_named_scope_value< CharT > >
    {
        //! Character type
        typedef CharT char_type;
        //! Scope names stack
        typedef basic_named_scope_list< char_type > scope_stack;
    
        //! Pointer to the actual scope value
        scope_stack* m_pValue;
        //! A thread-independent value
        optional< scope_stack > m_DetachedValue;
    
    public:
        //! Constructor
        explicit basic_named_scope_value(scope_stack* p) : m_pValue(p) {}
    
        //! The method dispatches the value to the given object. It returns true if the
        //! object was capable to consume the real attribute value type and false otherwise.
        bool dispatch(type_dispatcher& dispatcher)
        {
            register type_visitor< scope_stack >* visitor =
                dispatcher.get_visitor< scope_stack >();
            if (visitor)
            {
                visitor->visit(*m_pValue);
                return true;
            }
            else
                return false;
        }
    
        //! The method is called when the attribute value is passed to another thread (e.g.
        //! in case of asynchronous logging). The value should ensure it properly owns all thread-specific data.
        shared_ptr< attribute_value > detach_from_thread()
        {
            if (!m_DetachedValue)
            {
                m_DetachedValue = *m_pValue;
                m_pValue = m_DetachedValue.get_ptr();
            }

            return this->shared_from_this();
        }
    };

} // namespace

//! Named scope attribute implementation
template< typename CharT >
struct basic_named_scope< CharT >::implementation :
    public enable_shared_from_this< implementation >
{
    //! Writeable scope list type
    typedef basic_writeable_named_scope_list< char_type > scope_list;

    //! Pointer to the thread-specific scope stack
    thread_specific_ptr< scope_list > pScopes;

    //! Pointer to implementation instance
    static implementation* const pInstance;

    //! The method returns current thread scope stack
    scope_list& get_scope_list()
    {
        register scope_list* p = pScopes.get();
        if (!p)
        {
            std::auto_ptr< scope_list > pNew(new scope_list());
            pScopes.reset(pNew.get());
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


//! Copy constructor
template< typename CharT >
basic_named_scope_list< CharT >::basic_named_scope_list(basic_named_scope_list const& that)
    : allocator_type(static_cast< allocator_type const& >(that)), m_Size(that.size()), m_fNeedToDeallocate(!that.empty())
{
    if (m_Size > 0)
    {
        // Copy the container contents
        register pointer p = allocator_type::allocate(that.size());
        register aux::named_scope_list_node* prev = &m_RootNode;
        for (const_iterator src = that.begin(), end = that.end(); src != end; ++src, ++p)
        {
            allocator_type::construct(p, *src); // won't throw
            p->_m_pPrev = prev;
            prev->_m_pNext = p;
            prev = p;
        }
        m_RootNode._m_pPrev = prev;
        prev->_m_pNext = &m_RootNode;
    }
}

//! Destructor
template< typename CharT >
basic_named_scope_list< CharT >::~basic_named_scope_list()
{
    if (m_fNeedToDeallocate)
    {
        iterator it(m_RootNode._m_pNext);
        iterator end(&m_RootNode);
        for (; it != end; ++it)
            allocator_type::destroy(&*it);
        allocator_type::deallocate(static_cast< pointer >(m_RootNode._m_pNext), m_Size);
    }
}

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
    return shared_ptr< attribute_value >(
        new basic_named_scope_value< char_type >(&pImpl->get_scope_list()));
}

//! The method pushes the scope to the stack
template< typename CharT >
void basic_named_scope< CharT >::push_scope(scope_entry const& entry)
{
    typename implementation::scope_list& s = implementation::pInstance->get_scope_list();
    s.push_back(entry);
}

//! The method pops the top scope
template< typename CharT >
void basic_named_scope< CharT >::pop_scope()
{
    typename implementation::scope_list& s = implementation::pInstance->get_scope_list();
    s.pop_back();
}

//! Returns the current thread's scope stack
template< typename CharT >
typename basic_named_scope< CharT >::scope_stack const& basic_named_scope< CharT >::get_scopes()
{
    return implementation::pInstance->get_scope_list();
}

//! Explicitly instantiate named_scope implementation
template class BOOST_LOG_EXPORT basic_named_scope< char >;
template class BOOST_LOG_EXPORT basic_named_scope< wchar_t >;
template class basic_named_scope_list< char >;
template class basic_named_scope_list< wchar_t >;

} // namespace attributes

} // namespace log

} // namespace boost
