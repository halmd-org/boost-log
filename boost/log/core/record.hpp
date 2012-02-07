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

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_CORE_RECORD_HPP_INCLUDED_
#define BOOST_LOG_CORE_RECORD_HPP_INCLUDED_

#include <boost/intrusive_ptr.hpp>
#include <boost/move/move.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/utility/explicit_operator_bool.hpp>
#include <boost/log/attributes/attribute_values_view.hpp>
#ifndef BOOST_LOG_NO_THREADS
#include <boost/detail/atomic_count.hpp>
#endif // BOOST_LOG_NO_THREADS

#ifdef _MSC_VER
#pragma warning(push)
 // non dll-interface class 'A' used as base for dll-interface class 'B'
#pragma warning(disable: 4275)
#endif // _MSC_VER

namespace boost {

namespace BOOST_LOG_NAMESPACE {

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

public:
    //! Attribute values view type
    typedef attribute_values_view values_view_type;

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
        mutable boost::detail::atomic_count m_RefCounter;
#else
        mutable unsigned long m_RefCounter;
#endif // BOOST_LOG_NO_THREADS

        //! Attribute values view
        values_view_type m_AttributeValues;
        //! Pointer to the private implemntation
        private_data* m_Private;
        //! Shows if the record has already been detached from thread
        bool m_Detached;

        //! Constructor from the attribute sets
        explicit public_data(BOOST_RV_REF(values_view_type) values) :
            m_RefCounter(0),
            m_AttributeValues(values),
            m_Private(NULL),
            m_Detached(false)
        {
        }
        //! Constructor from the attribute sets
        explicit public_data(values_view_type const& values) :
            m_RefCounter(0),
            m_AttributeValues(values),
            m_Private(NULL),
            m_Detached(false)
        {
        }

        //! Destructor
        BOOST_LOG_EXPORT ~public_data();

        BOOST_LOG_DELETED_FUNCTION(public_data(public_data const&))
        BOOST_LOG_DELETED_FUNCTION(public_data& operator= (public_data const&))

        friend void intrusive_ptr_add_ref(const public_data* p) { ++p->m_RefCounter; }
        friend void intrusive_ptr_release(const public_data* p) { if (--p->m_RefCounter == 0) delete p; }
    };

private:
    //! A pointer to the log record data
    intrusive_ptr< public_data > m_pData;

#endif // BOOST_LOG_DOXYGEN_PASS

public:
    /*!
     * Default constructor. Creates an empty record that is equivalent to the invalid record handle.
     *
     * \post <tt>!*this == true</tt>
     */
    record() {}

    /*!
     * Copy constructor
     */
    record(record const& that) : m_pData(that.m_pData) {}

    /*!
     * Move constructor. Source record contents unspecified after the operation.
     */
    record(BOOST_RV_REF(record) that)
    {
        m_pData.swap(that.m_pData);
    }

    /*!
     * Destructor. Destroys the record, releases any sinks and attribute values that were involved in processing this record.
     */
    ~record() {}

    /*!
     * Copy assignment
     */
    record& operator= (BOOST_COPY_ASSIGN_REF(record) that)
    {
        m_pData = that.m_pData;
        return *this;
    }

    /*!
     * Move assignment. Source record contents unspecified after the operation.
     */
    record& operator= (BOOST_RV_REF(record) that)
    {
        m_pData.swap(that.m_pData);
        return *this;
    }

    /*!
     * \return A reference to the set of attribute values attached to this record
     *
     * \pre <tt>!!*this</tt>
     */
    values_view_type const& attribute_values() const
    {
        return m_pData->m_AttributeValues;
    }

    /*!
     * Equality comparison
     *
     * \param that Comparand
     * \return \c true if both <tt>*this</tt> and \a that identify the same log record or do not
     *         identify any record, \c false otherwise.
     */
    bool operator== (record const& that) const
    {
        return m_pData == that.m_pData;
    }
    /*!
     * Inequality comparison
     *
     * \param that Comparand
     * \return <tt>!(*this == that)</tt>
     */
    bool operator!= (record const& that) const
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
    bool operator! () const
    {
        return !m_pData;
    }

    /*!
     * Swaps two handles
     *
     * \param that Another record to swap with
     * <b>Throws:</b> Nothing
     */
    void swap(record& that)
    {
        m_pData.swap(that.m_pData);
    }

    /*!
     * Resets the log record handle. If there are no other handles left, the log record is closed
     * and all resources referenced by the record are released.
     *
     * \post <tt>!*this == true</tt>
     */
    void reset()
    {
        m_pData.reset();
    }

    /*!
     * The function ensures that the log record does not depend on any thread-specific data.
     *
     * \pre <tt>!!*this</tt>
     */
    BOOST_LOG_EXPORT void detach_from_thread();
};

/*!
 * A free-standing swap function overload for \c basic_record
 */
inline void swap(record& left, record& right)
{
    left.swap(right);
}

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_CORE_RECORD_HPP_INCLUDED_
