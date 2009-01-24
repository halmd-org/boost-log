/*!
 * (C) 2009 Andrey Semashev
 *
 * Use, modification and distribution is subject to the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * \file   attr_attribute_set.cpp
 * \author Andrey Semashev
 * \date   24.01.2009
 *
 * \brief  This header contains tests for the attribute set class.
 */

#define BOOST_TEST_MODULE attr_attribute_set

#include <vector>
#include <string>
#include <utility>
#include <iterator>
#include <boost/shared_ptr.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/log/attributes/constant.hpp>
#include <boost/log/attributes/attribute_set.hpp>
#include <boost/log/utility/slim_string.hpp>

namespace mpl = boost::mpl;
namespace logging = boost::log;
namespace attrs = logging::attributes;

namespace {

    typedef mpl::vector<
    #ifdef BOOST_LOG_USE_CHAR
        char
    #endif
    #if defined(BOOST_LOG_USE_CHAR) && defined(BOOST_LOG_USE_WCHAR_T)
        ,
    #endif
    #ifdef BOOST_LOG_USE_WCHAR_T
        wchar_t
    #endif
    >::type char_types;

    template< typename >
    struct test_data;

    template< >
    struct test_data< char >
    {
        static const char* attr1() { return "attr1"; }
        static const char* attr2() { return "attr2"; }
        static const char* attr3() { return "attr3"; }
    };

    template< >
    struct test_data< wchar_t >
    {
        static const wchar_t* attr1() { return L"attr1"; }
        static const wchar_t* attr2() { return L"attr2"; }
        static const wchar_t* attr3() { return L"attr3"; }
    };

} // namespace

// The test checks construction and assignment
BOOST_AUTO_TEST_CASE_TEMPLATE(construction, CharT, char_types)
{
    typedef logging::basic_attribute_set< CharT > attr_set;
    typedef test_data< CharT > data;

    boost::shared_ptr< logging::attribute > attr1(new attrs::constant< int >(10));
    boost::shared_ptr< logging::attribute > attr2(new attrs::constant< double >(5.5));
    boost::shared_ptr< logging::attribute > attr3(new attrs::constant< std::string >("Hello, world!"));

    attr_set set1;
    BOOST_CHECK(set1.empty());
    BOOST_CHECK_EQUAL(set1.size(), 0);

    attr_set set2 = set1;
    BOOST_CHECK(set2.empty());
    BOOST_CHECK_EQUAL(set2.size(), 0);

    set2[data::attr1()] = attr1;
    set2[data::attr2()] = attr2;
    BOOST_CHECK(set1.empty());
    BOOST_CHECK_EQUAL(set1.size(), 0);
    BOOST_CHECK(!set2.empty());
    BOOST_CHECK_EQUAL(set2.size(), 2);

    attr_set set3 = set2;
    BOOST_CHECK(!set3.empty());
    BOOST_CHECK_EQUAL(set3.size(), 2);
    BOOST_CHECK_EQUAL(set3.count(data::attr1()), 1);
    BOOST_CHECK_EQUAL(set3.count(data::attr2()), 1);
    BOOST_CHECK_EQUAL(set3.count(data::attr3()), 0);

    set1[data::attr3()] = attr3;
    BOOST_CHECK(!set1.empty());
    BOOST_CHECK_EQUAL(set1.size(), 1);
    BOOST_CHECK_EQUAL(set1.count(data::attr3()), 1);

    set2 = set1;
    BOOST_REQUIRE_EQUAL(set1.size(), set2.size());
    BOOST_CHECK(std::equal(set1.begin(), set1.end(), set2.begin()));
}

