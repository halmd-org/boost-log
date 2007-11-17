/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   basic_logger.cpp
 * \author Andrey Semashev
 * \date   19.04.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#include <boost/log/sources/basic_logger.hpp>

namespace boost {

namespace log {

//! Constructor
template< typename CharT >
basic_logger< CharT >::basic_logger() :
    m_pLoggingSystem(logging_core_type::get()),
    m_StreamBuf(m_Message),
    m_Stream(&m_StreamBuf)
{
}

//! Copy constructor
template< typename CharT >
basic_logger< CharT >::basic_logger(basic_logger const& that) :
    m_pLoggingSystem(logging_core_type::get()),
    m_StreamBuf(m_Message),
    m_Stream(&m_StreamBuf),
    m_Attributes(that.m_Attributes)
{
}

//! Destructor (just to make it linking from the shared lib)
template< typename CharT >
basic_logger< CharT >::~basic_logger() {}

//! Assignment
template< typename CharT >
basic_logger< CharT >& basic_logger< CharT >::operator= (basic_logger const& that)
{
    if (this != &that)
        m_Attributes = that.m_Attributes;
    return *this;
}

//! The method adds an attribute to the global attribute set
template< typename CharT >
std::pair< typename basic_logger< CharT >::attribute_set::iterator, bool >
basic_logger< CharT >::add_attribute(string_type const& name, shared_ptr< attribute > const& attr)
{
    return m_Attributes.insert(std::make_pair(name, attr));
}

//! The method removes an attribute from the global attribute set
template< typename CharT >
void basic_logger< CharT >::remove_attribute(typename attribute_set::iterator it)
{
    m_Attributes.erase(it);
}

//! The method removes all attributes from the logger
template< typename CharT >
void basic_logger< CharT >::remove_all_attributes()
{
    m_Attributes.clear();
}

//! The method checks if the message will pass filters to be output by at least one sink
template< typename CharT >
bool basic_logger< CharT >::open_record()
{
    return m_pLoggingSystem->open_record(m_Attributes);
}

//! The method pushes the constructed message to the sinks
template< typename CharT >
void basic_logger< CharT >::push_record()
{
    m_Stream.flush();
    m_pLoggingSystem->push_record(m_Message);
    m_Message.clear();
}

} // namespace log

} // namespace boost

//! Explicitly instantiate logger implementation
template class boost::log::basic_logger< char >;
template class boost::log::basic_logger< wchar_t >;
