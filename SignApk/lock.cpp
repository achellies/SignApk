#include "stdafx.h"
#include "lock.hpp"


///////////// CRITICAL_SECTION wrapper /////////////////
CriticalSection::CriticalSection()
{
	::InitializeCriticalSection(&m_criticalsection);
}

CriticalSection::~CriticalSection()
{
	::DeleteCriticalSection(&m_criticalsection);
}

void CriticalSection::Lock()
{
	::EnterCriticalSection(&m_criticalsection);
}

void CriticalSection::Unlock()
{
	::LeaveCriticalSection(&m_criticalsection);
}

AutoCriticalSection::AutoCriticalSection(CriticalSection& criticalsection)
	:m_criticalsection(criticalsection)
{
	m_criticalsection.Lock();
}

AutoCriticalSection::~AutoCriticalSection()
{
	m_criticalsection.Unlock();
}

CMutex::CMutex(LPCTSTR lpszName)
{
	m_hMutex = CreateMutex(NULL,FALSE,lpszName);
}

///////////// MUTEX wrapper /////////////////
CMutex::~CMutex()
{
	if (m_hMutex != NULL)
	{
		CloseHandle(m_hMutex);
	}
}

void CMutex::Lock()
{
	if (m_hMutex != NULL)
	{
		DWORD dwWaitResult = WaitForSingleObject(m_hMutex,INFINITE);
        switch (dwWaitResult) 
        {
		// The thread got ownership of the mutex
		case WAIT_OBJECT_0:
			break;
		// The thread got ownership of an abandoned mutex
		case WAIT_ABANDONED:
			break;
        }
	}
}
void CMutex::Unlock()
{
	if (m_hMutex != NULL)
	{
		ReleaseMutex(m_hMutex);
	}
}

CAutoMutex::CAutoMutex(CMutex& mutex)
	:m_mutex(mutex)
{
	m_mutex.Lock();
}

CAutoMutex::~CAutoMutex()
{
	m_mutex.Unlock();
}

///////////// EVENT wrapper /////////////////
CEvent::CEvent(LPCTSTR lpszName,BOOL bManualReset,BOOL bInitialState)
{
	m_hEvent = CreateEvent(NULL,bManualReset,bInitialState,lpszName);
}

CEvent::~CEvent()
{
	if (m_hEvent != NULL)
	{
		CloseHandle(m_hEvent);
	}
}

bool CEvent::IsLocked()
{
	bool IsLocked = false;
	if (m_hEvent != NULL)
	{
		DWORD dwWaitResult = WaitForSingleObject(m_hEvent,1);
        switch (dwWaitResult) 
        {
		// The thread got ownership of the mutex
		case WAIT_OBJECT_0:
			break;
		// The thread got ownership of an abandoned mutex
		case WAIT_ABANDONED:
			break;
		case WAIT_TIMEOUT:
			IsLocked = true;
			break;
        }
	}
	return IsLocked;
}

void CEvent::Lock()
{
	if (m_hEvent != NULL)
	{
		DWORD dwWaitResult = WaitForSingleObject(m_hEvent,INFINITE);
        switch (dwWaitResult) 
        {
		// The thread got ownership of the mutex
		case WAIT_OBJECT_0:
			break;
		// The thread got ownership of an abandoned mutex
		case WAIT_ABANDONED:
			break;
        }
		ResetEvent(m_hEvent);
	}
}
void CEvent::Unlock()
{
	if (m_hEvent != NULL)
	{
		SetEvent(m_hEvent);
	}
}

void CEvent::Reset()
{
	if (m_hEvent != NULL)
	{
		ResetEvent(m_hEvent);
	}
}

void CEvent::Set()
{
	if (m_hEvent != NULL)
	{
		SetEvent(m_hEvent);
	}
}

CAutoEvent::CAutoEvent(CEvent& e)
	:m_event(e)
{
	m_event.Lock();
}

CAutoEvent::~CAutoEvent()
{
	m_event.Unlock();
}

CManualEvent::CManualEvent(CEvent& e)
	:m_event(e)
{}

CManualEvent::~CManualEvent()
{
	m_event.Unlock();
}

void CManualEvent::Lock()
{
	m_event.Lock();
}

void CManualEvent::Unlock()
{
	m_event.Unlock();
}