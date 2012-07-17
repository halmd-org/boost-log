/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   settings.hpp
 * \author Andrey Semashev
 * \date   11.10.2009
 *
 * The header contains definition of the library settings container.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_UTILITY_INIT_SETTINGS_HPP_INCLUDED_
#define BOOST_LOG_UTILITY_INIT_SETTINGS_HPP_INCLUDED_

#include <cstddef>
#include <string>
#include <boost/assert.hpp>
#include <boost/move/move.hpp>
#include <boost/mpl/if.hpp>
#include <boost/optional/optional.hpp>
#include <boost/property_tree/ptree_fwd.hpp>
#include <boost/log/detail/setup_prologue.hpp>
#include <boost/log/detail/native_typeof.hpp>
#include <boost/log/utility/explicit_operator_bool.hpp>
#if !defined(BOOST_LOG_TYPEOF)
#include <boost/utility/enable_if.hpp>
#endif

#ifdef _MSC_VER
#pragma warning(push)
// 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
// non dll-interface class 'A' used as base for dll-interface class 'B'
#pragma warning(disable: 4275)
#endif // _MSC_VER

namespace boost {

namespace BOOST_LOG_NAMESPACE {

#if defined(BOOST_LOG_TYPEOF) && defined(BOOST_LOG_NO_TRAILING_RESULT_TYPE)
namespace aux {

template< typename T >
struct make_value
{
    static T const& get();
};

} // namespace aux
#endif

/*!
 * \brief The class represents a reference to the settings container section
 *
 * The section refers to a sub-tree of the library settings container. It does not
 * own the referred sub-tree but allows for convenient access to parameters within the subsection.
 */
template< typename CharT >
class basic_settings_section
{
public:
    //! Character type
    typedef CharT char_type;
    //! String type
    typedef std::basic_string< char_type > string_type;
    //! Property tree type
    typedef basic_ptree< std::string, string_type > property_tree_type;

private:
#if !defined(BOOST_LOG_DOXYGEN_PASS)

    //! A reference proxy object
#ifndef BOOST_LOG_NO_MEMBER_TEMPLATE_FRIENDS
    template< bool IsConstV >
    class param_reference;
    template< bool IsConstV >
    friend class param_reference;
#endif
    template< bool IsConstV >
    class param_reference
    {
    private:
        typedef typename mpl::if_c<
            IsConstV,
            basic_settings_section< char_type > const,
            basic_settings_section< char_type >
        >::type section_type;

    private:
        section_type& m_section;
        std::string m_path;

    public:
        param_reference(section_type& section, std::string const& section_name) :
            m_section(section),
            m_path(section_name)
        {
        }
        param_reference(section_type& section, const char* section_name) :
            m_section(section),
            m_path(section_name)
        {
        }

        param_reference& operator[] (std::string const& param_name)
        {
            if (!param_name.empty())
            {
                if (!m_path.empty())
                    m_path.push_back('.');
                m_path.append(param_name);
            }
            return *this;
        }

        param_reference& operator= (string_type const& value)
        {
            m_section.set_parameter(m_path, value);
            return *this;
        }

        template< bool V >
        param_reference& operator= (param_reference< V > const& value)
        {
            m_section.set_parameter(m_path, value.get());
            return *this;
        }

        template< typename T >
        param_reference& operator= (T const& value)
        {
            BOOST_ASSERT(m_section.m_ptree != NULL);
            m_section.m_ptree->put(m_path, value);
            return *this;
        }

        BOOST_LOG_EXPLICIT_OPERATOR_BOOL()

        bool operator! () const
        {
            return !m_section.get_section(m_path);
        }

        operator optional< string_type > () const
        {
            return get();
        }

        optional< string_type > get() const
        {
            return m_section.get_parameter(m_path);
        }

        template< typename T >
        optional< T > get() const
        {
            BOOST_ASSERT(m_section.m_ptree != NULL);
            return m_section.m_ptree->template get_optional< T >(m_path);
        }

        operator section_type () const
        {
            return get_section();
        }

