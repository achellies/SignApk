#include "stdafx.h"
#include "GdiImage.h"
#include "GDIImageOle.h"

using namespace Gdiplus;

CGDIImage::CGDIImage(const void * pVoid, int nSize, HWND hCallbackWnd, DWORD dwCallbackMsg):
m_dwFramesCount(0), m_pImage(NULL), m_pItem(NULL), m_hTimer(NULL), m_lRef(1),
m_hCallbackWnd(hCallbackWnd), m_dwCallbackMsg(dwCallbackMsg),
m_GdiToken(0),
m_dwWidth(0),
m_dwHeight(0),
m_dwCurrentFrame(0),
m_clrBack(RGB(255,255,255)),
m_global(NULL),
m_stream(NULL)
{
	InitializeCriticalSection(&m_csCallback);
	GdiplusStartupInput GdiInput;
	if (GdiplusStartup(&m_GdiToken, &GdiInput, NULL) != Ok)
		return;

	m_global = ::GlobalAlloc(GMEM_MOVEABLE, nSize);
	if (m_global)
	{
		if (::CreateStreamOnHGlobal(m_global, FALSE, &m_stream) == S_OK)
		{
			void* pBuffer = ::GlobalLock(m_global);
			if (pBuffer)
			{
				CopyMemory(pBuffer, pVoid, nSize);

				::GlobalUnlock(pBuffer);

				m_pImage = new Image(m_stream, false);
			}
		}
	}
	if (m_pImage && m_pImage->GetLastStatus() != Ok)
	{
		delete m_pImage;
		m_pImage = NULL;
	}

	if (!m_pImage)
	{
		return;
	}
	m_pItem = 0;
	if (UINT TotalBuffer = m_pImage->GetPropertyItemSize(PropertyTagFrameDelay))
	{
		m_pItem = (PropertyItem*)new char[TotalBuffer];
		memset(m_pItem, 0, TotalBuffer);
		m_dwCurrentFrame = 0;
		m_dwWidth  = 0;
		m_dwHeight = 0;
		if (!m_pImage->GetPropertyItem(PropertyTagFrameDelay, TotalBuffer, m_pItem))
		{
			SelectActiveFrame(m_dwCurrentFrame);
			if (DWORD dwFrameCount = GetFrameCount())
			{
				// [!] brain-ripper
				// GDI+ seemed to have bug,
				// and for some GIF's returned zero delay for all frames.
				// Make such a workaround: take 50ms delay on every frame
				bool bHaveDelay = false;
				for (DWORD i = 0; i < dwFrameCount; i++)
				{
					if (GetFrameDelay(i) > 0)
					{
						bHaveDelay = true;
						break;
					}
				}
				if (!bHaveDelay)
				{
					for (DWORD i = 0; i < dwFrameCount; i++)
					{
						((UINT*)m_pItem[0].value)[i] = 5;
					}
				}
			}
			// [!] brain-ripper
			// Strange bug - m_pImage->GetWidth() and m_pImage->GetHeight()
			// sometimes returns zero, even object successfuly loaded.
			// Cache this values here to return correct values later
			m_dwWidth = m_pImage->GetWidth();
			m_dwHeight = m_pImage->GetHeight();
		}
		else
		{
			delete [](char*) m_pItem;
			m_pItem = 0;
		}
	}
}

