
#pragma warning(disable:4819)
#include <boost/test/auto_unit_test.hpp>

#include "ticker.h"
#include "abstracer.h"

class naive_ticker
	:public ticker
{
public:
	void perform();
};

void naive_ticker::perform()
{
	PVOID d = NULL;
	PVOID f = NULL;
	d = GetFiberData();
	f = GetCurrentFiber();
//	printf("naive_ticker::perform() 1: f= 0x%x, d= 0x%x\n", f, d);
	yield();
//	printf("naive_ticker::perform() 2: f= 0x%x, d= 0x%x\n", f, d);
}

BOOST_AUTO_TEST_CASE(test_naive_ticker_stop)
{
	naive_ticker t;
	bool finished = false;
	//Init tick stop
	finished = t.tick(true);
	BOOST_CHECK_EQUAL(finished, true);
};

BOOST_AUTO_TEST_CASE(test_naive_ticker)
{
	naive_ticker t;
	bool finished = false;
	//Init tick
	finished = t.tick();
	BOOST_CHECK_EQUAL(finished, false);
	//yield_return
	finished = t.tick();
	BOOST_CHECK_EQUAL(finished, false);
	//perform_done
	finished = t.tick();
	BOOST_CHECK_EQUAL(finished, false);
	//Cleanup tick
	finished = t.tick();
	BOOST_CHECK_EQUAL(finished, true);
};

class loop_ticker
	:public ticker
{
public:
	void perform();
	int m_counter;
};

void loop_ticker::perform()
{
	for (m_counter=0; m_counter<3; m_counter++) {
		yield();
	}
}

BOOST_AUTO_TEST_CASE(test_loop_ticker)
{
	loop_ticker t;
	bool finished = false;
	finished = t.tick();
	//Init tick
	BOOST_CHECK_EQUAL(finished, false);
	for (int i=0; i<3; i++)
	{
		//yield_return
		finished = t.tick();
		BOOST_CHECK_EQUAL(finished, false);
		BOOST_CHECK_EQUAL(i, t.m_counter);
	}
	//perform_done
	finished = t.tick();
	BOOST_CHECK_EQUAL(finished, false);
	//Cleanup tick
	finished = t.tick();
	BOOST_CHECK_EQUAL(finished, true);
}

class resource_ticker
	:public ticker
{
public:
	char* m_pointer;
	bool m_not_here;
	resource_ticker() {
		m_pointer=NULL;
	}
	void cleanup() {
		delete[] m_pointer;
		m_pointer = NULL;
	}
	void perform() {
		m_not_here = true;
		m_pointer = new char[12345];
		yield();
		std::vector<char> v(54321);
		yield();
		m_not_here = false;
		std::vector<char> v2(11111);
		v2 = v;
	}
};
BOOST_AUTO_TEST_CASE(test_resource_ticker)
{
	resource_ticker t;
	bool finished = false;
	//Init tick
	finished = t.tick();
	BOOST_CHECK_EQUAL(finished, false);
	//new tick
	finished = t.tick();
	BOOST_CHECK_EQUAL(finished, false);
	BOOST_CHECK(t.m_pointer);
	//stop tick
	finished = t.tick(true);
	BOOST_CHECK_EQUAL(finished, true);
	BOOST_CHECK(t.m_pointer == NULL);
	BOOST_CHECK(t.m_not_here);
}

class resourcerelease_ticker
	:public ticker
{
public:
	bool m_not_here;
	void perform() {
		m_not_here = true;
		std::vector<char> v(54321);
		yield();
		yield();
		yield();
		m_not_here = false;
		std::vector<char> v2(11111);
		v2 = v;
	}
};
BOOST_AUTO_TEST_CASE(test_resourcerelease_ticker)
{
	resourcerelease_ticker t;
	bool finished = false;
	//Init tick
	finished = t.tick();
	BOOST_CHECK_EQUAL(finished, false);
	//vector v tick
	finished = t.tick();
	BOOST_CHECK_EQUAL(finished, false);
	//stop tick
	finished = t.tick(true);
	BOOST_CHECK_EQUAL(finished, true);
	BOOST_CHECK(t.m_not_here);
}

class sleep_loop_tracer
	:public abstracer
{
public:
	sleep_loop_tracer() {m_counter = 0;}
	void perform();
	int m_counter;
};

void sleep_loop_tracer::perform()
{
	for (m_counter=0; m_counter<3; m_counter++) {
		wait(1000);
	}
	yield();
}

