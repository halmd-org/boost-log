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
#include <string>
#include <locale>
#include <boost/limits.hpp>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/function/function1.hpp>
#include <boost/log/formatters/basic_formatters.hpp>
#include <boost/log/keywords/max_size.hpp>
#include <boost/log/keywords/min_free_space.hpp>
#include <boost/log/keywords/target.hpp>
#include <boost/log/keywords/file_name.hpp>
#include <boost/log/keywords/open_mode.hpp>
#include <boost/log/keywords/auto_flush.hpp>
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
 * \brief Base class for file collectors
 *
 * All file collectors, supported by file sink backends, should inherit this class.
 */
struct BOOST_LOG_NO_VTABLE collector :
    public noncopyable
{
    //! Path type that is used by Boost.Log
    typedef boost::log::aux::universal_path path_type;

    /*!
     * Virtual destructor
     */
    virtual ~collector() {}

    /*!
     * The function stores the specified file in the storage. May lead to an older file
     * deletion and a long file moving.
     *
     * \param src_path The name of the file to be stored
     */
    virtual void store_file(path_type const& src_path) = 0;

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
     * \param pattern The file name pattern if \a method is \c scan_matching. Otherwise the parameter
     *                is not used.
     * \param counter If not \c NULL and \a method is \c scan_matching, the method suggests initial value
     *                of a file counter that may be used in the file name pattern. The parameter
     *                is not used otherwise.
     * \return The number of found files.
     *
     * \note In case if \a method is \c scan_matching the effect of this function is highly dependent
     *       on the \a pattern definition. It is recommended to choose patterns with easily
     *       distinguished placeholders (i.e. having delimiters between them). Otherwise
     *       either some files can be mistakenly found or not found, which in turn may lead
     *       to an incorrect file deletion.
     */
    virtual uintmax_t scan_for_files(
        scan_method method, path_type const& pattern = path_type(), unsigned int* counter = 0) = 0;
};

namespace aux {

    //! Creates and returns a file collector with the specified parameters
    BOOST_LOG_EXPORT shared_ptr< collector > make_collector(
        collector::path_type const& target_dir,
        uintmax_t max_size,
        uintmax_t min_free_space
    );
    template< typename ArgsT >
    inline shared_ptr< collector > make_collector(ArgsT const& args)
    {
        return aux::make_collector(
            boost::log::aux::to_universal_path(args[keywords::target]),
            args[keywords::max_size | (std::numeric_limits< uintmax_t >::max)()],
            args[keywords::min_free_space | static_cast< uintmax_t >(0)]);
    }

} // namespace aux

#ifndef BOOST_LOG_DOXYGEN_PASS

template< typename T1 >
inline shared_ptr< collector > make_collector(T1 const& a1)
{
    return aux::make_collector(a1);
}
template< typename T1, typename T2 >
inline shared_ptr< collector > make_collector(T1 const& a1, T2 const& a2)
{
    return aux::make_collector((a1, a2));
}
template< typename T1, typename T2, typename T3 >
inline shared_ptr< collector > make_collector(T1 const& a1, T2 const& a2, T3 const& a3)
{
    return aux::make_collector((a1, a2, a3));
}

#else

/*!
 * The function creates a file collector for the specified target directory.
 * Each target directory is managed by a single file collector, so if
 * this function is called several times for the same directory,
 * it will return a reference to the same file collector. It is safe
 * to use the same collector in different sinks, even in a multithreaded
 * application.
 *
 * One can specify certain restrictions for the stored files, such as
 * maximum total size or minimum free space left in the target directory.
 * If any of the specified restrictions is not met, the oldest stored file
 * is deleted. If the same collector is requested more than once with
 * different restrictions, the collector will act according to the most strict
 * combination of all specified restrictions.
 *
 * The following named parameters are supported:
 *
 * \li \c target - Specifies the target directory for the files being stored in. This parameter
 *                 is mandatory.
 * \li \c max_size - Specifies the maximum total size, in bytes, of stored files that the collector
 *                   will try not to exceed. If the size exceeds this threshold the oldest file(s) is
 *                   deleted to free space. Note that the threshold may be exceeded if the size of
 *                   individual files exceed the \c max_size value. The threshold is not maintained,
 *                   if not specified.
 * \li \c min_free_space - Specifies the minimum free space, in bytes, in the target directory that
 *                         the collector tries to maintain. If the threshold is exceeded, the oldest
 *                         file(s) is deleted to free space. The threshold is not maintained, if not
 *                         specified.
 *
 * \return The file collector.
 */
template< typename... ArgsT >
shared_ptr< collector > make_collector(ArgsT... const& args);

