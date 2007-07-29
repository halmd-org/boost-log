/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   attachable_sstream_buf.hpp
 * \author Andrey Semashev
 * \date   29.07.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_ATTACHABLE_SSTREAM_BUF_HPP_INCLUDED_
#define BOOST_LOG_ATTACHABLE_SSTREAM_BUF_HPP_INCLUDED_

#include <string>
#include <streambuf>
#include <boost/log/detail/prologue.hpp>

#ifndef BOOST_LOG_DEFAULT_ATTACHABLE_SSTREAM_BUF_SIZE
//! The size (in chars) of a stream buffer used by logger. It affects logger object size.
//! \note The Boost.Log library should be rebuilt once this value is modified.
#define BOOST_LOG_DEFAULT_ATTACHABLE_SSTREAM_BUF_SIZE 16
#endif // BOOST_LOG_DEFAULT_ATTACHABLE_SSTREAM_BUF_SIZE

#ifdef _MSC_VER
#pragma warning(push)
// 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
#endif // _MSC_VER

namespace boost {

namespace log {

namespace aux {

    //! An output streambuf
    template< typename CharT >
    class BOOST_LOG_EXPORT basic_ostringstreambuf :
        public std::basic_streambuf< CharT >
    {
        //! Base type
        typedef std::basic_streambuf< CharT > base_type;

    public:
        //! Character type
        typedef typename base_type::char_type char_type;
        //! String type
        typedef std::basic_string< char_type > string_type;
        //! Traits type
        typedef typename base_type::traits_type traits_type;
        //! Int type
        typedef typename base_type::int_type int_type;

    private:
        //! A reference to the string that will be filled
        string_type& m_Storage;
        //! A buffer used to temporarily store output
        char_type m_Buffer[BOOST_LOG_DEFAULT_ATTACHABLE_SSTREAM_BUF_SIZE];

    private:
        //! Copy constructor (closed)
        basic_ostringstreambuf(basic_ostringstreambuf const& that);
        //! Assignment (closed)
        basic_ostringstreambuf& operator= (basic_ostringstreambuf const& that);

    public:
        //! Constructor
        explicit basic_ostringstreambuf(string_type& storage);
        //! Destructor
        ~basic_ostringstreambuf();

    protected:
        //! Puts all buffered data to the string
        int sync();
        //! Puts an unbuffered character to the string
        int_type overflow(int_type c);
        //! Puts a character sequence to the string
        std::streamsize xsputn(const char_type* s, std::streamsize n);
    };

} // namespace aux

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_ATTACHABLE_SSTREAM_BUF_HPP_INCLUDED_
