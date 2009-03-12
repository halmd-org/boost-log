/*
 * (C) 2009 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * This header is the Boost.Log library implementation, see the library documentation
 * at http://www.boost.org/libs/log/doc/log.html.
 */
/*!
 * \file   record_ostream.hpp
 * \author Andrey Semashev
 * \date   09.03.2009
 *
 * This header contains a wrapper class around a logging record that allows to compose the
 * record message with a streaming expression.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_SOURCES_RECORD_OSTREAM_HPP_INCLUDED_
#define BOOST_LOG_SOURCES_RECORD_OSTREAM_HPP_INCLUDED_

#include <new>
#include <string>
#include <ostream>
#include <boost/aligned_storage.hpp>
#include <boost/type_traits/alignment_of.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/detail/unspecified_bool.hpp>
#include <boost/log/detail/attachable_sstream_buf.hpp>
#include <boost/log/record.hpp>

#ifdef _MSC_VER
#pragma warning(push)
 // non dll-interface class 'A' used as base for dll-interface class 'B'
#pragma warning(disable: 4275)
#endif // _MSC_VER

namespace boost {

namespace BOOST_LOG_NAMESPACE {

/*!
 * \brief Logging record adapter with a streaming capability
 *
 * This class allows to compose the logging record message by streaming operations. It
 * aggregates the log record and provides the standard output stream interface.
 */
template< typename CharT, typename TraitsT = std::char_traits< CharT > >
class basic_record_ostream :
    public std::basic_ostream< CharT, TraitsT >
{
    //! Self type
    typedef basic_record_ostream< CharT, TraitsT > this_type;

public:
    //! Stream type
    typedef std::basic_ostream< CharT, TraitsT > ostream_type;
    //! Log record type
    typedef basic_record< CharT > record_type;
    //! Character type
    typedef typename record_type::char_type char_type;
    //! String type to be used as a message text holder
    typedef typename record_type::string_type string_type;

private:
    //! String buffer that is used with the stream
    typedef boost::log::aux::basic_ostringstreambuf< char_type > ostream_buf;

private:
    //! Log record
    record_type m_Record;
    //! Stream buffer
    aligned_storage< sizeof(ostream_buf), alignment_of< ostream_buf >::value > m_Buffer;

public:
    /*!
     * Default constructor. Creates an empty record that is equivalent to the invalid record handle.
     * The stream capability is not available after construction.
     *
     * \post <tt>!*this == true</tt>
     */
    basic_record_ostream() : ostream_type(NULL)
    {
    }

    /*!
     * Conversion from a record handle. Adopts the record referenced by the handle.
     *
     * \pre The handle, if valid, have been issued by the logging core with the same character type as the record being constructed.
     * \post <tt>this->handle() == rec</tt>
     * \param rec The record handle being adopted
     */
    explicit basic_record_ostream(record_handle const& rec) : ostream_type(NULL), m_Record(rec)
    {
        init_stream();
    }
    /*!
     * Conversion from a record object. Adopts the record referenced by the object.
     *
     * \pre The handle, if valid, have been issued by the logging core with the same character type as the record being constructed.
     * \post <tt>this->handle() == rec</tt>
     * \param rec The record handle being adopted
     */
    basic_record_ostream(record_type const& rec) : ostream_type(NULL), m_Record(rec)
    {
        init_stream();
    }

    /*!
     * Destructor. Destroys the record, releases any sinks and attribute values that were involved in processing this record.
     */
    ~basic_record_ostream()
    {
        detach_from_record();
    }

    /*!
     * Conversion to an unspecified boolean type
     *
     * \return \c true, if stream is valid and ready for formatting, \c false, if the stream is not valid. The latter also applies to
     *         the case when the stream is not attached to a log record.
     */
    BOOST_LOG_OPERATOR_UNSPECIFIED_BOOL()

    /*!
     * Inverted conversion to an unspecified boolean type
     *
     * \return \c false, if stream is valid and ready for formatting, \c true, if the stream is not valid. The latter also applies to
     *         the case when the stream is not attached to a log record.
     */
    bool operator! () const
    {
        return (!m_Record || ostream_type::fail());
    }

    /*!
     * Flushes internal buffers to complete all pending formatting operations and returns the aggregated log record
     *
     * \return The aggregated record object
     */
    record_type const& record() const
    {
        const_cast< this_type* >(this)->flush();
        return m_Record;
    }

    /*!
     * If the stream is attached to a log record, flushes internal buffers to complete all pending formatting operations.
     * Then reattaches the stream to another log record.
     *
     * \param rec New log record to attach to
     */
    void record(record_type rec)
    {
        detach_from_record();
        m_Record.swap(rec);
        init_stream();
    }

private:
    //  Copy and assignment are closed
    basic_record_ostream(basic_record_ostream const&);
    basic_record_ostream& operator= (basic_record_ostream const&);

    //! Returns the pointer to the stream buffer storage
    ostream_buf* buffer_storage()
    {
        return reinterpret_cast< ostream_buf* >(m_Buffer.address());
    }

    //! The function initializes the stream and the stream buffer
    void init_stream()
    {
        if (!!m_Record)
        {
            ostream_buf* p = buffer_storage();
            new (p) ostream_buf(m_Record.message());
            ostream_type::init(p);
        }
    }
    //! The function destroys the stream buffer and deinitializes the stream
    void detach_from_record()
    {
        if (!!m_Record)
        {
            ostream_type::flush();
            ostream_type::init(NULL);
            buffer_storage()->~ostream_buf();
        }
    }
};


#ifdef BOOST_LOG_USE_CHAR
typedef basic_record_ostream< char > record_ostream;        //!< Convenience typedef for narrow-character logging
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
typedef basic_record_ostream< wchar_t > wrecord_ostream;    //!< Convenience typedef for wide-character logging
#endif

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_SOURCES_RECORD_OSTREAM_HPP_INCLUDED_
