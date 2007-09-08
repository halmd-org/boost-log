/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   slim_string.hpp
 * \author Andrey Semashev
 * \date   08.09.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if defined(_MSC_VER) && _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_DETAIL_SLIM_STRING_HPP_INCLUDED_
#define BOOST_LOG_DETAIL_SLIM_STRING_HPP_INCLUDED_

#include <cstddef>
#include <iosfwd>
#include <string>
#include <boost/operators.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/log/detail/prologue.hpp>

namespace boost {

namespace log {

namespace aux {

//! A slim string class with shared data and read-only access
template<
    typename CharT,
    typename TraitsT = typename std::basic_string< CharT >::traits_type,
    typename AllocatorT = typename std::basic_string< CharT >::allocator_type
>
class basic_slim_string :
    public totally_ordered1< basic_slim_string< CharT, TraitsT, AllocatorT >,
        totally_ordered2< basic_slim_string< CharT, TraitsT, AllocatorT >, const CharT*,
            totally_ordered2<
                basic_slim_string< CharT, TraitsT, AllocatorT >,
                std::basic_string< CharT, TraitsT, AllocatorT >
            >
        >
    >
{
public:
    //  Non-standard typedefs
    typedef CharT char_type;
    typedef TraitsT traits_type;
    typedef AllocatorT allocator_type;
    typedef std::basic_string< char_type, traits_type, allocator_type > string_type;

    //  Standard typedefs
    typedef typename string_type::value_type value_type;
    typedef typename string_type::pointer pointer;
    typedef typename string_type::const_pointer const_pointer;
    typedef typename string_type::reference reference;
    typedef typename string_type::const_reference const_reference;
    typedef typename string_type::size_type size_type;
    typedef typename string_type::difference_type difference_type;
    typedef typename string_type::iterator iterator;
    typedef typename string_type::const_iterator const_iterator;
    typedef typename string_type::reverse_iterator reverse_iterator;
    typedef typename string_type::const_reverse_iterator const_reverse_iterator;

    enum { npos = string_type::npos };

private:
    //! Pointer to the actual string (we might move to a shared_array of chars in the future to avoid one allocation)
    shared_ptr< const string_type > m_pString;

public:
    //! Default constructor
    basic_slim_string() : m_pString(new string_type) {}
    //! Copy constructor
    basic_slim_string(basic_slim_string const& that) : m_pString(that.m_pString) {}

    //  Standard constructors
    explicit basic_slim_string(string_type const& that) : m_pString(new string_type(that)) {}
    explicit basic_slim_string(allocator_type const& a) : m_pString(new string_type(a)) {}
    basic_slim_string(string_type const& s, size_type pos, size_type n = npos, allocator_type const& a = allocator_type())
        : m_pString(new string_type(s, pos, n, a)) {}
    basic_slim_string(basic_slim_string const& that, size_type pos, size_type n = npos, allocator_type const& a = allocator_type())
        : m_pString(new string_type(*that.m_pString, pos, n, a)) {}
    basic_slim_string(const_pointer s, allocator_type const& a = allocator_type())
        : m_pString(new string_type(s, a)) {}
    basic_slim_string(const_pointer s, size_type n, allocator_type const& a = allocator_type())
        : m_pString(new string_type(s, n, a)) {}
    basic_slim_string(size_type n, char_type c, allocator_type const& a = allocator_type())
        : m_pString(new string_type(n, c, a)) {}
    template< typename IteratorT >
    basic_slim_string(IteratorT b, IteratorT e, allocator_type const& a = allocator_type())
        : m_pString(new string_type(b, e, a)) {}

    //  Assignment
    basic_slim_string& operator= (basic_slim_string const& that)
    {
        m_pString = that.m_pString;
        return *this;
    }
    basic_slim_string& operator= (string_type const& that)
    {
        m_pString.reset(new string_type(that));
        return *this;
    }
    basic_slim_string& operator= (const_pointer that)
    {
        m_pString.reset(new string_type(that));
        return *this;
    }
    basic_slim_string& operator= (char_type that)
    {
        m_pString.reset(new string_type(1, that));
        return *this;
    }

    //! Indexing
    const_reference operator[] (size_type n) const { return m_pString->operator[](n); }

    //  Equality comparison
    bool operator== (basic_slim_string const& that) const { return (m_pString == that.m_pString || *m_pString == *that.m_pString); }
    bool operator== (const_pointer s) const { return (*m_pString == s); }
    bool operator== (string_type const& s) const { return (*m_pString == s); }

    //  Less ordering
    bool operator< (basic_slim_string const& that) const { return (m_pString != that.m_pString && *m_pString < *that.m_pString); }
    bool operator< (const_pointer s) const { return (*m_pString < s); }
    bool operator< (string_type const& s) const { return (*m_pString < s); }

    //  Greater ordering
    bool operator> (basic_slim_string const& that) const { return (m_pString != that.m_pString && *m_pString > *that.m_pString); }
    bool operator> (const_pointer s) const { return (*m_pString > s); }
    bool operator> (string_type const& s) const { return (*m_pString > s); }

    //! Output
    friend std::basic_ostream< char_type, traits_type >& operator<< (std::basic_ostream< char_type, traits_type >& strm, basic_slim_string const& s)
    {
        strm << *s.m_pString;
        return strm;
    }
    //! Input
    friend std::basic_istream< char_type, traits_type >& operator>> (std::basic_istream< char_type, traits_type >& strm, basic_slim_string& s)
    {
        strm >> *const_cast< string_type* >(s.m_pString.get());
        return strm;
    }

    //  Forwarding methods
    const_reference at(size_type n) const { return m_pString->at(n); }
    const_pointer data() const { return m_pString->data(); }
    const_pointer c_str() const { return m_pString->c_str(); }
    size_type size() const { return m_pString->size(); }
    size_type length() const { return m_pString->length(); }
    bool empty() const { return m_pString->empty(); }
    size_type capacity() const { return m_pString->capacity(); }
    size_type max_size() const { return m_pString->max_size(); }
    allocator_type get_allocator() const { return m_pString->get_allocator(); }

    const_iterator begin() const { return m_pString->begin(); }
    const_iterator end() const { return m_pString->end(); }
    const_reverse_iterator rbegin() const { return m_pString->rbegin(); }
    const_reverse_iterator rend() const { return m_pString->rend(); }

    size_type copy(pointer s, size_type n, size_type pos = 0) const { return m_pString->copy(s, n, pos); }
    basic_slim_string substr(size_type pos = 0, size_type n = npos) const
    {
        return basic_slim_string(m_pString->substr(pos, n));
    }

    void swap(basic_slim_string& that) { m_pString.swap(that.m_pString); }

    size_type find(basic_slim_string const& that, size_type pos = 0) const { return m_pString->find(*that.m_pString, pos); }
    size_type find(string_type const& s, size_type pos = 0) const { return m_pString->find(s, pos); }
    size_type find(const_pointer s, size_type pos = 0) const { return m_pString->find(s, pos); }
    size_type find(const_pointer s, size_type pos, size_type n) const { return m_pString->find(s, pos, n); }
    size_type find(char_type c, size_type pos = 0) const { return m_pString->find(c, pos); }

    size_type rfind(basic_slim_string const& that, size_type pos = npos) const { return m_pString->rfind(*that.m_pString, pos); }
    size_type rfind(string_type const& s, size_type pos = npos) const { return m_pString->rfind(s, pos); }
    size_type rfind(const_pointer s, size_type pos = npos) const { return m_pString->rfind(s, pos); }
    size_type rfind(const_pointer s, size_type pos, size_type n) const { return m_pString->rfind(s, pos, n); }
    size_type rfind(char_type c, size_type pos = npos) const { return m_pString->rfind(c, pos); }

    size_type find_first_of(basic_slim_string const& that, size_type pos = 0) const { return m_pString->find_first_of(*that.m_pString, pos); }
    size_type find_first_of(string_type const& s, size_type pos = 0) const { return m_pString->find_first_of(s, pos); }
    size_type find_first_of(const_pointer s, size_type pos = 0) const { return m_pString->find_first_of(s, pos); }
    size_type find_first_of(const_pointer s, size_type pos, size_type n) const { return m_pString->find_first_of(s, pos, n); }
    size_type find_first_of(char_type c, size_type pos = 0) const { return m_pString->find_first_of(c, pos); }

    size_type find_last_of(basic_slim_string const& that, size_type pos = npos) const { return m_pString->find_last_of(*that.m_pString, pos); }
    size_type find_last_of(string_type const& s, size_type pos = npos) const { return m_pString->find_last_of(s, pos); }
    size_type find_last_of(const_pointer s, size_type pos = npos) const { return m_pString->find_last_of(s, pos); }
    size_type find_last_of(const_pointer s, size_type pos, size_type n) const { return m_pString->find_last_of(s, pos, n); }
    size_type find_last_of(char_type c, size_type pos = npos) const { return m_pString->find_last_of(c, pos); }

    size_type find_first_not_of(basic_slim_string const& that, size_type pos = 0) const { return m_pString->find_first_not_of(*that.m_pString, pos); }
    size_type find_first_not_of(string_type const& s, size_type pos = 0) const { return m_pString->find_first_not_of(s, pos); }
    size_type find_first_not_of(const_pointer s, size_type pos = 0) const { return m_pString->find_first_not_of(s, pos); }
    size_type find_first_not_of(const_pointer s, size_type pos, size_type n) const { return m_pString->find_first_not_of(s, pos, n); }
    size_type find_first_not_of(char_type c, size_type pos = 0) const { return m_pString->find_first_not_of(c, pos); }

    size_type find_last_not_of(basic_slim_string const& that, size_type pos = npos) const { return m_pString->find_last_not_of(*that.m_pString, pos); }
    size_type find_last_not_of(string_type const& s, size_type pos = npos) const { return m_pString->find_last_not_of(s, pos); }
    size_type find_last_not_of(const_pointer s, size_type pos = npos) const { return m_pString->find_last_not_of(s, pos); }
    size_type find_last_not_of(const_pointer s, size_type pos, size_type n) const { return m_pString->find_last_not_of(s, pos, n); }
    size_type find_last_not_of(char_type c, size_type pos = npos) const { return m_pString->find_last_not_of(c, pos); }

    int compare(basic_slim_string const& that) const { return m_pString->compare(*that.m_pString); }
    int compare(string_type const& s) const { return m_pString->compare(s); }
    int compare(size_type pos1, size_type n1, basic_slim_string const& that) const { return m_pString->compare(pos1, n1, *that.m_pString); }
    int compare(size_type pos1, size_type n1, string_type const& s) const { return m_pString->compare(pos1, n1, s); }
    int compare(size_type pos1, size_type n1, basic_slim_string const& that, size_type pos2, size_type n2) const { return m_pString->compare(pos1, n1, *that.m_pString, pos2, n2); }
    int compare(size_type pos1, size_type n1, string_type const& s, size_type pos2, size_type n2) const { return m_pString->compare(pos1, n1, s, pos2, n2); }
    int compare(const_pointer s) const { return m_pString->compare(s); }
    int compare(const_pointer s, size_type n2) const { return m_pString->compare(0, m_pString->size(), s, n2); }
    int compare(size_type pos1, size_type n1, const_pointer s) const { return m_pString->compare(pos1, n1, s); }
    int compare(size_type pos1, size_type n1, const_pointer s, size_type n2) const { return m_pString->compare(pos1, n1, s, n2); }
};

//! A free-standing swap
template< typename CharT, typename TraitsT, typename AllocatorT >
inline void swap(
    basic_slim_string< CharT, TraitsT, AllocatorT >& left,
    basic_slim_string< CharT, TraitsT, AllocatorT >& right)
{
    left.swap(right);
}

} // namespace aux

} // namespace log

} // namespace boost

#endif // BOOST_LOG_DETAIL_SLIM_STRING_HPP_INCLUDED_
