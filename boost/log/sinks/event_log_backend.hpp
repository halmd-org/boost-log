/*
 * (C) 2008 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * This header is the Boost.Log library implementation, see the library documentation
 * at http://www.boost.org/libs/log/doc/log.html.
 */
/*!
 * \file   event_log_backend.cpp
 * \author Andrey Semashev
 * \date   07.11.2008
 * 
 * The header contains a logging sink backend that uses Windows NT event log API
 * for signaling application events.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_SINKS_EVENT_LOG_BACKEND_HPP_INCLUDED_
#define BOOST_LOG_SINKS_EVENT_LOG_BACKEND_HPP_INCLUDED_

#include <string>
#include <boost/function/function1.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/sinks/basic_sink_backend.hpp>
#include <boost/log/sinks/attribute_mapping.hpp>
#include <boost/log/sinks/event_log_constants.hpp>
#include <boost/log/sinks/event_log_keywords.hpp>

#ifdef _MSC_VER
#pragma warning(push)
// 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
// non dll-interface class 'A' used as base for dll-interface class 'B'
#pragma warning(disable: 4275)
#endif // _MSC_VER

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace sinks {

namespace event_log {

    /*!
     * \brief Straightforward event type mapping
     * 
     * This type of mapping assumes that attribute with a particular name always
     * provides values that map directly onto the native event types. The mapping
     * simply returns the extracted attribute value converted to the native event type.
     */
    template< typename CharT, typename AttributeValueT = int >
    class direct_event_type_mapping :
        public basic_direct_mapping< CharT, event_type_t, AttributeValueT >
    {
        //! Base type
        typedef basic_direct_mapping< CharT, event_type_t, AttributeValueT > base_type;

    public:
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        /*!
         * Constructor
         * 
         * \param name Attribute name
         */
        explicit direct_event_type_mapping(string_type const& name) :
            base_type(name, info)
        {
        }
    };

    /*!
     * \brief Customizable event type mapping
     * 
     * The class allows to setup a custom mapping between an attribute and native event types.
     * The mapping should be initialized similarly to the standard \c map container, by using
     * indexing operator and assignment.
     */
    template< typename CharT, typename AttributeValueT = int >
    class custom_event_type_mapping :
        public basic_custom_mapping< CharT, event_type_t, AttributeValueT >
    {
        //! Base type
        typedef basic_custom_mapping< CharT, event_type_t, AttributeValueT > base_type;

    public:
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        /*!
         * Constructor
         * 
         * \param name Attribute name
         */
        explicit custom_event_type_mapping(string_type const& name) :
            base_type(name, info)
        {
        }
    };

} // namespace event_log

/*!
 * \brief An implementation of a simple logging sink backend that emits events into Windows NT event log
 *
 * The sink uses Windows NT 5 (Windows 2000) and later event log API to emit events
 * to an event log. The sink acts as an event source, it implements all needed resources and
 * source registration in the Windows registry that is needed for the event delivery.
 *
 * The backend performs message text formatting. The composed text is then passed as the first
 * and only string parameter of the event. The resource embedded into the backend describes the event
 * so that the parameter is inserted into the event description text, thus making it visible
 * in the event log.
 *
 * The backend allows to customize mapping of application severity levels to the native Windows event types.
 * This allows to write portable code even if OS-specific sinks, such as this one, are used.
 *
 * \note Since the backend registers itself into Windows registry as the resource file that contains
 *       event description, it is important to keep the library binary in a stable place of the filesystem.
 *       Otherwise Windows might not be able to load event resources from the library and display
 *       events correctly.
 *
 * \note It is known that Windows is not able to find event resources in the application executable,
 *       which is linked against the static build of the library. Users are advised to use dynamic
 *       builds of the library to solve this problem.
 */
template< typename CharT >
class BOOST_LOG_EXPORT basic_simple_event_log_backend :
    public basic_formatting_sink_backend< CharT >
{
    //! Base type
    typedef basic_formatting_sink_backend< CharT > base_type;
    //! Implementation type
    struct implementation;

public:
    //! Character type
    typedef typename base_type::char_type char_type;
    //! String type
    typedef typename base_type::string_type string_type;
    //! String type to be used as a message text holder
    typedef typename base_type::target_string_type target_string_type;
    //! Attribute values view type
    typedef typename base_type::values_view_type values_view_type;

    //! Mapper type for the event type
    typedef boost::function1<
        event_log::event_type_t,
        values_view_type const&
    > event_type_mapper_type;

private:
    //! Pointer to the backend implementation that hides various types from windows.h
    implementation* m_pImpl;

public:
    /*!
     * Default constructor. Registers event source with name based on the application
     * executable file name in the Application log. If such a registration is already
     * present, it is not overridden.
     */
    basic_simple_event_log_backend();
    /*!
     * Constructor. Registers event log source with the specified parameters.
     * The following named parameters are supported:
     *
     * \li \c log_name - Specifies the log in which the source should be registered.
     *     The result of \c get_default_log_name is used, if the parameter is not specified.
     * \li \c log_source - Specifies the source name. The result of \c get_default_source_name
     *     is used, if the parameter is not specified.
     * \li \c force - If \c true and Windows registry already contains the log source
     *     registration, the registry parameters are overwritten. If \c false, the registry
     *     is only modified if the log source was not previously registered. Default value: \c false.
     *
     * \param args A set of named parameters.
     */
    template< typename ArgsT >
    explicit basic_simple_event_log_backend(ArgsT const& args) : m_pImpl(construct(
        args[keywords::log_name || &basic_simple_event_log_backend::get_default_log_name],
        args[keywords::log_source || &basic_simple_event_log_backend::get_default_source_name],
        args[keywords::force | false]))
    {
    }

    /*!
     * Destructor. Unregisters event provider.
     */
    ~basic_simple_event_log_backend();

    /*!
     * The method installs the function object that maps application severity levels to WinAPI event types
     */
    void set_event_type_mapper(event_type_mapper_type const& mapper);

    /*!
     * \returns Default log name: Application
     */
    static string_type get_default_log_name();
    /*!
     * \returns Default log source name that is based on the application executable file name and the sink name
     */
    static string_type get_default_source_name();

private:
    //! The method puts the formatted message to the event log
    void do_write_message(values_view_type const& values, target_string_type const& formatted_message);

    //! Constructs backend implementation
    static implementation* construct(string_type const& log_name, string_type const& source_name, bool force);
};

#ifdef BOOST_LOG_USE_CHAR
typedef basic_simple_event_log_backend< char > simple_event_log_backend;      //!< Convenience typedef for narrow-character logging
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
typedef basic_simple_event_log_backend< wchar_t > wsimple_event_log_backend;  //!< Convenience typedef for wide-character logging
#endif

} // namespace sinks

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_SINKS_EVENT_LOG_BACKEND_HPP_INCLUDED_
