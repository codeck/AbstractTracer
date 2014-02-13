#include "abstracer.h"

#include <assert.h>

//A windows implementation of posix gettimeofday() from GroupsockHelper.cpp in live555 project
static int gettimeofday(struct timeval* tp, int* /*tz*/) {
	LARGE_INTEGER tickNow;
	static LARGE_INTEGER tickFrequency;
	static BOOL tickFrequencySet = FALSE;
	if (tickFrequencySet == FALSE) {
		QueryPerformanceFrequency(&tickFrequency);
		tickFrequencySet = TRUE;
	}
	QueryPerformanceCounter(&tickNow);
	tp->tv_sec = (long) (tickNow.QuadPart / tickFrequency.QuadPart);
	tp->tv_usec = (long) (((tickNow.QuadPart % tickFrequency.QuadPart) * 1000000L) / tickFrequency.QuadPart);
	return 0;
}

abstracer::abstracer(void)
{
	has_msg = false;
	has_frame = false;
	m_loglevel = LL_REPORT;
}

abstracer::~abstracer(void)
{
}

void abstracer::wait( int millisec )
{
	timeval timer_start;
	gettimeofday(&timer_start, NULL);
	yield();
	for(;;){
		timeval now;
		gettimeofday(&now, NULL);
		int msdiff = (int)(((double)now.tv_sec - timer_start.tv_sec)*1000.+((double)now.tv_usec - timer_start.tv_usec)/1000.0);
		if (msdiff < millisec) {
			Sleep(1);
			yield();
		}
		else {
			break;
		}
	}
}

void abstracer::wait()
{
	yield();
}

bool abstracer::wait( HANDLE hevent, int timeout_millisec )
{
	timeval timer_start;
	gettimeofday(&timer_start, NULL);
	yield();
	for(;;){
		timeval now;
		gettimeofday(&now, NULL);
		int msdiff = (int)(((double)now.tv_sec - timer_start.tv_sec)*1000.+((double)now.tv_usec - timer_start.tv_usec)/1000.0);
		if (msdiff < timeout_millisec) {
			if (WaitForSingleObject(hevent, 1) == WAIT_OBJECT_0) {
				return true;
			}
			yield();
		}
		else {
			return false;
		}
	}
}

bool abstracer::peek_logmsg( std::string& msg, log_level& ll )
{
	if (has_msg) {
		msg = m_logmsg;
		ll = m_loglevel;
		has_msg = false;
		return true;
	}
	else {
		return false;
	}
}

void abstracer::logmsg( std::string& msg, log_level ll )
{
	assert(has_msg == false && "logmsg unpeeked");
	m_logmsg = msg;
	m_loglevel = ll;
	has_msg = true;
	yield();
}