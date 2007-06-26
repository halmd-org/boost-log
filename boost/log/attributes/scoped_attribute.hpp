/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   scoped_attribute.hpp
 * \author Andrey Semashev
 * \date   13.05.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_SCOPED_ATTRIBUTE_HPP_INCLUDED_
#define BOOST_LOG_SCOPED_ATTRIBUTE_HPP_INCLUDED_

#include <boost/shared_ptr.hpp>
#include <boost/empty_deleter.hpp>
#include <boost/utility/addressof.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/logging_core.hpp>
#include <boost/log/sources/basic_logger.hpp>
#include <boost/log/attributes/attribute_set.hpp>

namespace boost {

namespace log {

namespace aux {

    //! A base class for all scoped attribute classes
    class attribute_scope_guard {};

} // namespace aux

typedef aux::attribute_scope_guard const& scoped_attribute;

namespace aux {

    //! A scoped logger attribute guard
    template< typename CharT >
    class scoped_logger_attribute :
        public attribute_scope_guard
    {
    private:
        //! Logger type
        typedef basic_logger< CharT > logger_type;

    private:
        //! A reference to the logger
        mutable logger_type* m_pLogger;
        //! An iterator to the added attribute
        typename logger_type::attribute_set::iterator m_itAttribute;

    public:
        //! Constructor
        scoped_logger_attribute(
            logger_type& l,
            typename logger_type::string_type const& name,
            shared_ptr< attribute > const& attr
        ) : m_pLogger(&l), m_itAttribute(l.add_attribute(name, attr))
        {
        }
        //! Copy constructor (implemented as move)
        scoped_logger_attribute(scoped_logger_attribute const& that)
            : m_pLogger(that.m_pLogger), m_itAttribute(that.m_itAttribute)
        {
            that.m_pLogger = 0;
        }

        //! Destructor
        ~scoped_logger_attribute()
        {
            if (m_pLogger)
                m_pLogger->remove_attribute(m_itAttribute);
        }

    private:
        //! Assignment (closed)
        scoped_logger_attribute& operator= (scoped_logger_attribute const&);
    };

} // namespace aux

//  Generator helper functions
template< typename CharT >
aux::scoped_logger_attribute< CharT > add_scoped_logger_attribute(
    basic_logger< CharT >& l, std::basic_string< CharT > const& name, shared_ptr< attribute > const& attr)
{
    return aux::scoped_logger_attribute< CharT >(l, name, attr);
}

template< typename CharT >
aux::scoped_logger_attribute< CharT > add_scoped_logger_attribute(
    basic_logger< CharT >& l, const CharT* name, shared_ptr< attribute > const& attr)
{
    return aux::scoped_logger_attribute< CharT >(l, name, attr);
}

template< typename CharT, typename AttributeT >
aux::scoped_logger_attribute< CharT > add_scoped_logger_attribute(
    basic_logger< CharT >& l, std::basic_string< CharT > const& name, AttributeT& attr)
{
    return aux::scoped_logger_attribute< CharT >(l, name, shared_ptr< attribute >(addressof(attr), empty_deleter()));
}

template< typename CharT, typename AttributeT >
aux::scoped_logger_attribute< CharT > add_scoped_logger_attribute(
    basic_logger< CharT >& l, const CharT* name, AttributeT& attr)
{
    return aux::scoped_logger_attribute< CharT >(l, name, shared_ptr< attribute >(addressof(attr), empty_deleter()));
}


namespace aux {

