/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   rotating_ofstream.cpp
 * \author Andrey Semashev
 * \date   04.01.2008
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <ctype.h>
#include <wctype.h>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <boost/ref.hpp>
#include <boost/optional.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
#ifndef BOOST_FILESYSTEM_NARROW_ONLY
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_same.hpp>
#endif // BOOST_FILESYSTEM_NARROW_ONLY
#include <boost/filesystem/fstream.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/utility/in_place_factory.hpp>
#include <boost/log/sinks/rotating_ofstream.hpp>
#include <boost/log/attributes/time_traits.hpp>

namespace boost {

namespace log {

namespace sinks {

//! Logical device implementation
struct BOOST_LOG_NO_VTABLE rotating_file_sink::implementation : noncopyable
{
    //! Record storage
    std::string m_Storage;
    //! File stream buffer
    filesystem::ofstream m_File;
    //! File open mode
    std::ios_base::openmode m_OpenMode;

    //! Chars written to the file
    uintmax_t m_Written;
    //! File size rotation limit
    uintmax_t m_RotationSize;

    //! Last rotation time
    std::time_t m_LastRotation;
    //! Rotation time interval in seconds
    unsigned int m_RotationInterval;

    //! File opening handler
    open_handler_type m_OpenHandler;
    //! File closing handler
    close_handler_type m_CloseHandler;

    //! Constructor
    implementation(std::ios_base::openmode mode, uintmax_t rot_size, unsigned int rot_int) :
        m_OpenMode(mode),
        m_Written(0),
        m_RotationSize(rot_size),
        m_LastRotation(0),
        m_RotationInterval(rot_int)
    {
        m_OpenMode |= std::ios_base::out | std::ios_base::trunc;
        m_OpenMode &= ~std::ios_base::in;
    }

    //! The function opens a new file
    virtual void open_file() = 0;
    //! The function closes the file
    virtual void close_file() = 0;
};

namespace {

    //! An auxiliary traits that contain various constants and functions regarding string and character operations
    template< typename CharT >
    struct char_consts;

    template< >
    struct char_consts< char >
    {
        enum
        {
            percent = '%',
            number_placeholder = 'N',
            digit_placeholder = 'u',
            space = ' ',
            plus = '+',
            minus = '-',
            zero = '0',
            dot = '.'
        };
        static bool is_digit(char c)
        {
            return (isdigit(c) != 0);
        }

        typedef int (*lazy_sprintf)(char*, size_t, const char*, ...);
        static lazy_sprintf get_lazy_sprintf()
        {
            using namespace std;
#ifndef _MSC_VER
            return &snprintf;
#else
            return &_snprintf;
#endif
        }
    };

    template< >
    struct char_consts< wchar_t >
    {
        enum
        {
            percent = L'%',
            number_placeholder = L'N',
            digit_placeholder = L'u',
            space = L' ',
            plus = L'+',
            minus = L'-',
            zero = L'0',
            dot = L'.'
        };
        static bool is_digit(wchar_t c)
        {
            return (iswdigit(c) != 0);
        }

        typedef int (*lazy_sprintf)(wchar_t*, size_t, const wchar_t*, ...);
        static lazy_sprintf get_lazy_sprintf()
        {
            using namespace std;
            return &swprintf;
        }
    };

