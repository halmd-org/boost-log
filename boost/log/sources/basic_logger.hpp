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

#include <string>
#include <utility>
#include <ostream>
#include <boost/shared_ptr.hpp>
#include <boost/utility/addressof.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/logging_core.hpp>
#include <boost/log/attributes/attribute_set.hpp>
#include <boost/log/detail/attachable_sstream_buf.hpp>

#ifdef _MSC_VER
#pragma warning(push)
// 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
#endif // _MSC_VER

namespace boost {

namespace log {

namespace sources {

template< typename CharT, typename FinalT >
class basic_logger;

namespace aux {

    //! Record pump implementation
    template< typename LoggerT >
    class record_pump
    {
    public:
        //! The metafunction allows to adopt the pump to another logger type
        template< typename T >
        struct rebind
        {
            typedef record_pump< T > other;
        };

    private:
        //! Logger type
        typedef LoggerT logger_type;
        //! Character type
        typedef typename logger_type::char_type char_type;
        //! Basic logger type
        typedef basic_logger< char_type, logger_type > basic_logger_type;
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
            static_cast< basic_logger_type* >(m_pLogger)->stream() << value;
            return *this;
        }

    private:
        //! Closed assignment
        record_pump& operator= (record_pump const&);
    };

} // namespace aux

//! Logger class
template< typename CharT, typename FinalT >
class basic_logger
{
    friend class aux::record_pump< FinalT >;

public:
    //! Character type
    typedef CharT char_type;
    //! Final logger type
    typedef FinalT final_type;

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
    typedef boost::log::aux::basic_ostringstreambuf< char_type > ostream_writer;
    //! Record pump type
    typedef aux::record_pump< final_type > record_pump_type;

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
    basic_logger() :
        m_pLoggingSystem(logging_core_type::get()),
        m_StreamBuf(m_Message),
        m_Stream(&m_StreamBuf)
    {
    }
    //! Copy constructor
    basic_logger(basic_logger const& that) :
        m_pLoggingSystem(logging_core_type::get()),
        m_StreamBuf(m_Message),
        m_Stream(&m_StreamBuf),
        m_Attributes(that.m_Attributes)
    {
    }
    //! Constructor with arguments
    template< typename ArgsT >
    explicit basic_logger(ArgsT const& args) :
        m_pLoggingSystem(logging_core_type::get()),
        m_StreamBuf(m_Message),
        m_Stream(&m_StreamBuf)
    {
    }
    //! Destructor
    virtual ~basic_logger() {}
    //! Assignment
    basic_logger& operator= (basic_logger const& that)
    {
        m_Attributes = that.m_Attributes;
        return *this;
    }

    //! Logging stream getter
    record_pump_type strm()
    {
        BOOST_LOG_ASSUME(this != NULL);
        return record_pump_type(static_cast< final_type* >(this));
    }

    //! The method adds an attribute to the logger
    std::pair< typename attribute_set::iterator, bool > add_attribute(
        string_type const& name, shared_ptr< attribute > const& attr)
    {
        return m_Attributes.insert(std::make_pair(name, attr));
    }
    //! The method removes an attribute from the logger
    void remove_attribute(typename attribute_set::iterator it)
    {
        m_Attributes.erase(it);
    }

    //! The method removes all attributes from the logger
    void remove_all_attributes()
    {
        m_Attributes.clear();
    }