        section_type get_section() const
        {
            return m_section.get_section(m_path);
        }

#if defined(BOOST_LOG_TYPEOF)
#if !defined(BOOST_LOG_NO_TRAILING_RESULT_TYPE)
        template< typename T >
        auto or_default(T const& def_value) const -> BOOST_LOG_TYPEOF(property_tree_ref_type().get(path_type(), def_value))
        {
            if (m_section.m_ptree)
                return m_section.m_ptree->get(m_path, def_value);
            else
                return def_value;
        }
#else
        template< typename T >
        BOOST_LOG_TYPEOF(property_tree_ref_type().get(path_type(), aux::make_value< T >::get())) or_default(T const& def_value) const
        {
            if (m_section.m_ptree)
                return m_section.m_ptree->get(m_path, def_value);
            else
                return def_value;
        }
#endif
#else
        template< typename T >
        T or_default(T const& def_value) const
        {
            if (m_section.m_ptree)
                return m_section.m_ptree->get(m_path, def_value);
            else
                return def_value;
        }

        template< typename T >
        typename enable_if< boost::property_tree::detail::is_character< T >, std::basic_string< T > >::type
        or_default(const T* def_value) const
        {
            if (m_section.m_ptree)
                return m_section.m_ptree->get(m_path, def_value);
            else
                return def_value;
        }
#endif
        string_type or_default(string_type const& def_value) const
        {
            return m_section.get_parameter(m_path).get_value_or(def_value);
        }
        string_type or_default(typename string_type::value_type const* def_value) const
        {
            if (optional< string_type > val = m_section.get_parameter(m_path))
                return val.get();
            else
                return def_value;
        }
    };

public:
    typedef param_reference< true > const_reference;
    typedef param_reference< false > reference;

#else

public:
    /*!
     * Constant reference to the parameter value
     */
    typedef implementation_defined const_reference;
    /*!
     * Mutable reference to the parameter value
     */
    typedef implementation_defined reference;

#endif // !defined(BOOST_LOG_DOXYGEN_PASS)

protected:
    //! Parameters
    property_tree_type* m_ptree;

public:
    /*!
     * Default constructor. Creates an empty settings container.
     */
    basic_settings_section() : m_ptree(NULL)
    {
    }

    /*!
     * Copy constructor.
     */
    basic_settings_section(basic_settings_section const& that) : m_ptree(that.m_ptree)
    {
    }

    /*!
     * Checks if the section refers to the container.
     */
    BOOST_LOG_EXPLICIT_OPERATOR_BOOL()

    /*!
     * Checks if the section refers to the container.
     */
    bool operator! () const
    {
        return !m_ptree;
    }

    /*!
     * Checks if the container is empty (i.e. contains no sections and parameters).
     */
    BOOST_LOG_SETUP_API bool empty() const;

    /*!
     * Accessor to a single parameter. This operator should be used in conjunction
     * with the subsequent subscript operator that designates the parameter name.
     *
     * \param section_name The name of the section in which the parameter resides
     * \return An unspecified reference type that can be used for parameter name specifying
     */
    reference operator[] (std::string const& section_name)
    {
        return reference(*this, section_name);
    }
    /*!
     * Accessor to a single parameter. This operator should be used in conjunction
     * with the subsequent subscript operator that designates the parameter name.
     *
     * \param section_name The name of the section in which the parameter resides
     * \return An unspecified reference type that can be used for parameter name specifying
     */
    const_reference operator[] (std::string const& section_name) const
    {
        return const_reference(*this, section_name);
    }

    /*!
     * Accessor to a single parameter. This operator should be used in conjunction
     * with the subsequent subscript operator that designates the parameter name.
     *
     * \param section_name The name of the section in which the parameter resides
     * \return An unspecified reference type that can be used for parameter name specifying
     */
    reference operator[] (const char* section_name)
    {
        return reference(*this, section_name);
    }
    /*!
     * Accessor to a single parameter. This operator should be used in conjunction
     * with the subsequent subscript operator that designates the parameter name.
     *
     * \param section_name The name of the section in which the parameter resides
     * \return An unspecified reference type that can be used for parameter name specifying
     */
    const_reference operator[] (const char* section_name) const
    {
        return const_reference(*this, section_name);
    }