// The test checks lookup methods
BOOST_AUTO_TEST_CASE_TEMPLATE(lookup, CharT, char_types)
{
    typedef logging::basic_attribute_set< CharT > attr_set;
    typedef test_data< CharT > data;
    typedef std::basic_string< CharT > string;
    typedef logging::basic_slim_string< CharT > slim_string;

    boost::shared_ptr< logging::attribute > attr1(new attrs::constant< int >(10));
    boost::shared_ptr< logging::attribute > attr2(new attrs::constant< double >(5.5));

    attr_set set1;
    set1[data::attr1()] = attr1;
    set1[data::attr2()] = attr2;

    // Traditional find methods
    typename attr_set::iterator it = set1.find(data::attr1());
    BOOST_CHECK(it != set1.end());
    BOOST_CHECK_EQUAL(it->second, attr1);

    string s1 = data::attr2();
    it = set1.find(s1);
    BOOST_CHECK(it != set1.end());
    BOOST_CHECK_EQUAL(it->second, attr2);

    slim_string ss1 = data::attr1();
    it = set1.find(ss1);
    BOOST_CHECK(it != set1.end());
    BOOST_CHECK_EQUAL(it->second, attr1);

    it = set1.find(data::attr3());
    BOOST_CHECK(it == set1.end());

    // Subscript operator
    boost::shared_ptr< logging::attribute > p = set1[data::attr1()];
    BOOST_CHECK_EQUAL(p, attr1);
    BOOST_CHECK_EQUAL(set1.size(), 2);

    p = set1[s1];
    BOOST_CHECK_EQUAL(p, attr2);
    BOOST_CHECK_EQUAL(set1.size(), 2);

    p = set1[ss1];
    BOOST_CHECK_EQUAL(p, attr1);
    BOOST_CHECK_EQUAL(set1.size(), 2);

    p = set1[data::attr3()];
    BOOST_CHECK(!p);
    BOOST_CHECK_EQUAL(set1.size(), 2);

    // Counting elements
    BOOST_CHECK_EQUAL(set1.count(data::attr1()), 1);
    BOOST_CHECK_EQUAL(set1.count(s1), 1);
    BOOST_CHECK_EQUAL(set1.count(ss1), 1);
    BOOST_CHECK_EQUAL(set1.count(data::attr3()), 0);
}

// The test checks insertion methods
BOOST_AUTO_TEST_CASE_TEMPLATE(insertion, CharT, char_types)
{
    typedef logging::basic_attribute_set< CharT > attr_set;
    typedef test_data< CharT > data;
    typedef std::basic_string< CharT > string;
    typedef logging::basic_slim_string< CharT > slim_string;

    boost::shared_ptr< logging::attribute > attr1(new attrs::constant< int >(10));
    boost::shared_ptr< logging::attribute > attr2(new attrs::constant< double >(5.5));
    boost::shared_ptr< logging::attribute > attr3(new attrs::constant< std::string >("Hello, world!"));

    attr_set set1;

    // Traditional insert methods
    std::pair< typename attr_set::iterator, bool > res = set1.insert(data::attr1(), attr1);
    BOOST_CHECK(res.second);
    BOOST_CHECK(res.first != set1.end());
    BOOST_CHECK(res.first->first == data::attr1());
    BOOST_CHECK_EQUAL(res.first->second, attr1);
    BOOST_CHECK(!set1.empty());
    BOOST_CHECK_EQUAL(set1.size(), 1);

    res = set1.insert(std::make_pair(typename attr_set::key_type(data::attr2()), attr2));
    BOOST_CHECK(res.second);
    BOOST_CHECK(res.first != set1.end());
    BOOST_CHECK(res.first->first == data::attr2());
    BOOST_CHECK_EQUAL(res.first->second, attr2);
    BOOST_CHECK(!set1.empty());
    BOOST_CHECK_EQUAL(set1.size(), 2);

    // Insertion attempt of an attribute with the name of an already existing attribute
    res = set1.insert(std::make_pair(typename attr_set::key_type(data::attr2()), attr3));
    BOOST_CHECK(!res.second);
    BOOST_CHECK(res.first != set1.end());
    BOOST_CHECK(res.first->first == data::attr2());
    BOOST_CHECK_EQUAL(res.first->second, attr2);
    BOOST_CHECK(!set1.empty());
    BOOST_CHECK_EQUAL(set1.size(), 2);

    // Mass insertion
    std::vector< typename attr_set::value_type > elems;
    elems.push_back(std::make_pair(typename attr_set::key_type(data::attr2()), attr2));
    elems.push_back(std::make_pair(typename attr_set::key_type(data::attr1()), attr1));
    elems.push_back(std::make_pair(typename attr_set::key_type(data::attr3()), attr3));
    // ... with element duplication
    elems.push_back(std::make_pair(typename attr_set::key_type(data::attr1()), attr3));

    attr_set set2;
    set2.insert(elems.begin(), elems.end());
    BOOST_CHECK(!set2.empty());
    BOOST_REQUIRE_EQUAL(set2.size(), 3);
    typename attr_set::const_iterator it = set2.begin();
    BOOST_CHECK(it->first == data::attr1());
    BOOST_CHECK_EQUAL(it->second, attr1);
    ++it;
    BOOST_CHECK(it->first == data::attr2());
    BOOST_CHECK_EQUAL(it->second, attr2);
    ++it;
    BOOST_CHECK(it->first == data::attr3());
    BOOST_CHECK_EQUAL(it->second, attr3);
    ++it;
    BOOST_CHECK(it == set2.end());

    // The same, but with insertion results collection
    std::vector< std::pair< typename attr_set::iterator, bool > > results;

    attr_set set3;
    set3.insert(elems.begin(), elems.end(), std::back_inserter(results));
    BOOST_REQUIRE_EQUAL(results.size(), elems.size());
    BOOST_CHECK(!set3.empty());
    BOOST_REQUIRE_EQUAL(set3.size(), 3);
    it = set3.begin();
    BOOST_CHECK(it->first == data::attr1());
    BOOST_CHECK_EQUAL(it->second, attr1);
    BOOST_CHECK(it == results[1].first);
    ++it;
    BOOST_CHECK(it->first == data::attr2());
    BOOST_CHECK_EQUAL(it->second, attr2);
    BOOST_CHECK(it == results[0].first);
    ++it;
    BOOST_CHECK(it->first == data::attr3());
    BOOST_CHECK_EQUAL(it->second, attr3);
    BOOST_CHECK(it == results[2].first);
    ++it;
    BOOST_CHECK(it == set3.end());

    BOOST_CHECK(results[0].second);
    BOOST_CHECK(results[1].second);
    BOOST_CHECK(results[2].second);
    BOOST_CHECK(!results[3].second);

    // Subscript operator
    attr_set set4;

    boost::shared_ptr< logging::attribute >& p1 = (set4[data::attr1()] = attr1);
    BOOST_CHECK_EQUAL(set4.size(), 1);
    BOOST_CHECK_EQUAL(p1, attr1);

    boost::shared_ptr< logging::attribute >& p2 = (set4[string(data::attr2())] = attr2);
    BOOST_CHECK_EQUAL(set4.size(), 2);
    BOOST_CHECK_EQUAL(p2, attr2);

    boost::shared_ptr< logging::attribute >& p3 = (set4[slim_string(data::attr3())] = attr3);
    BOOST_CHECK_EQUAL(set4.size(), 3);
    BOOST_CHECK_EQUAL(p3, attr3);

    // subscript operator can replace existing elements
    boost::shared_ptr< logging::attribute >& p4 = (set4[data::attr3()] = attr1);
    BOOST_CHECK_EQUAL(set4.size(), 3);
    BOOST_CHECK_EQUAL(p4, attr1);
}

