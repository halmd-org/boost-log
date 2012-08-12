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

#ifndef BOOST_LOG_WITHOUT_SETTINGS_PARSERS

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
#include <boost/type.hpp>
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
#include <boost/spirit/include/qi_lit.hpp>
#include <boost/spirit/include/qi_eoi.hpp>
#include <boost/spirit/include/qi_symbols.hpp>
#include <boost/log/detail/code_conversion.hpp>
#include <boost/log/detail/singleton.hpp>
#include <boost/log/detail/default_attribute_names.hpp>
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
#include <boost/log/utility/functional/bind_assign.hpp>
#include <boost/log/utility/functional/as_action.hpp>
#if !defined(BOOST_LOG_NO_ASIO)
#include <boost/asio/ip/address.hpp>
#endif
#if !defined(BOOST_LOG_NO_THREADS)
#include <boost/log/detail/locks.hpp>
#include <boost/log/detail/light_rw_mutex.hpp>
#endif
#include "parser_utils.hpp"
#include "spirit_encoding.hpp"

namespace qi = boost::spirit::qi;

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

BOOST_LOG_ANONYMOUS_NAMESPACE {

//! Throws an exception when a parameter value is not valid
BOOST_LOG_NORETURN void throw_invalid_value(const char* param_name)
{
    std::string descr = std::string("Invalid parameter \"")
                        + param_name
                        + "\" value";
    BOOST_LOG_THROW_DESCR(invalid_value, descr);
}

//! Extracts an integral value from parameter value
template< typename IntT, typename CharT >
inline IntT param_cast_to_int(const char* param_name, std::basic_string< CharT > const& value)
{
    IntT res = 0;
    typedef typename mpl::if_<
        is_unsigned< IntT >,
        qi::uint_parser< IntT >,
        qi::int_parser< IntT >
    >::type int_parser_t;
    if (qi::parse(value.begin(), value.end(), int_parser_t() >> qi::eoi, res))
        return res;
    else
        throw_invalid_value(param_name);
}

//! Extracts a boolean value from parameter value
template< typename CharT >
inline bool param_cast_to_bool(const char* param_name, std::basic_string< CharT > const& value)
{
    typedef boost::log::aux::encoding_specific< typename boost::log::aux::encoding< CharT >::type > encoding_specific;

    bool res = false;
    if (qi::parse(value.begin(), value.end(), encoding_specific::no_case[ qi::uint_ | qi::bool_ ] >> qi::eoi, res))
        return res;
    else
        throw_invalid_value(param_name);
}

#if !defined(BOOST_LOG_NO_ASIO)
//! Extracts a network address from parameter value
template< typename CharT >
inline std::string param_cast_to_address(const char* param_name, std::basic_string< CharT > const& value)
{
    return log::aux::to_narrow(value);
}
#endif // !defined(BOOST_LOG_NO_ASIO)

//! The function extracts the file rotation time point predicate from the parameter
template< typename CharT >
sinks::file::rotation_at_time_point param_cast_to_rotation_time_point(const char* param_name, std::basic_string< CharT > const& value)
{
    typedef CharT char_type;
    typedef boost::log::aux::encoding_specific< typename boost::log::aux::encoding< char_type >::type > encoding_specific;
    typedef boost::log::aux::char_constants< char_type > constants;
    typedef std::basic_string< char_type > string_type;

    const char_type colon = static_cast< char_type >(':');
    qi::uint_parser< unsigned char, 10, 2, 2 > time_component_p;
    qi::uint_parser< unsigned short, 10, 1, 2 > day_p;

    qi::symbols< CharT, date_time::weekdays > weekday_p;
    weekday_p.add
        (constants::monday_keyword(), date_time::Monday)
        (constants::tuesday_keyword(), date_time::Tuesday)
        (constants::wednesday_keyword(), date_time::Wednesday)
        (constants::thursday_keyword(), date_time::Thursday)
        (constants::friday_keyword(), date_time::Friday)
        (constants::saturday_keyword(), date_time::Saturday)
        (constants::sunday_keyword(), date_time::Sunday);
    weekday_p.add
        (constants::short_monday_keyword(), date_time::Monday)
        (constants::short_tuesday_keyword(), date_time::Tuesday)
        (constants::short_wednesday_keyword(), date_time::Wednesday)
        (constants::short_thursday_keyword(), date_time::Thursday)
        (constants::short_friday_keyword(), date_time::Friday)
        (constants::short_saturday_keyword(), date_time::Saturday)
        (constants::short_sunday_keyword(), date_time::Sunday);

    optional< date_time::weekdays > weekday;
    optional< unsigned short > day;
    unsigned char hour = 0, minute = 0, second = 0;

    bool result = qi::parse
    (
        value.begin(), value.end(),
        (
            -(
                (
                    // First check for a weekday
                    weekday_p[boost::log::as_action(boost::log::bind_assign(weekday))] |
                    // ... or a day in month
                    day_p[boost::log::as_action(boost::log::bind_assign(day))]
                ) >>
                (+encoding_specific::space)
            ) >>
            // Then goes the time of day
            (
                time_component_p[boost::log::as_action(boost::log::bind_assign(hour))] >> colon >>
                time_component_p[boost::log::as_action(boost::log::bind_assign(minute))] >> colon >>
                time_component_p[boost::log::as_action(boost::log::bind_assign(second))]
            ) >>
            qi::eoi
        )
    );

    if (!result)
        throw_invalid_value(param_name);

    if (weekday)
        return sinks::file::rotation_at_time_point(weekday.get(), hour, minute, second);
    else if (day)
        return sinks::file::rotation_at_time_point(gregorian::greg_day(day.get()), hour, minute, second);
    else
        return sinks::file::rotation_at_time_point(hour, minute, second);
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
#ifndef BOOST_LOG_WITHOUT_SYSLOG
        instance.m_Factories["Syslog"] = &sinks_repository< char_type >::default_syslog_sink_factory;
#endif
#ifndef BOOST_LOG_WITHOUT_DEBUG_OUTPUT
        instance.m_Factories["Debugger"] = &sinks_repository< char_type >::default_debugger_sink_factory;
#endif
#ifndef BOOST_LOG_WITHOUT_EVENT_LOG
        instance.m_Factories["SimpleEventLog"] = &sinks_repository< char_type >::default_simple_event_log_sink_factory;
#endif
    }

private:
    sinks_repository() {}

    //! Sink backend character selection function
    template< typename InitializerT >
    static shared_ptr< sinks::sink > select_backend_character_type(section const& params, InitializerT initializer)
    {
#if defined(BOOST_LOG_USE_CHAR) && defined(BOOST_LOG_USE_WCHAR_T)
        if (optional< string_type > wide_param = params["Wide"])
        {
            if (param_cast_to_bool("Wide", wide_param.get()))
                return initializer(params, type< wchar_t >());
        }

        return initializer(params, type< char >());
#elif defined(BOOST_LOG_USE_CHAR)
        return initializer(params, type< char >());
#elif defined(BOOST_LOG_USE_WCHAR_T)
        return initializer(params, type< wchar_t >());
#endif
    }

    //! The function constructs a sink that writes log records to a text file
    static shared_ptr< sinks::sink > default_text_file_sink_factory(section const& params)
    {
        typedef sinks::text_file_backend backend_t;
        shared_ptr< backend_t > backend = boost::make_shared< backend_t >();

        // FileName
        if (optional< string_type > file_name_param = params["FileName"])
        {
            backend->set_file_name_pattern(filesystem::path(file_name_param.get()));
        }
        else
            BOOST_LOG_THROW_DESCR(missing_value, "File name is not specified");

        // File rotation size
        if (optional< string_type > rotation_size_param = params["RotationSize"])
        {
            backend->set_rotation_size(param_cast_to_int< uintmax_t >("RotationSize", rotation_size_param.get()));
        }

        // File rotation interval
        if (optional< string_type > rotation_interval_param = params["RotationInterval"])
        {
            backend->set_time_based_rotation(sinks::file::rotation_at_time_interval(
                posix_time::seconds(param_cast_to_int< unsigned int >("RotationInterval", rotation_interval_param.get()))));
        }
        else if (optional< string_type > rotation_time_point_param = params["RotationTimePoint"])
        {
            // File rotation time point
            backend->set_time_based_rotation(param_cast_to_rotation_time_point("RotationTimePoint", rotation_time_point_param.get()));
        }

        // Auto flush
        if (optional< string_type > auto_flush_param = params["AutoFlush"])
        {
            backend->auto_flush(param_cast_to_bool("AutoFlush", auto_flush_param.get()));
        }

        // Append
        if (optional< string_type > append_param = params["Append"])
        {
            if (param_cast_to_bool("Append", append_param.get()))
                backend->set_open_mode(std::ios_base::out | std::ios_base::app);
        }

        // File collector parameters
        // Target directory
        if (optional< string_type > target_param = params["Target"])
        {
            filesystem::path target_dir(target_param.get());

            // Max total size
            uintmax_t max_size = (std::numeric_limits< uintmax_t >::max)();
            if (optional< string_type > max_size_param = params["MaxSize"])
                max_size = param_cast_to_int< uintmax_t >("MaxSize", max_size_param.get());

            // Min free space
            uintmax_t space = 0;
            if (optional< string_type > min_space_param = params["MinFreeSpace"])
                space = param_cast_to_int< uintmax_t >("MinFreeSpace", min_space_param.get());

            backend->set_file_collector(sinks::file::make_collector(
                keywords::target = target_dir,
                keywords::max_size = max_size,
                keywords::min_free_space = space));

            // Scan for log files
            if (optional< string_type > scan_param = params["ScanForFiles"])
            {
                string_type const& value = scan_param.get();
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
        }

        return init_sink(backend, params);
    }

    struct default_console_sink_factory_impl;
    friend struct default_console_sink_factory_impl;
    struct default_console_sink_factory_impl
    {
        typedef shared_ptr< sinks::sink > result_type;

        template< typename BackendCharT >
        result_type operator() (section const& params, type< BackendCharT >) const
        {
            // Construct the backend
            typedef boost::log::aux::char_constants< BackendCharT > constants;
            typedef sinks::basic_text_ostream_backend< BackendCharT > backend_t;
            shared_ptr< backend_t > backend = boost::make_shared< backend_t >();
            backend->add_stream(shared_ptr< typename backend_t::stream_type >(&constants::get_console_log_stream(), empty_deleter()));

            // Auto flush
            if (optional< string_type > auto_flush_param = params["AutoFlush"])
            {
                backend->auto_flush(param_cast_to_bool("AutoFlush", auto_flush_param.get()));
            }

            return sinks_repository::init_sink(backend, params);
        }
    };

    //! The function constructs a sink that writes log records to the console
    static shared_ptr< sinks::sink > default_console_sink_factory(section const& params)
    {
        return select_backend_character_type(params, default_console_sink_factory_impl());
    }

#ifndef BOOST_LOG_WITHOUT_SYSLOG

    //! The function constructs a sink that writes log records to the syslog service
    static shared_ptr< sinks::sink > default_syslog_sink_factory(section const& params)
    {
        // Construct the backend
        typedef sinks::syslog_backend backend_t;
        shared_ptr< backend_t > backend = boost::make_shared< backend_t >();

        // For now we use only the default level mapping. Will add support for configuration later.
        backend->set_severity_mapper(sinks::syslog::direct_severity_mapping< >(log::aux::default_attribute_names::severity()));

#if !defined(BOOST_LOG_NO_ASIO)
        // Setup local and remote addresses
        if (optional< string_type > local_address_param = params["LocalAddress"])
            backend->set_local_address(param_cast_to_address("LocalAddress", local_address_param.get()));

        if (optional< string_type > target_address_param = params["TargetAddress"])
            backend->set_target_address(param_cast_to_address("TargetAddress", target_address_param.get()));
#endif // !defined(BOOST_LOG_NO_ASIO)

        return init_sink(backend, params);
    }

#endif // BOOST_LOG_WITHOUT_SYSLOG

#ifndef BOOST_LOG_WITHOUT_DEBUG_OUTPUT

    struct default_debugger_sink_factory_impl;
    friend struct default_debugger_sink_factory_impl;
    struct default_debugger_sink_factory_impl
    {
        typedef shared_ptr< sinks::sink > result_type;

        template< typename BackendCharT >
        result_type operator() (section const& params, type< BackendCharT >) const
        {
            // Construct the backend
            typedef sinks::basic_debug_output_backend< BackendCharT > backend_t;
            shared_ptr< backend_t > backend = boost::make_shared< backend_t >();

            return sinks_repository::init_sink(backend, params);
        }
    };

    //! The function constructs a sink that writes log records to the debugger
    static shared_ptr< sinks::sink > default_debugger_sink_factory(section const& params)
    {
        return select_backend_character_type(params, default_debugger_sink_factory_impl());
    }

#endif // BOOST_LOG_WITHOUT_DEBUG_OUTPUT

#ifndef BOOST_LOG_WITHOUT_EVENT_LOG

    struct default_simple_event_log_sink_factory_impl;
    friend struct default_simple_event_log_sink_factory_impl;
    struct default_simple_event_log_sink_factory_impl
    {
        typedef shared_ptr< sinks::sink > result_type;

        template< typename BackendCharT >
        result_type operator() (section const& params, type< BackendCharT >) const
        {
            typedef sinks::basic_simple_event_log_backend< BackendCharT > backend_t;
            typedef typename backend_t::string_type backend_string_type;

            // Determine the log name
            backend_string_type log_name;
            if (optional< string_type > log_name_param = params["LogName"])
                log::aux::code_convert(log_name_param.get(), log_name);
            else
                log_name = backend_t::get_default_log_name();

            // Determine the log source name
            backend_string_type source_name;
            if (optional< string_type > log_source_param = params["LogSource"])
                log::aux::code_convert(log_source_param.get(), source_name);
            else
                source_name = backend_t::get_default_source_name();

            // Determine the registration mode
            sinks::event_log::registration_mode reg_mode = sinks::event_log::on_demand;
            if (optional< string_type > registration_param = params["Registration"])
            {
                string_type const& value = registration_param.get();
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

            // Construct the backend
            shared_ptr< backend_t > backend(boost::make_shared< backend_t >((
                keywords::log_name = log_name,
                keywords::log_source = source_name,
                keywords::registration = reg_mode)));

            // For now we use only the default event type mapping. Will add support for configuration later.
            backend->set_event_type_mapper(sinks::event_log::direct_event_type_mapping< >(log::aux::default_attribute_names::severity()));

            return sinks_repository::init_sink(backend, params);
        }
    };

    //! The function constructs a sink that writes log records to the Windows NT Event Log
    static shared_ptr< sinks::sink > default_simple_event_log_sink_factory(section const& params)
    {
        return select_backend_character_type(params, default_simple_event_log_sink_factory_impl());
    }

#endif // BOOST_LOG_WITHOUT_EVENT_LOG

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
        filter filt;
        if (optional< string_type > filter_param = params["Filter"])
        {
            filt = parse_filter(filter_param.get());
        }

        shared_ptr< sinks::basic_sink_frontend > p;

#if !defined(BOOST_LOG_NO_THREADS)
        // Asynchronous. TODO: make it more flexible.
        bool async = false;
        if (optional< string_type > async_param = params["Asynchronous"])
        {
            async = param_cast_to_bool("Asynchronous", async_param.get());
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
        if (optional< string_type > formatter_param = params["Formatter"])
        {
            typedef typename SinkT::char_type sink_char_type;
            std::basic_string< sink_char_type > formatter_str;
            log::aux::code_convert(formatter_param.get(), formatter_str);
            sink->set_formatter(parse_formatter(formatter_str));
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
void apply_core_settings(basic_settings_section< CharT > const& params)
{
    typedef CharT char_type;
    typedef std::basic_string< char_type > string_type;

    core_ptr core = boost::log::core::get();

    // Filter
    if (optional< string_type > filter_param = params["Filter"])
        core->set_filter(parse_filter(filter_param.get()));
    else
        core->reset_filter();

    // DisableLogging
    if (optional< string_type > disable_logging_param = params["DisableLogging"])
        core->set_logging_enabled(!param_cast_to_bool("DisableLogging", disable_logging_param.get()));
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
                new_sinks.push_back(sinks_repo.construct_sink_from_settings(sink_params));
            }
        }

        std::for_each(new_sinks.begin(), new_sinks.end(), boost::bind(&core::add_sink, core::get(), _1));
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

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_WITHOUT_SETTINGS_PARSERS
