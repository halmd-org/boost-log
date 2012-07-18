/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   init_from_settings.cpp
 * \author Andrey Semashev
 * \date   11.10.2009
 *
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#ifndef BOOST_LOG_NO_SETTINGS_PARSERS_SUPPORT

#if defined(_MSC_VER)
// 'const int' : forcing value to bool 'true' or 'false' (performance warning)
#pragma warning(disable: 4800)
#endif

#include "windows_version.hpp"
#include <ios>
#include <map>
#include <vector>
#include <string>
#include <utility>
#include <iostream>
#include <typeinfo>
#include <stdexcept>
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/limits.hpp>
#include <boost/cstdint.hpp>
#include <boost/make_shared.hpp>
#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/date_time/date_defs.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_unsigned.hpp>
#include <boost/spirit/include/qi_core.hpp>
#include <boost/phoenix/core.hpp>
#include <boost/log/detail/code_conversion.hpp>
#include <boost/log/detail/singleton.hpp>
#include <boost/log/core.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/exceptions.hpp>
#include <boost/log/sinks/frontend_requirements.hpp>
#include <boost/log/expressions/filter.hpp>
#include <boost/log/expressions/formatter.hpp>
#include <boost/log/utility/empty_deleter.hpp>
#include <boost/log/utility/init/from_settings.hpp>
#include <boost/log/utility/init/filter_parser.hpp>
#include <boost/log/utility/init/formatter_parser.hpp>
#if !defined(BOOST_LOG_NO_ASIO)
#include <boost/asio/ip/address.hpp>
#endif
#if !defined(BOOST_LOG_NO_THREADS)
#include <boost/log/detail/locks.hpp>
#include <boost/log/detail/light_rw_mutex.hpp>
#endif
#include "parser_utils.hpp"

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace qi = boost::spirit::qi;