// The test checks erasure methods
BOOST_AUTO_TEST_CASE_TEMPLATE(erasure, CharT, char_types)
{
    typedef logging::basic_attribute_set< CharT > attr_set;
    typedef test_data< CharT > data;
    typedef std::basic_string< CharT > string;
    typedef logging::basic_slim_string< CharT > slim_string;

    boost::shared_ptr< logging::attribute > attr1(new attrs::constant< int >(10));
    boost::shared_ptr< logging::attribute > attr2(new attrs::constant< double >(5.5));
    boost::shared_ptr< logging::attribute > attr3(new attrs::constant< std::string >("Hello, world!"));

    attr_set set1;
    set1[data::attr1()] = attr1;
    set1[data::attr2()] = attr2;
    set1[data::attr3()] = attr3;

    attr_set set2 = set1;
    BOOST_REQUIRE_EQUAL(set2.size(), 3);

    BOOST_CHECK_EQUAL(set2.erase(data::attr1()), 1);
    BOOST_CHECK_EQUAL(set2.size(), 2);
    BOOST_CHECK_EQUAL(set2.count(data::attr1()), 0);

    BOOST_CHECK_EQUAL(set2.erase(data::attr1()), 0);
    BOOST_CHECK_EQUAL(set2.size(), 2);

    set2.erase(set2.begin());
    BOOST_CHECK_EQUAL(set2.size(), 1);
    BOOST_CHECK_EQUAL(set2.count(data::attr2()), 0);

    set2 = set1;
    BOOST_REQUIRE_EQUAL(set2.size(), 3);

    typename attr_set::iterator it = set2.begin();
    set2.erase(++it, set2.end());
    BOOST_CHECK_EQUAL(set2.size(), 1);
    BOOST_CHECK_EQUAL(set2.count(data::attr1()), 1);
    BOOST_CHECK_EQUAL(set2.count(data::attr2()), 0);
    BOOST_CHECK_EQUAL(set2.count(data::attr3()), 0);

    set2 = set1;
    BOOST_REQUIRE_EQUAL(set2.size(), 3);

    set2.clear();
    BOOST_CHECK(set2.empty());
    BOOST_CHECK_EQUAL(set2.size(), 0);
}
