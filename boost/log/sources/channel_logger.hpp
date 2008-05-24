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
#include <boost/make_shared.hpp>
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

#ifdef BOOST_LOG_USE_CHAR
    template< >
    struct channel_attribute_name< char >
    {
        static const char* get() { return "Channel"; }
    };
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
    template< >
    struct channel_attribute_name< wchar_t >
    {
        static const wchar_t* get() { return L"Channel"; }
    };
#endif

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
    typedef typename base_type::attribute_set_type attribute_set_type;
    //! String type
    typedef typename base_type::string_type string_type;

    //! Channel attribute type
    typedef attributes::constant< string_type > channel_attribute;

private:
    //! Channel attribute
    shared_ptr< channel_attribute > m_pChannel;

public:
    //! Constructor
    basic_channel_logger() : base_type()
    {
    }
    //! Copy constructor
    basic_channel_logger(basic_channel_logger const& that) :
        base_type(static_cast< base_type const& >(that)),
        m_pChannel(that.m_pChannel)
    {
    }
    //! Constructor with arguments
    template< typename ArgsT >
    explicit basic_channel_logger(ArgsT const& args) :
        base_type(args)
    {
        string_type channel_name =
            args[keywords::channel || &basic_channel_logger< BaseT >::make_default_channel_name];
        if (!channel_name.empty())
        {
            m_pChannel = boost::make_shared< channel_attribute >(channel_name);
            base_type::add_attribute_unlocked(
                aux::channel_attribute_name< char_type >::get(),
                m_pChannel);
        }
    }

protected:
    //! Unlocked swap
    void swap_unlocked(basic_channel_logger& that)
    {
        base_type::swap_unlocked(static_cast< base_type& >(that));
        m_pChannel.swap(that.m_pChannel);
    }

private:
    //! Constructs an empty string as a default value for the channel name
    static string_type make_default_channel_name() { return string_type(); }
};

#ifdef BOOST_LOG_USE_CHAR

//! Narrow-char logger with channel support
BOOST_LOG_DECLARE_LOGGER(channel_logger, (basic_channel_logger));
//! Narrow-char thread-safe logger with channel support
BOOST_LOG_DECLARE_LOGGER_MT(channel_logger_mt, (basic_channel_logger));

#endif // BOOST_LOG_USE_CHAR

#ifdef BOOST_LOG_USE_WCHAR_T

//! Wide-char logger with channel support
BOOST_LOG_DECLARE_WLOGGER(wchannel_logger, (basic_channel_logger));
//! Wide-char thread-safe logger with channel support
BOOST_LOG_DECLARE_WLOGGER_MT(wchannel_logger_mt, (basic_channel_logger));

#endif // BOOST_LOG_USE_WCHAR_T

} // namespace sources

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_SOURCES_CHANNEL_LOGGER_HPP_INCLUDED_
