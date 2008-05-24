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

#include <boost/thread/tss.hpp>
#include <boost/log/sources/basic_logger.hpp>
#include <boost/log/detail/singleton.hpp>

namespace boost {

namespace log {

namespace sources {

namespace aux {

namespace {

//! The pool of stream compounds
template< typename CharT >
class stream_compound_pool :
    public log::aux::lazy_singleton<
        stream_compound_pool< CharT >,
        thread_specific_ptr< stream_compound_pool< CharT > >
    >
{
    //! Singleton base type
    typedef log::aux::lazy_singleton<
        stream_compound_pool< CharT >,
        thread_specific_ptr< stream_compound_pool< CharT > >
    > base_type;
    //! Stream compound type
    typedef typename stream_provider< CharT >::stream_compound stream_compound_t;

public:
    //! Pooled stream compounds
    stream_compound_t* m_Top;

    ~stream_compound_pool()
    {
        register stream_compound_t* p = NULL;
        while ((p = m_Top) != NULL)
        {
            m_Top = p->next;
            delete p;
        }
    }

    //! The method returns pool instance
    static stream_compound_pool& get()
    {
        return *base_type::get();
    }

    static void init_instance()
    {
        base_type::get_instance().reset(new stream_compound_pool< CharT >());
    }

private:
    stream_compound_pool() : m_Top(NULL) {}
};

} // namespace

//! The method returns an allocated stream compound
template< typename CharT >
typename stream_provider< CharT >::stream_compound* stream_provider< CharT >::allocate_compound()
{
    stream_compound_pool< char_type >& pool = stream_compound_pool< char_type >::get();
    if (pool.m_Top)
    {
        register stream_compound* p = pool.m_Top;
        pool.m_Top = p->next;
        p->next = NULL;
        return p;
    }
    else
        return new stream_compound();
}

//! The method releases a compound
template< typename CharT >
void stream_provider< CharT >::release_compound(stream_compound* compound) /* throw() */
{
    stream_compound_pool< char_type >& pool = stream_compound_pool< char_type >::get();
    compound->next = pool.m_Top;
    pool.m_Top = compound;
    compound->message.clear();
    compound->stream.clear();
}

//! Explicitly instantiate stream_provider implementation
#ifdef BOOST_LOG_USE_CHAR
template struct stream_provider< char >;
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
template struct stream_provider< wchar_t >;
#endif

} // namespace aux

} // namespace sources

} // namespace log

} // namespace boost
