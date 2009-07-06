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
#include <locale>
#include <boost/limits.hpp>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/function/function1.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/compatibility/cpp_c_headers/ctime>
#include <boost/log/formatters/basic_formatters.hpp>
#include <boost/log/keywords/max_size.hpp>
#include <boost/log/keywords/min_free_space.hpp>
#include <boost/log/keywords/file_name.hpp>
#include <boost/log/keywords/open_mode.hpp>
#include <boost/log/keywords/auto_flush.hpp>
#include <boost/log/keywords/scan_method.hpp>
#include <boost/log/keywords/rotation_size.hpp>
#include <boost/log/keywords/rotation_interval.hpp>
#include <boost/log/detail/prologue.hpp>
#include <boost/log/detail/universal_path.hpp>
#include <boost/log/detail/parameter_tools.hpp>
#include <boost/log/detail/cleanup_scope_guard.hpp>
#include <boost/log/detail/code_conversion.hpp>
#include <boost/log/detail/attachable_sstream_buf.hpp>
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

namespace file {

//! The enumeration of the stored files scan methods
enum scan_method
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
class fifo_collector
{
public:
    //! Functor result type
    typedef void result_type;

private:
    //! \cond

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

    //! \endcond

public:
    /*!
     * Constructor. Creates collector with default values for all parameters used.
     */
    BOOST_LOG_EXPORT fifo_collector();
    /*!
     * Copy constructor.
     *
     * \note The constructor is provided for technical reasons only. Different copies of the same
     *       collector instance should not be used simultaneously as it would introduce unexpected
     *       results due to conflicting access to the target directory.
     */
    BOOST_LOG_EXPORT fifo_collector(fifo_collector const& that);

    /*!
     * Constructor. Creates a file collector with the specified named parameters.
     * The following named parameters are supported:
     *
     * \li \c file_name - Specifies the file name pattern for the files being stored. The file name
     *                    portion of the pattern may contain placeholders for date and time in the form
     *                    compatible to Boost.DateTime. Addidionally, %N placeholder for the integral
     *                    file counter is supported. The placeholder may additionally contain width
     *                    specification in the printf-compatible form (e.g. %5N). Note that the
     *                    directory portion of the pattern should not contain any placeholders. If
     *                    not specified, the pattern "./%5N.log" will be used.
     * \li \c max_size - Specifies the maximum total size, in bytes, of stored files that the collector
     *                   will try not to exceed. If the size exceeds this threshold the oldest file(s) is
     *                   deleted to free space. Note that the threshold may be exceeded if the size of
     *                   individual files exceed the \c max_size value. The threshold is not maintained,
     *                   if not specified.
     * \li \c min_free_space - Specifies the minimum free space, in bytes, in the target directory that
     *                         the collector tries to maintain. If the threshold is exceeded, the oldest
     *                         file(s) is deleted to free space. The threshold is not maintained, if not
     *                         specified.
     * \li \c scan_method - Specifies how the collector should perform scanning for files in the target
     *                      directory that may have been left since previous runs of the application. All
     *                      found files will be considered as if they were stored during the current run
     *                      of the application and thus being candidates for deletion. The parameter
     *                      should be one of the <tt>file::scan_method</tt> values, with \c no_scan being the
     *                      default. Note that if \c scan_matching is specified, the collector will try
     *                      to adjust file counter according to the found files.
     *
     * \note As file scanning may occur during construction, it is recommended to follow the restrictions
     *       described in the \c scan_for_files documentation.
     */
#ifndef BOOST_LOG_DOXYGEN_PASS
    BOOST_LOG_PARAMETRIZED_CONSTRUCTORS_CALL(fifo_collector, construct)
#else
    template< typename... ArgsT >
    explicit fifo_collector(ArgsT... const& args);
#endif

    /*!
     * Destructor
     */
    BOOST_LOG_EXPORT ~fifo_collector();

    /*!
     * Assignment.
     *
     * \note See warning in the copy constructor documentation.
     */
    BOOST_LOG_EXPORT fifo_collector& operator= (fifo_collector that);

    /*!
     * The operator stores the specified file in the storage. May lead to an older file
     * deletion and a long file moving.
     *
     * \param p The name of the file to be stored
     */
    BOOST_LOG_EXPORT void operator()(boost::log::aux::universal_path const& p);

    /*!
     * Swaps two instances of the collector
     */
    BOOST_LOG_EXPORT void swap(fifo_collector& that);

