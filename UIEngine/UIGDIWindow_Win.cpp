#include "StdAfx.h"
#include "UIlib.h"

#pragma warning(disable : 4312)
#pragma warning(disable : 4244)

namespace DuiLib {

#ifdef UI_BUILD_FOR_WINGDI

/////////////////////////////////////////////////////////////////////////////////////
//
//


/////////////////////////////////////////////////////////////////////////////////////
//
//

class CMessageBox : public CWindowWnd, public INotifyUI
{
public:
	CMessageBox() : m_iRetCode(IDCANCEL) {}
	LPCTSTR GetWindowClassName() const { return _T("UIMESSAGEBOX"); }
	UINT GetClassStyle() const { return UI_CLASSSTYLE_DIALOG; };
	void OnFinalMessage(HWND hWnd) { }
	void SetContentText(LPCTSTR lpText) { m_sContent = lpText; }
	int GetRetCode() { return m_iRetCode; }
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
		if( uMsg == WM_CREATE ) {     
			paint_manager_.Init(m_hWnd);
			CDialogBuilder builder;
			CControlUI* pRoot = builder.Create(GetDialogResource(), (UINT)0, NULL, &paint_manager_);
			paint_manager_.AttachDialog(pRoot);
			paint_manager_.AddNotifier(this);

			CControlUI *pText = paint_manager_.FindControl(_T("content"));
			if( pText ) pText->SetText(m_sContent);
			CenterWindow();
			return 0;
		}
		else if( uMsg == WM_KEYDOWN ) {
			if( wParam == VK_RETURN ) {
				m_iRetCode = IDOK;
				Close();
				return 0;
			}
			else if( wParam == VK_ESCAPE ) {
				m_iRetCode = IDCANCEL;
				Close();
				return 0;
			}
		}
		LRESULT lRes = 0;
		if( paint_manager_.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	}
	void Notify(TNotifyUI& msg) {
		if( msg.sType == kClick ) {
			if( msg.pSender->GetName() == _T("ok")) {
				m_iRetCode = IDOK;
				Close();
			}
		}
	}
	LPCTSTR GetDialogResource() const {
		return _T("<Window size=\"360,120\">")
			_T("    <VerticalLayout bkcolor=\"#FFFFFFFF\" inset=\"12, 8, 12, 8\" >" )
			_T("        <Text name=\"content\" maxheight=\"48\" />")
			_T("        <Control />")
			_T("        <HorizontalLayout height=\"22\">")
			_T("            <Control />")
			_T("            <Button name=\"ok\" text=\"ȷ\" bordercolor=\"#FF000000\" width=\"60\"/>")
			_T("            <Control />")
			_T("        </HorizontalLayout>")
			_T("    </VerticalLayout>")
			_T("</Window>");
	}

public:
	CPaintManagerUI paint_manager_;
	CStdString m_sContent;
	int m_iRetCode;
};

/////////////////////////////////////////////////////////////////////////////////////
//
//
CWindowWnd::CWindowWnd() : m_hWnd(NULL), m_OldWndProc(::DefWindowProc), m_bSubclassed(false)
{
}

CWindowWnd::~CWindowWnd()
{
}

HWND CWindowWnd::GetHWND() const 
{ 
	return m_hWnd; 
}

UINT CWindowWnd::GetClassStyle() const
{
	return 0;
}

LPCTSTR CWindowWnd::GetSuperClassName() const
{
	return NULL;
}

CWindowWnd::operator HWND() const
{
	return m_hWnd;
}

HWND CWindowWnd::Create(HWND hwndParent, LPCTSTR pstrName, DWORD dwStyle, DWORD dwExStyle, const RECT rc, HMENU hMenu)
{
	return Create(hwndParent, pstrName, dwStyle, dwExStyle, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hMenu);
}

HWND CWindowWnd::Create(HWND hwndParent, LPCTSTR pstrName, DWORD dwStyle, DWORD dwExStyle, int x, int y, int cx, int cy, HMENU hMenu)
{
	if( GetSuperClassName() != NULL && !RegisterSuperclass() ) return NULL;
	if( GetSuperClassName() == NULL && !RegisterWindowClass() ) return NULL;

	m_hWnd = ::CreateWindowEx(dwExStyle, GetWindowClassName(), pstrName, dwStyle, x, y, cx, cy, hwndParent, hMenu, CPaintManagerUI::GetInstance(), this);
	ASSERT(m_hWnd!=NULL);
	return m_hWnd;
}

HWND CWindowWnd::Subclass(HWND hWnd)
{
	ASSERT(::IsWindow(hWnd));
	ASSERT(m_hWnd==NULL);
	m_OldWndProc = SubclassWindow(hWnd, __WndProc);
	if( m_OldWndProc == NULL ) return NULL;
	m_bSubclassed = true;
	m_hWnd = hWnd;
	return m_hWnd;
}

void CWindowWnd::Unsubclass()
{
	ASSERT(::IsWindow(m_hWnd));
	if( !::IsWindow(m_hWnd) ) return;
	if( !m_bSubclassed ) return;
	SubclassWindow(m_hWnd, m_OldWndProc);
	m_OldWndProc = ::DefWindowProc;
	m_bSubclassed = false;
}

void CWindowWnd::ShowWindow(bool bShow /*= true*/, bool bTakeFocus /*= false*/)
{
	ASSERT(::IsWindow(m_hWnd));
	if( !::IsWindow(m_hWnd) ) return;
	::ShowWindow(m_hWnd, bShow ? (bTakeFocus ? SW_SHOWNORMAL : SW_SHOWNOACTIVATE) : SW_HIDE);
}

UINT CWindowWnd::ShowModal()
{
	ASSERT(::IsWindow(m_hWnd));
	UINT nRet = 0;
	HWND hWndParent = GetWindowOwner(m_hWnd);
	ASSERT(hWndParent != m_hWnd);
	MSG msg = { 0 };
	bool bNeedClose = false;
	while( ::IsWindow(m_hWnd) && ::GetMessage(&msg, NULL, 0, 0) ) {
		if( msg.message == WM_CLOSE ) {
			nRet = msg.wParam;
			if( msg.hwnd == hWndParent ) {
				::EnableWindow(m_hWnd, TRUE);
				::SetFocus(m_hWnd);
			}
			else if( msg.hwnd == m_hWnd ) {
				ShowWindow(false, false);
				bNeedClose = true;
				break;
			}
		}

		if( msg.hwnd != m_hWnd ) {
			if( msg.hwnd == hWndParent ) {
				if( (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST) ) continue;
				if( msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST ) continue;
				if( msg.message == WM_SETCURSOR ) continue;
			}
			else {
				if( !IsChild(m_hWnd, msg.hwnd) ) {
					if( (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST) ) continue;
					if( msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST ) continue;
					if( msg.message == WM_SETCURSOR ) continue;
				}
			}
		}

		if( !CPaintManagerUI::TranslateMessage(&msg) ) {
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}

		if( msg.message == WM_QUIT ) break;
	}
	if( !bNeedClose ) ::EnableWindow(m_hWnd, TRUE);
	::SetFocus(m_hWnd);
	if( msg.message == WM_QUIT ) ::PostQuitMessage(msg.wParam);
	else if( bNeedClose ) SendMessage(WM_CLOSE);

	return nRet;
}

UINT CWindowWnd::ShowModal(HWND hWnd)
{
    ASSERT(::IsWindow(m_hWnd));
	UINT nRet = 0;
    HWND hWndParent = GetWindowOwner(m_hWnd);
    ::ShowWindow(m_hWnd, SW_SHOWNORMAL);
    ::EnableWindow(hWndParent, FALSE);
    MSG msg = { 0 };
    while( ::IsWindow(m_hWnd) && ::GetMessage(&msg, NULL, 0, 0) ) {
        if( msg.message == WM_CLOSE && msg.hwnd == m_hWnd ) {
			nRet = msg.wParam;
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
    return nRet;

}

void CWindowWnd::Close(UINT nRet)
{
	ASSERT(::IsWindow(m_hWnd));
	if( !::IsWindow(m_hWnd) ) return;
	PostMessage(WM_CLOSE, (WPARAM)nRet, 0L);
}

void CWindowWnd::CenterWindow()
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

void CWindowWnd::SetIcon(UINT nRes)
{
	HICON hIcon = (HICON)::LoadImage(CPaintManagerUI::GetInstance(), MAKEINTRESOURCE(nRes), IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	ASSERT(hIcon);
	::SendMessage(m_hWnd, WM_SETICON, (WPARAM) TRUE, (LPARAM) hIcon);
	hIcon = (HICON)::LoadImage(CPaintManagerUI::GetInstance(), MAKEINTRESOURCE(nRes), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	ASSERT(hIcon);
	::SendMessage(m_hWnd, WM_SETICON, (WPARAM) FALSE, (LPARAM) hIcon);
}

bool CWindowWnd::UnRegisterWindowClass()
{
	ATOM ret = ::UnregisterClass(GetWindowClassName(), CPaintManagerUI::GetInstance());
	DWORD dwError = GetLastError();
	ASSERT(ret!=NULL || ::GetLastError()==ERROR_CLASS_ALREADY_EXISTS);
	return ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

bool CWindowWnd::UnRegisterSuperclass()
{
	ATOM ret = ::UnregisterClass(GetWindowClassName(), CPaintManagerUI::GetInstance());
	ASSERT(ret!=NULL || ::GetLastError()==ERROR_CLASS_ALREADY_EXISTS);
	return ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

bool CWindowWnd::RegisterWindowClass()
{
	WNDCLASS wc = { 0 };
	wc.style = GetClassStyle();
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hIcon = NULL;
	wc.lpfnWndProc = CWindowWnd::__WndProc;
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

bool CWindowWnd::RegisterSuperclass()
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
	wc.lpfnWndProc = CWindowWnd::__ControlProc;
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
	wc.lpfnWndProc = CWindowWnd::__ControlProc;
	wc.hInstance = CPaintManagerUI::GetInstance();
	wc.lpszClassName = GetWindowClassName();
	ATOM ret = ::RegisterClass(&wc);
#endif
	ASSERT(ret!=NULL || ::GetLastError()==ERROR_CLASS_ALREADY_EXISTS);
	return ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

LRESULT CALLBACK CWindowWnd::__WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CWindowWnd* pThis = NULL;
#if defined(UI_BUILD_FOR_WIN32) && !defined(UI_BUILD_FOR_WINCE)
	if( uMsg == WM_NCCREATE ) {
		LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pThis = static_cast<CWindowWnd*>(lpcs->lpCreateParams);
		pThis->m_hWnd = hWnd;
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(pThis));
	} 
	else {
		pThis = reinterpret_cast<CWindowWnd*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
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
		pThis = static_cast<CWindowWnd*>(lpcs->lpCreateParams);
		pThis->m_hWnd = hWnd;
		::SetWindowLong(hWnd, GWL_USERDATA, reinterpret_cast<LPARAM>(pThis));
	} 
	else
	{
		pThis = reinterpret_cast<CWindowWnd*>(::GetWindowLong(hWnd, GWL_USERDATA));
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

LRESULT CALLBACK CWindowWnd::__ControlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CWindowWnd* pThis = NULL;
#if defined(UI_BUILD_FOR_WIN32) && !defined(UI_BUILD_FOR_WINCE)
	if( uMsg == WM_NCCREATE ) {
		LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pThis = static_cast<CWindowWnd*>(lpcs->lpCreateParams);
		::SetProp(hWnd, _T("WndX"), (HANDLE) pThis);
		pThis->m_hWnd = hWnd;
	} 
	else {
		pThis = reinterpret_cast<CWindowWnd*>(::GetProp(hWnd, _T("WndX")));
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
		pThis = static_cast<CWindowWnd*>(lpcs->lpCreateParams);
		::SetProp(hWnd, _T("WndX"), (HANDLE) pThis);
		pThis->m_hWnd = hWnd;
	} 
	else
	{
		pThis = reinterpret_cast<CWindowWnd*>(::GetProp(hWnd, _T("WndX")));
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

LRESULT CWindowWnd::SendMessage(UINT uMsg, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	ASSERT(::IsWindow(m_hWnd));
	return ::SendMessage(m_hWnd, uMsg, wParam, lParam);
} 

LRESULT CWindowWnd::PostMessage(UINT uMsg, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	ASSERT(::IsWindow(m_hWnd));
	return ::PostMessage(m_hWnd, uMsg, wParam, lParam);
}

void CWindowWnd::ResizeClient(int cx /*= -1*/, int cy /*= -1*/)
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
	::SetWindowPos(m_hWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
}

LRESULT CWindowWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ::CallWindowProc(m_OldWndProc, m_hWnd, uMsg, wParam, lParam);
}

void CWindowWnd::OnFinalMessage(HWND /*hWnd*/)
{}

/////////////////////////////////////////////////////////////////////////////////////
//
//

CWaitCursor::CWaitCursor()
{
	m_hOrigCursor = ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
}

CWaitCursor::~CWaitCursor()
{
	::SetCursor(m_hOrigCursor);
}

#endif

} // namespace DuiLib
