/*!
 * (C) 2007 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * \file   severity_level.cpp
 * \author Andrey Semashev
 * \date   10.05.2008
 * 
 * \brief  This header is the Boost.Log library implementation, see the library documentation
 *         at http://www.boost.org/libs/log/doc/log.html.
 */

#include <boost/make_shared.hpp>
#include <boost/log/sources/severity_logger.hpp>

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace sources {

namespace aux {

//! Default constructor
inline severity_level::severity_level()
{
}

//! Destructor
severity_level::~severity_level()
{
}

//! Returns an instance of the attribute
BOOST_LOG_EXPORT shared_ptr< severity_level > severity_level::get()
{
    return singleton_base::get();
}

//! The method returns the actual attribute value. It must not return NULL.
shared_ptr< attribute_value > severity_level::get_value()
{
    return shared_from_this();
}

//! The method dispatches the value to the given object
bool severity_level::dispatch(type_dispatcher& dispatcher)
{
    register type_visitor< held_type >* visitor =
        dispatcher.get_visitor< held_type >();
    if (visitor)
    {
#if !defined(BOOST_LOG_NO_THREADS)
        visitor->visit(m_Value.get());
#else
        visitor->visit(m_Value);
#endif
        return true;
    }
    else
        return false;
}

//! The method is called when the attribute value is passed to another thread
shared_ptr< attribute_value > severity_level::detach_from_thread()
{
#if !defined(BOOST_LOG_NO_THREADS)
    return boost::make_shared<
        attributes::basic_attribute_value< held_type >
    >(m_Value.get());
#else
    // With multithreading disabled we may safely return this here. This method will not be called anyway.
    return shared_from_this();
#endif
}

//! Initializes the singleton instance
void severity_level::init_instance()
{
    singleton_base::get_instance().reset(new severity_level());
}

} // namespace aux

} // namespace sources

} // namespace log

} // namespace boost
