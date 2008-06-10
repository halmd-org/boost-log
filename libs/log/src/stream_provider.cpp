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

#include <memory>
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
    //! Self type
    typedef stream_compound_pool< CharT > this_type;
    //! Thread-specific pointer type
    typedef thread_specific_ptr< this_type > tls_ptr_type;
    //! Singleton base type
    typedef log::aux::lazy_singleton<
        this_type,
        tls_ptr_type
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
        tls_ptr_type& ptr = base_type::get();
        if (!ptr.get())
        {
            std::auto_ptr< this_type > pNew(new this_type());
            ptr.reset(pNew.get());
            pNew.release();
        }
        return *ptr;
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