    //! A scoped thread or global attribute guard
    template<
        typename CharT,
        typename basic_logging_core< CharT >::attribute_set::iterator
            (basic_logging_core< CharT >::*adder)(
                typename basic_logging_core< CharT >::string_type const&, shared_ptr< attribute > const&),
        void (basic_logging_core< CharT >::*remover)(typename basic_logging_core< CharT >::attribute_set::iterator)
    >
    class scoped_core_attribute :
        public attribute_scope_guard
    {
    private:
        //! Logging core type
        typedef basic_logging_core< CharT > logging_core_type;

    private:
        //! A pointer to the logging core
        mutable shared_ptr< logging_core_type > m_pCore;
        //! An iterator to the added attribute
        typename logging_core_type::attribute_set::iterator m_itAttribute;

    public:
        //! Constructor
        scoped_core_attribute(
            typename logging_core_type::string_type const& name, shared_ptr< attribute > const& attr) :
            m_pCore(logging_core_type::get()),
            m_itAttribute(((m_pCore.get())->*(adder))(name, attr))
        {
        }
        //! Copy constructor (implemented as move)
        scoped_core_attribute(scoped_core_attribute const& that) : m_itAttribute(that.m_itAttribute)
        {
            m_pCore.swap(that.m_pCore);
        }

        //! Destructor
        ~scoped_core_attribute()
        {
            if (m_pCore)
                ((m_pCore.get())->*(remover))(m_itAttribute);
        }
    };

} // namespace aux

//  Generator helper functions
template< typename CharT >
aux::scoped_core_attribute<
    CharT,
    &basic_logging_core< CharT >::add_thread_attribute,
    &basic_logging_core< CharT >::remove_thread_attribute
> add_scoped_thread_attribute(std::basic_string< CharT > const& name, shared_ptr< attribute > const& attr)
{
    return aux::scoped_core_attribute<
        CharT,
        &basic_logging_core< CharT >::add_thread_attribute,
        &basic_logging_core< CharT >::remove_thread_attribute
    >(name, attr);
}

template< typename CharT >
aux::scoped_core_attribute<
    CharT,
    &basic_logging_core< CharT >::add_thread_attribute,
    &basic_logging_core< CharT >::remove_thread_attribute
> add_scoped_thread_attribute(const CharT* name, shared_ptr< attribute > const& attr)
{
    return aux::scoped_core_attribute<
        CharT,
        &basic_logging_core< CharT >::add_thread_attribute,
        &basic_logging_core< CharT >::remove_thread_attribute
    >(name, attr);
}

template< typename CharT, typename AttributeT >
aux::scoped_core_attribute<
    CharT,
    &basic_logging_core< CharT >::add_thread_attribute,
    &basic_logging_core< CharT >::remove_thread_attribute
> add_scoped_thread_attribute(std::basic_string< CharT > const& name, AttributeT& attr)
{
    return aux::scoped_core_attribute<
        CharT,
        &basic_logging_core< CharT >::add_thread_attribute,
        &basic_logging_core< CharT >::remove_thread_attribute
    >(name, shared_ptr< attribute >(addressof(attr), empty_deleter()));
}

template< typename CharT, typename AttributeT >
aux::scoped_core_attribute<
    CharT,
    &basic_logging_core< CharT >::add_thread_attribute,
    &basic_logging_core< CharT >::remove_thread_attribute
> add_scoped_thread_attribute(const CharT* name, AttributeT& attr)
{
    return aux::scoped_core_attribute<
        CharT,
        &basic_logging_core< CharT >::add_thread_attribute,
        &basic_logging_core< CharT >::remove_thread_attribute
    >(name, shared_ptr< attribute >(addressof(attr), empty_deleter()));
}


//  Generator helper functions
template< typename CharT >
aux::scoped_core_attribute<
    CharT,
    &basic_logging_core< CharT >::add_global_attribute,
    &basic_logging_core< CharT >::remove_global_attribute
> add_scoped_global_attribute(std::basic_string< CharT > const& name, shared_ptr< attribute > const& attr)
{
    return aux::scoped_core_attribute<
        CharT,
        &basic_logging_core< CharT >::add_global_attribute,
        &basic_logging_core< CharT >::remove_global_attribute
    >(name, attr);
}

template< typename CharT >
aux::scoped_core_attribute<
    CharT,
    &basic_logging_core< CharT >::add_global_attribute,
    &basic_logging_core< CharT >::remove_global_attribute
> add_scoped_global_attribute(const CharT* name, shared_ptr< attribute > const& attr)
{
    return aux::scoped_core_attribute<
        CharT,
        &basic_logging_core< CharT >::add_global_attribute,
        &basic_logging_core< CharT >::remove_global_attribute
    >(name, attr);
}

template< typename CharT, typename AttributeT >
aux::scoped_core_attribute<
    CharT,
    &basic_logging_core< CharT >::add_global_attribute,
    &basic_logging_core< CharT >::remove_global_attribute
> add_scoped_global_attribute(std::basic_string< CharT > const& name, AttributeT& attr)
{
    return aux::scoped_core_attribute<
        CharT,
        &basic_logging_core< CharT >::add_global_attribute,
        &basic_logging_core< CharT >::remove_global_attribute
    >(name, shared_ptr< attribute >(addressof(attr), empty_deleter()));
}

template< typename CharT, typename AttributeT >
aux::scoped_core_attribute<
    CharT,
    &basic_logging_core< CharT >::add_global_attribute,
    &basic_logging_core< CharT >::remove_global_attribute
> add_scoped_global_attribute(const CharT* name, AttributeT& attr)
{
    return aux::scoped_core_attribute<
        CharT,
        &basic_logging_core< CharT >::add_global_attribute,
        &basic_logging_core< CharT >::remove_global_attribute
    >(name, shared_ptr< attribute >(addressof(attr), empty_deleter()));
}

} // namespace log

} // namespace boost

#endif // BOOST_LOG_SCOPED_ATTRIBUTE_HPP_INCLUDED_
