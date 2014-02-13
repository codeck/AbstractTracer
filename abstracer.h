#pragma once
#ifndef ABSTRACER_H_
#define ABSTRACER_H_

#include "ticker.h"
#include "log_level.h"
#include <string>

class abstracer :
	public ticker
{
public:
	abstracer(void);
	virtual ~abstracer(void);

	bool peek_logmsg(std::string& msg, log_level& ll);
	bool peek_logframe(/*frametype& frame*/) {/*TODO*/return false;}

protected:
	void wait();
	void wait(int millisec);
	bool wait(HANDLE hevent, int timeout_millisec);

	void logmsg(std::string& msg, log_level ll);
private:
	bool has_msg;
	log_level m_loglevel;
	std::string m_logmsg;
	bool has_frame;
	/*frametype m_logframe;*/
};

#endif //ABSTRACER_H_