#endif // BOOST_LOG_DOXYGEN_PASS

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
    //! Path type that is used by Boost.Log
    typedef boost::log::aux::universal_path path_type;

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
     * \li \c file_name - Specifies the file name pattern where logs are actually written to. The pattern may
     *                    contain directory and file name portions, but only the file name may contain
     *                    placeholders. The backend supports Boost.DateTime placeholders for injecting
     *                    current time and date into the file name. Also, an additional %N placeholder is
     *                    supported, it will be replaced with an integral increasing file counter. The placeholder
     *                    may also contain width specification in the printf-compatible form (e.g. %5N). The
     *                    printed file counter will always be zero-filled. If \c file_name is not specified,
     *                    pattern "%5N.log" will be used.
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
     *
     * \note Read caution regarding file name pattern in the <tt>file::collector::scan_for_files</tt>
     *       documentation.
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
     * \param pattern The name pattern for the file being written.
     */
    template< typename PathT >
    void set_file_name_pattern(PathT const& pattern)
    {
        set_file_name_pattern_internal(boost::log::aux::to_universal_path(pattern));
    }

    /*!
     * The method sets the file open mode
     *
     * \param mode File open mode
     */
    BOOST_LOG_EXPORT void set_open_mode(std::ios_base::openmode mode);

    /*!
     * The method sets the log file collector function. The function is called
     * on file rotation and is being passed the written file name.
     *
     * \param collector The file collector function object
     */
    BOOST_LOG_EXPORT void set_file_collector(shared_ptr< file::collector > const& collector);

    /*!
     * The method sets file opening handler. The handler will be called every time
     * the backend opens a new temporary file. The handler may write a header to the
     * opened file in order to maintain file validity.
     *
     * \param handler The file open handler function object
     */
    BOOST_LOG_EXPORT void set_open_handler(open_handler_type const& handler);

    /*!
     * The method sets file closing handler. The handler will be called every time
     * the backend closes a temporary file. The handler may write a footer to the
     * opened file in order to maintain file validity.
     *
     * \param handler The file close handler function object
     */
    BOOST_LOG_EXPORT void set_close_handler(close_handler_type const& handler);

    /*!
     * The method sets maximum file size. When the size is reached, file rotation is performed.
     *
     * \note The size does not count any possible character translations that may happen in
     *       the underlying API. This may result in greater actual sizes of the written files.
     *
     * \param size The maximum file size, in characters.
     */
    BOOST_LOG_EXPORT void set_rotation_size(uintmax_t size);

    /*!
     * The method sets the maximum number of seconds between file rotations.
     *
     * \note The rotation always accurs on writing a log record, so the rotation is
     *       not strictly bound to the specified time interval.
     *
     * \param interval The file rotation interval, in seconds.
     */
    BOOST_LOG_EXPORT void set_rotation_interval(unsigned int interval);

    /*!
     * Sets the flag to automatically flush buffers of all attached streams after each log record
     */
    BOOST_LOG_EXPORT void auto_flush(bool f = true);

    /*!
     * Performs scanning of the target directory for log files that may have been left from
     * previous runs of the application. The found files are considered by the file collector
     * as if they were rotated.
     *
     * The file scan can be performed in two ways: either all files in the target directory will
     * be considered as log files, or only those files that satisfy the file name pattern.
     * See documentation on <tt>file::collector::scan_for_files</tt> for more information.
     *
     * \pre File collector and the proper file name patter have already been set.
     *
     * \param method File scanning method
     * \param update_counter If \c true and \a method is \c scan_matching, the method attempts
     *        to update the internal file counter according to the found files. The counter
     *        is unaffected otherwise.
     * \return The number of files found.
     *
     * \note The method essentially delegates to the same-named function of the file collector.
     */
    BOOST_LOG_EXPORT uintmax_t scan_for_files(
        file::scan_method method = file::scan_matching, bool update_counter = true);

private:
#ifndef BOOST_LOG_DOXYGEN_PASS
    //! Constructor implementation
    template< typename ArgsT >
    void construct(ArgsT const& args)
    {
        construct(
            boost::log::aux::to_universal_path(args[keywords::file_name | path_type()]),
            args[keywords::open_mode | (std::ios_base::trunc | std::ios_base::out)],
            args[keywords::rotation_size | (std::numeric_limits< uintmax_t >::max)()],
            args[keywords::rotation_interval | (std::numeric_limits< unsigned int >::max)()],
            args[keywords::auto_flush | false]);
    }
    //! Constructor implementation
    BOOST_LOG_EXPORT void construct(
        path_type const& pattern,
        std::ios_base::openmode mode,
        uintmax_t rotation_size,
        unsigned int rotation_interval,
        bool auto_flush);

    //! The method writes the message to the sink
    BOOST_LOG_EXPORT void do_consume(record_type const& record, target_string_type const& formatted_message);

    //! The method sets file name mask
    BOOST_LOG_EXPORT void set_file_name_pattern_internal(path_type const& pattern);

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
            boost::log::aux::cleanup_guard< streambuf_type > cleanup2(m_StreamBuf);
            boost::log::aux::cleanup_guard< result_type::string_type > cleanup3(m_FileName);

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
    //! Path type that is used by Boost.Log
    typedef boost::log::aux::universal_path path_type;

    //! File name composer functor type
    typedef function1< path_type, record_type const& > file_name_composer_type;

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
