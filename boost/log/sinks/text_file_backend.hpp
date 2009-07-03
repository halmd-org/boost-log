/*
 * (C) 2009 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * This header is the Boost.Log library implementation, see the library documentation
 * at http://www.boost.org/libs/log/doc/log.html.
 */
/*!
 * \file   text_file_backend.hpp
 * \author Andrey Semashev
 * \date   09.06.2009
 *
 * The header contains implementation of a text file sink backend.
 */

#if (defined(_MSC_VER) && _MSC_VER > 1000)
#pragma once
#endif // _MSC_VER > 1000

#ifndef BOOST_LOG_SINKS_TEXT_FILE_BACKEND_HPP_INCLUDED_
#define BOOST_LOG_SINKS_TEXT_FILE_BACKEND_HPP_INCLUDED_

#include <ios>
#include <list>
#include <string>
#include <boost/limits.hpp>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function/function1.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/compatibility/cpp_c_headers/ctime>
#include <boost/log/keywords/max_size.hpp>
#include <boost/log/keywords/min_free_space.hpp>
#include <boost/log/keywords/file_name.hpp>
#include <boost/log/keywords/open_mode.hpp>
#include <boost/log/keywords/auto_flush.hpp>
#include <boost/log/keywords/scan_method.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/detail/universal_path.hpp>
#include <boost/log/detail/parameter_tools.hpp>
#include <boost/log/sinks/basic_sink_backend.hpp>

#ifdef _MSC_VER
#pragma warning(push)
// 'm_A' : class 'A' needs to have dll-interface to be used by clients of class 'B'
#pragma warning(disable: 4251)
// non dll-interface class 'A' used as base for dll-interface class 'B'
#pragma warning(disable: 4275)
#endif // _MSC_VER

namespace boost {

namespace BOOST_LOG_NAMESPACE {

namespace sinks {

//! The enumeration of the stored files scan methods
enum file_scan_method
{
    no_scan,        //!< Don't scan for stored files
    scan_matching,  //!< Scan for files with names matching the specified mask
    scan_all        //!< Scan for all files in the directory
};

/*!
 * \brief A FIFO log file collector.
 *
 * The collector attempts to store files in the order they are written. It supports
 * flexible file naming, including a time stamp and file number embedding. The collector
 * monitors the size of the stored files and/or the free space on the target drive
 * and deletes the oldest files to fit within the specified limits.
 */
class BOOST_LOG_EXPORT fifo_file_collector
{
public:
    //! Functor result type
    typedef void result_type;

private:
    //! Information about a single stored file
    struct file_info
    {
        uintmax_t m_Size;
        std::time_t m_TimeStamp;
        boost::log::aux::universal_path m_Path;
    };
    //! A list of the stored files
    typedef std::list< file_info > file_list;
    //! The string type compatible with the universal path type
    typedef boost::log::aux::universal_path::string_type path_string_type;

private:
    //! Total file size upper limit
    uintmax_t m_MaxSize;
    //! Free space lower limit
    uintmax_t m_MinFreeSpace;
    //! Target directory to store files to
    boost::log::aux::universal_path m_StorageDir;

    //! File name generator
    function1< path_string_type, unsigned int > m_FileNameGenerator;

    //! The list of stored files
    file_list m_Files;
    //! Stored files counter
    unsigned int m_FileCounter;
    //! Total size of the stored files
    uintmax_t m_TotalSize;

public:
    /*!
     * Constructor. Creates collector that does not limit the number of the stored files.
     * The files are stored in the current directory, with names appended with the file number.
     */
    fifo_file_collector();
    /*!
     * Copy constructor
     */
    fifo_file_collector(fifo_file_collector const& that);

    // Constructors that accept named arguments
    BOOST_LOG_PARAMETRIZED_CONSTRUCTORS_CALL(fifo_file_collector, construct)

    /*!
     * Destructor
     */
    ~fifo_file_collector();

    /*!
     * Assignment
     */
    fifo_file_collector& operator= (fifo_file_collector that);

    /*!
     * The operator stores the specified file in the storage. May lead to an older file
     * deletion and a long file moving.
     *
     * \param p The name of the file to be stored
     */
    void operator()(boost::log::aux::universal_path const& p);

    /*!
     * Swaps two instances of the collector
     */
    void swap(fifo_file_collector& that);

