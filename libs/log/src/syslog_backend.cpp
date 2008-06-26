/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   syslog_backend.cpp
 * \author Andrey Semashev
 * \date   08.01.2008
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#include <syslog.h>
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/log/sinks/syslog_backend.hpp>
#include <boost/log/detail/narrowing_sstream_buf.hpp>
#include <boost/log/detail/cleanup_scope_guard.hpp>
#include <boost/log/detail/singleton.hpp>
#if !defined(BOOST_LOG_NO_THREADS)
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#endif

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace sinks {

namespace syslog {

#ifndef LOG_PERROR
#define LOG_PERROR 0
#endif
#ifndef LOG_DAEMON
#define LOG_DAEMON LOG_USER
#endif
#ifndef LOG_MAIL
#define LOG_MAIL LOG_USER
#endif
#ifndef LOG_NEWS
#define LOG_NEWS LOG_USER
#endif

    //  Syslog initialization options
    BOOST_LOG_EXPORT extern const options_t console_fallback = { LOG_CONS };
    BOOST_LOG_EXPORT extern const options_t no_delay = { LOG_NDELAY };
    BOOST_LOG_EXPORT extern const options_t no_wait = { LOG_NOWAIT };
    BOOST_LOG_EXPORT extern const options_t print_stderr = { LOG_PERROR };
    BOOST_LOG_EXPORT extern const options_t log_pid = { LOG_PID };

    //  Syslog record levels
    BOOST_LOG_EXPORT extern const level_t emergency = { LOG_EMERG };
    BOOST_LOG_EXPORT extern const level_t alert = { LOG_ALERT };
    BOOST_LOG_EXPORT extern const level_t critical = { LOG_CRIT };
    BOOST_LOG_EXPORT extern const level_t error = { LOG_ERR };
    BOOST_LOG_EXPORT extern const level_t warning = { LOG_WARNING };
    BOOST_LOG_EXPORT extern const level_t notice = { LOG_NOTICE };
    BOOST_LOG_EXPORT extern const level_t info = { LOG_INFO };
    BOOST_LOG_EXPORT extern const level_t debug = { LOG_DEBUG };

    //  Syslog facility codes
    BOOST_LOG_EXPORT extern const facility_t daemon = { LOG_DAEMON };
    BOOST_LOG_EXPORT extern const facility_t user = { LOG_USER };
    BOOST_LOG_EXPORT extern const facility_t mail = { LOG_MAIL };
    BOOST_LOG_EXPORT extern const facility_t news = { LOG_NEWS };
    BOOST_LOG_EXPORT extern const facility_t local0 = { LOG_LOCAL0 };
    BOOST_LOG_EXPORT extern const facility_t local1 = { LOG_LOCAL1 };
    BOOST_LOG_EXPORT extern const facility_t local2 = { LOG_LOCAL2 };
    BOOST_LOG_EXPORT extern const facility_t local3 = { LOG_LOCAL3 };
    BOOST_LOG_EXPORT extern const facility_t local4 = { LOG_LOCAL4 };
    BOOST_LOG_EXPORT extern const facility_t local5 = { LOG_LOCAL5 };
    BOOST_LOG_EXPORT extern const facility_t local6 = { LOG_LOCAL6 };
    BOOST_LOG_EXPORT extern const facility_t local7 = { LOG_LOCAL7 };

} // namespace syslog

namespace {

    //! Syslog initializer (implemented as a weak singleton)
#if !defined(BOOST_LOG_NO_THREADS)
    class syslog_initializer :
        private log::aux::lazy_singleton< syslog_initializer, mutex >
#else
    class syslog_initializer
#endif
    {
#if !defined(BOOST_LOG_NO_THREADS)
        friend class log::aux::lazy_singleton< syslog_initializer, mutex >;
        typedef log::aux::lazy_singleton< syslog_initializer, mutex > mutex_holder;
#endif

    private:
        explicit syslog_initializer(syslog::options_t const& options)
        {
            ::openlog("", options.value, LOG_USER);
        }

    public:
        ~syslog_initializer()
        {
            ::closelog();
        }

        static shared_ptr< syslog_initializer > get_instance(syslog::options_t const& options)
        {
#if !defined(BOOST_LOG_NO_THREADS)
            lock_guard< mutex > _(mutex_holder::get());
#endif
            static weak_ptr< syslog_initializer > instance;
            shared_ptr< syslog_initializer > p(instance.lock());
            if (!p)
            {
                p.reset(new syslog_initializer(options));
                instance = p;
            }
            return p;
        }
    };

} // namespace

template< typename CharT >
struct basic_syslog_backend< CharT >::implementation
{
    //! Stream buffer type
    typedef typename log::aux::make_narrowing_ostringstreambuf< char_type >::type streambuf_type;

    //! Formatted string
    std::string m_FormattedRecord;
    //! Output stream buffer
    streambuf_type m_StreamBuffer;
    //! Output stream
    stream_type m_FormattingStream;

    //! Formatter
    formatter_type m_Formatter;
    //! Level extractor
    level_extractor_type m_LevelExtractor;

    //! Logging facility
    const syslog::facility_t m_Facility;
    //! Reference to the syslog service initializer
    const shared_ptr< syslog_initializer > m_pSyslogInitializer;

    //! Constructor
    implementation(syslog::facility_t const& facility, syslog::options_t const& options) :
        m_StreamBuffer(m_FormattedRecord),
        m_FormattingStream(&m_StreamBuffer),
        m_Facility(facility),
        m_pSyslogInitializer(syslog_initializer::get_instance(options))
    {
    }
};

//! Constructor
template< typename CharT >
basic_syslog_backend< CharT >::basic_syslog_backend(
    syslog::facility_t facility, syslog::options_t options) : m_pImpl(new implementation(facility, options))
{
}

//! Destructor
template< typename CharT >
basic_syslog_backend< CharT >::~basic_syslog_backend() 
{
    delete m_pImpl;
}

//! The method sets formatter functor object
template< typename CharT >
void basic_syslog_backend< CharT >::set_formatter(formatter_type const& fmt)
{
    m_pImpl->m_Formatter = fmt;
}
//! The method resets the formatter
template< typename CharT >
void basic_syslog_backend< CharT >::reset_formatter()
{
    m_pImpl->m_Formatter.clear();
}

//! The method installs the syslog record level extraction function object
template< typename CharT >
void basic_syslog_backend< CharT >::set_level_extractor(level_extractor_type const& extractor)
{
    m_pImpl->m_LevelExtractor = extractor;
}

//! The method returns the current locale
template< typename CharT >
std::locale basic_syslog_backend< CharT >::getloc() const
{
    return m_pImpl->m_FormattingStream.getloc();
}
//! The method sets the locale used during formatting
template< typename CharT >
std::locale basic_syslog_backend< CharT >::imbue(std::locale const& loc)
{
    return m_pImpl->m_FormattingStream.imbue(loc);
}

//! The method writes the message to the sink
template< typename CharT >
void basic_syslog_backend< CharT >::write_message(
    values_view_type const& attributes, string_type const& message)
{
    log::aux::cleanup_guard< std::string > _(m_pImpl->m_FormattedRecord);

    if (!m_pImpl->m_Formatter.empty())
        m_pImpl->m_Formatter(m_pImpl->m_FormattingStream, attributes, message);
    else
        m_pImpl->m_FormattingStream << message;

    m_pImpl->m_FormattingStream.flush();

    const int facility = m_pImpl->m_Facility;
    const int level =
        m_pImpl->m_LevelExtractor.empty() ? syslog::info : m_pImpl->m_LevelExtractor(attributes);

    ::syslog(LOG_MAKEPRI(facility, level), "%s", m_pImpl->m_FormattedRecord.c_str());
}

//! Explicitly instantiate sink implementation
#ifdef BOOST_LOG_USE_CHAR
template class BOOST_LOG_EXPORT basic_syslog_backend< char >;
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
template class BOOST_LOG_EXPORT basic_syslog_backend< wchar_t >;
#endif

} // namespace sinks

} // namespace log

} // namespace boost
