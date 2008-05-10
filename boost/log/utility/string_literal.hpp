/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   string_literal.hpp
 * \author Andrey Semashev
 * \date   24.06.2007
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#if defined(_MSC_VER) && _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_UTILITY_STRING_LITERAL_HPP_INCLUDED_
#define BOOST_LOG_UTILITY_STRING_LITERAL_HPP_INCLUDED_

#include <stdexcept>
#include <iosfwd>
#include <string>
#include <iterator>
#include <algorithm>
#include <functional>
#include <boost/operators.hpp>
#include <boost/throw_exception.hpp>
#include <boost/compatibility/cpp_c_headers/cstddef>
#include <boost/type_traits/is_same.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/log/detail/prologue.hpp>

namespace boost {

namespace log {

//! String literals wrapper class
template< typename CharT, typename TraitsT = std::char_traits< CharT > >
class basic_string_literal :
    public totally_ordered1< basic_string_literal< CharT, TraitsT >,
        totally_ordered2< basic_string_literal< CharT, TraitsT >, const CharT*,
            totally_ordered2<
                basic_string_literal< CharT, TraitsT >,
                std::basic_string< CharT, TraitsT >
            >
        >
    >
{
    //! Self type
    typedef basic_string_literal< CharT, TraitsT > this_type;

public:
    typedef CharT value_type;
    typedef TraitsT traits_type;

    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef const value_type* const_pointer;
    typedef value_type const& const_reference;
    typedef const value_type* const_iterator;
    typedef std::reverse_iterator< const_iterator > const_reverse_iterator;

private:
    //! Corresponding STL string type
    typedef std::basic_string< value_type, traits_type > std_string_type;

private:
    //! Pointer to the beginning of the literal
    const_pointer m_pStart;
    //! Length
    size_type m_Len;

    //! Empty string literal to support clear
    static const value_type g_EmptyString[1];

public:
    //! Constructor
    basic_string_literal() { clear(); }
    //! Constructor from a string literal
    template< typename T, size_type LenV >
    basic_string_literal(T(&p)[LenV], typename enable_if< is_same< T, const value_type >, int >::type = 0)
        : m_pStart(p), m_Len(LenV - 1)
    {
    }
    //! Copy constructor
    basic_string_literal(basic_string_literal const& That) : m_pStart(That.m_pStart), m_Len(That.m_Len) {}

    //! Assignment
    this_type& operator= (this_type const& That)
    {
        return assign(That);
    }
    //! Assignment from a string literal
    template< typename T, size_type LenV >
    typename enable_if<
        is_same< T, const value_type >,
        this_type&
    >::type operator= (T(&p)[LenV])
    {
        return assign(p);
    }

    //! Equality comparison
    bool operator== (this_type const& That) const
    {
        return (compare_internal(m_pStart, m_Len, That.m_pStart, That.m_Len) == 0);
    }
    //! Equality comparison
    bool operator== (const_pointer p) const
    {
        return (compare_internal(m_pStart, m_Len, p, traits_type::length(p)) == 0);
    }
    //! Equality comparison
    bool operator== (std_string_type const& That) const
    {
        return (compare_internal(m_pStart, m_Len, That.c_str(), That.size()) == 0);
    }

    //! Less ordering
    bool operator< (this_type const& That) const
    {
        return (compare_internal(m_pStart, m_Len, That.m_pStart, That.m_Len) < 0);
    }
    //! Less ordering
    bool operator< (const_pointer p) const
    {
        return (compare_internal(m_pStart, m_Len, p, traits_type::length(p)) < 0);
    }
    //! Less ordering
    bool operator< (std_string_type const& That) const
    {
        return (compare_internal(m_pStart, m_Len, That.c_str(), That.size()) < 0);
    }

    //! Greater ordering
    bool operator> (this_type const& That) const
    {
        return (compare_internal(m_pStart, m_Len, That.m_pStart, That.m_Len) > 0);
    }
    //! Greater ordering
    bool operator> (const_pointer p) const
    {
        return (compare_internal(m_pStart, m_Len, p, traits_type::length(p)) > 0);
    }
    //! Greater ordering
    bool operator> (std_string_type const& That) const
    {
        return (compare_internal(m_pStart, m_Len, That.c_str(), That.size()) > 0);
    }

    //! Subscript
    const_reference operator[] (size_type i) const
    {
        return m_pStart[i];
    }
    //! Checked subscript
    const_reference at(size_type i) const
    {
        if (i < m_Len)
            return m_pStart[i];
        else
            boost::throw_exception(std::out_of_range("basic_string_literal::at: the index value is out of range"));
    }

    //! The method returns pointer to the beginning of the literal
    const_pointer c_str() const { return m_pStart; }
    //! The method returns pointer to the beginning of the literal
    const_pointer data() const { return m_pStart; }
    //! The method returns the length of the literal
    size_type size() const { return m_Len; }
    //! The method returns the length of the literal
    size_type length() const { return m_Len; }

    //! The method checks if the literal is empty
    bool empty() const
    {
        return (m_Len == 0);
    }

    //! Begin forward iterator
    const_iterator begin() const { return m_pStart; }
    //! End forward iterator
    const_iterator end() const { return m_pStart + m_Len; }
    //! Begin reverse iterator
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    //! End reverse iterator
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    //! The method constructs STL string from the literal
    std_string_type str() const
    {
        return std_string_type(m_pStart, m_Len);
    }

    //! The method clears the literal
    void clear()
    {
        m_pStart = g_EmptyString;
        m_Len = 0;
    }
    //! The method swaps two literals
    void swap(this_type& That)
    {
        std::swap(m_pStart, That.m_pStart);
        std::swap(m_Len, That.m_Len);
    }

    //! Assignment
    this_type& assign(this_type const& That)
    {
        m_pStart = That.m_pStart;
        m_Len = That.m_Len;
        return *this;
    }
    //! Assignment from a string literal
    template< typename T, size_type LenV >
    typename enable_if<
        is_same< T, const value_type >,
        this_type&
    >::type assign(T(&p)[LenV])
    {
        m_pStart = p;
        m_Len = LenV - 1;
        return *this;
    }

    //! The method copies the literal or its portion to an external buffer
    size_type copy(value_type* pStr, size_type Count, size_type pos = 0) const
    {
        if (pos <= size()) {
            const size_type len = (std::min)(Count, size() - pos);
            traits_type::copy(pStr, m_pStart + pos, len);
            return len;
        } else {
            boost::throw_exception(std::out_of_range("basic_string_literal::copy: the position is out of range"));
        }
    }
    //! Comparison
    int compare(size_type pos, size_type Count, const_pointer pStr, size_type Len) const
    {
        if (pos <= size()) {
            const size_type CompareSize = (std::min)((std::min)(Count, Len), size() - pos);
            return compare_internal(m_pStart + pos, CompareSize, pStr, CompareSize);
        } else {
            boost::throw_exception(std::out_of_range("basic_string_literal::compare: the position is out of range"));
        }
    }
    //! Comparison
    int compare(size_type pos, size_type Count, const_pointer pStr) const
    {
        return compare(pos, Count, pStr, traits_type::length(pStr));
    }
    //! Comparison
    int compare(size_type pos, size_type Count, this_type const& That) const
    {
        return compare(pos, Count, That.c_str(), That.size());
    }
    //! Comparison
    int compare(const_pointer pStr, size_type Len) const
    {
        return compare(0, m_Len, pStr, Len);
    }
    //! Comparison
    int compare(const_pointer pStr) const
    {
        return compare(0, m_Len, pStr, traits_type::length(pStr));
    }
    //! Comparison
    int compare(this_type const& That) const
    {
        return compare(0, m_Len, That.c_str(), That.size());
    }

private:
    //! Internal comparison implementation
    static int compare_internal(const_pointer pLeft, size_type LeftLen, const_pointer pRight, size_type RightLen)
    {
        if (pLeft != pRight)
        {
            register const int result = traits_type::compare(pLeft, pRight, (std::min)(LeftLen, RightLen));
            if (result != 0)
                return result;
        }
        return (LeftLen - RightLen);
    }
};

template< typename CharT, typename TraitsT >
typename basic_string_literal< CharT, TraitsT >::value_type const
basic_string_literal< CharT, TraitsT >::g_EmptyString[1] =
{
    typename basic_string_literal< CharT, TraitsT >::value_type()
};

//! Output operator
template< typename CharT, typename StrmTraitsT, typename LitTraitsT >
inline std::basic_ostream< CharT, StrmTraitsT >& operator<< (
    std::basic_ostream< CharT, StrmTraitsT >& strm, basic_string_literal< CharT, LitTraitsT > const& lit)
{
    strm.write(lit.c_str(), lit.size());
    return strm;
}

//! External swap
template< typename CharT, typename TraitsT >
inline void swap(
    basic_string_literal< CharT, TraitsT >& left,
    basic_string_literal< CharT, TraitsT >& right)
{
    left.swap(right);
}

//  Convenience typedefs
typedef basic_string_literal< char > string_literal;
typedef basic_string_literal< wchar_t > wstring_literal;

//  Convenience generators
template< typename T, std::size_t LenV >
inline typename enable_if<
    is_same< T, const char >,
    string_literal 
>::type str_literal(T(&p)[LenV])
{
    return string_literal(p);
}

template< typename T, std::size_t LenV >
inline typename enable_if<
    is_same< T, const wchar_t >,
    wstring_literal 
>::type str_literal(T(&p)[LenV])
{
    return wstring_literal(p);
}

} // namespace log

} // namespace boost

#endif // BOOST_LOG_UTILITY_STRING_LITERAL_HPP_INCLUDED_