    /*!
     * Scans the target directory for the files that have already been stored. The found
     * files are added to the collector in order to be tracked and erased, if needed.
     *
     * \param method The scan method. If \c no_scan is specified, the call has no effect.
     * \param pattern The target directory, in case if \a method is \c scan_all. The target
     *                directory and the file name pattern if \a method is \c scan_matching.
     */
    void scan_for_files(file_scan_method method, boost::log::aux::universal_path const& pattern);

private:
    //! Constructor implementation
    template< typename ArgsT >
    void construct(ArgsT const& args)
    {
        construct(
            args[keywords::max_size | (std::numeric_limits< uintmax_t >::max)()],
            args[keywords::min_free_space | static_cast< uintmax_t >(0)],
            boost::log::aux::to_universal_path(args[keywords::file_name | "./%0.5N.log"]),
            args[keywords::scan_method | no_scan]);
    }
    //! Constructor implementation
    void construct(
        uintmax_t max_size,
        uintmax_t min_free_space,
        boost::log::aux::universal_path const& pattern,
        file_scan_method scan);
};

/*!
 * \brief An implementation of a text file logging sink backend
 *
 * The sink backend puts formatted log records to a text file.
 * The sink supports file rotation and advanced file control, such as
 * size and file count restriction.
 */
template< typename CharT >
class BOOST_LOG_EXPORT basic_text_file_backend :
    public basic_formatting_sink_backend< CharT >
{
    //! Base type
    typedef basic_formatting_sink_backend< CharT > base_type;

public:
    //! Character type
    typedef typename base_type::char_type char_type;
    //! String type to be used as a message text holder
    typedef typename base_type::string_type string_type;
    //! String type to be used as a message text holder
    typedef typename base_type::target_string_type target_string_type;
    //! Attribute values view type
    typedef typename base_type::values_view_type values_view_type;
    //! Log record type
    typedef typename base_type::record_type record_type;

    //! File collector functor type
    typedef function1< void, boost::log::aux::universal_path > file_collector_type;

private:
    //! \cond

    struct implementation;
    implementation* m_pImpl;

    //! \endcond

public:
    /*!
     * Constructor. No streams attached to the constructed backend, auto flush feature disabled.
     */
    basic_text_file_backend();

    // Constructors that accept named arguments
    BOOST_LOG_PARAMETRIZED_CONSTRUCTORS_CALL(basic_text_file_backend, construct)

    /*!
     * Destructor
     */
    ~basic_text_file_backend();

    /*!
     * The method sets file name wildcard for the files being written. The wildcard supports
     * date and time injection into the file name.
     *
     * \param temp The mask for the file being written.
     */
    template< typename PathT >
    void temp_file_name(PathT const& temp)
    {
        set_temp_file_name(boost::log::aux::to_universal_path(temp));
    }

    /*!
     * The method sets the file open mode
     *
     * \param mode File open mode
     */
    void open_mode(std::ios_base::openmode mode);

    /*!
     * The method sets the log file collector function. The function is called
     * on file rotation and is being passed the written file name.
     *
     * \param collector The file collector function object
     */
    template< typename CollectorT >
    void file_collector(CollectorT const& collector)
    {
        set_file_collector(collector);
    }

    /*!
     * The method sets maximum file size. When the size is reached, file rotation is performed.
     *
     * \note The size does not count any possible character translations that may happen in
     *       the underlying API. This may result in greater actual sizes of the written files.
     *
     * \param size The maximum file size, in characters.
     */
    void max_file_size(uintmax_t size);

    /*!
     * Sets the flag to automatically flush buffers of all attached streams after each log record
     */
    void auto_flush(bool f = true);

private:
#ifndef BOOST_LOG_DOXYGEN_PASS
    //! Constructor implementation
    template< typename ArgsT >
    void construct(ArgsT const& args)
    {
        construct(
            args[keywords::file_name | boost::log::aux::universal_path()],
            args[keywords::open_mode | (std::ios_base::trunc | std::ios_base::out)],
            args[keywords::max_size | (std::numeric_limits< uintmax_t >::max)()],
            args[keywords::auto_flush | false]);
    }
    //! Constructor implementation
    void construct(
        boost::log::aux::universal_path const& temp,
        std::ios_base::openmode mode,
        uintmax_t max_size,
        bool auto_flush);

    //! The method writes the message to the sink
    void do_consume(record_type const& record, target_string_type const& formatted_message);

    //! The method sets file name mask
    void set_temp_file_name(boost::log::aux::universal_path const& temp);

    //! The method sets file collector
    void set_file_collector(file_collector_type const& collector);

    //! The method rotates the file
    void rotate_file();
#endif // BOOST_LOG_DOXYGEN_PASS
};

#ifdef BOOST_LOG_USE_CHAR
typedef basic_text_file_backend< char > text_file_backend;        //!< Convenience typedef for narrow-character logging
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
typedef basic_text_file_backend< wchar_t > wtext_file_backend;    //!< Convenience typedef for wide-character logging
#endif

} // namespace sinks

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_SINKS_TEXT_FILE_BACKEND_HPP_INCLUDED_
