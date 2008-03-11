/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   date_time_formatter.cpp
 * \author Andrey Semashev
 * \date   11.03.2008
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#include <memory>
#include <locale>
#include <boost/date_time/posix_time/conversion.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/log/formatters/date_time.hpp>

namespace boost {

namespace log {

namespace formatters {

namespace aux {

//! Default constructor
template< typename CharT >
inline date_time_formatter_base< CharT >::date_time_formatter_base()
    : m_StreamBuf(m_Buffer), m_Stream(&m_StreamBuf)
{
}

//! Constructor with format setup
template< typename CharT >
inline date_time_formatter_base< CharT >::date_time_formatter_base(string_type const& fmt)
    : m_StreamBuf(m_Buffer), m_Stream(&m_StreamBuf)
{
    typedef boost::date_time::time_facet< posix_time::ptime, char_type > facet_t;
    std::auto_ptr< facet_t > facet(new facet_t(fmt.c_str()));
    this->m_Stream.imbue(std::locale(this->m_Stream.getloc(), facet.get()));
    facet.release();
}


//! Default constructor
template< typename CharT >
basic_date_time_formatter< CharT >::basic_date_time_formatter()
{
}

//! Constructor with format setup
template< typename CharT >
basic_date_time_formatter< CharT >::basic_date_time_formatter(string_type const& fmt) : base_type(fmt)
{
}

//! Formatting method for time_t
template< typename CharT >
void basic_date_time_formatter< CharT >::operator()(std::time_t value)
{
    (*this)(posix_time::from_time_t(value));
}

//! Formatting method for tm
template< typename CharT >
void basic_date_time_formatter< CharT >::operator()(std::tm const& value)
{
    (*this)(posix_time::ptime_from_tm(value));
}

//! Formatting method for DATE type on Windows
template< typename CharT >
void basic_date_time_formatter< CharT >::operator()(double value)
{
}

//! Formatting method for Boost.DateTime POSIX time object
template< typename CharT >
void basic_date_time_formatter< CharT >::operator()(boost::posix_time::ptime const& value)
{
    this->m_Stream << value;
    this->m_Stream.flush();
}


//! Default constructor
template< typename CharT >
basic_time_period_formatter< CharT >::basic_time_period_formatter()
{
}

//! Constructor with format setup
template< typename CharT >
basic_time_period_formatter< CharT >::basic_time_period_formatter(string_type const& fmt) : base_type(fmt)
{
}

//! Formatting method for Boost.DateTime POSIX time period object
template< typename CharT >
void basic_time_period_formatter< CharT >::operator()(posix_time::time_period const& value)
{
    this->m_Stream << value;
    this->m_Stream.flush();
}

//  Explicitly instantiate exported templates
template class basic_date_time_formatter< char >;
template class basic_date_time_formatter< wchar_t >;
template class basic_time_period_formatter< char >;
template class basic_time_period_formatter< wchar_t >;

} // namespace aux

} // namespace formatters

} // namespace log

} // namespace boost
