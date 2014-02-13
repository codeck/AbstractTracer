#pragma warning(disable:4819)
//Only need defined once in whole project
#define BOOST_TEST_MODULE AbstractTracer
//Only need defined once in whole project
#define BOOST_AUTO_TEST_MAIN ...
#include <boost/test/auto_unit_test.hpp>

#include <string>
using std::string;

BOOST_AUTO_TEST_CASE( test1 )
{
	BOOST_CHECK_EQUAL(0,0);
}

BOOST_AUTO_TEST_CASE( test2 )
{
	BOOST_CHECK_EQUAL(string("yes"), string("yes"));
}

BOOST_AUTO_TEST_CASE( this_test_must_be_fail )
{
	BOOST_CHECK_EQUAL(true, false);
}
