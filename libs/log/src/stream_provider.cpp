/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   stream_provider.cpp
 * \author Andrey Semashev
 * \date   17.04.2008
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#include <exception>
#include <stack>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/thread/tss.hpp>
#include <boost/thread/once.hpp>
#include <boost/log/sources/basic_logger.hpp>

namespace boost {

namespace log {

namespace sources {

namespace aux {

namespace {

//! The pool of stream compounds
template< typename CharT >
struct stream_compound_pool : noncopyable
{
    //! Stream compound type
    typedef typename stream_provider< CharT >::stream_compound stream_compound_t;

    //! Stream compounds pool
    std::stack< stream_compound_t*, std::vector< stream_compound_t* > > m_Pool;

    ~stream_compound_pool()
    {
        while (!m_Pool.empty())
        {
            delete m_Pool.top();
            m_Pool.pop();
        }
    }

    //! The method returns pool instance
    static stream_compound_pool& get()
    {
        static once_flag flag = BOOST_ONCE_INIT;
        call_once(flag, &stream_compound_pool< CharT >::init_instance);
        return *get_instance();
    }

private:
    stream_compound_pool() {}

    static void init_instance()
    {
        get_instance().reset(new stream_compound_pool< CharT >());
    }
    static thread_specific_ptr< stream_compound_pool >& get_instance()
    {
        static thread_specific_ptr< stream_compound_pool > instance;
        return instance;
    }
};

} // namespace

//! The method returns an allocated stream compound
template< typename CharT >
typename stream_provider< CharT >::stream_compound* stream_provider< CharT >::allocate_compound()
{
    stream_compound_pool< char_type >& pool = stream_compound_pool< char_type >::get();
    if (!pool.m_Pool.empty())
    {
        register stream_compound* p = pool.m_Pool.top();
        pool.m_Pool.pop();
        return p;
    }
    else
        return new stream_compound();
}

//! The method releases a compound
template< typename CharT >
void stream_provider< CharT >::release_compound(stream_compound* compound) /* throw() */
{
    try
    {
        stream_compound_pool< char_type >& pool = stream_compound_pool< char_type >::get();
        pool.m_Pool.push(compound);
        compound->message.clear();
        compound->stream.clear();
    }
    catch (std::exception&)
    {
        // Just in case if stack allocation failed
        delete compound;
    }
}

//! Explicitly instantiate stream_provider implementation
template struct stream_provider< char >;
template struct stream_provider< wchar_t >;

} // namespace aux

} // namespace sources

} // namespace log

} // namespace boost