    /*!
     * Accessor for the embedded property tree
     */
    property_tree_type const& property_tree() const { return *m_ptree; }
    /*!
     * Accessor for the embedded property tree
     */
    property_tree_type& property_tree() { return *m_ptree; }

    /*!
     * Checks if the specified section is present in the container.
     *
     * \param section_name The name of the section
     */
    BOOST_LOG_SETUP_API bool has_section(string_type const& section_name) const;
    /*!
     * Checks if the specified parameter is present in the container.
     *
     * \param section_name The name of the section in which the parameter resides
     * \param param_name The name of the parameter
     */
    BOOST_LOG_SETUP_API bool has_parameter(string_type const& section_name, string_type const& param_name) const;

    /*!
     * Swaps two references to settings sections.
     */
    void swap(basic_settings_section& that)
    {
        property_tree_type* const p = m_ptree;
        m_ptree = that.m_ptree;
        that.m_ptree = p;
    }

protected:
    explicit basic_settings_section(property_tree_type* tree) : m_ptree(tree)
    {
    }

    BOOST_LOG_SETUP_API basic_settings_section get_section(std::string const& name) const;
    BOOST_LOG_SETUP_API optional< string_type > get_parameter(std::string const& name) const;
    BOOST_LOG_SETUP_API void set_parameter(std::string const& name, string_type const& value);
    BOOST_LOG_SETUP_API void set_parameter(std::string const& name, optional< string_type > const& value);
};

template< typename CharT >
inline void swap(basic_settings_section< CharT >& left, basic_settings_section< CharT >& right)
{
    left.swap(right);
}

/*!
 * \brief The class represents settings container
 *
 * All settings are presented as a number of named parameters divided into named sections.
 * The parameters values are stored as strings. Individual parameters may be queried via subscript operators, like this:
 *
 * <code><pre>
 * optional< string > param = settings["Section1"]["Param1"]; // reads parameter "Param1" in section "Section1"
 *                                                            // returns an empty value if no such parameter exists
 * settings["Section2"]["Param2"] = 10; // sets the parameter "Param2" in section "Section2"
 *                                      // to value "10"
 * </pre></code>
 *
 * There are also other methods to work with parameters.
 */
template< typename CharT >
class basic_settings :
    public basic_settings_section< CharT >
{
    typedef basic_settings this_type;
    BOOST_COPYABLE_AND_MOVABLE(this_type)

public:
    //! Section type
    typedef basic_settings_section< CharT > section;
    //! Property tree type
    typedef typename section::property_tree_type property_tree_type;

public:
    /*!
     * Default constructor. Creates an empty settings container.
     */
    BOOST_LOG_SETUP_API basic_settings();
    /*!
     * Copy constructor.
     */
    BOOST_LOG_SETUP_API basic_settings(basic_settings const& that);
    /*!
     * Move constructor.
     */
    basic_settings(BOOST_RV_REF(this_type) that)
    {
        this->swap(that);
    }
    /*!
     * Initializing constructor. Creates a settings container with the copy of the specified property tree.
     */
    BOOST_LOG_SETUP_API explicit basic_settings(property_tree_type const& tree);

    /*!
     * Destructor
     */
    BOOST_LOG_SETUP_API ~basic_settings();

    /*!
     * Copy assignment operator.
     */
    basic_settings& operator= (BOOST_COPY_ASSIGN_REF(basic_settings) that)
    {
        if (this != &that)
        {
            basic_settings tmp = that;
            this->swap(tmp);
        }
        return *this;
    }
    /*!
     * Move assignment operator.
     */
    basic_settings& operator= (BOOST_RV_REF(basic_settings) that)
    {
        this->swap(that);
        return *this;
    }
};

#ifdef BOOST_LOG_USE_CHAR
typedef basic_settings< char > settings;                        //!< Convenience typedef for narrow-character logging
typedef basic_settings_section< char > settings_section;        //!< Convenience typedef for narrow-character logging
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
typedef basic_settings< wchar_t > wsettings;                    //!< Convenience typedef for wide-character logging
typedef basic_settings_section< wchar_t > wsettings_section;    //!< Convenience typedef for wide-character logging
#endif

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_UTILITY_INIT_SETTINGS_HPP_INCLUDED_
