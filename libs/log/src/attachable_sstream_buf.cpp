/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   attachable_sstream_buf.cpp
 * \author Andrey Semashev
 * \date   19.04.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#include <boost/log/detail/attachable_sstream_buf.hpp>

namespace boost {

namespace log {

namespace aux {

//! Constructor
template< typename CharT >
basic_ostringstreambuf< CharT >::basic_ostringstreambuf(string_type& storage) : m_Storage(storage)
{
    base_type::setp(m_Buffer, m_Buffer + (sizeof(m_Buffer) / sizeof(*m_Buffer)));
}

//! Destructor
template< typename CharT >
basic_ostringstreambuf< CharT >::~basic_ostringstreambuf() {}


//! Puts all buffered data to the string
template< typename CharT >
int basic_ostringstreambuf< CharT >::sync()
{
    register char_type* pBase = this->pbase();
    register char_type* pPtr = this->pptr();
    if (pBase != pPtr)
    {
        m_Storage.append(pBase, pPtr);
        this->pbump(static_cast< int >(pBase - pPtr));
    }
    return 0;
}

//! Puts an unbuffered character to the string
template< typename CharT >
typename basic_ostringstreambuf< CharT >::int_type basic_ostringstreambuf< CharT >::overflow(int_type c)
{
    basic_ostringstreambuf::sync();
    if (!traits_type::eq_int_type(c, traits_type::eof()))
    {
        m_Storage.push_back(traits_type::to_char_type(c));
        return c;
    }
    else
        return traits_type::not_eof(c);
}

//! Puts a character sequence to the string
template< typename CharT >
std::streamsize basic_ostringstreambuf< CharT >::xsputn(const char_type* s, std::streamsize n)
{
    basic_ostringstreambuf::sync();
    typedef typename string_type::size_type string_size_type;
    register const string_size_type max_storage_left =
        m_Storage.max_size() - m_Storage.size();
    if (static_cast< string_size_type >(n) < max_storage_left)
    {
        m_Storage.append(s, static_cast< string_size_type >(n));
        return n;
    }
    else
    {
        m_Storage.append(s, max_storage_left);
        return static_cast< std::streamsize >(max_storage_left);
    }
}

//! Explicitly instantiate implementation
template class BOOST_LOG_EXPORT basic_ostringstreambuf< char >;
template class BOOST_LOG_EXPORT basic_ostringstreambuf< wchar_t >;

} // namespace aux

} // namespace log

} // namespace boost
