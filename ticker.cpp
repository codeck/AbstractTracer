#include "ticker.h"

#include <stdio.h>

#include <assert.h>
#ifdef  __cplusplus
extern "C" {
#endif
	_CRTIMP void __cdecl _wassert(__in_z const wchar_t * _Message, __in_z const wchar_t *_File, __in unsigned _Line);
#ifdef  __cplusplus
}
#endif
#define release_assert(_Expression) (void)( (!!(_Expression)) || (_wassert(_CRT_WIDE(#_Expression), _CRT_WIDE(__FILE__), __LINE__), 0) )

#ifndef TICKER_OTPT_MT
static DWORD g_ticker_tlsidx = TLS_OUT_OF_INDEXES;
#endif

typedef struct {
} stop_indicate_exception_t;

ticker::ticker(void)
{
#ifdef TICKER_OTPT_MT
	 g_ticker_tlsidx = TLS_OUT_OF_INDEXES;
#endif
	if (g_ticker_tlsidx == TLS_OUT_OF_INDEXES) {
		g_ticker_tlsidx = TlsAlloc();
		TlsSetValue(g_ticker_tlsidx, LPVOID(this));
	}
	m_status = NULL_STATUS;
}

ticker::~ticker(void)
{
	release_assert(m_status == NULL_STATUS && "ticker not properly stoped");
	if (g_ticker_tlsidx != TLS_OUT_OF_INDEXES) {
		ticker* root = (ticker*)(TlsGetValue(g_ticker_tlsidx));
		if (root == this) {
			BOOL ret = TlsFree(g_ticker_tlsidx);
			g_ticker_tlsidx = TLS_OUT_OF_INDEXES;
		}
	}
}

VOID CALLBACK ticker::_perform_proc( PVOID lpParameter )
{
	ticker *self = static_cast<ticker*>(lpParameter);
	assert(self && "ticker point invalid");
	try {
		self->perform();
	} catch (stop_indicate_exception_t& e) {
		e;
	}
	self->m_status = ticker::CLEANUP_STATUS;
	SwitchToFiber(self->m_scheduler_fiber);
}

bool ticker::tick(bool stop)
{
	if (stop) {
		if (m_status == ticker::NULL_STATUS) return true;
		m_status = ticker::CLEANUP_STATUS;
		SwitchToFiber(m_performer_fiber);
	}
	bool finished = false;
	switch (m_status) {
			case NULL_STATUS:
				if (TlsGetValue(g_ticker_tlsidx) == this) {
					ConvertThreadToFiber(NULL);
				}
				m_scheduler_fiber = GetCurrentFiber();
				m_performer_fiber = CreateFiber(0, ticker::_perform_proc, this);
				m_status = WORKING_STATUS;
				break;
			case WORKING_STATUS:
				SwitchToFiber(m_performer_fiber);
				break;
			case CLEANUP_STATUS:
				cleanup();
				DeleteFiber(m_performer_fiber);
				m_status = NULL_STATUS;
				finished = true;
				break;
			default:
				break;
	}
	return finished;
};

void ticker::yield()
{
	SwitchToFiber(m_scheduler_fiber);
	if (m_status == CLEANUP_STATUS)	{
		throw stop_indicate_exception_t();
	}
}
