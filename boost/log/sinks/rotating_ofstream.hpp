/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   rotating_ofstream.hpp
 * \author Andrey Semashev
 * \date   29.07.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_SINKS_ROTATING_OFSTREAM_HPP_INCLUDED_
#define BOOST_LOG_SINKS_ROTATING_OFSTREAM_HPP_INCLUDED_

#include <ios>
#include <string>
#include <ostream>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/parameter/keyword.hpp>
#include <boost/function/function1.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/positioning.hpp>
#include <boost/iostreams/code_converter.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/sinks/record_writer.hpp>

#ifdef _MSC_VER
#pragma warning(push)
// 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
// non dll-interface struct 'A' used as base for dll-interface class 'B'
#pragma warning(disable: 4275)
#endif // _MSC_VER

namespace boost {

namespace log {

namespace sinks {

namespace keywords {

    BOOST_PARAMETER_KEYWORD(tag, rotation_size)
    BOOST_PARAMETER_KEYWORD(tag, rotation_interval)
    BOOST_PARAMETER_KEYWORD(tag, open_mode)

} // namespace keywords

//! A logical device that implements writing and rotation detection
class BOOST_LOG_EXPORT rotating_file_sink
{
public:
    struct implementation;

    //! Device category
    struct category :
        virtual public iostreams::device_tag,
        virtual public iostreams::flushable_tag,
        virtual public iostreams::closable_tag,
        virtual public iostreams::output_seekable
    {
    };

    //! Character type
    typedef char char_type;

    //! The type of the handler of the event of opening of a new file
    typedef function1< void, std::ostream& > open_handler_type;
    //! The type of the handler of the event of closing of a file
    typedef function1< void, std::ostream& > close_handler_type;

private:
    //! Pointer to the implementation
    shared_ptr< implementation > m_pImpl;

public:

#ifndef BOOST_FILESYSTEM_NARROW_ONLY

#define BOOST_LOG_ROTATING_FILE_SINK_CTOR(z, it, data)\
    template< BOOST_PP_ENUM_PARAMS(it, typename T) >\
    rotating_file_sink(filesystem::path const& pattern, BOOST_PP_ENUM_BINARY_PARAMS(it, T, const& arg))\
        : m_pImpl(construct(pattern, (BOOST_PP_ENUM_PARAMS(it, arg))))\
    {}\
    template< BOOST_PP_ENUM_PARAMS(it, typename T) >\
    rotating_file_sink(filesystem::wpath const& pattern, BOOST_PP_ENUM_BINARY_PARAMS(it, T, const& arg))\
        : m_pImpl(construct(pattern, (BOOST_PP_ENUM_PARAMS(it, arg))))\
    {}

#else

#define BOOST_LOG_ROTATING_FILE_SINK_CTOR(z, it, data)\
    template< BOOST_PP_ENUM_PARAMS(it, typename T) >\
    rotating_file_sink(filesystem::path const& pattern, BOOST_PP_ENUM_BINARY_PARAMS(it, T, const& arg))\
        : m_pImpl(construct(pattern, (BOOST_PP_ENUM_PARAMS(it, arg))))\
    {}

#endif // BOOST_FILESYSTEM_NARROW_ONLY

    BOOST_PP_REPEAT_FROM_TO(1, 4, BOOST_LOG_ROTATING_FILE_SINK_CTOR, ~)

#undef BOOST_LOG_ROTATING_FILE_SINK_CTOR

    //! Destructor
    ~rotating_file_sink();

    //! The method changes the writing position
    std::streampos seek(iostreams::stream_offset off, std::ios_base::seekdir way);
    //! The method writes data to the device
    std::streamsize write(const char* s, std::streamsize n);
    //! The method flushes the data written to the file
    bool flush();
    //! The method closes the file
    void close();

    //! The method delimits records
    void flush_record();

    //! Sets a new handler for opening a new file
    void set_open_handler(open_handler_type const& handler);
    //! Resets the handler for opening a new file
    void clear_open_handler();
    //! Sets a new handler for closing a file
    void set_close_handler(close_handler_type const& handler);
    //! Resets the handler for closing a file
    void clear_close_handler();

private:
    //! An internal function to handle construction args
    template< typename PathT, typename ArgsT >
    static shared_ptr< implementation > construct(PathT const& pattern, ArgsT const& args)
    {
        return construct_implementation(
            pattern,
            args[keywords::open_mode | (std::ios_base::out | std::ios_base::trunc)],
            args[keywords::rotation_size | ~static_cast< uintmax_t >(0)],
            args[keywords::rotation_interval | static_cast< unsigned int >(0)]);
    }