    /*!
     * Scans the target directory for the files that have already been stored. The found
     * files are added to the collector in order to be tracked and erased, if needed.
     *
     * The function may scan the directory in two ways: it will either consider every
     * file in the directory a log file, or will only consider files with names that
     * match the specified pattern. The pattern may contain the following placeholders:
     *
     * \li %y, %Y, %m, %d - date components, in Boost.DateTime meaning.
     * \li %H, %M, %S, %F - time components, in Boost.DateTime meaning.
     * \li %N - numeric file counter. May also contain width specification
     *     in printf-compatible form (e.g. %5N). The resulting number will always be zero-filled.
     * \li %% - a percent sign
     *
     * All other placeholders are not supported.
     *
     * \param method The method of scanning. If \c no_scan is specified, the call has no effect.
     * \param pattern The target directory, in case if \a method is \c scan_all. The target
     *                directory and the file name pattern if \a method is \c scan_matching.
     * \param update_counter If \c true and \a method is \c scan_matching, the method attempts
     *                       to update the file counter according to the files found in the target
     *                       directory. The counter is unaffected otherwise. It usually should be
     *                       \c false unless you're scanning the target directory.
     * \return The number of found files.
     *
     * \note In case if \a method is \c scan_matching the effect of this function is highly dependent
     *       on the \a pattern definition. It is recommended to choose patterns with easily
     *       distinguished placeholders (i.e. having delimiters between them). Otherwise
     *       either some files can be mistakenly found or not found, which in turn may lead
     *       to an incorrect file deletion.
     */
    BOOST_LOG_EXPORT unsigned int scan_for_files(
        file::scan_method method, boost::log::aux::universal_path const& pattern, bool update_counter = false);

private:
#ifndef BOOST_LOG_DOXYGEN_PASS
    //! Constructor implementation
    template< typename ArgsT >
    void construct(ArgsT const& args)
    {
        construct(
            args[keywords::max_size | (std::numeric_limits< uintmax_t >::max)()],
            args[keywords::min_free_space | static_cast< uintmax_t >(0)],
            boost::log::aux::to_universal_path(args[keywords::file_name | "./%5N.log"]),
            args[keywords::scan_method | file::no_scan]);
    }
    //! Constructor implementation
    BOOST_LOG_EXPORT void construct(
        uintmax_t max_size,
        uintmax_t min_free_space,
        boost::log::aux::universal_path const& pattern,
        file::scan_method scan);
#endif // BOOST_LOG_DOXYGEN_PASS
};

} // namespace file


/*!
 * \brief An implementation of a text file logging sink backend
 *
 * The sink backend puts formatted log records to a text file.
 * The sink supports file rotation and advanced file control, such as
 * size and file count restriction.
 */
template< typename CharT >
class basic_text_file_backend :
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
    //! Log record type
    typedef typename base_type::record_type record_type;
    //! Stream type
    typedef typename base_type::stream_type stream_type;

    //! File collector functor type
    typedef function1< void, boost::log::aux::universal_path > file_collector_type;
    //! File open handler
    typedef function1< void, stream_type& > open_handler_type;
    //! File close handler
    typedef function1< void, stream_type& > close_handler_type;

private:
    //! \cond

    struct implementation;
    implementation* m_pImpl;

    //! \endcond

public:
    /*!
     * Default constructor. The constructed sink backend uses default values of all the parameters.
     */
    BOOST_LOG_EXPORT basic_text_file_backend();

    /*!
     * Constructor. Creates a sink backend with the specified named parameters.
     * The following named parameters are supported:
     *
     * \li \c file_name - Specifies the temporary file name where logs are actually written to. A temporary
     *                    file name will be automatically generated if not specified.
     * \li \c open_mode - File open mode. The mode should be presented in form of mask compatible to
     *                    <tt>std::ios_base::openmode</tt>. If not specified, <tt>trunc | out</tt> will be used.
     * \li \c rotation_size - Specifies the approximate size, in characters written, of the temporary file
     *                        upon which the file is passed to the file collector. Note the size does
     *                        not count any possible character conversions that may take place during
     *                        writing to the file. If not specified, the file won't be rotated upon reaching
     *                        any size.
     * \li \c rotation_interval - Specifies number of seconds between file rotations. No time-based file
     *                            rotations will be performed, if not specified.
     * \li \c auto_flush - Specifies a flag, whether or not to automatically flush the file after each
     *                     written log record. By default, is \c false.
     */
#ifndef BOOST_LOG_DOXYGEN_PASS
    BOOST_LOG_PARAMETRIZED_CONSTRUCTORS_CALL(basic_text_file_backend, construct)
