#include "StdAfx.h"
#include "UIlib.h"
#include "GDIImage.h"

#if defined(UI_BUILD_FOR_WIN32) && !defined(UI_BUILD_FOR_WINCE)

#pragma comment(lib, "GdiOle.lib")
#pragma comment(lib, "gdiplus.lib")

namespace DuiLib {
/////////////////////////////////////////////////////////////////////////////////////
//

CGifLabelUI::CGifLabelUI()
: timer_started_(false)
, frame_count_(0)
, frame_index_(0)
, m_pImage(NULL)
{}

CGifLabelUI::~CGifLabelUI()
{
	if (m_pImage != NULL)
		m_pImage->Release();
}

LPCTSTR CGifLabelUI::GetClass() const
{
    return kGifLabelUIClassName;
}

LPVOID CGifLabelUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcsicmp(pstrName, kGifLabelUIInterfaceName) == 0 ) return static_cast<CGifLabelUI*>(this);
    return CGifLabelUI::GetInterface(pstrName);
}

void CGifLabelUI::DoEvent(TEventUI& event)
{
	if ((UIEVENT_TIMER == event.Type) && (event.wParam == GifUpdateTimerId))
	{
		Invalidate();
		return;
	}
	return __super::DoEvent(event);
}

void CGifLabelUI::PaintBkImage(void* ctx)
{
	if (ctx == NULL)
		return;

    if (m_sBkImage.IsEmpty())
		return;

	if (m_sPreBkImage.IsEmpty())
		m_sPreBkImage = m_sBkImage;

	if (_tcsicmp(m_sBkImage.GetData(), m_sPreBkImage.GetData()) != 0)
	{
		if (m_pImage != NULL)
			m_pImage->Release();
		m_pImage = NULL;
	}	

	if (m_pImage == NULL)
	{
		LPBYTE pData = NULL;
		DWORD dwSize = 0;
		if (isdigit(*m_sBkImage.GetData()) || _tcsstr(m_sBkImage.GetData(), _T("res=\'")) != NULL)
		{
			CStdString sImageName = m_sBkImage.GetData();
			CStdString sImageResType = _T("GIF");
			if (!isdigit(*m_sBkImage.GetData()))
			{
				LPCTSTR pStrImage = m_sBkImage.GetData();
				CStdString sItem;
				CStdString sValue;
				while( *pStrImage != _T('\0') ) {
					sItem.Empty();
					sValue.Empty();
					while( *pStrImage > _T('\0') && *pStrImage <= _T(' ') ) pStrImage = ::CharNext(pStrImage);
					while( *pStrImage != _T('\0') && *pStrImage != _T('=') && *pStrImage > _T(' ') ) {
						LPTSTR pstrTemp = ::CharNext(pStrImage);
						while( pStrImage < pstrTemp) {
							sItem += *pStrImage++;
						}
					}
					while( *pStrImage > _T('\0') && *pStrImage <= _T(' ') ) pStrImage = ::CharNext(pStrImage);
					if( *pStrImage++ != _T('=') ) break;
					while( *pStrImage > _T('\0') && *pStrImage <= _T(' ') ) pStrImage = ::CharNext(pStrImage);
					if( *pStrImage++ != _T('\'') ) break;
					while( *pStrImage != _T('\0') && *pStrImage != _T('\'') ) {
						LPTSTR pstrTemp = ::CharNext(pStrImage);
						while( pStrImage < pstrTemp) {
							sValue += *pStrImage++;
						}
					}
					if( *pStrImage++ != _T('\'') ) break;
					if( !sValue.IsEmpty() ) {
						if( sItem == _T("file") || sItem == _T("res") ) {
							sImageName = sValue;
						}
						else if( sItem == _T("restype") ) {
							sImageResType = sValue;
						}
					}
					if( *pStrImage++ != _T(' ') ) break;
				}
			}
			HRSRC hResource = ::FindResource(GetManager()->GetResourceDll(), MAKEINTRESOURCE(_ttoi(sImageName.GetData())), sImageResType.GetData());
			if( hResource == NULL )
				return;
			HGLOBAL hGlobal = ::LoadResource(GetManager()->GetResourceDll(), hResource);
			if( hGlobal == NULL ) {
#if defined(WIN32) && !defined(UNDER_CE)
				FreeResource(hResource);
#endif
				return;
			}

			dwSize = ::SizeofResource(GetManager()->GetResourceDll(), hResource);
			if( dwSize == 0 )
				return;
			pData = new BYTE[ dwSize ];
			if (pData != NULL)
			{
				::CopyMemory(pData, (LPBYTE)::LockResource(hGlobal), dwSize);
			}
#if defined(WIN32) && !defined(UNDER_CE)
			::FreeResource(hResource);
#endif
			m_pImage = GetImageFromStream(pData, dwSize, NULL, NULL);

			delete[] pData;
			pData = NULL;
		}
		else
		{
			TCHAR szImagePath[MAX_PATH] = {0};
			if (GetManager()->GetResourceZip().IsEmpty())
			{
				CStdString sResourcePath = GetManager()->GetResourcePath();
				_stprintf_s(szImagePath, MAX_PATH - 1, _T("%s%s"), sResourcePath.GetData(), m_sBkImage.GetData());
				m_pImage = GetImage(szImagePath, NULL, NULL);
			}
			else
			{
				CStdString sFile = GetManager()->GetResourcePath();
				sFile += GetManager()->GetResourceZip();
				HZIP hz = NULL;
				LPBYTE lpZipData = NULL;

				CStdString sZipFile = GetManager()->GetResourceZip();
				if (isdigit(*sZipFile.GetData()) || _tcsstr(sZipFile.GetData(), _T("res=\'")) != NULL)
				{
					CStdString sResType = _T("ZIP");
					if (!isdigit(*sZipFile.GetData()))
					{
						LPCTSTR pStrBuffer = sZipFile.GetData();
						CStdString sItem;
						CStdString sValue;
						while( *pStrBuffer != _T('\0') ) {
							sItem.Empty();
							sValue.Empty();
							while( *pStrBuffer > _T('\0') && *pStrBuffer <= _T(' ') ) pStrBuffer = ::CharNext(pStrBuffer);
							while( *pStrBuffer != _T('\0') && *pStrBuffer != _T('=') && *pStrBuffer > _T(' ') ) {
								LPTSTR pstrTemp = ::CharNext(pStrBuffer);
								while( pStrBuffer < pstrTemp) {
									sItem += *pStrBuffer++;
								}
							}
							while( *pStrBuffer > _T('\0') && *pStrBuffer <= _T(' ') ) pStrBuffer = ::CharNext(pStrBuffer);
							if( *pStrBuffer++ != _T('=') ) break;
							while( *pStrBuffer > _T('\0') && *pStrBuffer <= _T(' ') ) pStrBuffer = ::CharNext(pStrBuffer);
							if( *pStrBuffer++ != _T('\'') ) break;
							while( *pStrBuffer != _T('\0') && *pStrBuffer != _T('\'') ) {
								LPTSTR pstrTemp = ::CharNext(pStrBuffer);
								while( pStrBuffer < pstrTemp) {
									sValue += *pStrBuffer++;
								}
							}
							if( *pStrBuffer++ != _T('\'') ) break;
							if( !sValue.IsEmpty() ) {
								if( sItem == _T("res") ) {
									sZipFile = sValue;
								}
								else if( sItem == _T("restype") ) {
									sResType = sValue;
								}
							}
							if( *pStrBuffer++ != _T(' ') ) break;
						}
					}

					HRSRC hResource = ::FindResource(GetManager()->GetResourceDll(), MAKEINTRESOURCE(_ttoi(sZipFile.GetData())), sResType.GetData());
					if( hResource == NULL )
						return;
					DWORD dwSize = 0;
					HGLOBAL hGlobal = ::LoadResource(GetManager()->GetResourceDll(), hResource);
					if( hGlobal == NULL ) {
#if defined(WIN32) && !defined(UNDER_CE)
						FreeResource(hResource);
#endif
						return;
					}
					dwSize = ::SizeofResource(GetManager()->GetResourceDll(), hResource);
					if( dwSize == 0 )
						return;
					lpZipData = new BYTE[ dwSize ];
					if (lpZipData != NULL)
						::CopyMemory(lpZipData, (LPBYTE)::LockResource(hGlobal), dwSize);
#if defined(WIN32) && !defined(UNDER_CE)
					::FreeResource(hResource);
#endif
					hz = OpenZip((void*)lpZipData, dwSize, NULL);
				}
				else
					hz = OpenZip(sFile.GetData(), NULL);

				if (hz == NULL)
				{
					if (lpZipData != NULL)
					{
						delete[] lpZipData;
						lpZipData = NULL;
					}
					return;
				}

				ZIPENTRY ze;
				int i = 0;
				if (FindZipItem(hz, m_sBkImage.GetData(), true, &i, &ze) != 0)
				{
					CloseZip(hz);
					if (lpZipData != NULL)
					{
						delete[] lpZipData;
						lpZipData = NULL;
					}
					return;
				}
				dwSize = ze.unc_size;
				if (dwSize == 0)
				{
					CloseZip(hz);
					if (lpZipData != NULL)
					{
						delete[] lpZipData;
						lpZipData = NULL;
					}
					return;
				}
				pData = new BYTE[dwSize];
				if (pData != NULL)
				{
					int res = UnzipItem(hz, i, pData, dwSize);
					if (res != 0x00000000 && res != 0x00000600)
					{
						delete[] pData;
						CloseZip(hz);
						if (lpZipData != NULL)
						{
							delete[] lpZipData;
							lpZipData = NULL;
						}
						return;
					}
				}
				CloseZip(hz);

				m_pImage = GetImageFromStream(pData, dwSize, NULL, NULL);

				delete[] pData;
				pData = NULL;

				if (lpZipData != NULL)
				{
					delete[] lpZipData;
					lpZipData = NULL;
				}
			}
		}

		if (m_pImage != NULL)
		{
			frame_count_ = m_pImage->GetFrameCount();
			if (!timer_started_ && frame_count_ > 1)
			{
				timer_started_ = true;
				GetManager()->SetTimer(this, GifUpdateTimerId, GifUpdateTimerInterval);
			}
			else if (timer_started_ && (frame_count_ == 1))
			{
				timer_started_ = false;
				GetManager()->KillTimer(this, GifUpdateTimerId);
			}
		}
	}

	if (m_pImage != NULL)
	{
		if (frame_index_ >= frame_count_)
			frame_index_ = 0;
		m_pImage->SelectActiveFrame(frame_index_++);
		HDC hDC = (HDC)ctx;
		m_pImage->Draw(hDC,
			m_rcItem.left,
			m_rcItem.top,
			DWORD(m_rcItem.right - m_rcItem.left),
			DWORD(m_rcItem.bottom - m_rcItem.top), 0, 0, 0, 0,
			DWORD(m_rcItem.right - m_rcItem.left),
			DWORD(m_rcItem.bottom - m_rcItem.top));
	}
}