    //! The method checks if the message passes filters to be output by at least one sink and opens a record if it does
    bool open_record()
    {
        return m_pLoggingSystem->open_record(m_Attributes);
    }
    //! The method checks if the message passes filters to be output by at least one sink and opens a record if it does
    template< typename ArgsT >
    bool open_record(ArgsT const& args)
    {
        return m_pLoggingSystem->open_record(m_Attributes);
    }
    //! The method pushes the constructed message to the sinks and closes the record
    void push_record()
    {
        m_Stream.flush();
        m_pLoggingSystem->push_record(m_Message);
        m_Message.clear();
    }

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

//! Narrow-char logger
class logger :
    public basic_logger< char, logger >
{
public:
    logger& operator= (logger const& that)
    {
        if (this != &that)
            basic_logger< char, logger >::operator= (that);
        return *this;
    }
};
//! Wide-char logger
class wlogger :
    public basic_logger< wchar_t, wlogger >
{
public:
    wlogger& operator= (wlogger const& that)
    {
        if (this != &that)
            basic_logger< wchar_t, wlogger >::operator= (that);
        return *this;
    }
};

} // namespace sources

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

//! The macro writes a record to the log
#define BOOST_LOG(logger)\
    if (!(logger).open_record())\
        ((void)0);\
    else\
        (logger).strm()

//! The macro writes a record to the log and allows to pass additional arguments to the logger
#define BOOST_LOG_WITH_PARAMS(logger, params_seq)\
    if (!(logger).open_record((BOOST_PP_SEQ_ENUM(params_seq))))\
        ((void)0);\
    else\
        (logger).strm()


#define BOOST_LOG_DECLARE_LOGGER_TYPE_INTERNAL1(r, data, elem) elem<
#define BOOST_LOG_DECLARE_LOGGER_TYPE_INTERNAL2(r, data, elem) >

#ifndef BOOST_LOG_MAX_CTOR_FORWARD_ARGS
//! The maximum number of arguments that can be forwarded by the logger constructor to its bases
#define BOOST_LOG_MAX_CTOR_FORWARD_ARGS 16
#endif

#define BOOST_LOG_CTOR_FORWARD(z, n, data)\
    template< BOOST_PP_ENUM_PARAMS(n, typename T) >\
    explicit data(BOOST_PP_ENUM_BINARY_PARAMS(n, T, const& arg)) : base_type((BOOST_PP_ENUM_PARAMS(n, arg))) {}

/*!
 *  \brief The macro declares a logger class that inherits a number of base classes
 * 
 *  \param type_name The name of the logger class to declare
 *  \param char_type The character type of the logger. Either char or wchar_t expected.
 *  \param base_seq A Boost.Preprocessor sequence of type identifiers of the base classes templates
 */
#define BOOST_LOG_DECLARE_LOGGER_TYPE(type_name, char_type, base_seq)\
    class type_name :\
        public BOOST_PP_SEQ_FOR_EACH(BOOST_LOG_DECLARE_LOGGER_TYPE_INTERNAL1, ~, base_seq)\
            ::boost::log::sources::basic_logger< char_type, type_name >\
            BOOST_PP_SEQ_FOR_EACH(BOOST_LOG_DECLARE_LOGGER_TYPE_INTERNAL2, ~, base_seq)\
    {\
        typedef BOOST_PP_SEQ_FOR_EACH(BOOST_LOG_DECLARE_LOGGER_TYPE_INTERNAL1, ~, base_seq)\
            ::boost::log::sources::basic_logger< char_type, type_name >\
            BOOST_PP_SEQ_FOR_EACH(BOOST_LOG_DECLARE_LOGGER_TYPE_INTERNAL2, ~, base_seq) base_type;\
    public:\
        type_name() {}\
        type_name(type_name const& that) : base_type(static_cast< base_type const& >(that)) {}\
        BOOST_PP_REPEAT_FROM_TO(1, BOOST_LOG_MAX_CTOR_FORWARD_ARGS, BOOST_LOG_CTOR_FORWARD, type_name)\
        type_name& operator= (type_name const& that)\
        {\
            if (this != ::boost::addressof(that))\
                base_type::operator= (static_cast< base_type const& >(that));\
            return *this;\
        }\
    }

/*!
 *  \brief The macro declares a narrow-char logger class that inherits a number of base classes
 * 
 *  Equivalent to BOOST_LOG_DECLARE_LOGGER_TYPE(type_name, char, base_seq)
 * 
 *  \param type_name The name of the logger class to declare
 *  \param base_seq A Boost.Preprocessor sequence of type identifiers of the base classes templates
 */
#define BOOST_LOG_DECLARE_LOGGER(type_name, base_seq) BOOST_LOG_DECLARE_LOGGER_TYPE(type_name, char, base_seq)
/*!
 *  \brief The macro declares a wide-char logger class that inherits a number of base classes
 * 
 *  Equivalent to BOOST_LOG_DECLARE_LOGGER_TYPE(type_name, wchar_t, base_seq)
 * 
 *  \param type_name The name of the logger class to declare
 *  \param base_seq A Boost.Preprocessor sequence of type identifiers of the base classes templates
 */
#define BOOST_LOG_DECLARE_WLOGGER(type_name, base_seq) BOOST_LOG_DECLARE_LOGGER_TYPE(type_name, wchar_t, base_seq)

#endif // BOOST_LOG_BASIC_LOGGER_HPP_INCLUDED_
