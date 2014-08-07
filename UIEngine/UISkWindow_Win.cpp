#include "StdAfx.h"
#include "UIlib.h"

#pragma warning(disable : 4312)
#pragma warning(disable : 4244)


#ifdef UI_BUILD_FOR_SKIA

#include "SkTypes.h"
#include "SkWindow.h"
#include "SkTime.h"
#include "SkUtils.h"
#include "SkDevice.h"
#include "SkGraphics.h"
#include "SkPaint.h"
#include "SkRect.h"
#include "SkImageEncoder.h"

#if defined(UI_BUILD_FOR_WIN32)

#include <Usp10.h>
#pragma comment(lib, "Usp10.lib")

#elif defined(UI_BUILD_FOR_WINCE)

#include <Usp10.h>
#pragma comment(lib, "Uspce.lib")

#endif

/////////////////////////////////////////////////////////////////////////////////////
//
//
static HWND gEventTarget;

#define WM_EVENT_CALLBACK (WM_USER+0)

void post_skwinevent()
{
	PostMessage(gEventTarget, WM_EVENT_CALLBACK, 0, 0);
}

CSkUIWindow::CSkUIWindow() : m_hWnd(NULL), m_OldWndProc(::DefWindowProc), m_bSubclassed(false)
{
	fWinWindow.rasterCanvas.drawColor(0xFFFFFFFF);
}

CSkUIWindow::~CSkUIWindow()
{}

HWND CSkUIWindow::GetHWND() const 
{ 
	return m_hWnd; 
}

UINT CSkUIWindow::GetClassStyle() const
{
	return 0;
}

LPCTSTR CSkUIWindow::GetSuperClassName() const
{
	return NULL;
}

CSkUIWindow::operator HWND() const
{
	return m_hWnd;
}

HWND CSkUIWindow::Create(HWND hwndParent, LPCTSTR pstrName, DWORD dwStyle, DWORD dwExStyle, const RECT rc, HMENU hMenu)
{
	return Create(hwndParent, pstrName, dwStyle, dwExStyle, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hMenu);
}

HWND CSkUIWindow::Create(HWND hwndParent, LPCTSTR pstrName, DWORD dwStyle, DWORD dwExStyle, int x, int y, int cx, int cy, HMENU hMenu)
{
	if( GetSuperClassName() != NULL && !RegisterSuperclass() ) return NULL;
	if( GetSuperClassName() == NULL && !RegisterWindowClass() ) return NULL;

	m_hWnd = ::CreateWindowEx(dwExStyle, GetWindowClassName(), pstrName, dwStyle, x, y, cx, cy, hwndParent, hMenu, CPaintManagerUI::GetInstance(), this);
	ASSERT(m_hWnd!=NULL);

	gEventTarget = (HWND)m_hWnd;

	return m_hWnd;
}

HWND CSkUIWindow::Subclass(HWND hWnd)
{
	ASSERT(::IsWindow(hWnd));
	ASSERT(m_hWnd==NULL);
	m_OldWndProc = SubclassWindow(hWnd, __WndProc);
	if( m_OldWndProc == NULL ) return NULL;
	m_bSubclassed = true;
	m_hWnd = hWnd;
	return m_hWnd;
}

void CSkUIWindow::Unsubclass()
{
	ASSERT(::IsWindow(m_hWnd));
	if( !::IsWindow(m_hWnd) ) return;
	if( !m_bSubclassed ) return;
	SubclassWindow(m_hWnd, m_OldWndProc);
	m_OldWndProc = ::DefWindowProc;
	m_bSubclassed = false;
}

void CSkUIWindow::ShowWindow(bool bShow /*= true*/, bool bTakeFocus /*= false*/)
{
	ASSERT(::IsWindow(m_hWnd));
	if( !::IsWindow(m_hWnd) ) return;
	::ShowWindow(m_hWnd, bShow ? (bTakeFocus ? SW_SHOWNORMAL : SW_SHOWNOACTIVATE) : SW_HIDE);
}