BOOST_LOG_ANONYMOUS_NAMESPACE {

//! Throws an exception when a parameter type is not valid
BOOST_LOG_NORETURN void throw_invalid_type(const char* param_name, log::type_info_wrapper const& param_type)
{
    std::string descr = std::string("Invalid parameter \"")
                        + param_name
                        + "\" type: "
                        + param_type.pretty_name();
    BOOST_LOG_THROW_DESCR(invalid_type, descr);
}

//! Throws an exception when a parameter value is not valid
BOOST_LOG_NORETURN void throw_invalid_value(const char* param_name)
{
    std::string descr = std::string("Invalid parameter \"")
                        + param_name
                        + "\" value";
    BOOST_LOG_THROW_DESCR(invalid_value, descr);
}

//! Extracts a filesystem path from any
inline filesystem::path param_cast_to_path(const CharT* param_name, any const& val)
{
    std::type_info const& type = val.type();
    if (type == typeid(filesystem::path))
        return log::aux::to_universal_path(any_cast< filesystem::path >(val));
    else if (type == typeid(std::string))
        return log::aux::to_universal_path(any_cast< std::string >(val));
    else
        throw_invalid_type(param_name, type);
}
//! Extracts an integral value from any
template< typename IntT, typename CharT >
inline IntT param_cast_to_int(const CharT* param_name, any const& val)
{
    typedef std::basic_string< CharT > string_type;
    std::type_info const& type = val.type();
    if (type == typeid(short))
        return static_cast< IntT >(any_cast< short >(val));
    else if (type == typeid(unsigned short))
        return static_cast< IntT >(any_cast< unsigned short >(val));
    else if (type == typeid(int))
        return static_cast< IntT >(any_cast< int >(val));
    else if (type == typeid(unsigned int))
        return static_cast< IntT >(any_cast< unsigned int >(val));
    else if (type == typeid(long))
        return static_cast< IntT >(any_cast< long >(val));
    else if (type == typeid(unsigned long))
        return static_cast< IntT >(any_cast< unsigned long >(val));
#if !defined(BOOST_NO_INT64_T)
    else if (type == typeid(long long))
        return static_cast< IntT >(any_cast< long long >(val));
    else if (type == typeid(unsigned long long))
        return static_cast< IntT >(any_cast< unsigned long long >(val));
#endif // !defined(BOOST_NO_INT64_T)
    else if (type == typeid(string_type))
    {
        string_type value = any_cast< string_type >(val);
        IntT res = 0;
        typedef typename mpl::if_<
            is_unsigned< IntT >,
            bsc::uint_parser< IntT >,
            bsc::int_parser< IntT >
        >::type int_parser_t;
        int_parser_t int_p;
        if (bsc::parse(value.begin(), value.end(), int_p[bsc::assign_a(res)]).full)
            return res;
        else
            throw_invalid_value(param_name);
    }
    else
        throw_invalid_type(param_name, type);
}

//! Extracts a boolean value from any
template< typename CharT >
inline bool param_cast_to_bool(const CharT* param_name, any const& val)
{
    typedef std::basic_string< CharT > string_type;
    typedef log::aux::char_constants< CharT > char_constants;
    std::type_info const& type = val.type();
    if (type == typeid(bool))
        return any_cast< bool >(val);
    else if (type == typeid(string_type))
    {
        string_type value = any_cast< string_type >(val);
        bool res = false;
        if (bsc::parse(value.begin(), value.end(), (
            bsc::int_p[bsc::assign_a(res)] ||
            bsc::as_lower_d[ bsc::str_p(char_constants::true_keyword()) ][bsc::assign_a(res, true)] ||
            bsc::as_lower_d[ bsc::str_p(char_constants::false_keyword()) ][bsc::assign_a(res, false)]
        )).full)
        {
            return res;
        }
        else
            throw_invalid_value(param_name);
    }
    else
        throw_invalid_type(param_name, type);
}

//! Extracts a filter from any
template< typename CharT >
inline boost::log::aux::light_function1<
    bool,
    basic_attribute_values_view< CharT > const&
> param_cast_to_filter(const CharT* param_name, any const& val)
{
    typedef std::basic_string< CharT > string_type;
    typedef basic_attribute_values_view< CharT > values_view_type;
    typedef function1< bool, values_view_type const& > filter_type1;
    typedef function< bool (values_view_type const&) > filter_type2;
    typedef boost::log::aux::light_function1< bool, values_view_type const& > filter_type3;

    std::type_info const& type = val.type();
    if (type == typeid(string_type))
        return parse_filter(any_cast< string_type >(val));
    else if (type == typeid(filter_type1))
        return any_cast< filter_type1 >(val);
    else if (type == typeid(filter_type2))
        return any_cast< filter_type2 >(val);
    else if (type == typeid(filter_type3))
        return any_cast< filter_type3 >(val);
    else
        throw_invalid_type(param_name, type);
}

//! Extracts a formatter from any
template< typename CharT >
inline boost::log::aux::light_function2<
    void,
    std::basic_ostream< CharT >&,
    basic_record< CharT > const&
> param_cast_to_formatter(const CharT* param_name, any const& val)
{
    typedef std::basic_string< CharT > string_type;
    typedef std::basic_ostream< CharT > stream_type;
    typedef basic_record< CharT > record_type;
    typedef function2< void, stream_type&, record_type const& > formatter_type1;
    typedef function< void (stream_type&, record_type const&) > formatter_type2;
    typedef boost::log::aux::light_function2< void, stream_type&, record_type const& > formatter_type3;

    std::type_info const& type = val.type();
    if (type == typeid(string_type))
        return parse_formatter(any_cast< string_type >(val));
    else if (type == typeid(formatter_type1))
        return any_cast< formatter_type1 >(val);
    else if (type == typeid(formatter_type2))
        return any_cast< formatter_type2 >(val);
    else if (type == typeid(formatter_type3))
        return any_cast< formatter_type3 >(val);
    else
        throw_invalid_type(param_name, type);
}

#if !defined(BOOST_LOG_NO_ASIO)
//! Extracts a network address from any
template< typename CharT >
inline std::string param_cast_to_address(const CharT* param_name, any const& val)
{
    typedef std::basic_string< CharT > string_type;

    std::type_info const& type = val.type();
    if (type == typeid(asio::ip::address))
        return any_cast< asio::ip::address >(val).to_string();
    else if (type == typeid(asio::ip::address_v4))
        return any_cast< asio::ip::address_v4 >(val).to_string();
    else if (type == typeid(asio::ip::address_v6))
        return any_cast< asio::ip::address_v6 >(val).to_string();
    else if (type == typeid(string_type))
        return log::aux::to_narrow(any_cast< string_type >(val));
    else
        throw_invalid_type(param_name, type);
}
#endif // !defined(BOOST_LOG_NO_ASIO)

//! The function extracts the file rotation time point predicate from the parameter
template< typename CharT >
boost::log::aux::light_function0< bool > param_cast_to_rotation_time_point(const CharT* param_name, any const& val)
{
    typedef CharT char_type;
    typedef boost::log::aux::char_constants< char_type > constants;
    typedef std::basic_string< char_type > string_type;
    typedef function0< bool > predicate_type1;
    typedef function< bool () > predicate_type2;
    typedef boost::log::aux::light_function0< bool > predicate_type3;

    std::type_info const& type = val.type();
    if (type == typeid(sinks::file::rotation_at_time_point))
        return any_cast< sinks::file::rotation_at_time_point >(val);
    else if (type == typeid(predicate_type1))
        return any_cast< predicate_type1 >(val);
    else if (type == typeid(predicate_type2))
        return any_cast< predicate_type2 >(val);
    else if (type == typeid(predicate_type3))
        return any_cast< predicate_type3 >(val);
    else if (type == typeid(string_type))
    {
        // We'll have to parse it from the string
        const char_type colon = static_cast< char_type >(':');
        bsc::uint_parser< unsigned char, 10, 2, 2 > time_component_p;
        bsc::uint_parser< unsigned short, 10, 1, 2 > day_p;

        optional< date_time::weekdays > weekday;
        optional< unsigned short > day;
        bool day_parsed = false;
        unsigned char hour = 0, minute = 0, second = 0;

        string_type str = any_cast< string_type >(val);
        bsc::parse_info< const char_type* > result =
            bsc::parse(
                str.c_str(),
                str.c_str() + str.size(),
                (
                    !(
                        (
                            // First check for a weekday
                            (
                                (bsc::str_p(constants::monday_keyword()) || bsc::str_p(constants::short_monday_keyword()))
                                    [bsc::assign_a(weekday, date_time::Monday)] ||
                                (bsc::str_p(constants::tuesday_keyword()) || bsc::str_p(constants::short_tuesday_keyword()))
                                    [bsc::assign_a(weekday, date_time::Tuesday)] ||
                                (bsc::str_p(constants::wednesday_keyword()) || bsc::str_p(constants::short_wednesday_keyword()))
                                    [bsc::assign_a(weekday, date_time::Wednesday)] ||
                                (bsc::str_p(constants::thursday_keyword()) || bsc::str_p(constants::short_thursday_keyword()))
                                    [bsc::assign_a(weekday, date_time::Thursday)] ||
                                (bsc::str_p(constants::friday_keyword()) || bsc::str_p(constants::short_friday_keyword()))
                                    [bsc::assign_a(weekday, date_time::Friday)] ||
                                (bsc::str_p(constants::saturday_keyword()) || bsc::str_p(constants::short_saturday_keyword()))
                                    [bsc::assign_a(weekday, date_time::Saturday)] ||
                                (bsc::str_p(constants::sunday_keyword()) || bsc::str_p(constants::short_sunday_keyword()))
                                    [bsc::assign_a(weekday, date_time::Sunday)]
                            ) ||
                            // ... or a day in month
                            (
                                day_p[bsc::assign_a(day)]
                            )
                        ) >>
                        bsc::space_p[bsc::assign_a(day_parsed, true)]
                    ) >>
                    // Then goes the time of day
                    (
                        time_component_p[bsc::assign_a(hour)] >> colon >>
                        time_component_p[bsc::assign_a(minute)] >> colon >>
                        time_component_p[bsc::assign_a(second)]
                    )
                )
            );

        if (!result.full)
            throw_invalid_value(param_name);

        if (day_parsed)
        {
            if (weekday)
                return sinks::file::rotation_at_time_point(weekday.get(), hour, minute, second);
            else if (day)
                return sinks::file::rotation_at_time_point(gregorian::greg_day(day.get()), hour, minute, second);
        }
        return sinks::file::rotation_at_time_point(hour, minute, second);
    }
    else
        throw_invalid_type(param_name, type);
}

//! The supported sinks repository
template< typename CharT >
struct sinks_repository :
    public log::aux::lazy_singleton< sinks_repository< CharT > >
{
    typedef log::aux::lazy_singleton< sinks_repository< CharT > > base_type;

#if !defined(BOOST_LOG_BROKEN_FRIEND_TEMPLATE_INSTANTIATIONS)
    friend class log::aux::lazy_singleton< sinks_repository< CharT > >;
#else
    friend class base_type;
#endif

    typedef CharT char_type;
    typedef std::basic_string< char_type > string_type;
    typedef boost::log::aux::char_constants< char_type > constants;
    typedef basic_settings_section< char_type > section;
    typedef typename section::const_reference param_const_reference;
    typedef typename section::reference param_reference;
    typedef boost::log::aux::light_function1<
        shared_ptr< sinks::sink >,
        section const&
    > sink_factory;
    typedef std::map< std::string, sink_factory > sink_factories;

#if !defined(BOOST_LOG_NO_THREADS)
    //! Synchronization mutex
    log::aux::light_rw_mutex m_Mutex;
#endif
    //! Map of the sink factories
    sink_factories m_Factories;

    //! The function constructs a sink from the settings
    shared_ptr< sinks::sink > construct_sink_from_settings(section const& params)
    {
        if (param_const_reference dest_node = params["Destination"])
        {
            std::string dest = log::aux::to_narrow(dest_node.get().get());

            BOOST_LOG_EXPR_IF_MT(log::aux::shared_lock_guard< log::aux::light_rw_mutex > lock(m_Mutex);)
            typename sink_factories::const_iterator it = m_Factories.find(dest);
            if (it != m_Factories.end())
            {
                return it->second(params);
            }
            else
            {
                BOOST_LOG_THROW_DESCR(invalid_value, "The sink destination is not supported: " + dest);
            }
        }
        else
        {
            BOOST_LOG_THROW_DESCR(missing_value, "The sink destination is not set");
        }
    }

    static void init_instance()
    {
        sinks_repository& instance = base_type::get_instance();
        instance.m_Factories["TextFile"] = &sinks_repository< char_type >::default_text_file_sink_factory;
        instance.m_Factories["Console"] = &sinks_repository< char_type >::default_console_sink_factory;
#ifndef BOOST_LOG_NO_SYSLOG_SUPPORT
        instance.m_Factories["Syslog"] = &sinks_repository< char_type >::default_syslog_sink_factory;
#endif
#ifndef BOOST_LOG_NO_DEBUG_OUTPUT_SUPPORT
        instance.m_Factories["Debugger"] = &sinks_repository< char_type >::default_debugger_sink_factory;
#endif
#ifndef BOOST_LOG_NO_EVENT_LOG_SUPPORT
        instance.m_Factories["SimpleEventLog"] = &sinks_repository< char_type >::default_simple_event_log_sink_factory;
#endif
    }

private:
    sinks_repository() {}

    //! The function constructs a sink that writes log records to a text file
    static shared_ptr< sinks::sink > default_text_file_sink_factory(section const& params)
    {
        typedef sinks::basic_text_file_backend< char_type > backend_t;
        typedef typename backend_t::path_type path_type;
        shared_ptr< backend_t > backend = boost::make_shared< backend_t >();

        // FileName
        typename section::const_iterator it = params.find(constants::file_name_param_name());
        if (it != params.end() && !it->second.empty())
        {
            backend->set_file_name_pattern(
                param_cast_to_path(constants::file_name_param_name(), it->second));
        }
        else
            BOOST_LOG_THROW_DESCR(missing_value, "File name is not specified");

        // File rotation size
        it = params.find(constants::rotation_size_param_name());
        if (it != params.end() && !it->second.empty())
        {
            backend->set_rotation_size(
                param_cast_to_int< uintmax_t >(constants::rotation_size_param_name(), it->second));
        }

        // File rotation interval
        it = params.find(constants::rotation_interval_param_name());
        if (it != params.end() && !it->second.empty())
        {
            backend->set_time_based_rotation(sinks::file::rotation_at_time_interval(posix_time::seconds(
                param_cast_to_int< unsigned int >(constants::rotation_interval_param_name(), it->second))));
        }
        else
        {
            // File rotation time point
            it = params.find(constants::rotation_time_point_param_name());
            if (it != params.end() && !it->second.empty())
            {
                backend->set_time_based_rotation(
                    param_cast_to_rotation_time_point(constants::rotation_time_point_param_name(), it->second));
            }
        }

        // Auto flush
        it = params.find(constants::auto_flush_param_name());
        if (it != params.end() && !it->second.empty())
        {
            backend->auto_flush(
                param_cast_to_bool(constants::auto_flush_param_name(), it->second));
        }

        // Append
        it = params.find(constants::append_param_name());
        if (it != params.end() && !it->second.empty() && param_cast_to_bool(constants::auto_flush_param_name(), it->second))
        {
            backend->set_open_mode(std::ios_base::out | std::ios_base::app);
        }

        // File collector parameters
        // Target directory
        it = params.find(constants::target_param_name());
        if (it != params.end() && !it->second.empty())
        {
            path_type target_dir = param_cast_to_path(constants::target_param_name(), it->second);

            // Max total size
            uintmax_t max_size = (std::numeric_limits< uintmax_t >::max)();
            it = params.find(constants::max_size_param_name());
            if (it != params.end() && !it->second.empty())
                max_size = param_cast_to_int< uintmax_t >(constants::max_size_param_name(), it->second);

            // Min free space
            uintmax_t space = 0;
            it = params.find(constants::min_free_space_param_name());
            if (it != params.end() && !it->second.empty())
                space = param_cast_to_int< uintmax_t >(constants::min_free_space_param_name(), it->second);

            backend->set_file_collector(sinks::file::make_collector(
                keywords::target = target_dir,
                keywords::max_size = max_size,
                keywords::min_free_space = space));

            // Scan for log files
            it = params.find(constants::scan_for_files_param_name());
            if (it != params.end() && !it->second.empty())
            {
                if (it->second.type() == typeid(sinks::file::scan_method))
                    backend->scan_for_files(any_cast< sinks::file::scan_method >(it->second));
                else if (it->second.type() == typeid(string_type))
                {
                    string_type value = any_cast< string_type >(it->second);
                    if (value == constants::scan_method_all())
                        backend->scan_for_files(sinks::file::scan_all);
                    else if (value == constants::scan_method_matching())
                        backend->scan_for_files(sinks::file::scan_matching);
                    else
                    {
                        BOOST_LOG_THROW_DESCR(invalid_value,
                            "File scan method \"" + boost::log::aux::to_narrow(value) + "\" is not supported");
                    }
                }
                else
                    throw_invalid_type(constants::scan_for_files_param_name(), it->second.type());
            }
        }

        return init_sink(backend, params);
    }

    //! The function constructs a sink that writes log records to the console
    static shared_ptr< sinks::sink > default_console_sink_factory(section const& params)
    {
        // Construct the backend
        typedef sinks::basic_text_ostream_backend< char_type > backend_t;
        shared_ptr< backend_t > backend = boost::make_shared< backend_t >();
        backend->add_stream(
            shared_ptr< typename backend_t::stream_type >(&constants::get_console_log_stream(), empty_deleter()));

        return init_text_ostream_sink(backend, params);
    }

#ifndef BOOST_LOG_NO_SYSLOG_SUPPORT

    //! The function constructs a sink that writes log records to the syslog service
    static shared_ptr< sinks::sink > default_syslog_sink_factory(section const& params)
    {
        // Construct the backend
        typedef sinks::syslog_backend backend_t;
        shared_ptr< backend_t > backend = boost::make_shared< backend_t >();

        // For now we use only the default level mapping. Will add support for configuration later.
        backend->set_severity_mapper(
            sinks::syslog::basic_direct_severity_mapping< char_type >(constants::default_level_attribute_name()));

#if !defined(BOOST_LOG_NO_ASIO)
        // Setup local and remote addresses
        typename section::const_iterator it = params.find(constants::local_address_param_name());
        if (it != params.end() && !it->second.empty())
            backend->set_local_address(param_cast_to_address(constants::local_address_param_name(), it->second));

        it = params.find(constants::target_address_param_name());
        if (it != params.end() && !it->second.empty())
            backend->set_target_address(param_cast_to_address(constants::target_address_param_name(), it->second));
#endif // !defined(BOOST_LOG_NO_ASIO)

        return init_sink(backend, params);
    }

#endif // BOOST_LOG_NO_SYSLOG_SUPPORT

#ifndef BOOST_LOG_NO_DEBUG_OUTPUT_SUPPORT

    //! The function constructs a sink that writes log records to the debugger
    static shared_ptr< sinks::sink > default_debugger_sink_factory(section const& params)
    {
        // Construct the backend
        typedef sinks::basic_debug_output_backend< char_type > backend_t;
        shared_ptr< backend_t > backend = boost::make_shared< backend_t >();

        return init_sink(backend, params);
    }

#endif // BOOST_LOG_NO_DEBUG_OUTPUT_SUPPORT

#ifndef BOOST_LOG_NO_EVENT_LOG_SUPPORT

    //! The function constructs a sink that writes log records to the Windows NT Event Log
    static shared_ptr< sinks::sink > default_simple_event_log_sink_factory(section const& params)
    {
        typedef sinks::basic_simple_event_log_backend< char_type > backend_t;

        // Determine the log name
        string_type log_name = backend_t::get_default_log_name();
        typename section::const_iterator it = params.find(constants::log_name_param_name());
        if (it != params.end() && !it->second.empty())
        {
            if (it->second.type() == typeid(string_type))
                log_name = any_cast< string_type >(it->second);
            else
                throw_invalid_type(constants::log_name_param_name(), it->second.type());
        }

        // Determine the log source name
        string_type source_name = backend_t::get_default_source_name();
        it = params.find(constants::source_name_param_name());
        if (it != params.end() && !it->second.empty())
        {
            if (it->second.type() == typeid(string_type))
                source_name = any_cast< string_type >(it->second);
            else
                throw_invalid_type(constants::source_name_param_name(), it->second.type());
        }

        // Determine the registration mode
        sinks::event_log::registration_mode reg_mode = sinks::event_log::on_demand;
        it = params.find(constants::registration_param_name());
        if (it != params.end() && !it->second.empty())
        {
            if (it->second.type() == typeid(sinks::event_log::registration_mode))
                reg_mode = any_cast< sinks::event_log::registration_mode >(it->second);
            else if (it->second.type() == typeid(string_type))
            {
                string_type value = any_cast< string_type >(it->second);
                if (value == constants::registration_never())
                    reg_mode = sinks::event_log::never;
                else if (value == constants::registration_on_demand())
                    reg_mode = sinks::event_log::on_demand;
                else if (value == constants::registration_forced())
                    reg_mode = sinks::event_log::forced;
                else
                {
                    BOOST_LOG_THROW_DESCR(invalid_value,
                        "The registration mode \"" + log::aux::to_narrow(value) + "\" is not supported");
                }
            }
            else
                throw_invalid_type(constants::registration_param_name(), it->second.type());
        }

        // Construct the backend
        shared_ptr< backend_t > backend(boost::make_shared< backend_t >((
            keywords::log_name = log_name,
            keywords::log_source = source_name,
            keywords::registration = reg_mode)));

        // For now we use only the default event type mapping. Will add support for configuration later.
        backend->set_event_type_mapper(
            sinks::event_log::basic_direct_event_type_mapping< char_type >(constants::default_level_attribute_name()));

        return init_sink(backend, params);
    }

#endif // BOOST_LOG_NO_EVENT_LOG_SUPPORT

    //! The function initializes common parameters of text stream sink and returns the constructed sink
    static shared_ptr< sinks::sink > init_text_ostream_sink(
        shared_ptr< sinks::basic_text_ostream_backend< char_type > > const& backend, section const& params)
    {
        typedef sinks::basic_text_ostream_backend< char_type > backend_t;

        // AutoFlush
        typename section::const_iterator it = params.find(constants::auto_flush_param_name());
        if (it != params.end() && !it->second.empty())
            backend->auto_flush(param_cast_to_bool(constants::auto_flush_param_name(), it->second));

        return init_sink(backend, params);
    }

    //! The function initializes common parameters of a formatting sink and returns the constructed sink
    template< typename BackendT >
    static shared_ptr< sinks::sink > init_sink(shared_ptr< BackendT > const& backend, section const& params)
    {
        typedef BackendT backend_t;
        typedef typename sinks::has_requirement<
            typename backend_t::frontend_requirements,
            sinks::formatted_records
        >::type is_formatting_t;

        // Filter
        typedef typename sinks::sink< char_type >::filter_type filter_type;
        filter_type filt;
        typename section::const_iterator it = params.find(constants::filter_param_name());
        if (it != params.end() && !it->second.empty())
        {
            filt = param_cast_to_filter(constants::filter_param_name(), it->second);
        }

        shared_ptr< sinks::basic_sink_frontend< char_type > > p;

#if !defined(BOOST_LOG_NO_THREADS)
        // Asynchronous. TODO: make it more flexible.
        bool async = false;
        it = params.find(constants::asynchronous_param_name());
        if (it != params.end() && !it->second.empty())
        {
            async = param_cast_to_bool(constants::asynchronous_param_name(), it->second);
        }

        // Construct the frontend, considering Asynchronous parameter
        if (!async)
            p = init_formatter(boost::make_shared< sinks::synchronous_sink< backend_t > >(backend), params, is_formatting_t());
        else
            p = init_formatter(boost::make_shared< sinks::asynchronous_sink< backend_t > >(backend), params, is_formatting_t());
#else
        // When multithreading is disabled we always use the unlocked sink frontend
        p = init_formatter(boost::make_shared< sinks::unlocked_sink< backend_t > >(backend), params, is_formatting_t());
#endif

        p->set_filter(filt);

        return p;
    }

    //! The function initializes formatter for the sinks that support formatting
    template< typename SinkT >
    static shared_ptr< SinkT > init_formatter(shared_ptr< SinkT > const& sink, section const& params, mpl::true_)
    {
        // Formatter
        typename section::const_iterator it = params.find(constants::format_param_name());
        if (it != params.end() && !it->second.empty())
        {
            sink->set_formatter(param_cast_to_formatter(constants::format_param_name(), it->second));
        }
        return sink;
    }
    template< typename SinkT >
    static shared_ptr< SinkT > init_formatter(shared_ptr< SinkT > const& sink, section const& params, mpl::false_)
    {
        return sink;
    }
};

//! The function applies the settings to the logging core
template< typename CharT >
void apply_core_settings(std::map< std::basic_string< CharT >, any > const& params)
{
    typedef CharT char_type;
    typedef std::basic_string< char_type > string_type;
    typedef std::map< string_type, any > params_t;
    typedef aux::char_constants< char_type > constants;
    typedef basic_core< char_type > core_t;
    shared_ptr< core_t > core = core_t::get();

    // Filter
    typename params_t::const_iterator it = params.find(constants::filter_param_name());
    if (it != params.end() && !it->second.empty())
        core->set_filter(param_cast_to_filter(constants::filter_param_name(), it->second));
    else
        core->reset_filter();

    // DisableLogging
    it = params.find(constants::core_disable_logging_param_name());
    if (it != params.end() && !it->second.empty())
        core->set_logging_enabled(!param_cast_to_bool(constants::core_disable_logging_param_name(), it->second));
    else
        core->set_logging_enabled(true);
}

} // namespace


