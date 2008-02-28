/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   channel_logger.hpp
 * \author Andrey Semashev
 * \date   28.02.2008
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_SOURCES_CHANNEL_LOGGER_HPP_INCLUDED_
#define BOOST_LOG_SOURCES_CHANNEL_LOGGER_HPP_INCLUDED_

#include <boost/shared_ptr.hpp>
#include <boost/empty_deleter.hpp>
#include <boost/parameter/keyword.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/sources/basic_logger.hpp>
#include <boost/log/attributes/constant.hpp>

#ifdef _MSC_VER
#pragma warning(push)
// 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
#endif // _MSC_VER

namespace boost {

namespace log {

namespace sources {

namespace keywords {

    BOOST_PARAMETER_KEYWORD(tag, channel)

} // namespace keywords

namespace aux {

    //! A helper traits to get channel attribute name constant in the proper type
    template< typename >
    struct channel_attribute_name;
    template< >
    struct channel_attribute_name< char >
    {
        static const char* get() { return "Channel"; }
    };
    template< >
    struct channel_attribute_name< wchar_t >
    {
        static const wchar_t* get() { return L"Channel"; }
    };

} // namespace aux

//! Logger class
template< typename BaseT >
class basic_channel_logger :
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
    //! String type
    typedef typename base_type::string_type string_type;

    //! Channel attribute type
    typedef attributes::constant< string_type > channel_attribute;

private:
    //! Channel attribute
    channel_attribute m_Channel;

public:
    //! Constructor
    basic_channel_logger() : base_type(), m_Channel(make_default_channel_name())
    {
    }
    //! Copy constructor
    basic_channel_logger(basic_channel_logger const& that) :
        base_type(static_cast< base_type const& >(that)),
        m_Channel(that.m_Channel)
    {
    }
    //! Constructor with arguments
    template< typename ArgsT >
    explicit basic_channel_logger(ArgsT const& args) :
        base_type(args),
        m_Channel(args[keywords::channel || &basic_channel_logger< BaseT >::make_default_channel_name])
    {
        if (!m_Channel.get().empty())
        {
            base_type::add_attribute(
                aux::channel_attribute_name< char_type >::get(),
                shared_ptr< attribute >(&m_Channel, empty_deleter()));
        }
    }

    //! Assignment
    basic_channel_logger& operator= (basic_channel_logger const& that)
    {
        base_type::operator= (static_cast< base_type const& >(that));
        m_Channel = that.m_Channel;

        typename attribute_set::iterator it =
            this->attributes().find(aux::channel_attribute_name< char_type >::get());
        if (it != this->attributes().end())
        {
            // Let the channel attribute point to the local attribute value
            it->second.reset(&m_Channel, empty_deleter());
        }

        return *this;
    }

protected:
    //! Channel attribute accessor
    channel_attribute& channel() { return m_Channel; }
    //! Channel attribute accessor
    channel_attribute const& channel() const { return m_Channel; }

private:
    //! Constructs an empty string as a default value for the channel name
    static string_type make_default_channel_name() { return string_type(); }
};

//! Narrow-char logger with channel support
BOOST_LOG_DECLARE_LOGGER(channel_logger, (basic_channel_logger));

//! Wide-char logger with channel support
BOOST_LOG_DECLARE_WLOGGER(wchannel_logger, (basic_channel_logger));

} // namespace sources

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_SOURCES_CHANNEL_LOGGER_HPP_INCLUDED_
