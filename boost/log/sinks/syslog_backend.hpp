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

#ifndef BOOST_LOG_SINKS_SYSLOG_BACKEND_HPP_INCLUDED_HPP_
#define BOOST_LOG_SINKS_SYSLOG_BACKEND_HPP_INCLUDED_HPP_

#include <map>
#include <string>
#include <locale>
#include <ostream>
#include <functional>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function/function1.hpp>
#include <boost/function/function3.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/sinks/basic_sink_backend.hpp>
#include <boost/log/sinks/threading_models.hpp>
#include <boost/log/sinks/syslog_constants.hpp>
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

    //! Base class for syslog level mapping function objects
    template< typename CharT >
    struct basic_level_mapping :
        public std::unary_function< basic_attribute_values_view< CharT >, level_t >
    {
        //! Char type
        typedef CharT char_type;
        //! String type
        typedef std::basic_string< char_type > string_type;
        //! Attribute values view type
        typedef basic_attribute_values_view< char_type > values_view_type;
    };

    /*!
     * \brief Straightforward syslog level extractor
     * 
     * The level extractor assumes that attribute with a particular name always
     * provides values that map directly onto syslog levels. The level extractor
     * simply returns the extracted attribute value as a syslog level.
     */
    template< typename CharT, typename AttributeValueT = int >
    class straightforward_level_mapping :
        public basic_level_mapping< CharT >
    {
        //! Base type
        typedef basic_level_mapping< CharT > base_type;

    public:
        //! Attribute contained value type
        typedef AttributeValueT attribute_value_type;
        //! String type
        typedef typename base_type::string_type string_type;
        //! Attribute values view type
        typedef typename base_type::values_view_type values_view_type;

    private:
        //! Extracted attribute name
        const string_type m_AttributeName;

    public:
        /*!
         * Constructor
         * 
         * \param name Attribute name
         */
        explicit straightforward_level_mapping(string_type const& name) : m_AttributeName(name) {}

        /*!
         * Extraction operator
         * 
         * \param values A set of attribute values attached to a logging record
         * \return An extracted attribute value
         */
        level_t operator() (values_view_type const& values) const
        {
            // Find attribute value
            typename values_view_type::const_iterator it = values.find(m_AttributeName);
            if (it != values.end())
            {
                optional< attribute_value_type const& > value = it->second->get< attribute_value_type >();
                if (!!value)
                    return make_level(value.get());
            }
            return info;
        }
    };

    /*!
     * \brief Customizable syslog level extractor
     * 
     * The level extractor allows to setup a custom mapping between an attribute and syslog levels
     */
    template< typename CharT, typename AttributeValueT = int >
    class level_mapping :
        public basic_level_mapping< CharT >
    {
        //! Base type
        typedef basic_level_mapping< CharT > base_type;

    public:
        //! Attribute contained value type
        typedef AttributeValueT attribute_value_type;
        //! String type
        typedef typename base_type::string_type string_type;
        //! Attribute values view type
        typedef typename base_type::values_view_type values_view_type;

    private:
        //! \cond

        //! Mapping type
        typedef std::map< attribute_value_type, level_t > mapping_type;
        //! Smart reference class for implementing insertion into the map
        class reference_proxy;
        friend class reference_proxy;
        class reference_proxy
        {
            mapping_type& m_Mapping;
            attribute_value_type m_Key;

        public:
            //! Constructor
            reference_proxy(mapping_type& mapping, attribute_value_type const& key) : m_Mapping(mapping), m_Key(key) {}
            //! Insertion
            reference_proxy const& operator= (level_t const& val) const
            {
                m_Mapping[m_Key] = val;
                return *this;
            }
        };

        //! \endcond

    private:
        //! Extracted attribute name
        const string_type m_AttributeName;
        //! Conversion mapping
        mapping_type m_Mapping;

    public:
        /*!
         * Constructor
         * 
         * \param name Attribute name
         */
        explicit level_mapping(string_type const& name) : m_AttributeName(name) {}
        /*!
         * Extraction operator. Extracts the attribute value and attempts to map it onto
         * a syslog level value.
         * 
         * \param values A set of attribute values attached to a logging record
         * \return A mapped syslog level, if mapping is successfull, or \c info if
         *         mapping did not succeed.
         */
        level_t operator() (values_view_type const& values) const
        {
            // Find attribute value
            typename values_view_type::const_iterator it = values.find(m_AttributeName);
            if (it != values.end())
            {
                optional< attribute_value_type const& > value = it->second->get< attribute_value_type >();
                if (!!value)
                {
                    typename mapping_type::const_iterator mapping = m_Mapping.find(value.get());
                    if (mapping != m_Mapping.end())
                        return mapping->second;
                }
            }
            return info;
        }
        /*!
         * Insertion operator
         * 
         * \param key Attribute value to be mapped
         * \return An object of unspecified type that allows to insert a new mapping through assignment.
         *         The \a key argument becomes the key attribute value, and the assigned value becomes the
         *         mapped syslog level.
         */
#ifndef BOOST_LOG_DOXYGEN_PASS
        reference_proxy operator[] (attribute_value_type const& key)
#else
        implementation_defined operator[] (attribute_value_type const& key)
#endif
        {
            return reference_proxy(m_Mapping, key);
        }
    };

} // namespace syslog

//! An implementation of a syslog sink backend
template< typename CharT >
class BOOST_LOG_EXPORT basic_syslog_backend :
    public basic_sink_backend< CharT, frontend_synchronization_tag >
{
    //! Base type
    typedef basic_sink_backend< CharT, frontend_synchronization_tag > base_type;
    //! Implementation type
    struct implementation;

public:
    //! Character type
    typedef typename base_type::char_type char_type;
    //! String type
    typedef typename base_type::string_type string_type;
    //! Attribute values view type
    typedef typename base_type::values_view_type values_view_type;
    //! Output stream type
    typedef std::basic_ostream< char_type > stream_type;
    //! Formatter function type
    typedef boost::function3<
        void,
        stream_type&,
        values_view_type const&,
        string_type const&
    > formatter_type;
    //! Syslog level extractor type
    typedef boost::function1<
        syslog::level_t,
        values_view_type const&
    > level_extractor_type;

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
     * The method sets formatter functional object
     * 
     * \param fmt Formatter object
     */
    void set_formatter(formatter_type const& fmt);
    /*!
     * The method resets the formatter. If the formatter is not set, the result of formatting
     * is equivalent to the log record message text.
     */
    void reset_formatter();

    /*!
     * The method installs the syslog record level extraction function object
     */
    void set_level_extractor(level_extractor_type const& extractor);

    /*!
     * The method returns the current locale used for formatting
     */
    std::locale getloc() const;
    /*!
     * The method sets the locale used for formatting
     */
    std::locale imbue(std::locale const& loc);

    /*!
     * The method formats the message and passes it to the syslog API.
     * 
     * \param attributes A set of attribute values attached to the log record
     * \param message Log record message text
     */
    void write_message(values_view_type const& attributes, string_type const& message);
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

#endif // BOOST_LOG_SINKS_SYSLOG_BACKEND_HPP_INCLUDED_HPP_
