/*
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * This header is the Boost.Log library implementation, see the library documentation
 * at http://www.boost.org/libs/log/doc/log.html.
 */
/*!
 * \file   syslog_backend.hpp
 * \author Andrey Semashev
 * \date   08.01.2008
 * 
 * The header contains implementation of a Syslog sink backend along with its setup facilities.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_SINKS_SYSLOG_BACKEND_HPP_INCLUDED_
#define BOOST_LOG_SINKS_SYSLOG_BACKEND_HPP_INCLUDED_

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/function/function1.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/sinks/basic_sink_backend.hpp>
#include <boost/log/sinks/syslog_constants.hpp>
#include <boost/log/sinks/attribute_mapping.hpp>
#include <boost/log/attributes/attribute_values_view.hpp>

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

namespace syslog {

    /*!
     * \brief Straightforward severity level mapping
     * 
     * This type of mapping assumes that attribute with a particular name always
     * provides values that map directly onto the Syslog levels. The mapping
     * simply returns the extracted attribute value converted to the Syslog severity level.
     */
    template< typename CharT, typename AttributeValueT = int >
    class basic_direct_severity_mapping :
        public basic_direct_mapping< CharT, level_t, AttributeValueT >
    {
        //! Base type
        typedef basic_direct_mapping< CharT, level_t, AttributeValueT > base_type;

    public:
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        /*!
         * Constructor
         * 
         * \param name Attribute name
         */
        explicit direct_severity_mapping(string_type const& name) :
            base_type(name, info)
        {
        }
    };

    /*!
     * \brief Customizable severity level mapping
     * 
     * The class allows to setup a custom mapping between an attribute and Syslog severity levels.
     * The mapping should be initialized similarly to the standard \c map container, by using
     * indexing operator and assignment.
     */
    template< typename CharT, typename AttributeValueT = int >
    class basic_custom_severity_mapping :
        public basic_custom_mapping< CharT, level_t, AttributeValueT >
    {
        //! Base type
        typedef basic_custom_mapping< CharT, level_t, AttributeValueT > base_type;

    public:
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        /*!
         * Constructor
         * 
         * \param name Attribute name
         */
        explicit custom_severity_mapping(string_type const& name) :
            base_type(name, info)
        {
        }
    };

#ifdef BOOST_LOG_USE_CHAR

    /*!
     * \brief Straightforward severity level mapping
     * 
     * This is a convenience template typedef over \c basic_direct_severity_mapping
     * for narrow-character logging.
     */
    template< typename AttributeValueT = int >
    class direct_severity_mapping :
        public basic_direct_severity_mapping< char, AttributeValueT >
    {
        //! Base type
        typedef basic_direct_severity_mapping< char, AttributeValueT > base_type;

    public:
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        /*!
         * Constructor
         * 
         * \param name Attribute name
         */
        explicit direct_severity_mapping(string_type const& name) : base_type(name)
        {
        }
    };

    /*!
     * \brief Customizable severity level mapping
     * 
     * This is a convenience template typedef over \c basic_custom_severity_mapping
     * for narrow-character logging.
     */
    template< typename AttributeValueT = int >
    class custom_severity_mapping :
        public basic_custom_severity_mapping< char, AttributeValueT >
    {
        //! Base type
        typedef basic_custom_severity_mapping< char, AttributeValueT > base_type;

    public:
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        /*!
         * Constructor
         * 
         * \param name Attribute name
         */
        explicit custom_severity_mapping(string_type const& name) : base_type(name)
        {
        }
    };

#endif // BOOST_LOG_USE_CHAR

#ifdef BOOST_LOG_USE_WCHAR_T

    /*!
     * \brief Straightforward severity level mapping
     * 
     * This is a convenience template typedef over \c basic_direct_severity_mapping
     * for wide-character logging.
     */
    template< typename AttributeValueT = int >
    class wdirect_severity_mapping :
        public basic_direct_severity_mapping< wchar_t, AttributeValueT >
    {
        //! Base type
        typedef basic_direct_severity_mapping< wchar_t, AttributeValueT > base_type;

    public:
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        /*!
         * Constructor
         * 
         * \param name Attribute name
         */
        explicit wdirect_severity_mapping(string_type const& name) : base_type(name)
        {
        }
    };

    /*!
     * \brief Customizable severity level mapping
     * 
     * This is a convenience template typedef over \c basic_custom_severity_mapping
     * for wide-character logging.
     */
    template< typename AttributeValueT = int >
    class wcustom_severity_mapping :
        public basic_custom_severity_mapping< wchar_t, AttributeValueT >
    {
        //! Base type
        typedef basic_custom_severity_mapping< wchar_t, AttributeValueT > base_type;

    public:
        //! String type
        typedef typename base_type::string_type string_type;

    public:
        /*!
         * Constructor
         * 
         * \param name Attribute name
         */
        explicit wcustom_severity_mapping(string_type const& name) : base_type(name)
        {
        }
    };

#endif // BOOST_LOG_USE_WCHAR_T

} // namespace syslog

//! An implementation of a syslog sink backend
template< typename CharT >
class BOOST_LOG_EXPORT basic_syslog_backend :
    public basic_formatting_sink_backend< CharT, char >
{
    //! Base type
    typedef basic_formatting_sink_backend< CharT, char > base_type;
    //! Implementation type
    struct implementation;

public:
    //! Character type
    typedef typename base_type::char_type char_type;
    //! String type
    typedef typename base_type::string_type string_type;
    //! String type that is used to pass message test
    typedef typename base_type::target_string_type target_string_type;
    //! Attribute values view type
    typedef typename base_type::values_view_type values_view_type;

    //! Syslog severity level mapper type
    typedef boost::function1<
        syslog::level_t,
        values_view_type const&
    > severity_mapper_type;

private:
    //! Pointer to the implementation
    implementation* m_pImpl;

public:
    /*!
     * Constructor. The first constructed syslog backend initializes syslog API with the provided parameters.
     * 
     * \param facility Logging facility
     * \param options Additional syslog initialization options
     */
    explicit basic_syslog_backend(
        syslog::facility_t facility = syslog::user,
        syslog::options_t options = syslog::no_delay);
    /*!
     * Destructor
     */
    ~basic_syslog_backend();

    /*!
     * The method installs the function object that maps application severity levels to Syslog levels
     */
    void set_severity_mapper(severity_mapper_type const& mapper);

private:
    //! The method passes the formatted message to the Syslog API
    void do_write_message(values_view_type const& attributes, target_string_type const& formatted_message);
};

#ifdef BOOST_LOG_USE_CHAR
typedef basic_syslog_backend< char > syslog_backend;        //!< Convenience typedef for narrow-character logging
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
typedef basic_syslog_backend< wchar_t > wsyslog_backend;    //!< Convenience typedef for wide-character logging
#endif

} // namespace sinks

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_SINKS_SYSLOG_BACKEND_HPP_INCLUDED_
