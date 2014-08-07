#ifndef _LOCK_HPP
#define _LOCK_HPP

class AutoCriticalSection;
class CriticalSection
{
	friend AutoCriticalSection;
public:
	CriticalSection();
	~CriticalSection();
protected:
	void Lock();
	void Unlock();
private:
	CRITICAL_SECTION    m_criticalsection;
};
class AutoCriticalSection
{
public:
	AutoCriticalSection(CriticalSection& criticalsection);
	~AutoCriticalSection();
protected:
	AutoCriticalSection();
	AutoCriticalSection(const AutoCriticalSection& other); // Prevent copying
	AutoCriticalSection& operator = (const AutoCriticalSection& other);  // Prevent assigning
private:
	CriticalSection&    m_criticalsection;
};

class CAutoMutex;
class CMutex
{
public:
	friend CAutoMutex;
	CMutex(LPCTSTR lpszName);
	~CMutex();
protected:
	void Lock();
	void Unlock();
private:
	HANDLE m_hMutex;
};

class CAutoMutex
{
public:
	CAutoMutex(CMutex& mutex);
	~CAutoMutex();
protected:
	CAutoMutex();
	CAutoMutex(const CAutoMutex& other);	//	Prevent copying
	CAutoMutex& operator = (const CAutoMutex& other);	//	Prevent assigning
private:
	CMutex&	m_mutex;
};

class CAutoEvent;
class CManualEvent;
class CEvent
{
	friend CAutoEvent;
	friend CManualEvent;
public:
	CEvent(LPCTSTR lpszName,BOOL bManualReset = TRUE,BOOL bInitialState = TRUE);
	~CEvent();
	void Reset();
	void Set();
	bool IsLocked();
protected:
	void Lock();
	void Unlock();
private:
	HANDLE m_hEvent;
};
class CAutoEvent
{
public:
	CAutoEvent(CEvent& e);
	~CAutoEvent();
protected:
	CAutoEvent();
	CAutoEvent(const CAutoEvent& other);	//	Prevent copying
	CAutoEvent& operator = (const CAutoEvent& other);	//	Prevent assigning
private:
	CEvent& m_event;
};
class CManualEvent
{
public:
	CManualEvent(CEvent& e);
	~CManualEvent();
	void Lock();
	void Unlock();
protected:
	CManualEvent();
	CManualEvent(const CManualEvent& other);	//	Prevent copying
	CManualEvent& operator = (const CManualEvent& other);	//	Prevent assigning
private:
	CEvent& m_event;
};


#endif