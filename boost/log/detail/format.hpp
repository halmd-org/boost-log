/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   format.hpp
 * \author Andrey Semashev
 * \date   15.11.2012
 *
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#ifndef BOOST_LOG_DETAIL_FORMAT_HPP_INCLUDED_
#define BOOST_LOG_DETAIL_FORMAT_HPP_INCLUDED_

#include <memory>
#include <string>
#include <vector>
#include <iosfwd>
#include <boost/assert.hpp>
#include <boost/move/move.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/detail/unhandled_exception_count.hpp>
#include <boost/log/detail/cleanup_scope_guard.hpp>
#include <boost/log/utility/formatting_stream.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace aux {

struct format_element
{
    //! Argument placeholder number or -1 if it's not a placeholder (i.e. a literal)
    int arg_number;
    //! If the element describes a constant literal, the starting character and length of the literal
    unsigned int literal_start_pos, literal_len;

    format_element() : arg_number(0), literal_start_pos(0), literal_len(0)
    {
    }
};

template< typename CharT >
struct format_description
{
    //! Character type
    typedef CharT char_type;
    //! String type
    typedef std::basic_string< char_type > string_type;

    //! Array of format element descriptors
    typedef std::vector< format_element > format_element_list;

    //! Characters of all literal parts of the format string
    string_type literal_chars;
    //! Format element descriptors
    format_element_list format_elements;
};

template< typename CharT >
BOOST_LOG_API format_description< CharT > parse_format(const CharT* begin, const CharT* end);

template< typename CharT >
BOOST_LOG_FORCEINLINE format_description< CharT > parse_format(const CharT* begin)
{
    return parse_format(begin, begin + std::char_traits< CharT >::length(begin));
}

template< typename CharT, typename TraitsT, typename AllocatorT >
BOOST_LOG_FORCEINLINE format_description< CharT > parse_format(std::basic_string< CharT, TraitsT, AllocatorT > const& fmt)
{
    const CharT* begin = fmt.c_str();
    return parse_format(begin, begin + fmt.size());
}

template< typename CharT >
class basic_format
{
public:
    //! Character type
    typedef CharT char_type;
    //! String type
    typedef std::basic_string< char_type > string_type;
    //! Stream type
    typedef basic_formatting_ostream< char_type > stream_type;
    //! Format description type
    typedef format_description< char_type > format_description_type;

    class pump;
    friend class pump;

private:
    struct formatting_params
    {
        //! Formatting element index in the format description
        unsigned int element_idx;
        //! Formatting result
        string_type target;

        formatting_params() : element_idx(~0u) {}
    };
    typedef std::vector< formatting_params > formatting_params_list;

private:
    //! Format string description
    format_description_type m_format;
    //! Formatting parameters for all placeholders
    formatting_params_list m_formatting_params;
    //! Current formatting position
    unsigned int m_current_idx;

public:
    //! Initializing constructor
    explicit basic_format(string_type const& fmt) : m_format(aux::parse_format(fmt)), m_current_idx(0)
    {
        init_params();
    }
    //! Initializing constructor
    explicit basic_format(const char_type* fmt) : m_format(aux::parse_format(fmt)), m_current_idx(0)
    {
        init_params();
    }

    //! Clears all formatted strings and resets the current formatting position
    void clear()
    {
        for (typename formatting_params_list::iterator it = m_formatting_params.begin(), end = m_formatting_params.end(); it != end; ++it)
        {
            it->target.clear();
        }
        m_current_idx = 0;
    }

    //! Creates a pump that will receive all format arguments and put the formatted string into the stream
    pump make_pump(stream_type& strm)
    {
        return pump(*this, strm);
    }

    //! Composes the final string from the formatted pieces
    void compose(string_type& str) const
    {
        typename format_description_type::format_element_list::const_iterator it = m_format.format_elements.begin(), end = m_format.format_elements.end();
        for (; it != end; ++it)
        {
            if (it->arg_number >= 0)
            {
                // This is a placeholder
                str.append(m_formatting_params[it->arg_number].target);
            }
            else
            {
                // This is a literal
                const char_type* p = m_format.literal_chars.c_str() + it->literal_start_pos;
                str.append(p, it->literal_len);
            }
        }
    }

    //! Composes the final string from the formatted pieces
    string_type str() const
    {
        string_type result;
        compose(result);
        return boost::move(result);
    }

private:
    //! Initializes the formatting params
    void init_params()
    {
        typename format_description_type::format_element_list::const_iterator it = m_format.format_elements.begin(), end = m_format.format_elements.end();
        for (; it != end; ++it)
        {
            if (it->arg_number >= 0)
            {
                if (it->arg_number >= m_formatting_params.size())
                    m_formatting_params.resize(it->arg_number + 1);
                m_formatting_params[it->arg_number].element_idx = it - m_format.format_elements.begin();
            }
        }
    }
};

template< typename CharT >
class basic_format< CharT >::pump
{
    BOOST_MOVABLE_BUT_NOT_COPYABLE(pump)

private:
    struct scoped_storage
    {
        scoped_storage(stream_type& strm, string_type& storage) : m_stream(strm), m_storage_backup(*strm.rdbuf()->storage())
        {
            strm.attach(storage);
        }
        ~scoped_storage()
        {
            strm.attach(m_storage_backup);
        }

    private:
        stream_type& m_stream;
        string_type& m_storage_backup;
    };

private:
    //! Reference to the owner
    basic_format* m_owner;
    //! Reference to the stream
    stream_type* m_stream;
    //! Unhandled exception count
    const unsigned int m_exception_count;

public:
    pump(basic_format& owner, stream_type& strm) BOOST_NOEXCEPT : m_owner(&owner), m_stream(&strm), m_exception_count(unhandled_exception_count())
    {
    }

    pump(BOOST_RV_REF(pump) that) BOOST_NOEXCEPT : m_owner(that.m_owner), m_stream(that.m_stream), m_exception_count(that.m_exception_count)
    {
        that.m_owner = NULL;
        that.m_stream = NULL;
    }

    ~pump()
    {
        if (m_owner)
        {
            boost::log::aux::cleanup_guard< basic_format< char_type > > cleanup1(*m_owner);

            if (m_stream && m_exception_count >= unhandled_exception_count())
            {
                m_stream->flush();
                m_owner->compose(*m_stream->rdbuf()->storage());
            }
        }
    }

    template< typename T >
    pump& operator% (T const& val)
    {
        BOOST_ASSERT_MSG(m_owner != NULL && m_stream != NULL, "Boost.Log: This basic_format::pump has already been moved from");

        if (m_owner->m_current_idx < m_owner->m_formatting_params.size())
        {
            scoped_storage storage_guard(*m_stream, m_owner->m_formatting_params[m_owner->m_current_idx].target);

            *m_stream << val;
            m_stream->flush();

            ++m_owner->m_current_idx;
        }

        return *this;
    }

    // This operator is used if the formatter is output to the stream
    friend stream_type& operator<< (stream_type& strm, pump p)
    {
        // The output will be done in the pump destructor, nothing to be done here
        p.m_stream = &strm;
        return strm;
    }
};

} // namespace aux

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_DETAIL_SNPRINTF_HPP_INCLUDED_