CGDIImage::CGDIImage(LPCWSTR pszFileName, HWND hCallbackWnd, DWORD dwCallbackMsg):
m_dwFramesCount(0), m_pImage(NULL), m_pItem(NULL), m_hTimer(NULL), m_lRef(1),
m_hCallbackWnd(hCallbackWnd), m_dwCallbackMsg(dwCallbackMsg),
m_GdiToken(0),
m_dwWidth(0),
m_dwHeight(0),
m_dwCurrentFrame(0),
m_clrBack(RGB(255,255,255)),
m_global(NULL),
m_stream(NULL)
{
	InitializeCriticalSection(&m_csCallback);
	GdiplusStartupInput GdiInput;
	if (GdiplusStartup(&m_GdiToken, &GdiInput, NULL) != Ok)
		return;
	m_pImage = new Image(pszFileName);
	if (m_pImage && m_pImage->GetLastStatus() != Ok)
	{
		delete m_pImage;
		m_pImage = NULL;
	}
	if (!m_pImage)
	{
		return;
	}
	m_pItem = 0;
	if (UINT TotalBuffer = m_pImage->GetPropertyItemSize(PropertyTagFrameDelay))
	{
		m_pItem = (PropertyItem*)new char[TotalBuffer];
		memset(m_pItem, 0, TotalBuffer);
		m_dwCurrentFrame = 0;
		m_dwWidth  = 0;
		m_dwHeight = 0;
		if (!m_pImage->GetPropertyItem(PropertyTagFrameDelay, TotalBuffer, m_pItem))
		{
			SelectActiveFrame(m_dwCurrentFrame);
			if (DWORD dwFrameCount = GetFrameCount())
			{
				// [!] brain-ripper
				// GDI+ seemed to have bug,
				// and for some GIF's returned zero delay for all frames.
				// Make such a workaround: take 50ms delay on every frame
				bool bHaveDelay = false;
				for (DWORD i = 0; i < dwFrameCount; i++)
				{
					if (GetFrameDelay(i) > 0)
					{
						bHaveDelay = true;
						break;
					}
				}
				if (!bHaveDelay)
				{
					for (DWORD i = 0; i < dwFrameCount; i++)
					{
						((UINT*)m_pItem[0].value)[i] = 5;
					}
				}
			}
			// [!] brain-ripper
			// Strange bug - m_pImage->GetWidth() and m_pImage->GetHeight()
			// sometimes returns zero, even object successfuly loaded.
			// Cache this values here to return correct values later
			m_dwWidth = m_pImage->GetWidth();
			m_dwHeight = m_pImage->GetHeight();
		}
		else
		{
			delete [](char*) m_pItem;  // TODO - убрать кривое распределени?па?ти
			m_pItem = 0;
		}
	}
}

CGDIImage::~CGDIImage()
{
	_ASSERTE(m_Callbacks.size() == 0);
	HANDLE hDeleteEvent = NULL;
	if (m_hTimer)
	{
		hDeleteEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (hDeleteEvent)
			DeleteTimerQueueTimer(NULL, m_hTimer, hDeleteEvent);
		m_hTimer = NULL;
	}

	if (hDeleteEvent)
	{
		WaitForSingleObject(hDeleteEvent, INFINITE);
		CloseHandle(hDeleteEvent);
	}

	if (m_global != NULL)
	{
		if (m_stream != NULL)
			m_stream->Release();
		::GlobalUnlock(m_global);
		::GlobalFree(m_global);
	}

	if (m_pImage)
		delete m_pImage;

	if (m_pItem)
		delete [](char*)m_pItem;

	if (m_GdiToken)
		GdiplusShutdown(m_GdiToken);

	DeleteCriticalSection(&m_csCallback);
}

void CGDIImage::Draw(HDC hDC, int xDst, int yDst, int wDst, int hDst, int xSrc, int ySrc, int xBk, int yBk, int wBk, int hBk)
{
    Gdiplus::Graphics Graph(hDC);
    Graph.DrawImage(m_pImage,
        xDst,
        yDst,
        0, 0,
        wDst, hDst,
        Gdiplus::UnitPixel);
}

DWORD CGDIImage::GetFrameDelay(DWORD dwFrame)
{
	if (m_pItem)
		return ((UINT*)m_pItem[0].value)[dwFrame] * 10;
	else
		return 5;
}

void CGDIImage::SelectActiveFrame(DWORD dwFrame)
{
	GUID Guid = FrameDimensionTime;
	m_pImage->SelectActiveFrame(&Guid, dwFrame);
}

