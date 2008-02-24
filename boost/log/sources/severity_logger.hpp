/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   severity_logger.hpp
 * \author Andrey Semashev
 * \date   08.03.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_SEVERITY_LOGGER_HPP_INCLUDED_
#define BOOST_LOG_SEVERITY_LOGGER_HPP_INCLUDED_

#include <boost/shared_ptr.hpp>
#include <boost/empty_deleter.hpp>
#include <boost/parameter/keyword.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/sources/basic_logger.hpp>
#include <boost/log/attributes/mutable_constant.hpp>

#ifdef _MSC_VER
#pragma warning(push)
// 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
#endif // _MSC_VER

namespace boost {

namespace log {

namespace sources {

namespace keywords {

    BOOST_PARAMETER_KEYWORD(tag, severity)

} // namespace keywords

namespace aux {

    //! A helper traits to get severity attribute name constant in the proper type
    template< typename >
    struct severity_attribute_name;
    template< >
    struct severity_attribute_name< char >
    {
        static const char* get() { return "Severity"; }
    };
    template< >
    struct severity_attribute_name< wchar_t >
    {
        static const wchar_t* get() { return L"Severity"; }
    };

} // namespace aux

//! Logger class
template< typename BaseT >
class basic_severity_logger :
    public BaseT
{
    //! Base type
    typedef BaseT base_type;

public:
    //! Character type
    typedef typename base_type::char_type char_type;
    //! Final type
    typedef typename base_type::final_type final_type;
    //! Attribute set type
    typedef typename base_type::attribute_set attribute_set;
    //! Output stream type
    typedef typename base_type::ostream_type ostream_type;

    //! Severity attribute type
    typedef attributes::mutable_constant< int > severity_attribute;

private:
    //! Default severity
    severity_attribute::held_type m_DefaultSeverity;
    //! Severity attribute
    severity_attribute m_Severity;

public:
    //! Constructor
    basic_severity_logger() :
        base_type(),
        m_DefaultSeverity(0),
        m_Severity(m_DefaultSeverity)
    {
        base_type::add_attribute(
            aux::severity_attribute_name< char_type >::get(),
            shared_ptr< attribute >(&m_Severity, empty_deleter()));
    }
    //! Copy constructor
    basic_severity_logger(basic_severity_logger const& that) :
        base_type(static_cast< base_type const& >(that)),
        m_DefaultSeverity(that.m_DefaultSeverity),
        m_Severity(that.m_Severity)
    {
        base_type::add_attribute(
            aux::severity_attribute_name< char_type >::get(),
            shared_ptr< attribute >(&m_Severity, empty_deleter()));
    }
    //! Constructor with arguments
    template< typename ArgsT >
    explicit basic_severity_logger(ArgsT const& args) :
        base_type(args),
        m_DefaultSeverity(args[keywords::severity | 0]),
        m_Severity(m_DefaultSeverity)
    {
        base_type::add_attribute(
            aux::severity_attribute_name< char_type >::get(),
            shared_ptr< attribute >(&m_Severity, empty_deleter()));
    }

    //! Assignment
    basic_severity_logger& operator= (basic_severity_logger const& that)
    {
        base_type::operator= (static_cast< base_type const& >(that));
        m_Severity = that.m_Severity;
        m_DefaultSeverity = that.m_DefaultSeverity;

        typename attribute_set::iterator it =
            this->attributes().find(aux::severity_attribute_name< char_type >::get());
        if (it != this->attributes().end())
        {
            // Let the severity attribute point to the local attribute value
            it->second.reset(&m_Severity, empty_deleter());
        }
        else
        {
            // Strange, shouldn't ever happen
            base_type::add_attribute(
                aux::severity_attribute_name< char_type >::get(),
                shared_ptr< attribute >(&m_Severity, empty_deleter()));
        }

        return *this;
    }

    //! The method opens a new logging record with the default severity
    bool open_record() { return base_type::open_record(); }

    //! The method allows to assign a severity to the opening record
    template< typename ArgsT >
    bool open_record(ArgsT const& args)
    {
        m_Severity.set_value(args[keywords::severity | m_DefaultSeverity]);
        const bool Result = base_type::open_record();
        if (!Result)
            m_Severity.set_value(m_DefaultSeverity);

        return Result;
    }
    //! The method pushes the constructed message to the sinks and closes the record
    void push_record()
    {
        base_type::push_record();
        m_Severity.set_value(m_DefaultSeverity);
    }

protected:
    //! Severity attribute accessor
    severity_attribute& severity() { return m_Severity; }
    //! Severity attribute accessor
    severity_attribute const& severity() const { return m_Severity; }
    //! Default severity value getter
    severity_attribute::held_type default_severity() const { return m_DefaultSeverity; }
};

//! Narrow-char logger with severity level support
BOOST_LOG_DECLARE_LOGGER(severity_logger, (basic_severity_logger));

//! Wide-char logger with severity level support
BOOST_LOG_DECLARE_WLOGGER(wseverity_logger, (basic_severity_logger));

} // namespace sources

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#define BOOST_LOG_SEV(logger, svty)\
    BOOST_LOG_WITH_PARAMS((logger), (::boost::log::sources::keywords::severity = (svty)))

#endif // BOOST_LOG_SEVERITY_LOGGER_HPP_INCLUDED_
