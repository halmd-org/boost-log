/*
 *          Copyright Andrey Semashev 2007 - 2012.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   extractor_adapter.hpp
 * \author Andrey Semashev
 * \date   21.07.2012
 *
 * The header contains attribute value extractor adapter for constructing expression template terminals.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_EXPRESSIONS_EXTRACTOR_ADAPTER_HPP_INCLUDED_
#define BOOST_LOG_EXPRESSIONS_EXTRACTOR_ADAPTER_HPP_INCLUDED_

#include <boost/fusion/sequence/intrinsic/at.hpp>
#include <boost/log/detail/prologue.hpp>

namespace boost {

BOOST_LOG_OPEN_NAMESPACE

namespace expressions {

/*!
 * \brief An attribute value extractor adapter
 *
 * This class is an adapter between Boost.Phoenix expression invokation protocol and
 * an attribute value extractor interface. It is required to embed value extractors
 * in template expressions.
 */
template< typename ExtractorT >
struct extractor_adapter :
    public ExtractorT
{
    //! Attribute value extractor type
    typedef ExtractorT extractor_type;
    //! Extraction result type
    typedef typename extractor_type::result_type result_type;

    //! Default constructor
    BOOST_LOG_DEFAULTED_FUNCTION(extractor_adapter(), {})
    //! Initializing constructor
    template< typename ArgT1 >
    explicit extractor_adapter(ArgT1 const& arg1) : extractor_type(arg1) {}
    //! Initializing constructor
    template< typename ArgT1, typename ArgT2 >
    extractor_adapter(ArgT1 const& arg1, ArgT2 const& arg2) : extractor_type(arg1, arg2) {}

    //! The operator extracts the value
    template< typename ArgsT >
    result_type operator() (ArgsT const& args) const
    {
        return extractor_type::operator()(fusion::at_c< 0 >(args));
    }
};

} // namespace expressions

BOOST_LOG_CLOSE_NAMESPACE // namespace log

} // namespace boost

#endif // BOOST_LOG_EXPRESSIONS_EXTRACTOR_ADAPTER_HPP_INCLUDED_
