/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   text_ostream_backend.cpp
 * \author Andrey Semashev
 * \date   19.04.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#include <algorithm>
#include <boost/log/sinks/text_ostream_backend.hpp>

namespace boost {

namespace log {

namespace sinks {

//! Constructor
template< typename CharT >
basic_text_ostream_backend< CharT >::basic_text_ostream_backend()
{
}

//! Destructor (just to make it link from the shared library)
template< typename CharT >
basic_text_ostream_backend< CharT >::~basic_text_ostream_backend() {}

//! The method adds a new stream to the sink
template< typename CharT >
void basic_text_ostream_backend< CharT >::add_stream(shared_ptr< stream_type > const& strm)
{
    typename ostream_sequence::iterator it = std::find(m_Streams.begin(), m_Streams.end(), strm);
    if (it == m_Streams.end())
        m_Streams.push_back(strm);
}

//! The method removes a stream from the sink
template< typename CharT >
void basic_text_ostream_backend< CharT >::remove_stream(shared_ptr< stream_type > const& strm)
{
    typename ostream_sequence::iterator it = std::find(m_Streams.begin(), m_Streams.end(), strm);
    if (it != m_Streams.end())
        m_Streams.erase(it);
}

//! The method writes the message to the sink
template< typename CharT >
void basic_text_ostream_backend< CharT >::do_write_message(
    string_type const& message, attribute_values_view const& attributes)
{
    typename string_type::const_pointer const p = message.data();
    typename string_type::size_type const s = message.size();
    typename ostream_sequence::const_iterator it = m_Streams.begin();
    for (; it != m_Streams.end(); ++it)
    {
        register stream_type* const strm = it->get();
        if (strm->good()) try
        {
            strm->write(p, static_cast< std::streamsize >(s));
        }
        catch (std::exception&)
        {
        }
    }
}

//! Explicitly instantiate sink backend implementation
template class BOOST_LOG_EXPORT basic_text_ostream_backend< char >;
template class BOOST_LOG_EXPORT basic_text_ostream_backend< wchar_t >;

} // namespace sinks

} // namespace log

} // namespace boost
