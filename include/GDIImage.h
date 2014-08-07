#pragma once
#if defined(UI_BUILD_FOR_WIN32) && !defined(UI_BUILD_FOR_WINCE)
#include <gdiplus.h>

#include <set>

EXTERN_C const CLSID CLSID_GDIImage;

class CGDIImage
{
	typedef bool (__cdecl *ONFRAMECHANGED)(CGDIImage *pImage, LPARAM lParam);

	ULONG_PTR m_GdiToken;
	Gdiplus::Image *m_pImage;
	Gdiplus::PropertyItem* m_pItem;
	DWORD m_dwWidth;
	DWORD m_dwHeight;
	DWORD m_dwFramesCount;
	DWORD m_dwCurrentFrame;
	COLORREF m_clrBack;

	HANDLE m_hTimer;
	volatile LONG m_lRef;

	static VOID CALLBACK OnTimer(PVOID lpParameter, BOOLEAN TimerOrWaitFired);

	struct CALLBACK_STRUCT
	{
		CALLBACK_STRUCT(ONFRAMECHANGED _pOnFrameChangedProc, LPARAM _lParam):
	pOnFrameChangedProc(_pOnFrameChangedProc),
		lParam(_lParam)
	{};

	ONFRAMECHANGED pOnFrameChangedProc;
	LPARAM lParam;

	bool operator<(const CALLBACK_STRUCT &cb) const
	{
		return (pOnFrameChangedProc < cb.pOnFrameChangedProc || (pOnFrameChangedProc == cb.pOnFrameChangedProc && lParam < cb.lParam));
	}
	};

	CRITICAL_SECTION m_csCallback;
	typedef std::set<CALLBACK_STRUCT> tCALLBACK;
	tCALLBACK m_Callbacks;
	HWND m_hCallbackWnd;
	DWORD m_dwCallbackMsg;

	HGLOBAL m_global;
	IStream* m_stream;

	CGDIImage(LPCWSTR pszFileName, HWND hCallbackWnd, DWORD dwCallbackMsg);
	CGDIImage(const void * pVoid, int nSize, HWND hCallbackWnd, DWORD dwCallbackMsg);
	~CGDIImage();

public:

	static CGDIImage *CreateInstance(LPCWSTR pszFileName, HWND hCallbackWnd, DWORD dwCallbackMsg);
	static CGDIImage *CreateInstance(const void * pVoid, int nSize, HWND hCallbackWnd, DWORD dwCallbackMsg);

	bool IsInited()
	{
		return m_pImage != NULL;
	}

	void Draw(HDC hDC, int xDst, int yDst, int wDst, int hDst, int xSrc, int ySrc, int xBk, int yBk, int wBk, int hBk);
	void DrawFrame();

	DWORD GetFrameDelay(DWORD dwFrame);
	void SelectActiveFrame(DWORD dwFrame);
	DWORD GetFrameCount();
	DWORD GetWidth();
	DWORD GetHeight();

	void RegisterCallback(ONFRAMECHANGED pOnFrameChangedProc, LPARAM lParam);
	void UnregisterCallback(ONFRAMECHANGED pOnFrameChangedProc, LPARAM lParam);

	HDC CreateBackDC(COLORREF clrBack, int iPaddingW, int iPaddingH);
	void DeleteBackDC(HDC hBackDC);

	LONG AddRef()
	{
		return InterlockedIncrement(&m_lRef);
	}

	LONG Release()
	{
		LONG lRef = InterlockedDecrement(&m_lRef);

		if (lRef == 0)
		{
			delete this;
		}

		return lRef;
	}
};

EXTERN_C CGDIImage* GetImage(LPCWSTR pszFileName, HWND hCallbackWnd, DWORD dwCallbackMsg);
EXTERN_C CGDIImage* GetImageFromStream(const void * pVoid, int nSize, HWND hCallbackWnd, DWORD dwCallbackMsg);
EXTERN_C IOleObject* GetImageObject(CGDIImage* pImage, LPCWSTR pStrData, IOleClientSite *pOleClientSite, IStorage *pStorage, HWND hCallbackWnd, DWORD dwCallbackMsg);
EXTERN_C IOleObject* GetImageObjectFromFile(LPCWSTR pszFileName, LPCWSTR pStrData, IOleClientSite *pOleClientSite, IStorage *pStorage, HWND hCallbackWnd, DWORD dwCallbackMsg);

#endif