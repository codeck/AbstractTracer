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
@brief һ���Զ������м�״̬��ִ����

����̳�ticker�����perform������������Ҫִ�е��߼����ⲿͨ�����ϵ���tick���perform��ִ�С���tickδ���ؽ���״̬ǰ���ⲿ������ٵ���tick��������Ҫ��tick(true)ָʾִ�������������������а�����ȫ������perform�в���״̬������cleanup������
*/
class ticker
{
public:
	ticker(void);
	virtual ~ticker(void);
public:
	/** 
	@fn bool tick(bool stop=false)
	�ⲿͨ���˽ӿ�ִ��ʵ�ʵ�perform�����ڵ��߼�
	@param stop
	- false: ����ִ��perform()
	- true: ǿ��ֹͣperform��ִ�С��Դ˲�������ʱ����ֵһ��Ϊtrue��
	@retval ִ�����Ƿ�ִ�н���
	- false: ִ����ִ��δ��������Ҫ�������á�
	- true: ִ����ִ�н�����ִ������λΪ��ʼ״̬��
	*/
	bool tick(bool stop=false);

protected:
	/** 
	@fn void yield();
	�������ص�perform�����У������Ҫ����״̬��ʹ��ǰtick���÷���ʱʹ�ô˺���
	@note �˺������ܻ��׳��ڲ��쳣
	*/
	void yield();
private:
	/** 
	@fn void perform()
	�����ִ���߼�������Ӧ�����ش˷�����
	@note �˺����ڲ�������catch(...)���������쳣����Ϊyield()�������ܻ��׳��ڲ��쳣��perform������ͨ�š�
	@sa yield()
	*/
	virtual void perform() = 0;
	/** 
	@fn void cleanup()
	ִ������Ҫ��ִֹͣ��perform������������perform������״̬��Ȼ����ô˺�������������ͷ���Դ
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