    //! The file controller implementation
    template< typename CharT >
    class file_controller :
        public rotating_file_sink::implementation
    {
        //! Base type
        typedef rotating_file_sink::implementation base_type;

    public:
        typedef CharT char_type;
        typedef std::basic_string< char_type > string_type;

#ifndef BOOST_FILESYSTEM_NARROW_ONLY
        typedef typename mpl::if_<
            is_same< char_type, char >,
            filesystem::path,
            filesystem::wpath
        >::type path_type;
#else
        typedef filesystem::path path_type;
#endif // BOOST_FILESYSTEM_NARROW_ONLY

    private:
        //! Date and time formatter
        class date_and_time_formatter
        {
            typedef date_time::time_facet< posix_time::ptime, char_type > time_facet_type;
            time_facet_type* m_pFacet;
            std::basic_ostringstream< char_type > m_Stream;
            const string_type m_EmptyString;

        public:
            //! Constructor
            date_and_time_formatter() : m_pFacet(NULL)
            {
                std::auto_ptr< time_facet_type > pFacet(new time_facet_type());
                m_pFacet = pFacet.get();
                std::locale loc(m_Stream.getloc(), m_pFacet);
                pFacet.release();
                m_Stream.imbue(loc);
            }
            //! The method formats the current date and time according to the format string str and writes the result into it
            void format(string_type& str)
            {
                m_pFacet->format(str.c_str());
                m_Stream.str(m_EmptyString);
                m_Stream << boost::log::attributes::local_time_traits::get_clock();
                if (m_Stream.good())
                    str = m_Stream.str();
                else
                    m_Stream.clear();
            }
        };

    private:
        //! The directory path
        path_type m_Path;
        //! The file name pattern
        string_type m_Pattern;

        //! The position in the pattern where the file counter placeholder is
        typename string_type::size_type m_FileCounterPosition;
        //! The file counter format
        string_type m_FileCounterFormat;

        //! The file counter
        unsigned int m_FileCounter;

        //! The date and time stamp formatter
        optional< date_and_time_formatter > m_DateFormatter;

    public:
        //! Constructor
        explicit file_controller(path_type const& pattern, std::ios_base::openmode mode, uintmax_t rot_size, unsigned int rot_int) :
            base_type(mode, rot_size, rot_int),
            m_Path(pattern.branch_path()),
            m_Pattern(pattern.leaf()),
            m_FileCounterPosition(0),
            m_FileCounter(0)
        {
            typedef char_consts< char_type > traits_t;
            typename string_type::const_iterator end = m_Pattern.end();
            typename string_type::const_iterator it = std::find(
                static_cast< typename string_type::const_iterator >(m_Pattern.begin()),
                end,
                static_cast< char_type >(traits_t::percent));
            unsigned int placeholder_count = 0;
            while (it != end)
            {
                ++placeholder_count;
                typename string_type::const_iterator placeholder_begin = it++;
                if (it == end)
                    break;
                typename string_type::const_iterator placeholder_end = parse_format_flag(it, end);
                
                if (placeholder_end != end)
                {
                    // We've found the file counter placeholder in the pattern
                    m_FileCounterFormat.assign(placeholder_begin, placeholder_end);
                    m_FileCounterFormat.push_back(static_cast< typename string_type::value_type >(traits_t::digit_placeholder));
                    m_FileCounterPosition = placeholder_begin - m_Pattern.begin();
                    m_Pattern.erase(m_FileCounterPosition, placeholder_end - placeholder_begin + 1);
                    --placeholder_count;
                    break;
                }
            }

            // Yes, this is not totally correct, because the pattern can contain '%' which are not placeholders.
            // It just optimises a bit in the obvious cases, so this is not crucial.
            if (placeholder_count > 0)
                m_DateFormatter = boost::in_place();
        }

        //! The function opens the file
        void open_file()
        {
            typedef char_consts< char_type > traits_t;

            // Let's construct the new file name
            string_type file_name = m_Pattern;
            if (!m_FileCounterFormat.empty())
            {
                // Calculate the needed buffer length to print the counter.
                // The std::numeric_limits< unsigned int >::digits10 shows the maximum number of decimals
                // in the printed counter minus one.
                // Also, it is possible to specify width of the printed value, which may be 9 at maximum.
                // We should also count for a possible sign character (who would ever need it, however?) and
                // the terminating null character.
                enum { unsigned_int_length = std::numeric_limits< unsigned int >::digits10 + 1 };
                char_type counter_buf[(unsigned_int_length < 9 ? 9 : unsigned_int_length) + 2];
                int printed = traits_t::get_lazy_sprintf()(
                    counter_buf, sizeof(counter_buf) / sizeof(*counter_buf), m_FileCounterFormat.c_str(), m_FileCounter);
                if (printed > 0)
                {
                    file_name.insert(m_FileCounterPosition, counter_buf, printed);
                }
            }
            ++m_FileCounter;

            if (!!m_DateFormatter)
                m_DateFormatter->format(file_name);

            path_type full_file_name = m_Path / file_name;
            this->m_File.open(full_file_name, this->m_OpenMode);
            if (this->m_File.is_open())
            {
                if (!this->m_OpenHandler.empty()) try
                {
                    this->m_OpenHandler(boost::ref(this->m_File));
                }
                catch (...)
                {
                }
            }
            else
                boost::throw_exception(std::runtime_error("failed to open file"));
        }

        //! The function closes the file
        void close_file()
        {
            if (this->m_File.is_open())
            {
                if (!this->m_CloseHandler.empty()) try
                {
                    this->m_CloseHandler(boost::ref(this->m_File));
                }
                catch (...)
                {
                }

                this->m_File.close();
            }
        }

    private:
        static typename string_type::const_iterator parse_format_flag(
            typename string_type::const_iterator& it, typename string_type::const_iterator end)
        {
            typedef char_consts< char_type > traits_t;
            switch (*it)
            {
                case traits_t::plus:
                case traits_t::minus:
                case traits_t::space:
                case traits_t::zero:
                    // Format flag detected
                    if (++it == end)
                        return end;
                    if (traits_t::is_digit(*it))
                    {
                        // Format width detected
                        if (++it == end)
                            return end;
                    }
                default:
                    return parse_format_precision(it, end);
            }
        }
        static typename string_type::const_iterator parse_format_precision(
            typename string_type::const_iterator& it, typename string_type::const_iterator end)
        {
            typedef char_consts< char_type > traits_t;
            switch (*it)
            {
                case traits_t::dot:
                    // Format precision detected
                    if (++it == end)
                        return end;
                    if (traits_t::is_digit(*it))
                    {
                        if (++it == end)
                            return end;
                    }
                    else
                        return end;

                    if (*it != traits_t::number_placeholder)
                        return end;

                case traits_t::number_placeholder:
                    // Placeholder detected
                    return it;

                default:
                    return end;
            }
        }
    };

} // namespace

//! An internal function to construct the device implementation
shared_ptr< rotating_file_sink::implementation > rotating_file_sink::construct_implementation(
    filesystem::path const& pattern, std::ios_base::openmode mode, uintmax_t rot_size, unsigned int rot_int)
{
    return shared_ptr< implementation >(new file_controller< char >(pattern, mode, rot_size, rot_int));
}

#ifndef BOOST_FILESYSTEM_NARROW_ONLY
//! An internal function to construct the device implementation
shared_ptr< rotating_file_sink::implementation > rotating_file_sink::construct_implementation(
    filesystem::wpath const& pattern, std::ios_base::openmode mode, uintmax_t rot_size, unsigned int rot_int)
{
    return shared_ptr< implementation >(new file_controller< wchar_t >(pattern, mode, rot_size, rot_int));
}
#endif // BOOST_FILESYSTEM_NARROW_ONLY

//! Destructor
rotating_file_sink::~rotating_file_sink()
{
}

//! The method changes the writing position
std::streampos rotating_file_sink::seek(iostreams::stream_offset off, std::ios_base::seekdir way)
{
    return m_pImpl->m_File.rdbuf()->pubseekoff(off, way, std::ios_base::out);
}

//! The method writes data to the device
std::streamsize rotating_file_sink::write(const char* s, std::streamsize n)
{
    m_pImpl->m_Storage.append(s, static_cast< std::string::size_type >(n));
    return n;
}

//! The method flushes the data written to the file
bool rotating_file_sink::flush()
{
    m_pImpl->m_File.flush();
    return true;
}

//! The method closes the file
void rotating_file_sink::close()
{
    m_pImpl->close_file();
}

//! The method delimits records
void rotating_file_sink::flush_record()
{
    // Clear the storage anyway
    struct storage_clearer
    {
        explicit storage_clearer(std::string& s) : m_Storage(s) {}
        ~storage_clearer() { m_Storage.clear(); }

    private:
        std::string& m_Storage;
    } _(m_pImpl->m_Storage);

    // Check whether it's time to rotate the file
    if (m_pImpl->m_File.is_open() &&
        m_pImpl->m_RotationSize < m_pImpl->m_Written + static_cast< uintmax_t >(m_pImpl->m_Storage.size()) ||
        (m_pImpl->m_RotationInterval > 0 && m_pImpl->m_RotationInterval < (std::time(NULL) - m_pImpl->m_LastRotation)))
    {
        m_pImpl->close_file();
    }

    if (!m_pImpl->m_File.is_open())
    {
        m_pImpl->open_file();
        m_pImpl->m_LastRotation = std::time(NULL);
    }

    // Put the record to the file
    register std::string::size_type to_write = m_pImpl->m_Storage.size();
    register const std::streamsize max_block_size = (std::numeric_limits< std::streamsize >::max)();
    std::filebuf* file_buf = m_pImpl->m_File.rdbuf();
    while (to_write > 0)
    {
        register const std::streamsize block_size =
            to_write > static_cast< std::string::size_type >(max_block_size) ? max_block_size : static_cast< std::streamsize >(to_write);
        register const std::streamsize written = file_buf->sputn(&*(m_pImpl->m_Storage.end() - to_write), block_size);
        if (written == 0)
            boost::throw_exception(std::runtime_error("failed to write record to the file"));
        to_write -= written;
        m_pImpl->m_Written += written;
    }
}

//! Sets a new handler for opening a new file
void rotating_file_sink::set_open_handler(open_handler_type const& handler)
{
    m_pImpl->m_OpenHandler = handler;
}

//! Resets the handler for opening a new file
void rotating_file_sink::clear_open_handler()
{
    m_pImpl->m_OpenHandler.clear();
}

//! Sets a new handler for closing a file
void rotating_file_sink::set_close_handler(close_handler_type const& handler)
{
    m_pImpl->m_CloseHandler = handler;
}

//! Resets the handler for closing a file
void rotating_file_sink::clear_close_handler()
{
    m_pImpl->m_CloseHandler.clear();
}


namespace {

