/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   init_from_stream.cpp
 * \author Andrey Semashev
 * \date   22.03.2008
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <locale>
#include <utility>
#include <stdexcept>
#include <algorithm>
#include <boost/throw_exception.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/log/logging_core.hpp>
#include <boost/log/init/from_stream.hpp>
#include <boost/log/init/filter_parser.hpp>
#include "parser_utils.hpp"

namespace boost {

namespace log {

namespace {

//! The class represents parsed logging settings
template< typename CharT >
class settings
{
public:
    //! Character type
    typedef CharT char_type;
    //! String type
    typedef std::basic_string< char_type > string_type;
    //! The type of the map of parameters and their names
    typedef std::map< string_type, string_type > params_t;
    //! The type of the map of sections
    typedef std::map< string_type, params_t > sections_t;
    //! Structure with character constants
    typedef aux::char_constants< char_type > constants;

private:
    //! Parameters
    sections_t m_Sections;

public:
    //! The constructor reads parameters from the stream
    explicit settings(std::basic_istream< char_type >& strm)
    {
        typedef typename string_type::iterator str_iterator;
        std::locale loc = strm.getloc();
        typename sections_t::iterator current_section = m_Sections.end();
        string_type line;
        for (unsigned int line_counter = 1; strm.good(); ++line_counter)
        {
            line.clear();
            std::getline(strm, line);
            boost::algorithm::trim(line, loc);

            // Skipping empty lines and comments
            // NOTE: The comments are only allwed to be the whole line.
            //       Comments beginning in the middle of the line are not supported.
            if (!line.empty() && line[0] != constants::char_comment)
            {
                // Check if the line is a section starter
                if (line[0] == constants::char_section_bracket_left)
                {
                    str_iterator it = std::find(line.begin() + 1, line.end(), constants::char_section_bracket_right);
                    string_type section_name(line.begin() + 1, it);
                    boost::algorithm::trim(section_name, loc);
                    if (it != line.end() && !section_name.empty())
                    {
                        // Creating a new section
                        current_section = m_Sections.insert(std::make_pair(section_name, params_t())).first;
                    }
                    else
                    {
                        // The section starter is broken
                        std::ostringstream descr;
                        descr << "At line " << line_counter << ". The section header is invalid.";
                        boost::throw_exception(std::runtime_error(descr.str()));
                    }
                }
                else
                {
                    // Check that we've already started a section
                    if (current_section != m_Sections.end())
                    {
                        // Find the '=' between the parameter name and value
                        str_iterator it = std::find(line.begin(), line.end(), constants::char_equal);
                        string_type param_name(line.begin(), it);
                        boost::algorithm::trim_right(param_name, loc);
                        if (it != line.end() && !param_name.empty())
                        {
                            // Put the parameter value into the map
                            string_type param_value(++it, line.end());
                            boost::algorithm::trim_left(param_value, loc);
                            if (param_value.size() >= 2
                                && param_value[0] == constants::char_quote && *param_value.rbegin() == constants::char_quote)
                            {
                                param_value = param_value.substr(1, param_value.size() - 2);
                            }

                            current_section->second[param_name] = param_value;
                        }
                        else
                        {
                            // The parameter name is not valid
                            std::ostringstream descr;
                            descr << "At line " << line_counter << ". Parameter description is not valid.";
                            boost::throw_exception(std::runtime_error(descr.str()));
                        }
                    }
                    else
                    {
                        // The parameter encountered before any section starter
                        std::ostringstream descr;
                        descr << "At line " << line_counter << ". Parameters are only allowed in sections.";
                        boost::throw_exception(std::runtime_error(descr.str()));
                    }
                }
            }
        }
    }

    //! Accessor for the map of sections
    sections_t const& sections() const { return m_Sections; }
};

//! The function applies the settings to the logging core
template< typename CharT >
void apply_core_settings(std::map< std::basic_string< CharT >, std::basic_string< CharT > > const& params)
{
    typedef CharT char_type;
    typedef std::basic_string< char_type > string_type;
    typedef std::map< string_type, string_type > params_t;
    typedef aux::char_constants< char_type > constants;
    typedef std::basic_istringstream< char_type > isstream;
    typedef basic_logging_core< char_type > core_t;
    shared_ptr< core_t > core = core_t::get();

    // Filter
    typename params_t::const_iterator it =
        params.find(constants::filter_param_name());
    if (it != params.end())
        core->set_filter(parse_filter(it->second));
    else
        core->reset_filter();

    // DisableLogging
    it = params.find(constants::core_disable_logging_param_name());
    if (it != params.end())
    {
        isstream strm(it->second);
        strm.setf(std::ios_base::boolalpha);
        bool f = false;
        strm >> f;
        core->set_logging_enabled(!f);
    }
    else
        core->set_logging_enabled(true);
}

} // namespace

//! The function initializes the logging library from a stream containing logging settings
template< typename CharT >
void init_from_stream(std::basic_istream< CharT >& strm)
{
    // Parse the settings
    typedef settings< CharT > settings_t;
    typedef typename settings_t::constants constants;
    settings_t setts(strm);

    // Apply core settings
    typename settings_t::sections_t const& sections = setts.sections();
    typename settings_t::sections_t::const_iterator core_params =
        sections.find(constants::core_section_name());
    if (core_params != sections.end())
        apply_core_settings(core_params->second);
}

template void init_from_stream< char >(std::basic_istream< char >& strm);
template void init_from_stream< wchar_t >(std::basic_istream< wchar_t >& strm);

} // namespace log

} // namespace boost
