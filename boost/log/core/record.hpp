/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   record.hpp
 * \author Andrey Semashev
 * \date   09.03.2009
 *
 * This header contains a logging record class definition.
 */

#ifndef BOOST_LOG_CORE_RECORD_HPP_INCLUDED_
#define BOOST_LOG_CORE_RECORD_HPP_INCLUDED_

#include <boost/intrusive_ptr.hpp>
#include <boost/move/move.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/utility/explicit_operator_bool.hpp>
#include <boost/log/attributes/attribute_value_set.hpp>
#include <boost/log/expressions/keyword_fwd.hpp>
#ifndef BOOST_LOG_NO_THREADS
#include <boost/detail/atomic_count.hpp>
#endif // BOOST_LOG_NO_THREADS

#ifdef BOOST_LOG_HAS_PRAGMA_ONCE
#pragma once
#endif

#ifdef _MSC_VER
#pragma warning(push)
// non dll-interface class 'A' used as base for dll-interface class 'B'
#pragma warning(disable: 4275)
#endif // _MSC_VER

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

class core;

/*!
 * \brief Logging record class
 *
 * The logging record incapsulates all information related to a single logging statement,
 * in particular, attribute values view and the log message string.
 */
class record
{
    BOOST_COPYABLE_AND_MOVABLE(record)

    friend class core;

#ifndef BOOST_LOG_DOXYGEN_PASS
private:
    //! Private data
    struct private_data;
    friend struct private_data;

    //! Publicly available record data
    struct public_data
    {
        //! Reference counter
#ifndef BOOST_LOG_NO_THREADS
        mutable boost::detail::atomic_count m_ref_counter;
#else
        mutable unsigned int m_ref_counter;
#endif // BOOST_LOG_NO_THREADS

        //! Attribute values view
        attribute_value_set m_attribute_values;
        //! Shows if the record has already been detached from thread
        bool m_detached;

        //! Constructor from the attribute sets
        explicit public_data(BOOST_RV_REF(attribute_value_set) values) :
            m_ref_counter(0),
            m_attribute_values(values),
            m_detached(false)
        {
        }

        //! Destructor
        BOOST_LOG_API static void destroy(const public_data* p) BOOST_NOEXCEPT;

    protected:
        ~public_data() {}

        BOOST_LOG_DELETED_FUNCTION(public_data(public_data const&))
        BOOST_LOG_DELETED_FUNCTION(public_data& operator= (public_data const&))

        friend void intrusive_ptr_add_ref(const public_data* p) { ++p->m_ref_counter; }
        friend void intrusive_ptr_release(const public_data* p) { if (--p->m_ref_counter == 0) public_data::destroy(p); }
    };

private:
    //! A pointer to the log record implementation
    intrusive_ptr< public_data > m_impl;

#endif // BOOST_LOG_DOXYGEN_PASS

public:
    /*!
     * Default constructor. Creates an empty record that is equivalent to the invalid record handle.
     *
     * \post <tt>!*this == true</tt>
     */
    BOOST_LOG_DEFAULTED_FUNCTION(record(), {})

    /*!
     * Copy constructor
     */
    record(record const& that) BOOST_NOEXCEPT : m_impl(that.m_impl) {}

    /*!
     * Move constructor. Source record contents unspecified after the operation.
     */
    record(BOOST_RV_REF(record) that) BOOST_NOEXCEPT
    {
        m_impl.swap(that.m_impl);
    }

    /*!
     * Destructor. Destroys the record, releases any sinks and attribute values that were involved in processing this record.
     */
    ~record() BOOST_NOEXCEPT {}

    /*!
     * Copy assignment
     */
    record& operator= (BOOST_COPY_ASSIGN_REF(record) that) BOOST_NOEXCEPT
    {
        m_impl = that.m_impl;
        return *this;
    }

    /*!
     * Move assignment. Source record contents unspecified after the operation.
     */
    record& operator= (BOOST_RV_REF(record) that) BOOST_NOEXCEPT
    {
        m_impl.swap(that.m_impl);
        return *this;
    }

    /*!
     * \return A reference to the set of attribute values attached to this record
     *
     * \pre <tt>!!*this</tt>
     */
    attribute_value_set& attribute_values() BOOST_NOEXCEPT
    {
        return m_impl->m_attribute_values;
    }

    /*!
     * \return A reference to the set of attribute values attached to this record
     *
     * \pre <tt>!!*this</tt>
     */
    attribute_value_set const& attribute_values() const BOOST_NOEXCEPT
    {
        return m_impl->m_attribute_values;
    }

    /*!
     * Equality comparison
     *
     * \param that Comparand
     * \return \c true if both <tt>*this</tt> and \a that identify the same log record or do not
     *         identify any record, \c false otherwise.
     */
    bool operator== (record const& that) const BOOST_NOEXCEPT
    {
        return m_impl == that.m_impl;
    }
    /*!
     * Inequality comparison
     *
     * \param that Comparand
     * \return <tt>!(*this == that)</tt>
     */
    bool operator!= (record const& that) const BOOST_NOEXCEPT
    {
        return !operator== (that);
    }

    /*!
     * Conversion to an unspecified boolean type
     *
     * \return \c true, if the <tt>*this</tt> identifies a log record, \c false, if the <tt>*this</tt> is not valid
     */
    BOOST_LOG_EXPLICIT_OPERATOR_BOOL()

    /*!
     * Inverted conversion to an unspecified boolean type
     *
     * \return \c false, if the <tt>*this</tt> identifies a log record, \c true, if the <tt>*this</tt> is not valid
     */
    bool operator! () const BOOST_NOEXCEPT
    {
        return !m_impl;
    }

    /*!
     * Swaps two handles
     *
     * \param that Another record to swap with
     * <b>Throws:</b> Nothing
     */
    void swap(record& that) BOOST_NOEXCEPT
    {
        m_impl.swap(that.m_impl);
    }

    /*!
     * Resets the log record handle. If there are no other handles left, the log record is closed
     * and all resources referenced by the record are released.
     *
     * \post <tt>!*this == true</tt>
     */
    void reset() BOOST_NOEXCEPT
    {
        m_impl.reset();
    }

    /*!
     * The function ensures that the log record does not depend on any thread-specific data.
     *
     * \pre <tt>!!*this</tt>
     */
    BOOST_LOG_API void detach_from_thread();

    /*!
     * Attribute value lookup.
     *
     * \param keyword Attribute keyword.
     * \return An \c optional with extracted attribute value if it is found, empty value otherwise.
     */
    template< typename DescriptorT, template< typename > class ActorT >
    typename result_of::extract< typename expressions::attribute_keyword< DescriptorT, ActorT >::value_type >::type
    operator[] (expressions::attribute_keyword< DescriptorT, ActorT > const& keyword) const
    {
        return m_impl->m_attribute_values[keyword];
    }
};

/*!
 * A free-standing swap function overload for \c basic_record
 */
inline void swap(record& left, record& right) BOOST_NOEXCEPT
{
    left.swap(right);
}

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_CORE_RECORD_HPP_INCLUDED_