BOOST_AUTO_TEST_CASE(test_sleep_loop_tracer)
{
	sleep_loop_tracer t;
	bool finished = false;
	finished = t.tick();
	//Init tick
	BOOST_CHECK_EQUAL(finished, false);
	for (int i=0; i<3; i++)
	{
		//yield_return
		do {
			finished = t.tick();
		} while(t.m_counter==i);
		BOOST_CHECK_EQUAL(finished, false);
	}
	//perform_done
	finished = t.tick();
	BOOST_CHECK_EQUAL(finished, false);

	//Cleanup tick
	finished = t.tick();
	BOOST_CHECK_EQUAL(finished, true);
}

class logger_tracer
	:public abstracer
{
public:
	void perform();
};

void logger_tracer::perform()
{
	logmsg(std::string("log report"), LL_REPORT);
	logmsg(std::string("log debug3"), LL_DEBUG3);
	logmsg(std::string("log critical"), LL_CRITICAL);
}

BOOST_AUTO_TEST_CASE(test_logger_tracer)
{
	logger_tracer t;
	bool finished = false;
	bool haslog = false;
	log_level loglevel = LL_YAP;
	std::string logmsg;
	//Init tick
	finished = t.tick();
	BOOST_CHECK_EQUAL(finished, false);
	haslog = t.peek_logmsg(logmsg, loglevel);
	BOOST_CHECK_EQUAL(haslog, false);

	finished = t.tick();
	BOOST_CHECK_EQUAL(finished, false);
	haslog = t.peek_logmsg(logmsg, loglevel);
	BOOST_CHECK_EQUAL(haslog, true);
	BOOST_CHECK_EQUAL(logmsg, std::string("log report"));
	BOOST_CHECK_EQUAL(loglevel, LL_REPORT);

	finished = t.tick();
	BOOST_CHECK_EQUAL(finished, false);
	haslog = t.peek_logmsg(logmsg, loglevel);
	BOOST_CHECK_EQUAL(haslog, true);
	BOOST_CHECK_EQUAL(logmsg, std::string("log debug3"));
	BOOST_CHECK_EQUAL(loglevel, LL_DEBUG3);

	finished = t.tick();
	BOOST_CHECK_EQUAL(finished, false);
	haslog = t.peek_logmsg(logmsg, loglevel);
	BOOST_CHECK_EQUAL(haslog, true);
	BOOST_CHECK_EQUAL(logmsg, std::string("log critical"));
	BOOST_CHECK_EQUAL(loglevel, LL_CRITICAL);

	//perform_done
	finished = t.tick();
	BOOST_CHECK_EQUAL(finished, false);
	haslog = t.peek_logmsg(logmsg, loglevel);
	BOOST_CHECK_EQUAL(haslog, false);

	//Cleanup tick
	finished = t.tick();
	BOOST_CHECK_EQUAL(finished, true);
}

class big_ticker
	:public ticker
{
public:
	void perform();
	void cleanup();
	naive_ticker t1;
	//naive_ticker t2;
};

void big_ticker::perform() {
	//PVOID d = NULL;
	//PVOID f = NULL;
	for(int i=0; i>=0; i++) {

		//printf("big_ticker::perform() 0: i=%d\n", i);
		//d = GetFiberData();
		//f = GetCurrentFiber();
		//printf("big_ticker::perform() 1: f= 0x%x, d= 0x%x\n", f, d);

		//printf("big_ticker::perform(): >t1.tick()\n");
		t1.tick();
		//t2.tick();
		//printf("big_ticker::perform(): <t1.tick()\n");
		
		//d = GetFiberData();
		//f = GetCurrentFiber();
		//printf("big_ticker::perform() 2: f= 0x%x, d= 0x%x\n", f, d);

		//printf("big_ticker::perform(): >yield()\n");
		yield();
		//printf("big_ticker::perform(): <yield()\n");

		//d = GetFiberData();
		//f = GetCurrentFiber();
		//printf("big_ticker::perform() 3: f= 0x%x, d= 0x%x\n", f, d);
	}
}
void big_ticker::cleanup() {
	t1.tick(true);
	//t2.tick(true);
}

BOOST_AUTO_TEST_CASE(test_ticker_in_ticker)
{
	//PVOID d=NULL;
	//PVOID f=NULL;
	big_ticker bt;
	
	//printf("test 0: bigt= 0x%x, t1= 0x%x\n", &bt, &bt.t1);
	//d = GetFiberData();
	//f = GetCurrentFiber();
	//printf("test 1: f= 0x%x, d= 0x%x\n", f, d);

	//printf("test: >bt.tick()\n");
	bt.tick();
	//printf("test: <bt.tick()\n");

	//d = GetFiberData();
	//f = GetCurrentFiber();
	//printf("test 2: f= 0x%x, d= 0x%x\n", f, d);

	//printf("test: >bt.tick()\n");
	bt.tick();
	//printf("test: <bt.tick()\n");

	//d = GetFiberData();
	//f = GetCurrentFiber();
	//printf("test 3: f= 0x%x, d= 0x%x\n", f, d);

	bt.tick(true);
}
