/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   ostream_aggregate.hpp
 * \author Andrey Semashev
 * \date   01.05.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_OSTREAM_AGGREGATE_HPP_INCLUDED_
#define BOOST_LOG_OSTREAM_AGGREGATE_HPP_INCLUDED_

#include <ostream>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/log/detail/prologue.hpp>

namespace boost {

namespace log {

namespace aux {

    //! A simple output stream aggregate class
    template<
        typename CharT,
        typename TraitsT = std::char_traits< CharT >,
        typename AllocatorT = std::allocator< shared_ptr< std::basic_ostream< CharT, TraitsT > > >
    >
    class basic_ostream_aggregate
    {
    public:
        //! Char type
        typedef CharT char_type;
        //! Char traits
        typedef TraitsT traits_type;
        typedef typename traits_type::int_type int_type;
        typedef typename traits_type::pos_type pos_type;
        typedef typename traits_type::off_type off_type;
        //! Output streams type
        typedef std::basic_ostream< char_type, traits_type > ostream_type;

        //! Type of the container that holds all aggregated streams
        typedef std::vector< shared_ptr< ostream_type >, AllocatorT > ostream_sequence;

    private:
        //! Aggregated output streams
        ostream_sequence m_Streams;

    public:
        //! The method adds a new stream to the container
        bool add_stream(shared_ptr< ostream_type > const& strm)
        {
            typename ostream_sequence::iterator it = std::find(m_Streams.begin(), m_Streams.end(), strm);
            if (it == m_Streams.end())
            {
                m_Streams.push_back(strm);
                return true;
            }
            else
                return false;
        }
        //! The method removes the stream from the container
        bool remove_stream(shared_ptr< ostream_type > const& strm)
        {
            typename ostream_sequence::iterator it = std::find(m_Streams.begin(), m_Streams.end(), strm);
            if (it != m_Streams.end())
            {
                m_Streams.erase(it);
                return true;
            }
            else
                return false;
        }

        //! The method returns true if there are no streams aggregated and false otherwise
        bool empty() const { return m_Streams.empty(); }

        //! An output operator
        template< typename T >
        basic_ostream_aggregate& operator<< (T const& obj) const
        {
            typename ostream_sequence::const_iterator it = m_Streams.begin();
            for (; it != m_Streams.end(); ++it)
            {
                try
                {
                    (**it) << obj;
                }
                catch (std::exception&)
                {
                }
            }

            return *const_cast< basic_ostream_aggregate* >(this);
        }
    };

} // namespace aux

} // namespace log

} // namespace boost

#endif // BOOST_LOG_OSTREAM_AGGREGATE_HPP_INCLUDED_