DWORD CGDIImage::GetFrameCount()
{
	if (m_dwFramesCount == 0)
	{
		//First of all we should get the number of frame dimensions
		//Images considered by GDI+ as:
		//frames[animation_frame_index][how_many_animation];
		if (UINT count = m_pImage->GetFrameDimensionsCount())
		{


			//Now we should get the identifiers for the frame dimensions
			GUID *m_pDimensionIDs = new GUID[count];
			m_pImage->GetFrameDimensionsList(m_pDimensionIDs, count);

			//For gif image , we only care about animation set#0
			WCHAR strGuid[39];
			StringFromGUID2(m_pDimensionIDs[0], strGuid, 39);
			m_dwFramesCount = m_pImage->GetFrameCount(&m_pDimensionIDs[0]);

			delete m_pDimensionIDs;
		}
	}

	return m_dwFramesCount;
}

DWORD CGDIImage::GetWidth()
{
	return m_dwWidth;
}

DWORD CGDIImage::GetHeight()
{
	return m_dwHeight;
}

void CGDIImage::DrawFrame()
{
	EnterCriticalSection(&m_csCallback);
	if (m_Callbacks.size())
	{
		for (tCALLBACK::iterator i = m_Callbacks.begin(); i != m_Callbacks.end(); ++i)
		{
			if (!i->pOnFrameChangedProc(this, i->lParam))
			{
				i = m_Callbacks.erase(i);
				if (i == m_Callbacks.end())
					break;
			}
		}
	}
	LeaveCriticalSection(&m_csCallback);
}

VOID CALLBACK CGDIImage::OnTimer(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
	CGDIImage *pGDIImage = (CGDIImage *)lpParameter;

	//Change Active frame
	pGDIImage->SelectActiveFrame(pGDIImage->m_dwCurrentFrame);

	DWORD dwDelay = pGDIImage->GetFrameDelay(pGDIImage->m_dwCurrentFrame);

	if (dwDelay == 0)
		dwDelay++;

	EnterCriticalSection(&pGDIImage->m_csCallback);
	if (pGDIImage->m_hTimer)
	{
		// We have non-periodic one-shot timer.
		// Since we can't restart one-shot timer,
		// we have to delete it and create new one.
		// It is safe to give NULL event, because
		// we call it from callback function and timer
		// will be destroyed just after it returns
		DeleteTimerQueueTimer(NULL, pGDIImage->m_hTimer, NULL);

		// Don't null timer handle, to prevent creating new one
		// from RegisterCallback
		// pGDIImage->m_hTimer = NULL;
	}
	LeaveCriticalSection(&pGDIImage->m_csCallback);

	if (pGDIImage->m_hCallbackWnd)
	{
		// We should call DrawFrame in context of window thread
		SendMessage(pGDIImage->m_hCallbackWnd, pGDIImage->m_dwCallbackMsg, 0, (LPARAM)pGDIImage);
	}
	else
		pGDIImage->DrawFrame();

	EnterCriticalSection(&pGDIImage->m_csCallback);
	if (pGDIImage->m_Callbacks.size())
	{
		CreateTimerQueueTimer(&pGDIImage->m_hTimer, NULL, OnTimer, pGDIImage, dwDelay, 0, WT_EXECUTEDEFAULT);
	}
	else
		pGDIImage->m_hTimer = NULL;
	LeaveCriticalSection(&pGDIImage->m_csCallback);

	// Move to the next frame
	if (pGDIImage->m_dwFramesCount)
		pGDIImage->m_dwCurrentFrame = (++ pGDIImage->m_dwCurrentFrame) % pGDIImage->m_dwFramesCount;
}

void CGDIImage::RegisterCallback(ONFRAMECHANGED pOnFrameChangedProc, LPARAM lParam)
{
	if (GetFrameCount() > 1)
	{
		EnterCriticalSection(&m_csCallback);
		m_Callbacks.insert(CALLBACK_STRUCT(pOnFrameChangedProc, lParam));
		if (!m_hTimer)
			CreateTimerQueueTimer(&m_hTimer, NULL, OnTimer, this, 0, 0, WT_EXECUTEDEFAULT);
		LeaveCriticalSection(&m_csCallback);
	}
}

