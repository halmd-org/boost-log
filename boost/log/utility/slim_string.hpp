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

#ifndef BOOST_LOG_UTILITY_SLIM_STRING_HPP_INCLUDED_
#define BOOST_LOG_UTILITY_SLIM_STRING_HPP_INCLUDED_

#include <iosfwd>
#include <string>
#include <memory>
#include <iterator>
#include <algorithm>
#include <boost/compatibility/cpp_c_headers/cstddef>
#include <boost/log/detail/prologue.hpp>

#ifdef _MSC_VER
#pragma warning(push)
 // non dll-interface class 'A' used as base for dll-interface class 'B'
#pragma warning(disable: 4275)
#endif // _MSC_VER

namespace boost {

namespace log {

//! A slim string class with shared data and read-only access
template<
    typename CharT,
    typename TraitsT = std::char_traits< CharT >
>
class basic_slim_string :
    private std::allocator< char >
{
public:
    //  Non-standard typedefs
    typedef CharT char_type;
    typedef TraitsT traits_type;
    typedef std::allocator< char_type > allocator_type;
    typedef std::basic_string< char_type, traits_type > string_type;

    //  Standard typedefs
    typedef typename allocator_type::value_type value_type;
    typedef typename allocator_type::pointer pointer;
    typedef typename allocator_type::const_pointer const_pointer;
    typedef typename allocator_type::reference reference;
    typedef typename allocator_type::const_reference const_reference;
    typedef typename allocator_type::size_type size_type;
    typedef typename allocator_type::difference_type difference_type;
    typedef const_pointer const_iterator;
    typedef std::reverse_iterator< const_iterator > const_reverse_iterator;

    enum { npos = ~0U };

private:
    //! Internal allocator
    typedef std::allocator< char > internal_allocator_type;

    //! Internal implementation class
    struct implementation;
    friend struct implementation;
    //! Pointer to the shared implementation
    implementation* m_pImpl;

public:
    //! Default constructor
    BOOST_LOG_EXPORT basic_slim_string();
    //! Copy constructor
    BOOST_LOG_EXPORT basic_slim_string(basic_slim_string const& that);

    //  Standard constructors
    BOOST_LOG_EXPORT explicit basic_slim_string(string_type const& that);
    BOOST_LOG_EXPORT basic_slim_string(string_type const& s, size_type pos, size_type n = npos);
    BOOST_LOG_EXPORT basic_slim_string(basic_slim_string const& that, size_type pos, size_type n = npos);
    BOOST_LOG_EXPORT basic_slim_string(const_pointer s);
    BOOST_LOG_EXPORT basic_slim_string(const_pointer s, size_type n);
    BOOST_LOG_EXPORT basic_slim_string(size_type n, char_type c);

    BOOST_LOG_EXPORT ~basic_slim_string();

    //  Assignment
    basic_slim_string& operator= (basic_slim_string const& that)
    {
        if (this != &that)
        {
            basic_slim_string tmp(that);
            swap(tmp);
        }
        return *this;
    }
    template< typename T >
    basic_slim_string& operator= (T const& that)
    {
        basic_slim_string tmp(that);
        swap(tmp);
        return *this;
    }

    //! Indexing
    BOOST_LOG_EXPORT const_reference operator[] (size_type n) const;

    //  Comparison and ordering
    template< typename T >
    bool operator== (T const& that) const { return (compare(that) == 0); }
    template< typename T >
    bool operator!= (T const& that) const { return (compare(that) != 0); }
    template< typename T >
    bool operator< (T const& that) const { return (compare(that) < 0); }
    template< typename T >
    bool operator> (T const& that) const { return (compare(that) > 0); }
    template< typename T >
    bool operator<= (T const& that) const { return (compare(that) <= 0); }
    template< typename T >
    bool operator>= (T const& that) const { return (compare(that) >= 0); }

    //  Accessors
    BOOST_LOG_EXPORT const_reference at(size_type n) const;
    BOOST_LOG_EXPORT const_pointer data() const;
    const_pointer c_str() const { return data(); }
    BOOST_LOG_EXPORT size_type size() const;
    size_type length() const { return size(); }
    bool empty() const { return (size() == 0); }
    size_type capacity() const { return size(); }
    size_type max_size() const { return size(); }

    BOOST_LOG_EXPORT const_iterator begin() const;
    BOOST_LOG_EXPORT const_iterator end() const;
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    BOOST_LOG_EXPORT size_type copy(pointer s, size_type n, size_type pos = 0) const;
    basic_slim_string substr(size_type pos = 0, size_type n = npos) const
    {
        return basic_slim_string(*this, pos, n);
    }

    void swap(basic_slim_string& that) { std::swap(m_pImpl, that.m_pImpl); }

    BOOST_LOG_EXPORT size_type find(basic_slim_string const& that, size_type pos = 0) const;
    BOOST_LOG_EXPORT size_type find(string_type const& s, size_type pos = 0) const;
    BOOST_LOG_EXPORT size_type find(const_pointer s, size_type pos = 0) const;
    BOOST_LOG_EXPORT size_type find(const_pointer s, size_type pos, size_type n) const;
    BOOST_LOG_EXPORT size_type find(char_type c, size_type pos = 0) const;

    BOOST_LOG_EXPORT size_type rfind(basic_slim_string const& that, size_type pos = npos) const;
    BOOST_LOG_EXPORT size_type rfind(string_type const& s, size_type pos = npos) const;
    BOOST_LOG_EXPORT size_type rfind(const_pointer s, size_type pos = npos) const;
    BOOST_LOG_EXPORT size_type rfind(const_pointer s, size_type pos, size_type n) const;
    BOOST_LOG_EXPORT size_type rfind(char_type c, size_type pos = npos) const;

    BOOST_LOG_EXPORT size_type find_first_of(basic_slim_string const& that, size_type pos = 0) const;
    BOOST_LOG_EXPORT size_type find_first_of(string_type const& s, size_type pos = 0) const;
    BOOST_LOG_EXPORT size_type find_first_of(const_pointer s, size_type pos = 0) const;
    BOOST_LOG_EXPORT size_type find_first_of(const_pointer s, size_type pos, size_type n) const;
    BOOST_LOG_EXPORT size_type find_first_of(char_type c, size_type pos = 0) const;

    BOOST_LOG_EXPORT size_type find_last_of(basic_slim_string const& that, size_type pos = npos) const;
    BOOST_LOG_EXPORT size_type find_last_of(string_type const& s, size_type pos = npos) const;
    BOOST_LOG_EXPORT size_type find_last_of(const_pointer s, size_type pos = npos) const;
    BOOST_LOG_EXPORT size_type find_last_of(const_pointer s, size_type pos, size_type n) const;
    BOOST_LOG_EXPORT size_type find_last_of(char_type c, size_type pos = npos) const;

    BOOST_LOG_EXPORT size_type find_first_not_of(basic_slim_string const& that, size_type pos = 0) const;
    BOOST_LOG_EXPORT size_type find_first_not_of(string_type const& s, size_type pos = 0) const;
    BOOST_LOG_EXPORT size_type find_first_not_of(const_pointer s, size_type pos = 0) const;
    BOOST_LOG_EXPORT size_type find_first_not_of(const_pointer s, size_type pos, size_type n) const;
    BOOST_LOG_EXPORT size_type find_first_not_of(char_type c, size_type pos = 0) const;

    BOOST_LOG_EXPORT size_type find_last_not_of(basic_slim_string const& that, size_type pos = npos) const;
    BOOST_LOG_EXPORT size_type find_last_not_of(string_type const& s, size_type pos = npos) const;
    BOOST_LOG_EXPORT size_type find_last_not_of(const_pointer s, size_type pos = npos) const;
    BOOST_LOG_EXPORT size_type find_last_not_of(const_pointer s, size_type pos, size_type n) const;
    BOOST_LOG_EXPORT size_type find_last_not_of(char_type c, size_type pos = npos) const;

    BOOST_LOG_EXPORT int compare(basic_slim_string const& that) const;
    BOOST_LOG_EXPORT int compare(string_type const& s) const;
    BOOST_LOG_EXPORT int compare(size_type pos1, size_type n1, basic_slim_string const& that) const;
    BOOST_LOG_EXPORT int compare(size_type pos1, size_type n1, string_type const& s) const;
    BOOST_LOG_EXPORT int compare(size_type pos1, size_type n1, basic_slim_string const& that, size_type pos2, size_type n2) const;
    BOOST_LOG_EXPORT int compare(size_type pos1, size_type n1, string_type const& s, size_type pos2, size_type n2) const;
    BOOST_LOG_EXPORT int compare(const_pointer s) const;
    BOOST_LOG_EXPORT int compare(const_pointer s, size_type n2) const;
    BOOST_LOG_EXPORT int compare(size_type pos1, size_type n1, const_pointer s) const;
    BOOST_LOG_EXPORT int compare(size_type pos1, size_type n1, const_pointer s, size_type n2) const;
};

//! Output
template< typename CharT, typename TraitsT >
inline std::basic_ostream< CharT, TraitsT >& operator<< (
    std::basic_ostream< CharT, TraitsT >& strm, basic_slim_string< CharT, TraitsT > const& s)
{
    strm.write(s.data(), s.size());
    return strm;
}

//! A free-standing swap
template< typename CharT, typename TraitsT >
inline void swap(basic_slim_string< CharT, TraitsT >& left, basic_slim_string< CharT, TraitsT >& right)
{
    left.swap(right);
}

#ifdef BOOST_LOG_USE_CHAR
typedef basic_slim_string< char > slim_string;
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
typedef basic_slim_string< wchar_t > slim_wstring;
#endif

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_UTILITY_SLIM_STRING_HPP_INCLUDED_