#else
    template< typename... ArgsT >
    explicit basic_text_file_backend(ArgsT... const& args);
#endif

    /*!
     * Destructor
     */
    BOOST_LOG_EXPORT ~basic_text_file_backend();

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
    BOOST_LOG_EXPORT void open_mode(std::ios_base::openmode mode);

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
     * The method sets file opening handler. The handler will be called every time
     * the backend opens a new temporary file. The handler may write a header to the
     * opened file in order to maintain file validity.
     *
     * \param handler The file open handler function object
     */
    template< typename HandlerT >
    void open_handler(HandlerT const& handler)
    {
        set_open_handler(handler);
    }

    /*!
     * The method sets file closing handler. The handler will be called every time
     * the backend closes a temporary file. The handler may write a footer to the
     * opened file in order to maintain file validity.
     *
     * \param handler The file close handler function object
     */
    template< typename HandlerT >
    void close_handler(HandlerT const& handler)
    {
        set_close_handler(handler);
    }

    /*!
     * The method sets maximum file size. When the size is reached, file rotation is performed.
     *
     * \note The size does not count any possible character translations that may happen in
     *       the underlying API. This may result in greater actual sizes of the written files.
     *
     * \param size The maximum file size, in characters.
     */
    BOOST_LOG_EXPORT void max_file_size(uintmax_t size);

    /*!
     * Sets the flag to automatically flush buffers of all attached streams after each log record
     */
    BOOST_LOG_EXPORT void auto_flush(bool f = true);

private:
#ifndef BOOST_LOG_DOXYGEN_PASS
    //! Constructor implementation
    template< typename ArgsT >
    void construct(ArgsT const& args)
    {
        construct(
            boost::log::aux::to_universal_path(args[keywords::file_name | boost::log::aux::universal_path()]),
            args[keywords::open_mode | (std::ios_base::trunc | std::ios_base::out)],
            args[keywords::rotation_size | (std::numeric_limits< uintmax_t >::max)()],
            args[keywords::rotation_interval | (std::numeric_limits< unsigned int >::max)()],
            args[keywords::auto_flush | false]);
    }
    //! Constructor implementation
    BOOST_LOG_EXPORT void construct(
        boost::log::aux::universal_path const& temp,
        std::ios_base::openmode mode,
        uintmax_t rotation_size,
        unsigned int rotation_interval,
        bool auto_flush);

    //! The method writes the message to the sink
    BOOST_LOG_EXPORT void do_consume(record_type const& record, target_string_type const& formatted_message);

    //! The method sets file name mask
    BOOST_LOG_EXPORT void set_temp_file_name(boost::log::aux::universal_path const& temp);

    //! The method sets file collector
    BOOST_LOG_EXPORT void set_file_collector(file_collector_type const& collector);

    //! The method sets file open handler
    BOOST_LOG_EXPORT void set_open_handler(open_handler_type const& handler);

    //! The method sets file close handler
    BOOST_LOG_EXPORT void set_close_handler(close_handler_type const& handler);

    //! The method rotates the file
    void rotate_file();
#endif // BOOST_LOG_DOXYGEN_PASS
};


namespace file {

    /*!
     * An adapter class that allows to use regular formatters as file name generators.
     */
    template< typename FormatterT >
    class file_name_composer_adapter
    {
    public:
        //! Functor result type
        typedef boost::log::aux::universal_path result_type;
        //! The adopted formatter type
        typedef FormatterT formatter_type;
        //! Character type used by the formatter
        typedef typename formatter_type::char_type char_type;
        //! String type used by the formatter
        typedef typename formatter_type::string_type string_type;
        //! Log record type
        typedef typename formatter_type::record_type record_type;

        //! Stream buffer type used to store formatted data
        typedef typename mpl::if_<
            is_same< char_type, result_type::string_type::value_type >,
            boost::log::aux::basic_ostringstreambuf< char_type >,
            boost::log::aux::converting_ostringstreambuf< char_type >
        >::type streambuf_type;
        //! Stream type used for formatting
        typedef std::basic_ostream< char_type > stream_type;

    private:
        //! The adopted formatter
        formatter_type m_Formatter;
        //! Formatted file name storage
        mutable result_type::string_type m_FileName;
        //! Stream buffer to fill the storage
        mutable streambuf_type m_StreamBuf;
        //! Formatting stream
        mutable stream_type m_FormattingStream;