bool CSkUIWindow::ShowModal()
{
    ASSERT(::IsWindow(m_hWnd));
    HWND hWndParent = GetWindowOwner(m_hWnd);
    ::ShowWindow(m_hWnd, SW_SHOWNORMAL);
    ::EnableWindow(hWndParent, FALSE);
    MSG msg = { 0 };
    while( ::IsWindow(m_hWnd) && ::GetMessage(&msg, NULL, 0, 0) ) {
        if( msg.message == WM_CLOSE && msg.hwnd == m_hWnd ) {
            ::EnableWindow(hWndParent, TRUE);
            ::SetFocus(hWndParent);
        }
        if( !CPaintManagerUI::TranslateMessage(&msg) ) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
        if( msg.message == WM_QUIT ) break;
    }
    ::EnableWindow(hWndParent, TRUE);
    ::SetFocus(hWndParent);
    if( msg.message == WM_QUIT ) ::PostQuitMessage(msg.wParam);
    return true;
}

void CSkUIWindow::Close()
{
	ASSERT(::IsWindow(m_hWnd));
	if( !::IsWindow(m_hWnd) ) return;
	PostMessage(WM_CLOSE);
}

void CSkUIWindow::CenterWindow()
{
	ASSERT(::IsWindow(m_hWnd));
	ASSERT((GetWindowStyle(m_hWnd)&WS_CHILD)==0);
	RECT rcDlg = { 0 };
	::GetWindowRect(m_hWnd, &rcDlg);
	RECT rcArea = { 0 };
	RECT rcCenter = { 0 };
	HWND hWndParent = ::GetParent(m_hWnd);
	HWND hWndCenter = ::GetWindowOwner(m_hWnd);
	::SystemParametersInfo(SPI_GETWORKAREA, NULL, &rcArea, NULL);
	if( hWndCenter == NULL ) rcCenter = rcArea; else ::GetWindowRect(hWndCenter, &rcCenter);

	int DlgWidth = rcDlg.right - rcDlg.left;
	int DlgHeight = rcDlg.bottom - rcDlg.top;

	// Find dialog's upper left based on rcCenter
	int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
	int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

	// The dialog is outside the screen, move it inside
	if( xLeft < rcArea.left ) xLeft = rcArea.left;
	else if( xLeft + DlgWidth > rcArea.right ) xLeft = rcArea.right - DlgWidth;
	if( yTop < rcArea.top ) yTop = rcArea.top;
	else if( yTop + DlgHeight > rcArea.bottom ) yTop = rcArea.bottom - DlgHeight;
	::SetWindowPos(m_hWnd, NULL, xLeft, yTop, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void CSkUIWindow::SetIcon(UINT nRes)
{
	HICON hIcon = (HICON)::LoadImage(CPaintManagerUI::GetInstance(), MAKEINTRESOURCE(nRes), IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	ASSERT(hIcon);
	::SendMessage(m_hWnd, WM_SETICON, (WPARAM) TRUE, (LPARAM) hIcon);
	hIcon = (HICON)::LoadImage(CPaintManagerUI::GetInstance(), MAKEINTRESOURCE(nRes), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	ASSERT(hIcon);
	::SendMessage(m_hWnd, WM_SETICON, (WPARAM) FALSE, (LPARAM) hIcon);
}

bool CSkUIWindow::UnRegisterWindowClass()
{
	ATOM ret = ::UnregisterClass(GetWindowClassName(), CPaintManagerUI::GetInstance());
	ASSERT(ret!=NULL || ::GetLastError()==ERROR_CLASS_ALREADY_EXISTS);
	return ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

bool CSkUIWindow::UnRegisterSuperclass()
{
	ATOM ret = ::UnregisterClass(GetWindowClassName(), CPaintManagerUI::GetInstance());
	ASSERT(ret!=NULL || ::GetLastError()==ERROR_CLASS_ALREADY_EXISTS);
	return ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

bool CSkUIWindow::RegisterWindowClass()
{
	WNDCLASS wc = { 0 };
	wc.style = GetClassStyle();
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hIcon = NULL;
	wc.lpfnWndProc = CSkUIWindow::__WndProc;
	wc.hInstance = CPaintManagerUI::GetInstance();
#if defined(UI_BUILD_FOR_WIN32) && !defined(UI_BUILD_FOR_WINCE)
	wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
#else
	wc.hCursor = NULL;
#endif
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = GetWindowClassName();
	ATOM ret = ::RegisterClass(&wc);
	ASSERT(ret!=NULL || ::GetLastError()==ERROR_CLASS_ALREADY_EXISTS);
	return ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

bool CSkUIWindow::RegisterSuperclass()
{
	// Get the class information from an existing
	// window so we can subclass it later on...
#if defined(UI_BUILD_FOR_WIN32) && !defined(UI_BUILD_FOR_WINCE)
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEX);
	if( !::GetClassInfoEx(NULL, GetSuperClassName(), &wc) ) {
		if( !::GetClassInfoEx(CPaintManagerUI::GetInstance(), GetSuperClassName(), &wc) ) {
			ASSERT(!"Unable to locate window class");
			return NULL;
		}
	}
	m_OldWndProc = wc.lpfnWndProc;
	wc.lpfnWndProc = CSkUIWindow::__ControlProc;
	wc.hInstance = CPaintManagerUI::GetInstance();
	wc.lpszClassName = GetWindowClassName();
	ATOM ret = ::RegisterClassEx(&wc);
#else
	WNDCLASS wc = { 0 };

	if( !::GetClassInfo(NULL, GetSuperClassName(), &wc) ) {
		if( !::GetClassInfo(CPaintManagerUI::GetInstance(), GetSuperClassName(), &wc) ) {
			ASSERT(!"Unable to locate window class");
			return NULL;
		}
	}

	wc.hbrBackground = (HBRUSH)::GetStockObject(WHITE_BRUSH);
	wc.hCursor		 = ::LoadCursor(NULL, IDC_ARROW);
	m_OldWndProc = wc.lpfnWndProc;
	wc.lpfnWndProc = CSkUIWindow::__ControlProc;
	wc.hInstance = CPaintManagerUI::GetInstance();
	wc.lpszClassName = GetWindowClassName();
	ATOM ret = ::RegisterClass(&wc);
#endif
	ASSERT(ret!=NULL || ::GetLastError()==ERROR_CLASS_ALREADY_EXISTS);
	return ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

LRESULT CALLBACK CSkUIWindow::__WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CSkUIWindow* pThis = NULL;
#if defined(UI_BUILD_FOR_WIN32) && !defined(UI_BUILD_FOR_WINCE)
	if( uMsg == WM_NCCREATE ) {
		LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pThis = static_cast<CSkUIWindow*>(lpcs->lpCreateParams);
		pThis->m_hWnd = hWnd;
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(pThis));
	} 
	else {
		pThis = reinterpret_cast<CSkUIWindow*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
		if( uMsg == WM_NCDESTROY && pThis != NULL ) {
			LRESULT lRes = ::CallWindowProc(pThis->m_OldWndProc, hWnd, uMsg, wParam, lParam);
			::SetWindowLongPtr(pThis->m_hWnd, GWLP_USERDATA, 0L);
			if( pThis->m_bSubclassed ) pThis->Unsubclass();
			pThis->m_hWnd = NULL;
			pThis->OnFinalMessage(hWnd);
			return lRes;
		}
	}
#else
	if( uMsg == WM_CREATE )
	{
		LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pThis = static_cast<CSkUIWindow*>(lpcs->lpCreateParams);
		pThis->m_hWnd = hWnd;
		::SetWindowLong(hWnd, GWL_USERDATA, reinterpret_cast<LPARAM>(pThis));
	} 
	else
	{
		pThis = reinterpret_cast<CSkUIWindow*>(::GetWindowLong(hWnd, GWL_USERDATA));
		if( uMsg == WM_DESTROY && pThis != NULL )
		{
			pThis->HandleMessage(uMsg, wParam, lParam);
			LRESULT lRes = ::CallWindowProc(pThis->m_OldWndProc, hWnd, uMsg, wParam, lParam);
			::SetWindowLong(pThis->m_hWnd, GWL_USERDATA, 0L);
			if( pThis->m_bSubclassed ) 
				pThis->Unsubclass();
			pThis->m_hWnd = NULL;
			pThis->OnFinalMessage(hWnd);
			return lRes;
		}
	}
#endif
	if( pThis != NULL ) {
		return pThis->HandleMessage(uMsg, wParam, lParam);
	} 
	else {
		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}

LRESULT CALLBACK CSkUIWindow::__ControlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CSkUIWindow* pThis = NULL;
#if defined(UI_BUILD_FOR_WIN32) && !defined(UI_BUILD_FOR_WINCE)
	if( uMsg == WM_NCCREATE ) {
		LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pThis = static_cast<CSkUIWindow*>(lpcs->lpCreateParams);
		::SetProp(hWnd, _T("WndX"), (HANDLE) pThis);
		pThis->m_hWnd = hWnd;
	} 
	else {
		pThis = reinterpret_cast<CSkUIWindow*>(::GetProp(hWnd, _T("WndX")));
		if( uMsg == WM_NCDESTROY && pThis != NULL ) {
			LRESULT lRes = ::CallWindowProc(pThis->m_OldWndProc, hWnd, uMsg, wParam, lParam);
			if( pThis->m_bSubclassed ) pThis->Unsubclass();
			::SetProp(hWnd, _T("WndX"), NULL);
			pThis->m_hWnd = NULL;
			pThis->OnFinalMessage(hWnd);
			return lRes;
		}
	}
#else
	if( uMsg == WM_CREATE )
	{
		LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pThis = static_cast<CSkUIWindow*>(lpcs->lpCreateParams);
		::SetProp(hWnd, _T("WndX"), (HANDLE) pThis);
		pThis->m_hWnd = hWnd;
	} 
	else
	{
		pThis = reinterpret_cast<CSkUIWindow*>(::GetProp(hWnd, _T("WndX")));
		if( uMsg == WM_DESTROY && pThis != NULL )
		{
			pThis->HandleMessage(uMsg, wParam, lParam);

			LRESULT lRes = ::CallWindowProc(pThis->m_OldWndProc, hWnd, uMsg, wParam, lParam);
			if( pThis->m_bSubclassed )
				pThis->Unsubclass();
			::SetProp(hWnd, _T("WndX"), NULL);
			pThis->m_hWnd = NULL;
			pThis->OnFinalMessage(hWnd);
			return lRes;
		}
	}
#endif
	if( pThis != NULL ) {
		return pThis->HandleMessage(uMsg, wParam, lParam);
	} 
	else {
		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}

LRESULT CSkUIWindow::SendMessage(UINT uMsg, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	ASSERT(::IsWindow(m_hWnd));
	return ::SendMessage(m_hWnd, uMsg, wParam, lParam);
} 

LRESULT CSkUIWindow::PostMessage(UINT uMsg, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	ASSERT(::IsWindow(m_hWnd));
	return ::PostMessage(m_hWnd, uMsg, wParam, lParam);
}

void CSkUIWindow::ResizeClient(int cx /*= -1*/, int cy /*= -1*/)
{
	ASSERT(::IsWindow(m_hWnd));
	RECT rc = { 0 };
	if( !::GetClientRect(m_hWnd, &rc) ) return;
	if( cx != -1 ) rc.right = cx;
	if( cy != -1 ) rc.bottom = cy;
#if defined(UI_BUILD_FOR_WIN32) && !defined(UI_BUILD_FOR_WINCE)
	if( !::AdjustWindowRectEx(&rc, GetWindowStyle(m_hWnd), (!(GetWindowStyle(m_hWnd) & WS_CHILD) && (::GetMenu(m_hWnd) != NULL)), GetWindowExStyle(m_hWnd)) ) return;
#else
	if( !::AdjustWindowRectEx(&rc, GetWindowStyle(m_hWnd), (!(GetWindowStyle(m_hWnd) & WS_CHILD)), GetWindowExStyle(m_hWnd)) ) return;
#endif
	UINT uFlags = SWP_NOZORDER | SWP_NOMOVE;
	::SetWindowPos(m_hWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, uFlags);
}

void CSkUIWindow::OnFinalMessage(HWND /*hWnd*/)
{
	if( GetSuperClassName() != NULL ) RegisterSuperclass();
	if( GetSuperClassName() == NULL ) RegisterWindowClass();
}

LRESULT CSkUIWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_paintManager.SetSkiaContext(&fWinWindow);

	bHandled = TRUE;
	return 0;
}

LRESULT CSkUIWindow::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CSkUIWindow::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

#if defined(UI_BUILD_FOR_WIN32) && !defined(UI_BUILD_FOR_WINCE)
LRESULT CSkUIWindow::OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return (wParam == 0) ? TRUE : FALSE;
}

LRESULT CSkUIWindow::OnNcCalcSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CSkUIWindow::OnNcPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CSkUIWindow::OnNcHitTest(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return HTCLIENT;
}
#endif

LRESULT CSkUIWindow::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RECT rcClient;
	::GetClientRect(GetHWND(), &rcClient);
	this->resize(rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, SkBitmap::kARGB_8888_Config);

	m_backBitmap.reset();
	InvalidateRect(m_hWnd, NULL, TRUE);

	bHandled = FALSE;
	return 0;
}

#if defined(UI_BUILD_FOR_WIN32) && !defined(UI_BUILD_FOR_WINCE)
LRESULT CSkUIWindow::OnGetMinMaxInfo(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
{
	MONITORINFO oMonitor = {};
	oMonitor.cbSize = sizeof(oMonitor);
	::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
	CRect rcWork = oMonitor.rcWork;
	rcWork.Offset(-rcWork.left, -rcWork.top);

	LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;
	lpMMI->ptMaxPosition.x	= rcWork.left;
	lpMMI->ptMaxPosition.y	= rcWork.top;
	lpMMI->ptMaxSize.x		= rcWork.right;
	lpMMI->ptMaxSize.y		= rcWork.bottom;

	bHandled = FALSE;
	return 0;
}
#endif

LRESULT CSkUIWindow::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;
	LRESULT lRes = ::CallWindowProc(m_OldWndProc, m_hWnd, uMsg, wParam, lParam);
	return lRes;
}

LRESULT CSkUIWindow::OnMouseActivate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CSkUIWindow::OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CSkUIWindow::OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CSkUIWindow::OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CSkUIWindow::OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CSkUIWindow::OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CSkUIWindow::OnMouseWheel(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CSkUIWindow::OnKillFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	return 0;
}

LRESULT CSkUIWindow::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	return 0;
}

void CSkUIWindow::onHandleInval(const SkIRect& r)
{
	RECT rcInval = {0};
	rcInval.left = r.fLeft;
	rcInval.top = r.fTop;
	rcInval.right = r.fRight;
	rcInval.bottom = r.fBottom;

	InvalidateRect(m_hWnd, &rcInval, TRUE);
}

LRESULT CSkUIWindow::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (m_paintManager.GetPaintWindow() == NULL)
		return 0;

	bHandled = TRUE;

	SkBitmap bm = this->getBitmap();
	const SkBitmap& bitmap = this->getBitmap();

	CRect rcPaint;
	CRect rcClient;
	GetUpdateRect(m_hWnd, &rcPaint, FALSE);
	GetClientRect(m_hWnd, &rcClient);

	HDC hdc = GetDC(m_hWnd);
	wParam = (WPARAM)hdc;

	fWinWindow.hWnd = m_hWnd;
	fWinWindow.hDC = hdc;

	lParam = (LPARAM)&fWinWindow;
	SkCanvas*	canvas = &fWinWindow.rasterCanvas;
	SkDevice*	device = new SkDevice(canvas, bm, false);
	canvas->setDevice(device)->unref();

	LRESULT lRes = 0;
	SkRect updateRect = SkRect::MakeXYWH(rcPaint.left, rcPaint.top, rcPaint.Width(), rcPaint.Height());

	if (canvas != NULL)
		m_paintManager.MessageHandler(uMsg, wParam, lParam, lRes);

	OnPaint(uMsg, wParam, lParam);

	if ((rcPaint == rcClient) && m_backBitmap.isNull()) {
		m_backBitmap.setConfig(bitmap.getConfig(), bitmap.width(), bitmap.height());
		m_backBitmap.allocPixels();
		m_backBitmap.copyPixelsFrom(bitmap.getPixels(), bitmap.getSize());
	}
	{
		SkPaint paint;
		paint.setAntiAlias(true);
		paint.setStyle(SkPaint::kFill_Style);

		SkCanvas canvas(m_backBitmap);
		SkIRect src = SkIRect::MakeXYWH(updateRect.fLeft, updateRect.fTop, updateRect.width(), updateRect.height());
		SkRect dest = updateRect;
		canvas.drawBitmapRect(bitmap, &src, dest, &paint);
	}

	BITMAPINFO bmi;
	memset(&bmi, 0, sizeof(bmi));
	bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth       = m_backBitmap.width();
	bmi.bmiHeader.biHeight      = -m_backBitmap.height(); // top-down image 
	bmi.bmiHeader.biPlanes      = 1;
	bmi.bmiHeader.biBitCount    = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage   = 0;

	// 
	// Do the SetDIBitsToDevice. 
	// 
	// TODO(wjmaclean):
	//       Fix this call to handle SkBitmaps that have rowBytes != width,
	//       i.e. may have padding at the end of lines. The SkASSERT below
	//       may be ignored by builds, and the only obviously safe option
	//       seems to be to copy the bitmap to a temporary (contiguous)
	//       buffer before passing to SetDIBitsToDevice().
	SkASSERT(m_backBitmap.width() * m_backBitmap.bytesPerPixel() == m_backBitmap.rowBytes());
	m_backBitmap.lockPixels();
	int iRet = SetDIBitsToDevice(hdc,
		0, 0,
		m_backBitmap.width(), m_backBitmap.height(),
		0, 0,
		0, m_backBitmap.height(),
		m_backBitmap.getPixels(),
		&bmi,
		DIB_RGB_COLORS);

	m_backBitmap.unlockPixels();

	ReleaseDC(m_hWnd, hdc);

	//if (0)
	//{
	//	SkBitmap bm;
	//	bm.setConfig(bitmap.getConfig(), updateRect.width(), updateRect.height());
	//	bm.allocPixels();
	//	SkCanvas canvas(bm);

	//	SkIRect src = SkIRect::MakeXYWH(updateRect.fLeft, updateRect.fTop, updateRect.width(), updateRect.height());
	//	SkRect dst = SkRect::MakeXYWH(0, 0, updateRect.width(), updateRect.height());

	//	canvas.save();
	//	canvas.drawBitmapRect(bitmap, &src, dst);
	//	canvas.restore();

	//	static int nSnapshotIndex = 0;
	//	char szBuf[MAX_PATH] = {0};
	//	sprintf(szBuf, "c:\\snapshot\\snapshot_%d.png", nSnapshotIndex++);
	//	SkImageEncoder::EncodeFile(szBuf, bm, SkImageEncoder::kPNG_Type, /* Quality ranges from 0..100 */ 100);
	//}

	return 1;
}

#if defined(UI_BUILD_FOR_WIN32) && !defined(UI_BUILD_FOR_WINCE)
LRESULT CSkUIWindow::OnPrintClient(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;

	SkBitmap bm = this->getBitmap();
	const SkBitmap& bitmap = this->getBitmap();

	fWinWindow.hWnd = m_hWnd;
	fWinWindow.hDC = GetDC(m_hWnd);

	SkCanvas*	canvas = &fWinWindow.rasterCanvas;
	SkDevice*	device = new SkDevice(canvas, bm, false);
	canvas->setDevice(device)->unref();
	if (canvas != NULL)
	{
		LRESULT lRes = 0;
		lParam = (LPARAM)&fWinWindow;
		m_paintManager.MessageHandler(uMsg, wParam, lParam, lRes);
	}
	ReleaseDC(m_hWnd, (HDC)fWinWindow.hDC);
	return 1;
}
#endif

LRESULT CSkUIWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	BOOL bHandled = FALSE;
	switch( uMsg ) {
		case WM_CREATE:			lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
		case WM_CLOSE:			lRes = OnClose(uMsg, wParam, lParam, bHandled); break;
		case WM_DESTROY:		lRes = OnDestroy(uMsg, wParam, lParam, bHandled); break;
#if defined(UI_BUILD_FOR_WIN32) && !defined(UI_BUILD_FOR_WINCE)
		case WM_NCACTIVATE:		lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
		case WM_NCCALCSIZE:		lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
		case WM_NCPAINT:		lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
		case WM_NCHITTEST:		lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
		case WM_GETMINMAXINFO:	lRes = OnGetMinMaxInfo(uMsg, wParam, lParam, bHandled); break;
		case WM_PRINTCLIENT:	lRes = OnPrintClient(uMsg, wParam, lParam, bHandled); break;
#endif
		case WM_SIZE:			lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
		case WM_SYSCOMMAND:		lRes = OnSysCommand(uMsg, wParam, lParam, bHandled); break;
		case WM_PAINT:			lRes = OnPaint(uMsg, wParam, lParam, bHandled); break;
		case WM_MOUSEWHEEL:		lRes = OnMouseWheel(uMsg, wParam, lParam, bHandled); break;
		case WM_KILLFOCUS:		lRes = OnKillFocus(uMsg, wParam, lParam, bHandled); break;
		case WM_SETFOCUS:		lRes = OnSetFocus(uMsg, wParam, lParam, bHandled); break;
		case WM_KEYDOWN:		lRes = OnKeyDown(uMsg, wParam, lParam, bHandled); break;
		case WM_LBUTTONUP:		lRes = OnLButtonUp(uMsg, wParam, lParam, bHandled); break;
		case WM_LBUTTONDOWN:	lRes = OnLButtonDown(uMsg, wParam, lParam, bHandled); break;
		case WM_ERASEBKGND:		lRes = OnEraseBkgnd(uMsg, wParam, lParam, bHandled); break;
		case WM_MOUSEACTIVATE:	lRes = OnMouseActivate(uMsg, wParam, lParam, bHandled); break;
		default:
			bHandled = FALSE;
	}
	if( bHandled ) return lRes;

	lRes = HandleMessage(uMsg, wParam, lParam, bHandled);
	if( bHandled ) return lRes;

	if( m_paintManager.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;

	return ::CallWindowProc(m_OldWndProc, m_hWnd, uMsg, wParam, lParam);
}


///////////////////////////////////////////////////////////////////////////////////////

void SkEvent::SignalNonEmptyQueue()
{
	post_skwinevent();
	//SkDebugf("signal nonempty\n");
}

static UINT_PTR gTimer;

VOID CALLBACK sk_timer_proc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	SkEvent::ServiceQueueTimer();
	//SkDebugf("timer task fired\n");
}

void SkEvent::SignalQueueTimer(SkMSec delay)
{
	if (gTimer)
	{
		KillTimer(NULL, gTimer);
		gTimer = NULL;
	}
	if (delay)
	{     
		gTimer = SetTimer(NULL, 0, delay, sk_timer_proc);
		//SkDebugf("SetTimer of %d returned %d\n", delay, gTimer);
	}
}

#endif