CGifButtonUI::CGifButtonUI()
: timer_started_(false)
, frame_count_(0)
, frame_index_(0)
, m_pImage(NULL)
{}

CGifButtonUI::~CGifButtonUI()
{
	if (m_pImage != NULL)
		m_pImage->Release();
}

LPCTSTR CGifButtonUI::GetClass() const
{
    return kGifButtonUIClassName;
}

LPVOID CGifButtonUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcsicmp(pstrName, kGifButtonUIInterfaceName) == 0 ) return static_cast<CGifButtonUI*>(this);
    return CButtonUI::GetInterface(pstrName);
}

void CGifButtonUI::DoEvent(TEventUI& event)
{
	if ((UIEVENT_TIMER == event.Type) && (event.wParam == GifUpdateTimerId))
	{
		Invalidate();
		return;
	}
	return __super::DoEvent(event);
}

void CGifButtonUI::PaintBkImage(void* ctx)
{
	if (ctx == NULL)
		return;

    if (m_sBkImage.IsEmpty())
		return;

	if (m_sPreBkImage.IsEmpty())
		m_sPreBkImage = m_sBkImage;

	if (_tcsicmp(m_sBkImage.GetData(), m_sPreBkImage.GetData()) != 0)
	{
		if (m_pImage != NULL)
			m_pImage->Release();
		m_pImage = NULL;
	}	

	if (m_pImage == NULL)
	{
		LPBYTE pData = NULL;
		DWORD dwSize = 0;
		if (isdigit(*m_sBkImage.GetData()) || _tcsstr(m_sBkImage.GetData(), _T("res=\'")) != NULL)
		{
			CStdString sImageName = m_sBkImage.GetData();
			CStdString sImageResType = _T("GIF");
			if (!isdigit(*m_sBkImage.GetData()))
			{
				LPCTSTR pStrImage = m_sBkImage.GetData();
				CStdString sItem;
				CStdString sValue;
				while( *pStrImage != _T('\0') ) {
					sItem.Empty();
					sValue.Empty();
					while( *pStrImage > _T('\0') && *pStrImage <= _T(' ') ) pStrImage = ::CharNext(pStrImage);
					while( *pStrImage != _T('\0') && *pStrImage != _T('=') && *pStrImage > _T(' ') ) {
						LPTSTR pstrTemp = ::CharNext(pStrImage);
						while( pStrImage < pstrTemp) {
							sItem += *pStrImage++;
						}
					}
					while( *pStrImage > _T('\0') && *pStrImage <= _T(' ') ) pStrImage = ::CharNext(pStrImage);
					if( *pStrImage++ != _T('=') ) break;
					while( *pStrImage > _T('\0') && *pStrImage <= _T(' ') ) pStrImage = ::CharNext(pStrImage);
					if( *pStrImage++ != _T('\'') ) break;
					while( *pStrImage != _T('\0') && *pStrImage != _T('\'') ) {
						LPTSTR pstrTemp = ::CharNext(pStrImage);
						while( pStrImage < pstrTemp) {
							sValue += *pStrImage++;
						}
					}
					if( *pStrImage++ != _T('\'') ) break;
					if( !sValue.IsEmpty() ) {
						if( sItem == _T("file") || sItem == _T("res") ) {
							sImageName = sValue;
						}
						else if( sItem == _T("restype") ) {
							sImageResType = sValue;
						}
					}
					if( *pStrImage++ != _T(' ') ) break;
				}
			}
			HRSRC hResource = ::FindResource(GetManager()->GetResourceDll(), MAKEINTRESOURCE(_ttoi(sImageName.GetData())), sImageResType.GetData());
			if( hResource == NULL )
				return;
			HGLOBAL hGlobal = ::LoadResource(GetManager()->GetResourceDll(), hResource);
			if( hGlobal == NULL ) {
#if defined(WIN32) && !defined(UNDER_CE)
				FreeResource(hResource);
#endif
				return;
			}

			dwSize = ::SizeofResource(GetManager()->GetResourceDll(), hResource);
			if( dwSize == 0 )
				return;
			pData = new BYTE[ dwSize ];
			if (pData != NULL)
			{
				::CopyMemory(pData, (LPBYTE)::LockResource(hGlobal), dwSize);
			}
#if defined(WIN32) && !defined(UNDER_CE)
			::FreeResource(hResource);
#endif
			m_pImage = GetImageFromStream(pData, dwSize, NULL, NULL);

			delete[] pData;
			pData = NULL;
		}
		else
		{
			TCHAR szImagePath[MAX_PATH] = {0};
			if (GetManager()->GetResourceZip().IsEmpty())
			{
				CStdString sResourcePath = GetManager()->GetResourcePath();
				_stprintf_s(szImagePath, MAX_PATH - 1, _T("%s%s"), sResourcePath.GetData(), m_sBkImage.GetData());
				m_pImage = GetImage(szImagePath, NULL, NULL);
			}
			else
			{
				CStdString sFile = GetManager()->GetResourcePath();
				sFile += GetManager()->GetResourceZip();
				HZIP hz = NULL;
				LPBYTE lpZipData = NULL;

				CStdString sZipFile = GetManager()->GetResourceZip();
				if (isdigit(*sZipFile.GetData()) || _tcsstr(sZipFile.GetData(), _T("res=\'")) != NULL)
				{
					CStdString sResType = _T("ZIP");
					if (!isdigit(*sZipFile.GetData()))
					{
						LPCTSTR pStrBuffer = sZipFile.GetData();
						CStdString sItem;
						CStdString sValue;
						while( *pStrBuffer != _T('\0') ) {
							sItem.Empty();
							sValue.Empty();
							while( *pStrBuffer > _T('\0') && *pStrBuffer <= _T(' ') ) pStrBuffer = ::CharNext(pStrBuffer);
							while( *pStrBuffer != _T('\0') && *pStrBuffer != _T('=') && *pStrBuffer > _T(' ') ) {
								LPTSTR pstrTemp = ::CharNext(pStrBuffer);
								while( pStrBuffer < pstrTemp) {
									sItem += *pStrBuffer++;
								}
							}
							while( *pStrBuffer > _T('\0') && *pStrBuffer <= _T(' ') ) pStrBuffer = ::CharNext(pStrBuffer);
							if( *pStrBuffer++ != _T('=') ) break;
							while( *pStrBuffer > _T('\0') && *pStrBuffer <= _T(' ') ) pStrBuffer = ::CharNext(pStrBuffer);
							if( *pStrBuffer++ != _T('\'') ) break;
							while( *pStrBuffer != _T('\0') && *pStrBuffer != _T('\'') ) {
								LPTSTR pstrTemp = ::CharNext(pStrBuffer);
								while( pStrBuffer < pstrTemp) {
									sValue += *pStrBuffer++;
								}
							}
							if( *pStrBuffer++ != _T('\'') ) break;
							if( !sValue.IsEmpty() ) {
								if( sItem == _T("res") ) {
									sZipFile = sValue;
								}
								else if( sItem == _T("restype") ) {
									sResType = sValue;
								}
							}
							if( *pStrBuffer++ != _T(' ') ) break;
						}
					}

					HRSRC hResource = ::FindResource(GetManager()->GetResourceDll(), MAKEINTRESOURCE(_ttoi(sZipFile.GetData())), sResType.GetData());
					if( hResource == NULL )
						return;
					DWORD dwSize = 0;
					HGLOBAL hGlobal = ::LoadResource(GetManager()->GetResourceDll(), hResource);
					if( hGlobal == NULL ) {
#if defined(WIN32) && !defined(UNDER_CE)
						FreeResource(hResource);
#endif
						return;
					}
					dwSize = ::SizeofResource(GetManager()->GetResourceDll(), hResource);
					if( dwSize == 0 )
						return;
					lpZipData = new BYTE[ dwSize ];
					if (lpZipData != NULL)
						::CopyMemory(lpZipData, (LPBYTE)::LockResource(hGlobal), dwSize);
#if defined(WIN32) && !defined(UNDER_CE)
					::FreeResource(hResource);
#endif
					hz = OpenZip((void*)lpZipData, dwSize, NULL);
				}
				else
					hz = OpenZip(sFile.GetData(), NULL);

				if (hz == NULL)
				{
					if (lpZipData != NULL)
					{
						delete[] lpZipData;
						lpZipData = NULL;
					}
					return;
				}

				ZIPENTRY ze;
				int i = 0;
				if (FindZipItem(hz, m_sBkImage.GetData(), true, &i, &ze) != 0)
				{
					CloseZip(hz);
					if (lpZipData != NULL)
					{
						delete[] lpZipData;
						lpZipData = NULL;
					}
					return;
				}
				dwSize = ze.unc_size;
				if (dwSize == 0)
				{
					CloseZip(hz);
					if (lpZipData != NULL)
					{
						delete[] lpZipData;
						lpZipData = NULL;
					}
					return;
				}
				pData = new BYTE[dwSize];
				if (pData != NULL)
				{
					int res = UnzipItem(hz, i, pData, dwSize);
					if (res != 0x00000000 && res != 0x00000600)
					{
						delete[] pData;
						CloseZip(hz);
						if (lpZipData != NULL)
						{
							delete[] lpZipData;
							lpZipData = NULL;
						}
						return;
					}
				}
				CloseZip(hz);

				m_pImage = GetImageFromStream(pData, dwSize, NULL, NULL);

				delete[] pData;
				pData = NULL;

				if (lpZipData != NULL)
				{
					delete[] lpZipData;
					lpZipData = NULL;
				}
			}
		}

		if (m_pImage != NULL)
		{
			frame_count_ = m_pImage->GetFrameCount();
			if (!timer_started_ && frame_count_ > 1)
			{
				timer_started_ = true;
				GetManager()->SetTimer(this, GifUpdateTimerId, GifUpdateTimerInterval);
			}
			else if (timer_started_ && (frame_count_ == 1))
			{
				timer_started_ = false;
				GetManager()->KillTimer(this, GifUpdateTimerId);
			}
		}
	}

	if (m_pImage != NULL)
	{
		if (frame_index_ >= frame_count_)
			frame_index_ = 0;
		m_pImage->SelectActiveFrame(frame_index_++);
		HDC hDC = (HDC)ctx;
		m_pImage->Draw(hDC,
			m_rcItem.left,
			m_rcItem.top,
			DWORD(m_rcItem.right - m_rcItem.left),
			DWORD(m_rcItem.bottom - m_rcItem.top), 0, 0, 0, 0,
			DWORD(m_rcItem.right - m_rcItem.left),
			DWORD(m_rcItem.bottom - m_rcItem.top));
	}
}


} // namespace DuiLib

#endif