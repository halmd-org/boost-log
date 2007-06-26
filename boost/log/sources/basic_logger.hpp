/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   basic_logger.hpp
 * \author Andrey Semashev
 * \date   08.03.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_BASIC_LOGGER_HPP_INCLUDED_
#define BOOST_LOG_BASIC_LOGGER_HPP_INCLUDED_

#include <ostream>
#include <streambuf>
#include <boost/shared_ptr.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/logging_core.hpp>
#include <boost/log/attributes/attribute_set.hpp>

#ifndef BOOST_LOG_DEFAULT_LOGGER_BUFFER_SIZE
//! The size (in chars) of a stream buffer used by logger. It affects logger object size.
//! \note The Boost.Log library should be rebuilt once this value is modified.
#define BOOST_LOG_DEFAULT_LOGGER_BUFFER_SIZE 16
#endif // BOOST_LOG_DEFAULT_LOGGER_BUFFER_SIZE

#ifdef _MSC_VER
#pragma warning(push)
// 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
#endif // _MSC_VER

namespace boost {

namespace log {

namespace aux {

    //! An output streambuf
    template< typename CharT >
    class BOOST_LOG_EXPORT basic_ostream_writer :
        public std::basic_streambuf< CharT >
    {
        //! Base type
        typedef std::basic_streambuf< CharT > base_type;

    public:
        //! Character type
        typedef typename base_type::char_type char_type;
        //! String type
        typedef std::basic_string< char_type > string_type;
        //! Traits type
        typedef typename base_type::traits_type traits_type;
        //! Int type
        typedef typename base_type::int_type int_type;

    private:
        //! A reference to the string that will hold the message
        string_type& m_Message;
        //! A buffer used to temporarily store output
        char_type m_Buffer[BOOST_LOG_DEFAULT_LOGGER_BUFFER_SIZE];

    private:
        //! Copy constructor (closed)
        basic_ostream_writer(basic_ostream_writer const& that);
        //! Assignment (closed)
        basic_ostream_writer& operator= (basic_ostream_writer const& that);

    public:
        //! Constructor
        explicit basic_ostream_writer(string_type& message);
        //! Destructor
        ~basic_ostream_writer();

    protected:
        //! Puts all buffered data to the string
        int sync();
        //! Puts an unbuffered character to the string
        int_type overflow(int_type c);
        //! Puts a character sequence to the string
        std::streamsize xsputn(const char_type* s, std::streamsize n);
    };

    //! Record pump implementation
    template< typename LoggerT >
    class record_pump
    {
    public:
        //! The metafunction allows to adopt the pump to anothe logger type
        template< typename T >
        struct rebind
        {
            typedef record_pump< T > other;
        };

    private:
        //! Logger type
        typedef LoggerT logger_type;
        //! Output stream type
        typedef typename logger_type::ostream_type ostream_type;

    protected:
        //! A reference to the logger
        mutable logger_type* m_pLogger;

    public:
        //! Constructor
        explicit record_pump(logger_type* p) : m_pLogger(p) {}
        //! Copy constructor (implemented as move)
        record_pump(record_pump const& that) : m_pLogger(that.m_pLogger)
        {
            that.m_pLogger = 0;
        }
        //! Destructor
        ~record_pump()
        {
            if (m_pLogger)
                m_pLogger->push_record();
        }

        //! Forwarding output operators
        template< typename T >
        record_pump const& operator<< (T const& value) const
        {
            m_pLogger->_pump_stream() << value;
            return *this;
        }

    private:
        //! Closed assignment
        record_pump& operator= (record_pump const&);
    };

} // namespace aux

//! Logger class
template< typename CharT >
class basic_logger
{
public:
    //! Character type
    typedef CharT char_type;
    //! String type to be used as a message text holder
    typedef std::basic_string< char_type > string_type;
    //! Attribute set type
    typedef basic_attribute_set< char_type > attribute_set;
    //! Logging system core type
    typedef basic_logging_core< char_type > logging_core_type;
    //! Output stream type
    typedef std::basic_ostream< char_type > ostream_type;

protected:
    //! Stream device type
    typedef aux::basic_ostream_writer< char_type > ostream_writer;
    //! Record pump type
    typedef aux::record_pump< basic_logger< char_type > > record_pump_type;

private:
    //! A pointer to the logging system
    shared_ptr< logging_core_type > m_pLoggingSystem;

    //! The string to be written to
    string_type m_Message;
    //! The streambuf
    ostream_writer m_StreamBuf;
    //! Output stream
    ostream_type m_Stream;

    //! Logger-specific attribute set
    attribute_set m_Attributes;

public:
    //! Constructor
    BOOST_LOG_EXPORT basic_logger();
    //! Copy constructor
    BOOST_LOG_EXPORT basic_logger(basic_logger const& that);
    //! Destructor
    BOOST_LOG_EXPORT ~basic_logger();
    //! Assignment
    BOOST_LOG_EXPORT basic_logger& operator= (basic_logger const& that);

    //! Logging stream getter
    record_pump_type strm()
    {
        return record_pump_type(this);
    }

    //! The method adds an attribute to the logger
    BOOST_LOG_EXPORT typename attribute_set::iterator add_attribute(
        string_type const& name, shared_ptr< attribute > const& attr);
    //! The method removes an attribute from the logger
    BOOST_LOG_EXPORT void remove_attribute(typename attribute_set::iterator it);

    //! The method checks if the message passes filters to be output by at least one sink and opens a record if it does
    BOOST_LOG_EXPORT bool open_record();
    //! The method pushes the constructed message to the sinks and closes the record
    BOOST_LOG_EXPORT void push_record();

    //! Implementation detail - an accessor to the logging stream for th pump
    ostream_type& _pump_stream() { return m_Stream; }

protected:
    //! An accessor to the logging system pointer
    shared_ptr< logging_core_type > const& core() const { return m_pLoggingSystem; }
    //! An accessor to the output stream
    ostream_type& stream() { return m_Stream; }
    //! An accessor to the output stream
    ostream_type const& stream() const { return m_Stream; }
    //! An accessor to the logger attributes
    attribute_set& attributes() { return m_Attributes; }
    //! An accessor to the logger attributes
    attribute_set const& attributes() const { return m_Attributes; }
};

typedef basic_logger< char > logger;
typedef basic_logger< wchar_t > wlogger;

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#define BOOST_LOG(logger)\
    if (!logger.open_record())\
        ((void)0);\
    else\
        logger.strm()

#endif // BOOST_LOG_BASIC_LOGGER_HPP_INCLUDED_