void CGDIImage::UnregisterCallback(ONFRAMECHANGED pOnFrameChangedProc, LPARAM lParam)
{
	if (GetFrameCount() > 1)
	{
		//HANDLE hDeleteEvent = NULL;
		EnterCriticalSection(&m_csCallback);
		tCALLBACK::iterator i = m_Callbacks.find(CALLBACK_STRUCT(pOnFrameChangedProc, lParam));
		if (i != m_Callbacks.end())
			m_Callbacks.erase(i);

		/*if (m_Callbacks.size() == 0 && m_hTimer)
		{
		hDeleteEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		DeleteTimerQueueTimer(NULL, m_hTimer, NULL);//hDeleteEvent);
		m_hTimer = NULL;
		}
		*/
		LeaveCriticalSection(&m_csCallback);

		/*
		if (hDeleteEvent)
		{
		WaitForSingleObject(hDeleteEvent, INFINITE);
		CloseHandle(hDeleteEvent);
		}
		*/
	}
}

HDC CGDIImage::CreateBackDC(COLORREF clrBack, int iPaddingW, int iPaddingH)
{
	m_clrBack = clrBack;
	HDC hRetDC = NULL;

	if (m_pImage)
	{
		HDC hDC = ::GetDC(NULL);
		hRetDC = CreateCompatibleDC(hDC);
		::SaveDC(hRetDC);
		HBITMAP hBitmap = CreateCompatibleBitmap(hDC, m_pImage->GetWidth() + iPaddingW, m_pImage->GetHeight() + iPaddingH);
		HBRUSH hBrush = CreateSolidBrush(m_clrBack);
		SelectObject(hRetDC, hBitmap);
		SelectObject(hRetDC, hBrush);
		::ReleaseDC(NULL, hDC);

		BitBlt(hRetDC, 0, 0, m_pImage->GetWidth() + iPaddingW, m_pImage->GetHeight() + iPaddingH, NULL, 0, 0, PATCOPY);
	}

	return hRetDC;
}

void CGDIImage::DeleteBackDC(HDC hBackDC)
{
	HBITMAP hBmp = (HBITMAP)GetCurrentObject(hBackDC, OBJ_BITMAP);
	HBRUSH hBrush = (HBRUSH)GetCurrentObject(hBackDC, OBJ_BRUSH);

	RestoreDC(hBackDC, -1);

	DeleteDC(hBackDC);
	DeleteObject(hBmp);
	DeleteObject(hBrush);
}

CGDIImage *CGDIImage::CreateInstance(LPCWSTR pszFileName, HWND hCallbackWnd, DWORD dwCallbackMsg)
{
	return new CGDIImage(pszFileName, hCallbackWnd, dwCallbackMsg);
}

CGDIImage *CGDIImage::CreateInstance(const void * pVoid, int nSize, HWND hCallbackWnd, DWORD dwCallbackMsg)
{
	return new CGDIImage(pVoid, nSize, hCallbackWnd, dwCallbackMsg);
}

std::wstring ascii2unicode(LPCSTR pstr)
{
	std::wstring wUnicode;

	if (pstr == NULL)
	{
		throw std::invalid_argument("Ascii2Unicode() --- Invalid input parameter!");
	}

	ULONG cbMultiByte = 0;
	ULONG cchWideChar = 0;

	cbMultiByte = static_cast<ULONG>(strlen(pstr));
	cchWideChar = MultiByteToWideChar(CP_ACP,0,pstr,cbMultiByte,NULL,0);

	LPWSTR pwstr = new wchar_t[cchWideChar+1];
	if (pwstr == NULL)
	{
		throw std::overflow_error("Ascii2Unicode() --- memory overflow!");
	}
	
	ULONG cchSize = MultiByteToWideChar(CP_ACP,0,pstr,cbMultiByte,pwstr,cchWideChar);

	if (cchSize != cchWideChar)
	{
		delete[] pwstr;
		pwstr = NULL;
		throw std::runtime_error("Ascii2Unicode() --- runtime error!");
	}

	pwstr[cchWideChar] = L'\0';
	wUnicode.append(pwstr);

	delete[] pwstr;
	pwstr = NULL;

	return wUnicode;
}