    //! Device accessor
    static inline rotating_file_sink& get_sink(rotating_file_sink& device)
    {
        return device;
    }
    template< typename T >
    static inline rotating_file_sink& get_sink(T& device)
    {
        return get_sink(*device);
    }

} // namespace

//! Default constructor
template< typename CharT, typename DeviceT, typename TraitsT >
basic_rotating_ofstreambuf< CharT, DeviceT, TraitsT >::basic_rotating_ofstreambuf() : base_type()
{
}

//! Destructor
template< typename CharT, typename DeviceT, typename TraitsT >
basic_rotating_ofstreambuf< CharT, DeviceT, TraitsT >::~basic_rotating_ofstreambuf()
{
}

//! Sets a new handler for opening a new file
template< typename CharT, typename DeviceT, typename TraitsT >
void basic_rotating_ofstreambuf< CharT, DeviceT, TraitsT >::set_open_handler(open_handler_type const& handler)
{
    get_sink(*this).set_open_handler(handler);
}
//! Resets the handler for opening a new file
template< typename CharT, typename DeviceT, typename TraitsT >
void basic_rotating_ofstreambuf< CharT, DeviceT, TraitsT >::clear_open_handler()
{
    get_sink(*this).clear_open_handler();
}
//! Sets a new handler for closing a file
template< typename CharT, typename DeviceT, typename TraitsT >
void basic_rotating_ofstreambuf< CharT, DeviceT, TraitsT >::set_close_handler(close_handler_type const& handler)
{
    get_sink(*this).set_close_handler(handler);
}
//! Resets the handler for closing a file
template< typename CharT, typename DeviceT, typename TraitsT >
void basic_rotating_ofstreambuf< CharT, DeviceT, TraitsT >::clear_close_handler()
{
    get_sink(*this).clear_close_handler();
}


//! Default constructor
template< typename CharT, typename TraitsT >
basic_rotating_ofstream< CharT, TraitsT >::basic_rotating_ofstream() : stream_base(NULL)
{
    stream_base::init(&m_Buf);
}

//! Destructor
template< typename CharT, typename TraitsT >
basic_rotating_ofstream< CharT, TraitsT >::~basic_rotating_ofstream()
{
}

//! The method closes the file
template< typename CharT, typename TraitsT >
void basic_rotating_ofstream< CharT, TraitsT >::close()
{
    m_Buf.close();
}

//! The method is called after all data of the record is written to the stream
template< typename CharT, typename TraitsT >
void basic_rotating_ofstream< CharT, TraitsT >::on_end_record()
{
    try
    {
        get_sink(m_Buf).flush_record();
    }
    catch (std::exception&)
    {
    }
}

//! Sets a new handler for opening a new file
template< typename CharT, typename TraitsT >
void basic_rotating_ofstream< CharT, TraitsT >::set_open_handler(open_handler_type const& handler)
{
    get_sink(m_Buf).set_open_handler(handler);
}
//! Resets the handler for opening a new file
template< typename CharT, typename TraitsT >
void basic_rotating_ofstream< CharT, TraitsT >::clear_open_handler()
{
    get_sink(m_Buf).clear_open_handler();
}
//! Sets a new handler for closing a file
template< typename CharT, typename TraitsT >
void basic_rotating_ofstream< CharT, TraitsT >::set_close_handler(close_handler_type const& handler)
{
    get_sink(m_Buf).set_close_handler(handler);
}
//! Resets the handler for closing a file
template< typename CharT, typename TraitsT >
void basic_rotating_ofstream< CharT, TraitsT >::clear_close_handler()
{
    get_sink(m_Buf).clear_close_handler();
}


//! Explicitly instantiate implementation
template class basic_rotating_ofstreambuf<
    char,
    rotating_file_sink,
    std::char_traits< char >
>;
template class basic_rotating_ofstreambuf<
    wchar_t,
    iostreams::code_converter< rotating_file_sink >,
    std::char_traits< wchar_t >
>;
template class basic_rotating_ofstream< char, std::char_traits< char > >;
template class basic_rotating_ofstream< wchar_t, std::char_traits< wchar_t > >;

} // namespace sinks

} // namespace log

} // namespace boost
