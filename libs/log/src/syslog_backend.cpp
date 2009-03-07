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

#include "windows_version.hpp"
#include <boost/assert.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/host_name.hpp>
#include <boost/system/error_code.hpp>
#include <boost/date_time/c_time.hpp>
#include <boost/intrusive/set.hpp>
#include <boost/intrusive/set_hook.hpp>
#include <boost/intrusive/options.hpp>
#include <boost/compatibility/cpp_c_headers/ctime>
#include <boost/log/sinks/syslog_backend.hpp>
#include <boost/log/detail/singleton.hpp>
#include <boost/log/detail/snprintf.hpp>
#if !defined(BOOST_LOG_NO_THREADS)
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/detail/atomic_count.hpp>
#endif

#ifdef BOOST_LOG_USE_NATIVE_SYSLOG
#include <syslog.h>
#endif // BOOST_LOG_USE_NATIVE_SYSLOG

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace sinks {

namespace syslog {

    //  Syslog record levels
    BOOST_LOG_EXPORT extern const level_t emergency = { 0 };
    BOOST_LOG_EXPORT extern const level_t alert = { 1 };
    BOOST_LOG_EXPORT extern const level_t critical = { 2 };
    BOOST_LOG_EXPORT extern const level_t error = { 3 };
    BOOST_LOG_EXPORT extern const level_t warning = { 4 };
    BOOST_LOG_EXPORT extern const level_t notice = { 5 };
    BOOST_LOG_EXPORT extern const level_t info = { 6 };
    BOOST_LOG_EXPORT extern const level_t debug = { 7 };

    //  Syslog facility codes
    BOOST_LOG_EXPORT extern const facility_t kernel = { 0 * 8 };
    BOOST_LOG_EXPORT extern const facility_t user = { 1 * 8 };
    BOOST_LOG_EXPORT extern const facility_t mail = { 2 * 8 };
    BOOST_LOG_EXPORT extern const facility_t daemon = { 3 * 8 };
    BOOST_LOG_EXPORT extern const facility_t security0 = { 4 * 8 };
    BOOST_LOG_EXPORT extern const facility_t syslogd = { 5 * 8 };
    BOOST_LOG_EXPORT extern const facility_t printer = { 6 * 8 };
    BOOST_LOG_EXPORT extern const facility_t news = { 7 * 8 };
    BOOST_LOG_EXPORT extern const facility_t uucp = { 8 * 8 };
    BOOST_LOG_EXPORT extern const facility_t clock0 = { 9 * 8 };
    BOOST_LOG_EXPORT extern const facility_t security1 = { 10 * 8 };
    BOOST_LOG_EXPORT extern const facility_t ftp = { 11 * 8 };
    BOOST_LOG_EXPORT extern const facility_t ntp = { 12 * 8 };
    BOOST_LOG_EXPORT extern const facility_t log_audit = { 13 * 8 };
    BOOST_LOG_EXPORT extern const facility_t log_alert = { 14 * 8 };
    BOOST_LOG_EXPORT extern const facility_t clock1 = { 15 * 8 };
    BOOST_LOG_EXPORT extern const facility_t local0 = { 16 * 8 };
    BOOST_LOG_EXPORT extern const facility_t local1 = { 17 * 8 };
    BOOST_LOG_EXPORT extern const facility_t local2 = { 18 * 8 };
    BOOST_LOG_EXPORT extern const facility_t local3 = { 19 * 8 };
    BOOST_LOG_EXPORT extern const facility_t local4 = { 20 * 8 };
    BOOST_LOG_EXPORT extern const facility_t local5 = { 21 * 8 };
    BOOST_LOG_EXPORT extern const facility_t local6 = { 22 * 8 };
    BOOST_LOG_EXPORT extern const facility_t local7 = { 23 * 8 };

} // namespace syslog

////////////////////////////////////////////////////////////////////////////////
//! Syslog sink backend implementation
////////////////////////////////////////////////////////////////////////////////
template< typename CharT >
struct basic_syslog_backend< CharT >::implementation
{
#ifdef BOOST_LOG_USE_NATIVE_SYSLOG
    struct native;
#endif // BOOST_LOG_USE_NATIVE_SYSLOG
    struct udp_socket_based;

    //! Level mapper
    severity_mapper_type m_LevelMapper;

    //! Logging facility (portable or native, depending on the backend implementation)
    const int m_Facility;

    //! Constructor
    explicit implementation(int facility) :
        m_Facility(facility)
    {
    }
    //! Virtual destructor
    virtual ~implementation() {}

    //! The method sends the formatted message to the syslog host
    virtual void send(syslog::level_t level, target_string_type const& formatted_message) = 0;
};


////////////////////////////////////////////////////////////////////////////////
//  Native syslog API support
////////////////////////////////////////////////////////////////////////////////

#ifdef BOOST_LOG_USE_NATIVE_SYSLOG

namespace {

    //! Syslog service initializer (implemented as a weak singleton)
#if !defined(BOOST_LOG_NO_THREADS)
    class native_syslog_initializer :
        private log::aux::lazy_singleton< native_syslog_initializer, mutex >
#else
    class native_syslog_initializer
#endif
    {
#if !defined(BOOST_LOG_NO_THREADS)
        friend class log::aux::lazy_singleton< native_syslog_initializer, mutex >;
        typedef log::aux::lazy_singleton< native_syslog_initializer, mutex > mutex_holder;
#endif

    public:
        native_syslog_initializer()
        {
            ::openlog("", 0, LOG_USER);
        }
        ~native_syslog_initializer()
        {
            ::closelog();
        }

        static shared_ptr< native_syslog_initializer > get_instance()
        {
#if !defined(BOOST_LOG_NO_THREADS)
            lock_guard< mutex > _(mutex_holder::get());
#endif
            static weak_ptr< native_syslog_initializer > instance;
            shared_ptr< native_syslog_initializer > p(instance.lock());
            if (!p)
            {
                p = boost::make_shared< native_syslog_initializer >();
                instance = p;
            }
            return p;
        }
    };

} // namespace

template< typename CharT >
struct basic_syslog_backend< CharT >::implementation::native :
    public implementation
{
    //! Reference to the syslog service initializer
    const shared_ptr< native_syslog_initializer > m_pSyslogInitializer;

    //! Constructor
    explicit native(syslog::facility_t const& facility) :
        implementation(convert_facility(facility)),
        m_pSyslogInitializer(native_syslog_initializer::get_instance())
    {
    }

    //! The method sends the formatted message to the syslog host
    void send(syslog::level_t level, target_string_type const& formatted_message)
    {
        int native_level;
        switch (static_cast< int >(level))
        {
        case 0:
            native_level = LOG_EMERG; break;
        case 1:
            native_level = LOG_ALERT; break;
        case 2:
            native_level = LOG_CRIT; break;
        case 3:
            native_level = LOG_ERR; break;
        case 4:
            native_level = LOG_WARNING; break;
        case 5:
            native_level = LOG_NOTICE; break;
        case 7:
            native_level = LOG_DEBUG; break;
        default:
            native_level = LOG_INFO; break;
        }

        ::syslog(LOG_MAKEPRI(this->m_Facility, native_level), "%s", formatted_message.c_str());
    }

private:
    //! The function converts portable facility codes to the native codes
    static int convert_facility(syslog::facility_t const& facility)
    {
        // POSIX does not specify anything except for LOG_USER and LOG_LOCAL*
        #ifndef LOG_KERN
        #define LOG_KERN LOG_USER
        #endif
        #ifndef LOG_DAEMON
        #define LOG_DAEMON LOG_KERN
        #endif
        #ifndef LOG_MAIL
        #define LOG_MAIL LOG_USER
        #endif
        #ifndef LOG_AUTH
        #define LOG_AUTH LOG_DAEMON
        #endif
        #ifndef LOG_SYSLOG
        #define LOG_SYSLOG LOG_DAEMON
        #endif
        #ifndef LOG_LPR
        #define LOG_LPR LOG_DAEMON
        #endif
        #ifndef LOG_NEWS
        #define LOG_NEWS LOG_USER
        #endif
        #ifndef LOG_UUCP
        #define LOG_UUCP LOG_USER
        #endif
        #ifndef LOG_CRON
        #define LOG_CRON LOG_DAEMON
        #endif
        #ifndef LOG_AUTHPRIV
        #define LOG_AUTHPRIV LOG_AUTH
        #endif
        #ifndef LOG_FTP
        #define LOG_FTP LOG_DAEMON
        #endif

        static const int native_facilities[24] =
        {
            LOG_KERN,
            LOG_USER,
            LOG_MAIL,
            LOG_DAEMON,
            LOG_AUTH,
            LOG_SYSLOG,
            LOG_LPR,
            LOG_NEWS,
            LOG_UUCP,
            LOG_CRON,
            LOG_AUTHPRIV,
            LOG_FTP,

            // reserved values
            LOG_USER,
            LOG_USER,
            LOG_USER,
            LOG_USER,

            LOG_LOCAL0,
            LOG_LOCAL1,
            LOG_LOCAL2,
            LOG_LOCAL3,
            LOG_LOCAL4,
            LOG_LOCAL5,
            LOG_LOCAL6,
            LOG_LOCAL7
        };

        register std::size_t n = static_cast< int >(facility) / 8;
        BOOST_ASSERT(n < sizeof(native_facilities) / sizeof(*native_facilities));
        return native_facilities[n];
    }
};

#endif // BOOST_LOG_USE_NATIVE_SYSLOG


////////////////////////////////////////////////////////////////////////////////
//  Socket-based implementation
////////////////////////////////////////////////////////////////////////////////
namespace {

    class sockets_repository;

    typedef intrusive::set_base_hook<
        intrusive::tag< struct ordered_by_local_address >,
        intrusive::optimize_size< true >,
        intrusive::link_mode< intrusive::safe_link >
    > shared_socket_base_hook;

    //! The shared UDP socket
    struct shared_socket :
        public shared_socket_base_hook
    {
        //! The predicate orders sockets by their local addresses and ports
        struct order_by_local_address;
        friend struct order_by_local_address;
        struct order_by_local_address
        {
            typedef bool result_type;

            // NOTE: We assume here that calling local_endpoint() from different threads concurrently
            // is actually safe. Otherwise we'd have to lock both sockets prior to comparison, which
            // would lead to horrible ordering performance in the set container. Note also that
            // having a copy of a local endpoint in the shared_socket class won't help, since
            // we don't have thread safety guarantees for its copy constructor or operator<() either.
            result_type operator() (shared_socket const& left, shared_socket const& right) const
            {
                return (left.m_Socket.local_endpoint() < right.m_Socket.local_endpoint());
            }
            result_type operator() (asio::ip::udp::endpoint const& left, shared_socket const& right) const
            {
                return (left < right.m_Socket.local_endpoint());
            }
            result_type operator() (shared_socket const& left, asio::ip::udp::endpoint const& right) const
            {
                return (left.m_Socket.local_endpoint() < right);
            }
        };

    private:
#if !defined(BOOST_LOG_NO_THREADS)
        mutable detail::atomic_count m_RefCount;
        mutex m_Mutex;
#else
        mutable long m_RefCount;
#endif // !defined(BOOST_LOG_NO_THREADS)

        shared_ptr< sockets_repository > m_pRepo;
        asio::ip::udp::socket m_Socket;

    public:
        //! The constructor creates a socket bound to the specified local address and port
        explicit shared_socket(
            shared_ptr< sockets_repository > const& repo,
            asio::io_service& service,
            asio::ip::udp::endpoint const& local_address
        ) :
            m_RefCount(0),
            m_pRepo(repo),
            m_Socket(service, local_address)
        {
        }

        //! The method sends the syslog message to the specified endpoint
        void send_syslog_message(int pri, asio::ip::udp::endpoint const& target, const char* message);

        friend void intrusive_ptr_add_ref(const shared_socket* p);
        friend void intrusive_ptr_release(const shared_socket* p);

    private:
        shared_socket(shared_socket const&);
        shared_socket& operator= (shared_socket const&);
    };

    //! The class contains the list of sockets, arranged by their local addresses
    class sockets_repository :
        public log::aux::lazy_singleton< sockets_repository, shared_ptr< sockets_repository > >
    {
        friend class log::aux::lazy_singleton< sockets_repository, shared_ptr< sockets_repository > >;
        typedef log::aux::lazy_singleton< sockets_repository, shared_ptr< sockets_repository > > base_type;

        friend void intrusive_ptr_release(const shared_socket* p);

        //! Type of the map of sokets
        typedef intrusive::set<
            shared_socket,
            intrusive::base_hook< shared_socket_base_hook >,
            intrusive::compare< shared_socket::order_by_local_address >,
            intrusive::constant_time_size< false >
        > sockets_map;

    public:
#if !defined(BOOST_LOG_NO_THREADS)
        //! Synchronization primitive
        mutex m_Mutex;
#endif // !defined(BOOST_LOG_NO_THREADS)

        //! The core IO service instance
        asio::io_service m_IOService;
        //! The resolver is used to acquire connection endpoints
        asio::ip::udp::resolver m_HostNameResolver;

        //! The local host name to put into log message
        std::string m_LocalHostName;

    private:
        //! The map of sockets arranged by their local addresses and ports
        sockets_map m_Sockets;

    public:
        //! Retrieves or opens a socket for the specified local address
        intrusive_ptr< shared_socket > get_socket(asio::ip::address const& addr, unsigned short port)
        {
            asio::ip::udp::endpoint local_address(addr, port);
            return get_socket(local_address);
        }

        //! Retrieves or opens a socket for the specified local address
        intrusive_ptr< shared_socket > get_socket(asio::ip::udp::endpoint const& local_address)
        {
#if !defined(BOOST_LOG_NO_THREADS)
            lock_guard< mutex > _(m_Mutex);
#endif // !defined(BOOST_LOG_NO_THREADS)

            intrusive_ptr< shared_socket > p;
            sockets_map::iterator it = m_Sockets.find(local_address, m_Sockets.key_comp());
            if (it == m_Sockets.end())
            {
                p = new shared_socket(base_type::get(), m_IOService, local_address);
                m_Sockets.insert(*p);
            }
            else
                p = &*it;

            return p;
        }

    private:
        //! Default constructor
        sockets_repository() :
            m_HostNameResolver(m_IOService)
        {
            boost::system::error_code err;
            m_LocalHostName = asio::ip::host_name(err);
        }
        //! Initializes the singleton instance
        static void init_instance()
        {
            base_type::get_instance().reset(new sockets_repository());
        }
    };

    inline void intrusive_ptr_add_ref(const shared_socket* p)
    {
        ++p->m_RefCount;
    }

    inline void intrusive_ptr_release(const shared_socket* p)
    {
        if (--p->m_RefCount == 0)
        {
#if !defined(BOOST_LOG_NO_THREADS)
            lock_guard< mutex > _(p->m_pRepo->m_Mutex);
            // We have to double check that the reference counter hasn't changed
            // while we were locking the repository
            register long counter = p->m_RefCount;
            if (counter)
                return;
#endif // !defined(BOOST_LOG_NO_THREADS)

            p->m_pRepo->m_Sockets.erase(*p);
            delete p;
        }
    }

    //! The method sends the syslog message to the specified endpoint
    void shared_socket::send_syslog_message(int pri, asio::ip::udp::endpoint const& target, const char* message)
    {
        std::time_t t = std::time(NULL);
        std::tm ts;
        boost::date_time::c_time::localtime(&t, &ts);
        char time_stamp[16];
        std::strftime(time_stamp, sizeof(time_stamp), "%e %H:%M:%S", &ts);

        // Month will have to be injected separately, as involving locale won't do here
        static const char months[12][4] =
        {
            "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
        };

        // The packet size is mandated in RFC3164, plus one for the terminating zero
        char packet[1025];
        std::size_t packet_size = boost::log::aux::snprintf(
            packet,
            sizeof(packet),
            "<%d> %s %s %s %s",
            pri,
            months[ts.tm_mon],
            time_stamp,
            m_pRepo->m_LocalHostName.c_str(),
            message
        );

#if !defined(BOOST_LOG_NO_THREADS)
        lock_guard< mutex > _(m_Mutex);
#endif

        m_Socket.send_to(asio::buffer(packet, packet_size), target);
    }

} // namespace

template< typename CharT >
struct basic_syslog_backend< CharT >::implementation::udp_socket_based :
    public implementation
{
    //! Pointer to the list of sockets
    shared_ptr< sockets_repository > m_pSockets;
    //! Pointer to the socket being used
    intrusive_ptr< shared_socket > m_pSocket;
    //! The target host to send packets to
    asio::ip::udp::endpoint m_TargetHost;

    //! Constructor
    explicit udp_socket_based(syslog::facility_t const& facility) :
        implementation(facility),
        m_pSockets(sockets_repository::get()),
        m_TargetHost(asio::ip::address_v4(0x7F000001), 514) // 127.0.0.1:514
    {
    }

    //! The method sends the formatted message to the syslog host
    void send(syslog::level_t level, target_string_type const& formatted_message)
    {
        if (!m_pSocket)
            m_pSocket = m_pSockets->get_socket(asio::ip::address_v4(0x7F000001), 514); // 127.0.0.1:514

        m_pSocket->send_syslog_message(this->m_Facility | static_cast< int >(level), m_TargetHost, formatted_message.c_str());
    }
};


////////////////////////////////////////////////////////////////////////////////
//  Sink backend implementation
////////////////////////////////////////////////////////////////////////////////
//! Destructor
template< typename CharT >
basic_syslog_backend< CharT >::~basic_syslog_backend()
{
    delete m_pImpl;
}

//! The method installs the function object that maps application severity levels to Syslog levels
template< typename CharT >
void basic_syslog_backend< CharT >::set_severity_mapper(severity_mapper_type const& mapper)
{
    m_pImpl->m_LevelMapper = mapper;
}

//! The method writes the message to the sink
template< typename CharT >
void basic_syslog_backend< CharT >::do_consume(
    values_view_type const& attributes, target_string_type const& formatted_message)
{
    m_pImpl->send(
        m_pImpl->m_LevelMapper.empty() ? syslog::info : m_pImpl->m_LevelMapper(attributes),
        formatted_message);
}


//! The method creates the backend implementation
template< typename CharT >
typename basic_syslog_backend< CharT >::implementation*
basic_syslog_backend< CharT >::construct(syslog::facility_t facility, syslog::impl_types use_impl)
{
#ifdef BOOST_LOG_USE_NATIVE_SYSLOG
    if (use_impl == syslog::native)
    {
        typedef typename implementation::native native_impl;
        return new native_impl(facility);
    }
#endif // BOOST_LOG_USE_NATIVE_SYSLOG

    typedef typename implementation::udp_socket_based udp_socket_based_impl;
    return new udp_socket_based_impl(facility);
}

//! The method sets the local address which log records will be sent from.
template< typename CharT >
void basic_syslog_backend< CharT >::set_local_address(std::string const& addr)
{
    typedef typename implementation::udp_socket_based udp_socket_based_impl;
    if (udp_socket_based_impl* impl = dynamic_cast< udp_socket_based_impl* >(m_pImpl))
    {
        asio::ip::udp::resolver::query q(addr, "syslog");
        asio::ip::udp::endpoint local_address;

        {
#if !defined(BOOST_LOG_NO_THREADS)
            lock_guard< mutex > _(impl->m_pSockets->m_Mutex);
#endif

            local_address = *impl->m_pSockets->m_HostNameResolver.resolve(q);
        }

        impl->m_pSocket = impl->m_pSockets->get_socket(local_address);
    }
}
//! The method sets the local address which log records will be sent from.
template< typename CharT >
void basic_syslog_backend< CharT >::set_local_address(boost::asio::ip::address const& addr, unsigned short port)
{
    typedef typename implementation::udp_socket_based udp_socket_based_impl;
    if (udp_socket_based_impl* impl = dynamic_cast< udp_socket_based_impl* >(m_pImpl))
    {
        impl->m_pSocket = impl->m_pSockets->get_socket(addr, port);
    }
}

//! The method sets the address of the remote host where log records will be sent to.
template< typename CharT >
void basic_syslog_backend< CharT >::set_target_address(std::string const& addr)
{
    typedef typename implementation::udp_socket_based udp_socket_based_impl;
    if (udp_socket_based_impl* impl = dynamic_cast< udp_socket_based_impl* >(m_pImpl))
    {
        asio::ip::udp::resolver::query q(addr, "syslog");
        asio::ip::udp::endpoint remote_address;

        {
#if !defined(BOOST_LOG_NO_THREADS)
            lock_guard< mutex > _(impl->m_pSockets->m_Mutex);
#endif

            remote_address = *impl->m_pSockets->m_HostNameResolver.resolve(q);
        }

        impl->m_TargetHost = remote_address;
    }
}
//! The method sets the address of the remote host where log records will be sent to.
template< typename CharT >
void basic_syslog_backend< CharT >::set_target_address(boost::asio::ip::address const& addr, unsigned short port)
{
    typedef typename implementation::udp_socket_based udp_socket_based_impl;
    if (udp_socket_based_impl* impl = dynamic_cast< udp_socket_based_impl* >(m_pImpl))
    {
        impl->m_TargetHost = asio::ip::udp::endpoint(addr, port);
    }
}

//! Explicitly instantiate sink implementation
#ifdef BOOST_LOG_USE_CHAR
template class basic_syslog_backend< char >;
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
template class basic_syslog_backend< wchar_t >;
#endif

} // namespace sinks

} // namespace log

} // namespace boost