std::string unicode2ascii(LPCWSTR pwstr)
{
	std::string sAscii;

	if (pwstr == NULL)
	{
		throw std::invalid_argument("Unicode2Ascii() ---- Invalid input parameter!");
	}

	ULONG cbMultiByte = 0;
	ULONG cchWideChar = 0;

	cchWideChar = static_cast<ULONG>(wcslen(pwstr));
	cbMultiByte = WideCharToMultiByte(CP_ACP,0,pwstr,cchWideChar,NULL,0,NULL,NULL);

	LPSTR pstr = new char[cbMultiByte+1];
	if (pstr == NULL)
	{
		throw std::overflow_error("Unicode2Ascii() ---- memory overflow!");
	}
	
	ULONG cchSize = WideCharToMultiByte(CP_ACP,0,pwstr,cchWideChar,pstr,cbMultiByte,NULL,NULL);

	if (cchSize != cbMultiByte)
	{
		delete[] pstr;
		pstr = NULL;
		throw std::runtime_error("Unicode2Ascii() ---- runtime error!");
	}
	pstr[cbMultiByte] = '\0';

	sAscii.append(pstr);

	delete[] pstr;
	pstr = NULL;

	return sAscii;
}

EXTERN_C CGDIImage* GetImage(LPCTSTR pszFileName, HWND hCallbackWnd, DWORD dwCallbackMsg)
{
	std::wstring file_name;
#if defined(UNICODE)
	file_name = pszFileName;
#else
	file_name = ascii2unicode(pszFileName);
#endif
	// N.B. don't delete this object, use Release method!
	CGDIImage* pGdiImage = CGDIImage::CreateInstance(file_name.c_str(), hCallbackWnd, dwCallbackMsg);

	if (!pGdiImage->IsInited())
	{
		pGdiImage->Release();
		pGdiImage = NULL;
	}

	return pGdiImage;
}

EXTERN_C CGDIImage* GetImageFromStream(const void * pVoid, int nSize, HWND hCallbackWnd, DWORD dwCallbackMsg)
{
	// N.B. don't delete this object, use Release method!
	CGDIImage* pGdiImage = CGDIImage::CreateInstance(pVoid, nSize, hCallbackWnd, dwCallbackMsg);

	if (!pGdiImage->IsInited())
	{
		pGdiImage->Release();
		pGdiImage = NULL;
	}

	return pGdiImage;
}

EXTERN_C IOleObject* GetImageObject(CGDIImage* pImage, LPCTSTR pStrData, IOleClientSite *pOleClientSite, IStorage *pStorage, HWND hCallbackWnd, DWORD dwCallbackMsg)
{
	if (pImage == NULL || pStrData == NULL)
		return NULL;

	std::wstring data;
#if defined(UNICODE)
	data = pStrData;
#else
	data = ascii2unicode(pStrData);
#endif

	IOleObject *pObject = NULL;

	if (pImage && !data.empty())
	{
		CComObject<CGDIImageOle> *pGdiImageObject = NULL;
		if (pImage)
		{
			// Create view instance of single graphic object (pImage).
			// Instance will be automatically removed after releasing last pointer;
			CComObject<CGDIImageOle>::CreateInstance(&pGdiImageObject);

			if (pGdiImageObject)
			{
				if (pGdiImageObject->put_SetImage(pImage, data.c_str(), hCallbackWnd, dwCallbackMsg) != S_OK)
				{
					delete pGdiImageObject;
					pGdiImageObject = NULL;
				}
			}
		}

		if (pGdiImageObject)
		{
			pGdiImageObject->QueryInterface(IID_IOleObject, (void**)&pObject);
			pObject->SetClientSite(pOleClientSite);
		}
		else
		{
			if (pImage)
			{
				pImage->Release();
				pImage = NULL;
			}
		}
	}

	return pObject;
}

EXTERN_C IOleObject* GetImageObjectFromFile(LPCTSTR pszFileName, LPCTSTR pStrData, IOleClientSite *pOleClientSite, IStorage *pStorage, HWND hCallbackWnd, DWORD dwCallbackMsg)
{
	CGDIImage* pImage = GetImage(pszFileName, hCallbackWnd, dwCallbackMsg);

	return GetImageObject(pImage, pStrData, pOleClientSite, pStorage, hCallbackWnd, dwCallbackMsg);
}