//! The function initializes the logging library from a settings container
template< typename CharT >
void init_from_settings(basic_settings_section< CharT > const& setts)
{
    typedef basic_settings_section< CharT > section;
    typedef typename section::char_type char_type;
    typedef typename section::string_type string_type;
    typedef sinks_repository< char_type > sinks_repo_t;

    // Apply core settings
    if (section core_params = setts["Core"])
        apply_core_settings(core_params);

    // Construct and initialize sinks
    if (section sink_params = setts["Sinks"])
    {
        sinks_repo_t& sinks_repo = sinks_repo_t::get();
        std::vector< shared_ptr< sinks::sink > > new_sinks;

        for (typename section::const_iterator it = sink_params.begin(), end = sink_params.end(); it != end; ++it)
        {
            section sink_params = *it;

            // Ignore empty sections as they are most likely individual parameters (which should not be here anyway)
            if (!sink_params.empty())
            {
                new_sinks.push_back(sinks_repo.construct_sink_from_settings(it->second));
            }
        }

        std::for_each(new_sinks.begin(), new_sinks.end(), boost::bind(&core_t::add_sink, core_t::get(), _1));
    }
}


//! The function registers a factory for a sink
template< typename CharT >
void register_sink_factory(
    const char* sink_name,
    boost::log::aux::light_function1<
        shared_ptr< sinks::sink >,
        basic_settings_section< CharT > const&
    > const& factory)
{
    sinks_repository< CharT >& repo = sinks_repository< CharT >::get();
    BOOST_LOG_EXPR_IF_MT(lock_guard< log::aux::light_rw_mutex > _(repo.m_Mutex);)
    repo.m_Factories[sink_name] = factory;
}

#ifdef BOOST_LOG_USE_CHAR
template BOOST_LOG_SETUP_API
void register_sink_factory< char >(
    const char* sink_name,
    boost::log::aux::light_function1<
        shared_ptr< sinks::sink >,
        basic_settings_section< char > const&
    > const& factory);
template BOOST_LOG_SETUP_API void init_from_settings< char >(basic_settings_section< char > const& setts);
#endif

#ifdef BOOST_LOG_USE_WCHAR_T
template BOOST_LOG_SETUP_API
void register_sink_factory< wchar_t >(
    const char* sink_name,
    boost::log::aux::light_function1<
        shared_ptr< sinks::sink >,
        basic_settings_section< wchar_t > const&
    > const& factory);
template BOOST_LOG_SETUP_API void init_from_settings< wchar_t >(basic_settings_section< wchar_t > const& setts);
#endif

} // namespace log

} // namespace boost

#endif // BOOST_LOG_NO_SETTINGS_PARSERS_SUPPORT
