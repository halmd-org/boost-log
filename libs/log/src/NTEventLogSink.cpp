#include <algorithm>
#include <boost/log/sinks/NTEventLog_sink.hpp>
#include <boost/log/formatters/basic_formatters.hpp>
#include <windows.h>

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


template< typename CharT >
 NTEventLogSink< CharT >::NTEventLogSink() :
    m_StreamBuf(m_FormattedRecord),
    m_FormattingStream(&m_StreamBuf),
    m_Formatter(formatters::fmt_message< char_type >() << endl_literal< char_type >::value)
{
}

template< typename CharT >
 NTEventLogSink< CharT >::~NTEventLogSink() 
{
	for(size_t i=0;i<m_source_handlers.size();i++)
		DeregisterEventSource(m_source_handlers[i]);
}

 template< typename CharT >
 bool NTEventLogSink< CharT >::add_source(const char* source)
 {
	 HANDLE h;
	 h= RegisterEventSource(NULL,TEXT(source));
	 if(h==NULL)
			return false;

	 m_source_handlers.push_back(h);
	 return true;
 }

//! The method sets the locale used during formatting
template< typename CharT >
std::locale NTEventLogSink< CharT >::imbue(std::locale const& loc)
{
    return m_FormattingStream.imbue(loc);
}

template< typename CharT >
bool NTEventLogSink< CharT >::will_write_message(attribute_values_view const& attributes)
{
    scoped_read_lock lock(this->mutex());
    if (m_FormattingStream.good() && m_source_handlers.size()>0 )
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
void NTEventLogSink< CharT >::write_message(
    attribute_values_view const& attributes, string_type const& message)
{
    scoped_write_lock lock(this->mutex());
    clear_invoker< string_type > storage_cleanup(m_FormattedRecord);

    m_Formatter(m_FormattingStream, attributes, message);
    m_FormattingStream.flush();

    typename string_type::const_pointer const p = m_FormattedRecord.data();
    typename string_type::size_type const s = m_FormattedRecord.size();	

	for(size_t i=0;i<m_source_handlers.size();i++)
	{
		bool retval=ReportEvent(m_source_handlers[i],
					EVENTLOG_SUCCESS, // information event
					0, // No custom category
					0, // No eventID
					NULL, //No sid
					1, //Number of messages in the message array
					0, //No binary data to log
					(LPCTSTR*) &p, //Pointer to string array LPCWSTR if UNICODE is defined LPCSTR otherwhise
					NULL //Pointer to data
					);
		if(retval)
		{
			int l=0;
		}
	}
	/*
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
	*/

	// QUA DEVO SCRIVERE IL DATO A TUTTI GLI EFFETTI SUL LOG

}

//! Explicitly instantiate sink implementation
template class BOOST_LOG_EXPORT  NTEventLogSink< char >;
template class BOOST_LOG_EXPORT  NTEventLogSink< wchar_t >;


} //namespace log

} //namespace boost