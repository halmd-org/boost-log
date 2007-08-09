/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   text_ostream_sink.cpp
 * \author Andrey Semashev
 * \date   19.04.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#include <algorithm>
#include <boost/log/sinks/text_ostream_sink.hpp>
#include <boost/log/formatters/basic_formatters.hpp>

namespace boost {

namespace log {

namespace {

    //! A helper struct that holds line feed char constant
    template< typename >
    struct endl_literal;

    template< >
    struct endl_literal< char >
    {
        static const char value = '\n';
    };
    const char endl_literal< char >::value;

    template< >
    struct endl_literal< wchar_t >
    {
        static const wchar_t value = L'\n';
    };
    const wchar_t endl_literal< wchar_t >::value;

} // namespace

//! Constructor
template< typename CharT >
basic_text_ostream_sink< CharT >::basic_text_ostream_sink() :
    m_StreamBuf(m_FormattedRecord),
    m_FormattingStream(&m_StreamBuf),
    m_Formatter(formatters::fmt_message< char_type >() << endl_literal< char_type >::value)
{
}

//! Destructor (just to make it link from the shared library)
template< typename CharT >
basic_text_ostream_sink< CharT >::~basic_text_ostream_sink() {}


//! The method adds a new stream to the sink
template< typename CharT >
void basic_text_ostream_sink< CharT >::add_stream(shared_ptr< stream_type > const& strm)
{
    scoped_write_lock lock(this->mutex());
    typename ostream_sequence::iterator it = std::find(m_Streams.begin(), m_Streams.end(), strm);
    if (it == m_Streams.end())
        m_Streams.push_back(strm);
}

//! The method removes a stream from the sink
template< typename CharT >
void basic_text_ostream_sink< CharT >::remove_stream(shared_ptr< stream_type > const& strm)
{
    scoped_write_lock lock(this->mutex());
    typename ostream_sequence::iterator it = std::find(m_Streams.begin(), m_Streams.end(), strm);
    if (it != m_Streams.end())
        m_Streams.erase(it);
}

//! The method sets the locale used during formatting
template< typename CharT >
std::locale basic_text_ostream_sink< CharT >::imbue(std::locale const& loc)
{
    return m_FormattingStream.imbue(loc);
}

//! The method returns true if the attribute values pass the filter
template< typename CharT >
bool basic_text_ostream_sink< CharT >::will_write_message(attribute_values_view const& attributes)
{
    scoped_read_lock lock(this->mutex());
    if (m_FormattingStream.good() && !m_Streams.empty())
        return this->will_write_message_unlocked(attributes);
    else
        return false;
}

namespace {

    //! Scope guard to automatically clear the storage
    template< typename T >
    class clear_invoker
    {
        T& m_T;

    public:
        explicit clear_invoker(T& t) : m_T(t) {}
        ~clear_invoker() { m_T.clear(); }
    };

} // namespace

//! The method writes the message to the sink
template< typename CharT >
void basic_text_ostream_sink< CharT >::write_message(
    attribute_values_view const& attributes, string_type const& message)
{
    scoped_write_lock lock(this->mutex());
    clear_invoker< string_type > storage_cleanup(m_FormattedRecord);

    m_Formatter(m_FormattingStream, attributes, message);
    m_FormattingStream.flush();

    typename string_type::const_pointer const p = m_FormattedRecord.data();
    typename string_type::size_type const s = m_FormattedRecord.size();
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

//! Explicitly instantiate sink implementation
template class BOOST_LOG_EXPORT basic_text_ostream_sink< char >;
template class BOOST_LOG_EXPORT basic_text_ostream_sink< wchar_t >;

} // namespace log

} // namespace boost
