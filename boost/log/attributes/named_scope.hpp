/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   named_scope.hpp
 * \author Andrey Semashev
 * \date   24.06.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_ATTRIBUTES_NAMED_SCOPE_HPP_INCLUDED_
#define BOOST_LOG_ATTRIBUTES_NAMED_SCOPE_HPP_INCLUDED_

#include <ostream>
#include <memory>
#include <iterator>
#include <boost/shared_ptr.hpp>
#include <boost/current_function.hpp>
#include <boost/mpl/if.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/detail/string_literal.hpp>

#ifdef _MSC_VER
#pragma warning(push)
// non dll-interface class 'A' used as base for dll-interface class 'B'
#pragma warning(disable: 4275)
// 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
#endif // _MSC_VER

namespace boost {

namespace log {

namespace attributes {

namespace aux {

    //! Double-linked list node
    struct named_scope_list_node
    {
        mutable named_scope_list_node* _m_pPrev;
        mutable named_scope_list_node* _m_pNext;

        named_scope_list_node() { _m_pPrev = _m_pNext = this; }
    };

} // namespace aux

//! The structure contains all information about a named scope
template< typename CharT >
struct basic_named_scope_entry :
    public aux::named_scope_list_node
{
    //! Character type
    typedef CharT char_type;

    //! The scope (e.g. function signatute) name
    basic_string_literal< char_type > scope_name;
    //! The source file name
    basic_string_literal< char_type > file_name;
    //! The line number in the source file
    unsigned int line;

    //! Initializing constructor
    template< typename T1, unsigned int N1, typename T2, unsigned int N2 >
    basic_named_scope_entry(T1 (&sn)[N1], T2 (&fn)[N2], unsigned int ln)
        : scope_name(sn), file_name(fn), line(ln)
    {
    }
};

//! The class implements the list of scopes
template< typename CharT >
class basic_named_scope_list :
    protected std::allocator< basic_named_scope_entry< CharT > >
{
    //! Self type
    typedef basic_named_scope_list< CharT >this_type;

public:
    //! Character type
    typedef CharT char_type;
    //! Allocator type
    typedef std::allocator< basic_named_scope_entry< char_type > > allocator_type;

    //  Standard types
    typedef typename allocator_type::value_type value_type;
    typedef typename allocator_type::reference reference;
    typedef typename allocator_type::const_reference const_reference;
    typedef typename allocator_type::pointer pointer;
    typedef typename allocator_type::const_pointer const_pointer;
    typedef typename allocator_type::size_type size_type;
    typedef typename allocator_type::difference_type difference_type;

protected:
    //! Iterator class
    template< bool fConstV > class iter;
    template< bool fConstV > friend class iter;
    template< bool fConstV >
    class iter
    {
        friend class iter< !fConstV >;

    public:
        //  Standard typedefs
        typedef typename this_type::difference_type difference_type;
        typedef typename this_type::value_type value_type;
        typedef typename mpl::if_c<
            fConstV,
            typename this_type::const_reference,
            typename this_type::reference
        >::type reference;
        typedef typename mpl::if_c<
            fConstV,
            typename this_type::const_pointer,
            typename this_type::pointer
        >::type pointer;
        typedef std::bidirectional_iterator_tag iterator_category;

    public:
        //  Constructors
        iter() : m_pNode(NULL) {}
        explicit iter(aux::named_scope_list_node* pNode) : m_pNode(pNode) {}
        iter(iter< false > const& that) : m_pNode(that.m_pNode) {}

        //! Assignment
        template< bool f >
        iter& operator= (iter< f > const& that)
        {
            m_pNode = that.m_pNode;
            return *this;
        }

        //  Comparison
        template< bool f >
        bool operator== (iter< f > const& that) const { return (m_pNode == that.m_pNode); }
        template< bool f >
        bool operator!= (iter< f > const& that) const { return (m_pNode != that.m_pNode); }

        //  Modification
        iter& operator++ ()
        {
            m_pNode = m_pNode->_m_pNext;
            return *this;
        }
        iter& operator-- ()
        {
            m_pNode = m_pNode->_m_pPrev;
            return *this;
        }
        iter operator++ (int)
        {
            iter tmp(*this);
            m_pNode = m_pNode->_m_pNext;
            return tmp;
        }
        iter operator-- (int)
        {
            iter tmp(*this);
            m_pNode = m_pNode->_m_pPrev;
            return tmp;
        }

        //  Dereferencing
        pointer operator-> () const { return static_cast< pointer >(m_pNode); }
        reference operator* () const { return *static_cast< pointer >(m_pNode); }

    private:
        aux::named_scope_list_node* m_pNode;
    };

public:
    typedef iter< true > const_iterator;
    typedef iter< false > iterator;
    typedef std::reverse_iterator< const_iterator > const_reverse_iterator;
    typedef std::reverse_iterator< iterator > reverse_iterator;

protected:
    //! The root node of the container
    aux::named_scope_list_node m_RootNode;
    //! The size of the container
    size_type m_Size;
    //! The flag shows if the contained elements are dynamically allocated
    bool m_fNeedToDeallocate;

public:
    //  Structors
    basic_named_scope_list() : m_Size(0), m_fNeedToDeallocate(false) {}
    BOOST_LOG_EXPORT basic_named_scope_list(basic_named_scope_list const& that);
    BOOST_LOG_EXPORT ~basic_named_scope_list();

    //! Assignment
    basic_named_scope_list& operator= (basic_named_scope_list const& that)
    {
        if (this != &that)
        {
            basic_named_scope_list tmp(that);
            swap(tmp);
        }
        return *this;
    }
    
    //  Iterators
    const_iterator begin() const { return const_iterator(m_RootNode._m_pNext); }
    const_iterator end() const { return const_iterator(const_cast< aux::named_scope_list_node* >(&m_RootNode)); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    //! Returns the size of the container
    size_type size() const { return m_Size; }
    //! Returns true if the container is empty and false otherwise
    bool empty() const { return (m_Size == 0); }

    //! Swaps two instances of the container
    void swap(basic_named_scope_list& that)
    {
        using std::swap;
        swap(m_RootNode, that.m_RootNode);
        swap(m_Size, that.m_Size);
        swap(m_fNeedToDeallocate, that.m_fNeedToDeallocate);
    }

    //! Last pushed scope accessor
    const_reference back() const { return *rbegin(); }
    //! First pushed scope accessor
    const_reference front() const { return *begin(); }
};

//! Stream output operator
template< typename CharT, typename TraitsT >
inline std::basic_ostream< CharT, TraitsT >& operator<< (
    std::basic_ostream< CharT, TraitsT >& strm, basic_named_scope_list< CharT > const& sl)
{
    typename basic_named_scope_list< CharT >::const_iterator it = sl.begin(), end = sl.end();
    if (it != end)
        strm << (it++)->scope_name;
    for (; it != end; ++it)
        strm << "->" << it->scope_name;
    return strm;
}

//! A class of an attribute that holds stack of named scopes of the current thread
template< typename CharT >
class BOOST_LOG_EXPORT basic_named_scope :
    public attribute
{
public:
    //! Character type
    typedef CharT char_type;
    //! Scope names stack (the attribute value type)
    typedef basic_named_scope_list< char_type > scope_stack;
    //! Scope entry
    typedef typename scope_stack::value_type scope_entry;

    //! Sentry object class to automatically push and pop scopes
    struct sentry
    {
        //! Attribute type
        typedef basic_named_scope< char_type > named_scope_type;

        //! Constructor
        template< typename T1, unsigned int N1, typename T2, unsigned int N2 >
        sentry(T1 (&sn)[N1], T2 (&fn)[N2], unsigned int ln) : m_Entry(sn, fn, ln)
        {
            named_scope_type::push_scope(m_Entry);
        }

        //! Destructor
        ~sentry()
        {
            named_scope_type::pop_scope();
        }

    private:
        scope_entry m_Entry;
    };

private:
    //! Attribute implementation class
    struct implementation;

private:
    //! Pointer to the implementation
    shared_ptr< implementation > pImpl;

public:
    //! Constructor
    basic_named_scope();

    //! The method returns the actual attribute value. It must not return NULL.
    shared_ptr< attribute_value > get_value();

    //! The method pushes the scope to the stack
    static void push_scope(scope_entry const& entry);
    //! The method pops the top scope
    static void pop_scope();

    /*!
     *  \brief Returns the current thread's scope stack
     * 
     *  \note The returned reference is only valid until the current thread ends. The scopes in the
     *        returned container may change if the execution scope is changed (i.e. either push_scope
     *        or pop_scope is called). User has to copy the stack if he wants to keep it intact regardless
     *        of the execution scope.
     */
    static scope_stack const& get_scopes();
};

typedef basic_named_scope< char > named_scope;
typedef basic_named_scope< wchar_t > wnamed_scope;

} // namespace attributes

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#define BOOST_LOG_UNIQUE_IDENTIFIER_NAME_INTERNAL_(prefix, postfix)\
    BOOST_PP_CAT(prefix, postfix)
#define BOOST_LOG_UNIQUE_IDENTIFIER_NAME_INTERNAL(prefix, postfix)\
    BOOST_LOG_UNIQUE_IDENTIFIER_NAME_INTERNAL_(prefix, postfix)

// In VC 7.0 and later when compiling with /ZI option __LINE__ macro is corrupted
#if BOOST_WORKAROUND(BOOST_MSVC, >=1300)
#  define BOOST_LOG_UNIQUE_IDENTIFIER_NAME(prefix)\
    BOOST_LOG_UNIQUE_IDENTIFIER_NAME_INTERNAL(prefix, __COUNTER__)
#else
#  define BOOST_LOG_UNIQUE_IDENTIFIER_NAME(prefix)\
    BOOST_LOG_UNIQUE_IDENTIFIER_NAME_INTERNAL(prefix, __LINE__)
#endif // BOOST_WORKAROUND(BOOST_MSVC, >= 1300)

//! Macro for scope markup
#define BOOST_LOG_NAMED_SCOPE(name)\
    ::boost::log::attributes::named_scope::sentry BOOST_LOG_UNIQUE_IDENTIFIER_NAME(_boost_log_named_scope_sentry_)(name, __FILE__, __LINE__)

//! Macro for scope markup
#define BOOST_LOG_WNAMED_SCOPE(name)\
    ::boost::log::attributes::wnamed_scope::sentry BOOST_LOG_UNIQUE_IDENTIFIER_NAME(_boost_log_named_scope_sentry_)(name, BOOST_PP_CAT(L, __FILE__), __LINE__)

//! Macro for function scope markup
#define BOOST_LOG_FUNCTION() BOOST_LOG_NAMED_SCOPE(BOOST_CURRENT_FUNCTION)

//! Macro for function scope markup
#define BOOST_LOG_WFUNCTION() BOOST_LOG_WNAMED_SCOPE(BOOST_PP_CAT(L, BOOST_CURRENT_FUNCTION))

#endif // BOOST_LOG_ATTRIBUTES_NAMED_SCOPE_HPP_INCLUDED_