    //! An internal function to construct the device implementation
    static shared_ptr< implementation > construct_implementation(
        filesystem::path const& pattern, std::ios_base::openmode mode, uintmax_t rot_size, unsigned int rot_int);

#ifndef BOOST_FILESYSTEM_NARROW_ONLY
    //! An internal function to construct the device implementation
    static shared_ptr< implementation > construct_implementation(
        filesystem::wpath const& pattern, std::ios_base::openmode mode, uintmax_t rot_size, unsigned int rot_int);
#endif // BOOST_FILESYSTEM_NARROW_ONLY
};

//! A rotating file output stream buffer class
template< typename CharT, typename DeviceT, typename TraitsT = std::char_traits< CharT > >
class basic_rotating_ofstreambuf :
    public iostreams::stream_buffer< DeviceT, TraitsT >
{
    //! Base type
    typedef iostreams::stream_buffer< DeviceT > base_type;

public:
    typedef rotating_file_sink::open_handler_type open_handler_type;
    typedef rotating_file_sink::close_handler_type close_handler_type;

public:
    //! Default constructor
    BOOST_LOG_EXPORT basic_rotating_ofstreambuf();

#ifndef BOOST_FILESYSTEM_NARROW_ONLY

#define BOOST_LOG_ROTATING_OFSTREAMBUF_CTOR(z, it, data)\
    template< BOOST_PP_ENUM_PARAMS(it, typename T) >\
    basic_rotating_ofstreambuf(filesystem::path const& pattern, BOOST_PP_ENUM_BINARY_PARAMS(it, T, const& arg))\
        : base_type(construct(pattern, (BOOST_PP_ENUM_PARAMS(it, arg))))\
    {}\
    template< BOOST_PP_ENUM_PARAMS(it, typename T) >\
    basic_rotating_ofstreambuf(filesystem::wpath const& pattern, BOOST_PP_ENUM_BINARY_PARAMS(it, T, const& arg))\
        : base_type(construct(pattern, (BOOST_PP_ENUM_PARAMS(it, arg))))\
    {}\
    template< BOOST_PP_ENUM_PARAMS(it, typename T) >\
    void open(filesystem::path const& pattern, BOOST_PP_ENUM_BINARY_PARAMS(it, T, const& arg))\
    {\
        base_type::open(construct(pattern, (BOOST_PP_ENUM_PARAMS(it, arg))));\
    }\
    template< BOOST_PP_ENUM_PARAMS(it, typename T) >\
    void open(filesystem::wpath const& pattern, BOOST_PP_ENUM_BINARY_PARAMS(it, T, const& arg))\
    {\
        base_type::open(construct(pattern, (BOOST_PP_ENUM_PARAMS(it, arg))));\
    }

#else

#define BOOST_LOG_ROTATING_OFSTREAMBUF_CTOR(z, it, data)\
    template< BOOST_PP_ENUM_PARAMS(it, typename T) >\
    basic_rotating_ofstreambuf(filesystem::path const& pattern, BOOST_PP_ENUM_BINARY_PARAMS(it, T, const& arg))\
        : base_type(construct(pattern, (BOOST_PP_ENUM_PARAMS(it, arg))))\
    {}\
    template< BOOST_PP_ENUM_PARAMS(it, typename T) >\
    void open(filesystem::path const& pattern, BOOST_PP_ENUM_BINARY_PARAMS(it, T, const& arg))\
    {\
        base_type::open(construct(pattern, (BOOST_PP_ENUM_PARAMS(it, arg))));\
    }

#endif // BOOST_FILESYSTEM_NARROW_ONLY

    BOOST_PP_REPEAT_FROM_TO(1, 4, BOOST_LOG_ROTATING_OFSTREAMBUF_CTOR, ~)

#undef BOOST_LOG_ROTATING_OFSTREAMBUF_CTOR

    //! Destructor
    BOOST_LOG_EXPORT ~basic_rotating_ofstreambuf();

    //! Sets a new handler for opening a new file
    BOOST_LOG_EXPORT void set_open_handler(open_handler_type const& handler);
    //! Resets the handler for opening a new file
    BOOST_LOG_EXPORT void clear_open_handler();
    //! Sets a new handler for closing a file
    BOOST_LOG_EXPORT void set_close_handler(close_handler_type const& handler);
    //! Resets the handler for closing a file
    BOOST_LOG_EXPORT void clear_close_handler();

private:
    //! Device construction helper
    template< typename PathT, typename ArgsT >
    static DeviceT construct(PathT const& pattern, ArgsT const& args)
    {
        return DeviceT(rotating_file_sink(pattern, args));
    }

    //! Copying and assignment are closed
    basic_rotating_ofstreambuf(basic_rotating_ofstreambuf const&);
    basic_rotating_ofstreambuf& operator= (basic_rotating_ofstreambuf const&);
};

//! A helper streambuf type generator to aid metaprogramming
template< typename CharT, typename TraitsT = std::char_traits< CharT > >
struct make_rotating_ofstreambuf;

template< typename TraitsT >
struct make_rotating_ofstreambuf< char, TraitsT >
{
    typedef basic_rotating_ofstreambuf< char, rotating_file_sink, TraitsT > type;
};
template< typename TraitsT >
struct make_rotating_ofstreambuf< wchar_t, TraitsT >
{
    typedef basic_rotating_ofstreambuf< wchar_t, iostreams::code_converter< rotating_file_sink >, TraitsT > type;
};

//! Narrow-char stream buffer type
typedef make_rotating_ofstreambuf< char >::type rotating_ofstreambuf;
//! Wide-char stream buffer type
typedef make_rotating_ofstreambuf< wchar_t >::type rotating_wofstreambuf;

//! A rotating file output stream class
template< typename CharT, typename TraitsT = std::char_traits< CharT > >
class BOOST_LOG_EXPORT basic_rotating_ofstream :
    public record_writer,
    public std::basic_ostream< CharT, TraitsT >
{
    //! Base class of the stream
    typedef std::basic_ostream< CharT, TraitsT > stream_base;

public:
    typedef rotating_file_sink::open_handler_type open_handler_type;
    typedef rotating_file_sink::close_handler_type close_handler_type;

private:
    //! Stream buffer
    typename make_rotating_ofstreambuf< CharT, TraitsT >::type m_Buf;

public:
    //! Default constructor
    basic_rotating_ofstream();

#ifndef BOOST_FILESYSTEM_NARROW_ONLY

#define BOOST_LOG_ROTATING_OFSTREAM_CTOR(z, it, data)\
    template< BOOST_PP_ENUM_PARAMS(it, typename T) >\
    basic_rotating_ofstream(filesystem::path const& pattern, BOOST_PP_ENUM_BINARY_PARAMS(it, T, const& arg))\
        : stream_base(NULL), m_Buf(pattern, BOOST_PP_ENUM_PARAMS(it, arg))\
    { stream_base::init(&m_Buf); }\
    template< BOOST_PP_ENUM_PARAMS(it, typename T) >\
    basic_rotating_ofstream(filesystem::wpath const& pattern, BOOST_PP_ENUM_BINARY_PARAMS(it, T, const& arg))\
        : stream_base(NULL), m_Buf(pattern, BOOST_PP_ENUM_PARAMS(it, arg))\
    { stream_base::init(&m_Buf); }\
    template< BOOST_PP_ENUM_PARAMS(it, typename T) >\
    void open(filesystem::path const& pattern, BOOST_PP_ENUM_BINARY_PARAMS(it, T, const& arg))\
    {\
        m_Buf.open(pattern, BOOST_PP_ENUM_PARAMS(it, arg));\
    }\
    template< BOOST_PP_ENUM_PARAMS(it, typename T) >\
    void open(filesystem::wpath const& pattern, BOOST_PP_ENUM_BINARY_PARAMS(it, T, const& arg))\
    {\
        m_Buf.open(pattern, BOOST_PP_ENUM_PARAMS(it, arg));\
    }

#else

#define BOOST_LOG_ROTATING_OFSTREAM_CTOR(z, it, data)\
    template< BOOST_PP_ENUM_PARAMS(it, typename T) >\
    basic_rotating_ofstreambuf(filesystem::path const& pattern, BOOST_PP_ENUM_BINARY_PARAMS(it, T, const& arg))\
        : stream_base(NULL), m_Buf(pattern, BOOST_PP_ENUM_PARAMS(it, arg))\
    { stream_base::init(&m_Buf); }\
    template< BOOST_PP_ENUM_PARAMS(it, typename T) >\
    void open(filesystem::path const& pattern, BOOST_PP_ENUM_BINARY_PARAMS(it, T, const& arg))\
    {\
        m_Buf.open(pattern, BOOST_PP_ENUM_PARAMS(it, arg));\
    }

#endif // BOOST_FILESYSTEM_NARROW_ONLY

    BOOST_PP_REPEAT_FROM_TO(1, 4, BOOST_LOG_ROTATING_OFSTREAM_CTOR, ~)

#undef BOOST_LOG_ROTATING_OFSTREAM_CTOR

    //! Destructor
    ~basic_rotating_ofstream();

    //! The method closes the file
    void close();
    
    //! The method is called after all data of the record is written to the stream
    void on_end_record();

    //! Sets a new handler for opening a new file
    void set_open_handler(open_handler_type const& handler);
    //! Resets the handler for opening a new file
    void clear_open_handler();
    //! Sets a new handler for closing a file
    void set_close_handler(close_handler_type const& handler);
    //! Resets the handler for closing a file
    void clear_close_handler();

private:
    //! Copying and assignment are closed
    basic_rotating_ofstream(basic_rotating_ofstream const&);
    basic_rotating_ofstream& operator= (basic_rotating_ofstream const&);
};

//! Narrow-char stream type
typedef basic_rotating_ofstream< char > rotating_ofstream;
//! Wide-char stream type
typedef basic_rotating_ofstream< wchar_t > rotating_wofstream;

} // namespace sinks

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_SINKS_ROTATING_OFSTREAM_HPP_INCLUDED_