    public:
        /*!
         * Initializing constructor
         */
        explicit file_name_composer_adapter(formatter_type const& formatter, std::locale const& loc = std::locale()) :
            m_Formatter(formatter),
            m_StreamBuf(m_FileName),
            m_FormattingStream(&m_StreamBuf)
        {
            m_FormattingStream.exceptions(std::ios_base::badbit | std::ios_base::failbit);
            m_FormattingStream.imbue(loc);
        }
        /*!
         * Copy constructor
         */
        file_name_composer_adapter(file_name_composer_adapter const& that) :
            m_Formatter(that.m_Formatter),
            m_StreamBuf(m_FileName),
            m_FormattingStream(&m_StreamBuf)
        {
            m_FormattingStream.exceptions(std::ios_base::badbit | std::ios_base::failbit);
            m_FormattingStream.imbue(that.m_FormattingStream.getloc());
        }
        /*!
         * Assignment
         */
        file_name_composer_adapter& operator= (file_name_composer_adapter const& that)
        {
            m_Formatter = that.m_Formatter;
            return *this;
        }

        /*!
         * The operator generates a file name based on the log record
         */
        result_type operator() (record_type const& record) const
        {
            boost::log::aux::cleanup_guard< stream_type > cleanup1(m_FormattingStream);
            boost::log::aux::cleanup_guard< result_type::string_type > cleanup2(m_FileName);

            m_Formatter(m_FormattingStream, record);
            m_FormattingStream.flush();

            return result_type(m_FileName);
        }
    };

    /*!
     * The function adopts a log record formatter into a file name generator
     */
    template< typename FormatterT >
    inline file_name_composer_adapter< FormatterT > as_file_name_composer(
        FormatterT const& fmt, std::locale const& loc = std::locale())
    {
        return file_name_composer_adapter< FormatterT >(fmt, loc);
    }

} // namespace file


/*!
 * \brief An implementation of a text multiple files logging sink backend
 *
 * The sink backend puts formatted log records to one of the text files.
 * The particular file is chosen upon each record's attribute values, which allows
 * to distribute records into individual files or to group records related to
 * some entity or process in a separate file.
 */
template< typename CharT >
class basic_text_multifile_backend :
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
    //! Log record type
    typedef typename base_type::record_type record_type;

    //! File name composer functor type
    typedef function1< boost::log::aux::universal_path, record_type const& > file_name_composer_type;

private:
    //! \cond

    struct implementation;
    implementation* m_pImpl;

    //! \endcond

public:
    /*!
     * Default constructor. The constructed sink backend has no file name composer and
     * thus will not write any files.
     */
    BOOST_LOG_EXPORT basic_text_multifile_backend();

    /*!
     * Destructor
     */
    BOOST_LOG_EXPORT ~basic_text_multifile_backend();

    /*!
     * The method sets file name composer functional object. Log record formatters are accepted, too.
     *
     * \param composer File name composer functor
     */
    template< typename ComposerT >
    void file_name_composer(ComposerT const& composer)
    {
        set_file_name_composer(composer, typename formatters::is_formatter< ComposerT >::type());
    }

private:
#ifndef BOOST_LOG_DOXYGEN_PASS
    //! The method writes the message to the sink
    BOOST_LOG_EXPORT void do_consume(record_type const& record, target_string_type const& formatted_message);

    //! The method sets the file name composer
    template< typename ComposerT >
    void set_file_name_composer(ComposerT const& composer, mpl::true_ const&)
    {
        set_file_name_composer(file::as_file_name_composer(composer));
    }
    //! The method sets the file name composer
    template< typename ComposerT >
    void set_file_name_composer(ComposerT const& composer, mpl::false_ const&)
    {
        set_file_name_composer(composer);
    }
    //! The method sets the file name composer
    BOOST_LOG_EXPORT void set_file_name_composer(file_name_composer_type const& composer);
#endif // BOOST_LOG_DOXYGEN_PASS
};

#ifdef BOOST_LOG_USE_CHAR
typedef basic_text_file_backend< char > text_file_backend;               //!< Convenience typedef for narrow-character logging
typedef basic_text_multifile_backend< char > text_multifile_backend;     //!< Convenience typedef for narrow-character logging
#endif
#ifdef BOOST_LOG_USE_WCHAR_T
typedef basic_text_file_backend< wchar_t > wtext_file_backend;           //!< Convenience typedef for wide-character logging
typedef basic_text_multifile_backend< wchar_t > wtext_multifile_backend; //!< Convenience typedef for wide-character logging
#endif

} // namespace sinks

} // namespace log

} // namespace boost

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // BOOST_LOG_SINKS_TEXT_FILE_BACKEND_HPP_INCLUDED_
