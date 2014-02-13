#pragma once

#ifndef TICKER_H_
#define TICKER_H_

//Easy way: Define as Win2k
#define _WIN32_WINNT 0x0500
//Precise way: cry out
#if (_WIN32_WINNT < 0x0400)
#error "Only support OS newer than NT4.0(0x0400)"
#endif

#include <windows.h>

#define TICKER_OTPT_MT //One-Ticker-Per-Thread, multithread support

/**
@file ticker.h
@author Cheng Kuan
@date 2008-6-20
*/

//A bridge class between static(class) hierarchical struct and dynamic(executing) hierarchical struct
//class ticker also act as a user-mode thread scheduler

/**
@class ticker
@brief 一个自动产生中间状态的执行器

子类继承ticker类后，在perform函数中重载需要执行的逻辑，外部通过不断调用tick完成perform的执行。在tick未返回结束状态前，外部如果不再调用tick函数，需要以tick(true)指示执行器进行清理工作，其中包括安全的清理perform中残余状态并调用cleanup方法。
*/
class ticker
{
public:
	ticker(void);
	virtual ~ticker(void);
public:
	/** 
	@fn bool tick(bool stop=false)
	外部通过此接口执行实际的perform函数内的逻辑
	@param stop
	- false: 继续执行perform()
	- true: 强制停止perform的执行。以此参数调用时返回值一定为true。
	@retval 执行器是否执行结束
	- false: 执行器执行未结束，需要继续调用。
	- true: 执行器执行结束，执行器复位为初始状态。
	*/
	bool tick(bool stop=false);

protected:
	/** 
	@fn void yield();
	子类重载的perform函数中，如果需要保存状态并使当前tick调用返回时使用此函数
	@note 此函数可能会抛出内部异常
	*/
	void yield();
private:
	/** 
	@fn void perform()
	具体的执行逻辑，子类应该重载此方法。
	@note 此函数内不可以用catch(...)俘获所有异常，因为yield()函数可能会抛出内部异常与perform函数外通信。
	@sa yield()
	*/
	virtual void perform() = 0;
	/** 
	@fn void cleanup()
	执行器被要求停止执行perform函数后，先清理perform函数内状态，然后调用此函数对象申请的释放资源
	*/
	virtual void cleanup() {};

	static VOID CALLBACK _perform_proc(PVOID lpParameter);
	friend void ticker::_perform_proc(PVOID lpParameter);
	enum {
		NULL_STATUS,
		WORKING_STATUS,
		CLEANUP_STATUS,
	} m_status;
	LPVOID m_scheduler_fiber;
	LPVOID m_performer_fiber;
#ifdef TICKER_OTPT_MT
	DWORD g_ticker_tlsidx;
#endif

};

/////ticker coupling including:
//Low Speed Low Reliability
//Low Speed High Reliability
//High Speed Low Reliability
//High Speed High Reliability --TODO
//////////////////////////////

//TODO:
//Filter component  ->[Filter]->
//Layer component (coupled filters) <=>[Layer]<=>

#endif //TICKER_H_