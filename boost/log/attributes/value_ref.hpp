/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   value_ref.hpp
 * \author Andrey Semashev
 * \date   27.07.2012
 *
 * The header contains implementation of an attrubute value reference wrapper.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_ATTRIBUTES_VALUE_REF_HPP_INCLUDED_
#define BOOST_LOG_ATTRIBUTES_VALUE_REF_HPP_INCLUDED_

#include <cstddef>
#include <boost/assert.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/is_sequence.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/mpl/equal_to.hpp>
#include <boost/mpl/contains.hpp>
#include <boost/mpl/index_of.hpp>
#include <boost/utility/addressof.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/utility/explicit_operator_bool.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace aux {

//! Attribute value reference implementation for a single type case
template< typename T, typename TagT >
class singular_ref
{
public:
    //! Referenced value type
    typedef T value_type;
    //! Tag type
    typedef TagT tag_type;

protected:
    //! The metafunction tests if the type is compatible with the reference wrapper
#if !defined(BOOST_NO_TEMPLATE_ALIASES) && !defined(BOOST_NO_CXX11_TEMPLATE_ALIASES)
    template< typename U >
    using is_compatible = is_same< U, value_type >;
#else
    template< typename U >
    struct is_compatible :
        public is_same< U, value_type >
    {
    };
#endif

protected:
    //! Pointer to the value
    const value_type* m_ptr;

protected:
    //! Default constructor
    singular_ref() BOOST_NOEXCEPT : m_ptr(NULL)
    {
    }

    //! Initializing constructor
    explicit singular_ref(const value_type* p) BOOST_NOEXCEPT : m_ptr(p)
    {
    }

public:
    //! Returns a pointer to the referred value
    const value_type* operator-> () const BOOST_NOEXCEPT
    {
        BOOST_ASSERT(m_ptr != NULL);
        return m_ptr;
    }

    //! Returns a pointer to the referred value
    const value_type* get_ptr() const BOOST_NOEXCEPT
    {
        return m_ptr;
    }

    //! Returns a pointer to the referred value
    template< typename U >
    typename enable_if< is_compatible< U >, const U* >::type get_ptr() const BOOST_NOEXCEPT
    {
        return m_ptr;
    }

    //! Returns a reference to the value
    value_type const& get() const BOOST_NOEXCEPT
    {
        BOOST_ASSERT(m_ptr != NULL);
        return *m_ptr;
    }

    //! Returns a reference to the value
    template< typename U >
    typename enable_if< is_compatible< U >, U const& >::type get() const BOOST_NOEXCEPT
    {
        BOOST_ASSERT(m_ptr != NULL);
        return *m_ptr;
    }


    //! Resets the reference
    void reset() BOOST_NOEXCEPT
    {
        m_ptr = NULL;
    }

    //! Returns the stored type index
    static BOOST_CONSTEXPR unsigned int which()
    {
        return 0u;
    }
};


//! Attribute value reference implementation for multiple types case
template< typename T, typename TagT >
class variant_ref
{
public:
    //! Referenced value type
    typedef T value_type;
    //! Tag type
    typedef TagT tag_type;

protected:
    //! The metafunction tests if the type is compatible with the reference wrapper
#if !defined(BOOST_NO_TEMPLATE_ALIASES) && !defined(BOOST_NO_CXX11_TEMPLATE_ALIASES)
    template< typename U >
    using is_compatible = mpl::contains< value_type, U >;
#else
    template< typename U >
    struct is_compatible :
        public mpl::contains< value_type, U >
    {
    };
#endif

protected:
    //! Pointer to the value
    const void* m_ptr;
    //! Type index
    unsigned int m_type_idx;

protected:
    //! Default constructor
    variant_ref() BOOST_NOEXCEPT : m_ptr(NULL), m_type_idx(0)
    {
    }

    //! Initializing constructor
    template< typename U >
    explicit variant_ref(const U* p) BOOST_NOEXCEPT : m_ptr(p), m_type_idx(mpl::index_of< value_type, U >::value)
    {
    }

public:
    //! Resets the reference
    void reset() BOOST_NOEXCEPT
    {
        m_ptr = NULL;
        m_type_idx = 0;
    }

    //! Returns a pointer to the referred value
    template< typename U >
    typename enable_if< is_compatible< U >, const U* >::type get_ptr() const BOOST_NOEXCEPT
    {
        if (m_type_idx == mpl::index_of< value_type, U >::value)
            return static_cast< const U* >(m_ptr);
        else
            return NULL;
    }

    //! Returns a reference to the value
    template< typename U >
    typename enable_if< is_compatible< U >, U const& >::type get() const BOOST_NOEXCEPT
    {
        const U* const p = get_ptr< U >();
        BOOST_ASSERT(p != NULL);
        return *p;
    }

    //! Returns the stored type index
    unsigned int which() const BOOST_NOEXCEPT
    {
        return m_type_idx;
    }
};

template< typename T, typename TagT >
struct value_ref_base
{
    typedef typename mpl::eval_if<
        mpl::and_< mpl::is_sequence< T >, mpl::equal_to< mpl::size< T >, mpl::int_< 1 > > >,
        mpl::front< T >,
        mpl::identity< T >
    >::type value_type;

    typedef typename mpl::if_<
        mpl::is_sequence< value_type >,
        variant_ref< value_type, TagT >,
        singular_ref< value_type, TagT >
    >::type type;
};

} // namespace aux

/*!
 * \brief Reference wrapper for a stored attribute value.
 *
 * The \c value_ref class template provides access to the stored attribute value. It is not a traditional reference wrapper
 * since it may be empty (i.e. refer to no value at all) and it can also refer to values of different types. Therefore its
 * interface and behavior combines features of Boost.Ref, Boost.Optional and Boost.Variant, depending on the use case.
 *
 * The template parameter \c T can be a single type or an MPL seqence of possible types being referred. The reference wrapper
 * will act as either an optional reference or an optional variant of references to the specified types. In any case, the
 * referred values will not be modifiable (i.e. \c value_ref always models a const reference).
 *
 * Template parameter \c TagT is optional. It can be used for customizing the operations on this reference wrapper, such as
 * putting the referred value to log.
 */
template< typename T, typename TagT = void >
class value_ref :
    public value_ref_base< T, TagT >::type
{
private:
    //! Base implementation type
    typedef typename value_ref_base< T, TagT >::type base_type;

public:
    /*!
     * Default constructor. Creates a reference wrapper that does not refer to a value.
     */
    BOOST_LOG_DEFAULTED_FUNCTION(value_ref(), BOOST_NOEXCEPT {})

    /*!
     * Copy constructor.
     */
    BOOST_LOG_DEFAULTED_FUNCTION(value_ref(value_ref const& that), BOOST_NOEXCEPT : base_type(static_cast< base_type const& >(that)) {})

    /*!
     * Initializing constructor. Creates a reference wrapper that refers to the specified value.
     */
    template< typename U >
    explicit value_ref(U const& val, typename enable_if< typename base_type::BOOST_NESTED_TEMPLATE is_compatible< U >, int >::type = 0) BOOST_NOEXCEPT :
        base_type(boost::addressof(val))
    {
    }

    /*!
     * The operator verifies if the wrapper refers to a value.
     */
    BOOST_LOG_EXPLICIT_OPERATOR_BOOL()

    /*!
     * The operator verifies if the wrapper does not refer to a value.
     */
    bool operator! () const BOOST_NOEXCEPT
    {
        return !this->m_ptr;
    }
};

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost


#endif // BOOST_LOG_ATTRIBUTES_VALUE_REF_HPP_INCLUDED_
