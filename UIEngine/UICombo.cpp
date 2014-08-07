#include "StdAfx.h"
#include "UIlib.h"

namespace DuiLib {


/////////////////////////////////////////////////////////////////////////////////////
//
//
class CEditWndEx : public CWindowWnd
{
public:
    CEditWndEx();
	~CEditWndEx();

    void Init(IEditUI* pOwner);
	RECT CalPos();
	void SetEditWndText(LPCTSTR lpsText,bool bSelect=true);
	void MoveEditWnd(RECT rc);

    LPCTSTR GetWindowClassName() const;
    LPCTSTR GetSuperClassName() const;
    void OnFinalMessage(HWND hWnd);

    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnEditChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void BreakLinkage();

	DWORD GetBkColor();
	DWORD GetEditTextColor();

protected:	
    IEditUI* m_pOwner;
	HBRUSH m_hBkBrush;
};


CEditWndEx::CEditWndEx() : m_pOwner(NULL), m_hBkBrush(NULL)
{
}

CEditWndEx::~CEditWndEx()
{
}

void CEditWndEx::Init(IEditUI* pOwner)
{
	m_pOwner = pOwner;
	RECT rcPos = CalPos();
	UINT uStyle = WS_CHILD;
	if (m_pOwner->IsPasswordMode()) uStyle |= ES_PASSWORD;
	if (m_pOwner->IsMultiLine())
	{
		uStyle |= ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL;
	}
	else
	{
		uStyle |= ES_AUTOHSCROLL;
	}
#if(WINVER >= 0x0400)
	if (m_pOwner->IsDigitalNumber()) 
	{
		uStyle |= ES_NUMBER;
	}
#endif

	Create(m_pOwner->GetManager()->GetPaintWindow(), NULL, uStyle, 0, rcPos);
	SetWindowFont(m_hWnd, m_pOwner->GetManager()->GetDefaultFontInfo()->hFont, TRUE);
	if (!m_pOwner->IsMultiLine()) Edit_LimitText(m_hWnd, m_pOwner->GetMaxChar());
	if (m_pOwner->IsPasswordMode()) Edit_SetPasswordChar(m_hWnd, m_pOwner->GetPasswordChar());	
	Edit_SetText(m_hWnd, m_pOwner->GetText());
	Edit_SetModify(m_hWnd, FALSE);
	SendMessage(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(0, 0));
	Edit_Enable(m_hWnd, m_pOwner->IsEnabled() == true);
	Edit_SetReadOnly(m_hWnd, m_pOwner->IsReadOnly() == true);
	::ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
	::SetFocus(m_hWnd);
}

RECT CEditWndEx::CalPos()
{
	CRect rcPos = m_pOwner->GetEditPos();
	RECT rcPadding = m_pOwner->GetEditTextPadding();
	rcPos.left += rcPadding.left;
	rcPos.top += rcPadding.top;
	rcPos.right -= rcPadding.right;
	rcPos.bottom -= rcPadding.bottom;
	if (!m_pOwner->IsMultiLine())
	{
		LONG lEditHeight = m_pOwner->GetManager()->GetDefaultFontInfo()->tm.tmHeight;
		if( lEditHeight < rcPos.GetHeight() ) {
			rcPos.top += (rcPos.GetHeight() - lEditHeight) / 2;
			rcPos.bottom = rcPos.top + lEditHeight;
		}
	}
    return rcPos;
}

void CEditWndEx::MoveEditWnd(RECT rc)
{
	::MoveWindow(m_hWnd,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,FALSE);
}

void CEditWndEx::SetEditWndText(LPCTSTR lpsText,bool bSelect)
{
	Edit_SetText(m_hWnd, lpsText);
	Edit_SetModify(m_hWnd, FALSE);

	if(bSelect){
		int cchLen = ::GetWindowTextLength(m_hWnd) + 1;
		Edit_SetSel(m_hWnd,0,cchLen);
	}
}

LPCTSTR CEditWndEx::GetWindowClassName() const
{
    return _T("EditWnd");
}

LPCTSTR CEditWndEx::GetSuperClassName() const
{
    return WC_EDIT;
}

void CEditWndEx::BreakLinkage()
{
	m_pOwner = NULL;
}

DWORD CEditWndEx::GetEditTextColor()
{
	LPTSTR pstr = NULL;
	DWORD dwColor = 0xFF000000;
	if (m_pOwner != NULL)
		dwColor = m_pOwner->GetEditTextColor();
	return dwColor;
}

DWORD CEditWndEx::GetBkColor()
{
	LPTSTR pstr = NULL;
	DWORD dwColor = 0xFFFFFFFF;
	if (m_pOwner != NULL)
		dwColor = m_pOwner->GetBkColor();
	return dwColor;
}

void CEditWndEx::OnFinalMessage(HWND /*hWnd*/)
{
    // Clear reference and die
    if( m_hBkBrush != NULL ) ::DeleteObject(m_hBkBrush);
    if (m_pOwner != NULL) m_pOwner->SetEidtWndNull();
    delete this;
}

LRESULT CEditWndEx::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRes = 0;
	BOOL bHandled = TRUE;
	if( uMsg == WM_KILLFOCUS ) lRes = OnKillFocus(uMsg, wParam, lParam, bHandled);
    if( uMsg == OCM_COMMAND ) {
        if( GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE ) lRes = OnEditChanged(uMsg, wParam, lParam, bHandled);
        else if( GET_WM_COMMAND_CMD(wParam, lParam) == EN_UPDATE ) {
            RECT rcClient;
            ::GetClientRect(m_hWnd, &rcClient);
            ::InvalidateRect(m_hWnd, &rcClient, FALSE);
        }
    }
    else if( uMsg == WM_KEYDOWN && TCHAR(wParam) == VK_RETURN ) {
        m_pOwner->GetManager()->SendNotify(m_pOwner->GetHostedControl(), kReturn);
    }
	else if( uMsg == WM_CHAR ) {
		lRes = OnChar(uMsg, wParam, lParam, bHandled);
	}
	else if ( ( m_pOwner != NULL ) && (( uMsg == OCM__BASE + WM_CTLCOLOREDIT ) || ( uMsg == OCM__BASE + WM_CTLCOLORSTATIC ) ) ) {
		// Refer To: http://msdn.microsoft.com/en-us/library/bb761691(v=vs.85).aspx
		// Read-only or disabled edit controls do not send the WM_CTLCOLOREDIT message; instead, they send the WM_CTLCOLORSTATIC message.
		if( m_pOwner->GetBkColor() == 0 ) return NULL;
		HDC	hDC = (HDC) wParam;
		::SetBkMode(hDC, TRANSPARENT);
		if( m_hBkBrush == NULL ) {
			DWORD dwTextColor = m_pOwner->GetEditTextColor();
			DWORD dwBkColor = m_pOwner->GetBkColor();
			::SetTextColor(hDC, RGB(GetBValue(dwTextColor),GetGValue(dwTextColor),GetRValue(dwTextColor)));
			::SetBkColor(hDC, RGB(GetBValue(dwBkColor),GetGValue(dwBkColor),GetRValue(dwBkColor)));
			m_hBkBrush = ::CreateSolidBrush(RGB(GetBValue(dwBkColor), GetGValue(dwBkColor), GetRValue(dwBkColor)));
		}
		bHandled = TRUE;
		return (LRESULT)m_hBkBrush;
	}
	else bHandled = FALSE;

    if( !bHandled ) return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
    return lRes;
}

LRESULT CEditWndEx::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CEditWndEx::OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	switch (wParam) 
	{ 
	case 0x08:
		// Process a backspace.
		break;
	case 0x0A:
		// Process a linefeed.
		break;
	case 0x1B:
		// Process an escape.
		break;
	case 0x09:
		// Process a tab.
		break;
	case 0x0D:
		// Process a carriage return.
		break;
	default:
		// Process displayable characters.
		{
			bHandled = TRUE;
			CWindowWnd::HandleMessage(uMsg, wParam, lParam);

			if( m_pOwner == NULL ) return 0;
			// Copy text back
			int cchLen = ::GetWindowTextLength(m_hWnd) + 1;
			LPTSTR pstr = static_cast<LPTSTR>(_alloca(cchLen * sizeof(TCHAR)));
			ASSERT(pstr);
			if( pstr == NULL ) return 0;
			::GetWindowText(m_hWnd, pstr, cchLen);	
			if (m_pOwner->GetHostedControl() != NULL)
			{
				m_pOwner->GetHostedControl()->SetText(pstr);
				m_pOwner->GetHostedControl()->Invalidate();
				m_pOwner->GetManager()->SendNotify(m_pOwner->GetHostedControl(), kTextChanged);
			}
		}
		break;
	}
	return 0;
}

LRESULT CEditWndEx::OnEditChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	//if( m_pOwner == NULL ) return 0;
	//// Copy text back
	//int cchLen = ::GetWindowTextLength(m_hWnd) + 1;
	//LPTSTR pstr = static_cast<LPTSTR>(_alloca(cchLen * sizeof(TCHAR)));
	//ASSERT(pstr);
	//if( pstr == NULL ) return 0;
	//::GetWindowText(m_hWnd, pstr, cchLen);	
	//if (m_pOwner->GetHostedControl() != NULL)
	//{
	//	m_pOwner->GetHostedControl()->SetText(pstr);
	//	m_pOwner->GetHostedControl()->Invalidate();
	//	m_pOwner->GetManager()->SendNotify(m_pOwner->GetHostedControl(), kTextChanged);
	//}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

#if defined(UI_BUILD_FOR_WINGDI)
class CComboWnd : public CWindowWnd
#elif defined(UI_BUILD_FOR_SKIA)
class CComboWnd : public CSkUIWindow
#endif
{
#if defined(UI_BUILD_FOR_SKIA)
typedef CSkUIWindow INHERITED;
#endif
public:
	CComboWnd();
    void Init(CComboUI* pOwner);
    LPCTSTR GetWindowClassName() const;
    void OnFinalMessage(HWND hWnd);

#if defined(UI_BUILD_FOR_WINGDI)
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
#elif defined(UI_BUILD_FOR_SKIA)
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
#endif

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    void EnsureVisible(int iIndex);
    void Scroll(int dx, int dy);

	void BreakLinkage();

public:
#if defined(UI_BUILD_FOR_WINGDI)
    CPaintManagerUI m_paintManager;
#endif
    CComboUI* m_pOwner;
    CVerticalLayoutUI* m_pLayout;
    int m_iOldSel;
};

CComboWnd::CComboWnd()
: m_pOwner(NULL)
, m_pLayout(NULL)
{}

void CComboWnd::Init(CComboUI* pOwner)
{
    m_pOwner = pOwner;
    m_pLayout = NULL;
    m_iOldSel = m_pOwner->GetCurSel();

	UINT uDropBoxAlgin = pOwner->GetDropBoxAlign();

    // Position the popup window in absolute space
    SIZE szDrop = m_pOwner->GetDropBoxSize();
    RECT rcOwner = pOwner->GetPos();
    RECT rc = rcOwner;
	if ((uDropBoxAlgin & CComboUI::DROPBOXALIGN_BOTTOM) != 0)
	{
		rc.top = rc.bottom;
		rc.bottom = rc.top + szDrop.cy;
	}
	else
	{
		rc.bottom = rc.top;
		rc.top = rc.bottom - szDrop.cy;
	}

    if( szDrop.cx > 0 )
	{
		if ((uDropBoxAlgin & CComboUI::DROPBOXALIGN_LEFT) != 0)
		{
			rc.right = rc.left + szDrop.cx;
		}
		else
		{
			rc.right = rc.right;
			rc.left = rc.right - szDrop.cx;
		}
	}

    SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
    int cyFixed = 0;
    for( int it = 0; it < pOwner->GetCount(); it++ ) {
        CControlUI* pControl = static_cast<CControlUI*>(pOwner->GetItemAt(it));
        if( !pControl->IsVisible() ) continue;
		SIZE sz = pControl->EstimateSize(szAvailable);
		cyFixed += sz.cy;
	}
	cyFixed += 4; // CVerticalLayoutUI 默认的Inset 调整

	if ((uDropBoxAlgin & CComboUI::DROPBOXALIGN_BOTTOM) != 0)
		rc.bottom = rc.top + MIN(cyFixed, szDrop.cy);
	else
		rc.top = rc.bottom - MIN(cyFixed, szDrop.cy);

	::MapWindowRect(pOwner->GetManager()->GetPaintWindow(), HWND_DESKTOP, &rc);

	MONITORINFO oMonitor = {};
	oMonitor.cbSize = sizeof(oMonitor);
	::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
	CRect rcWork = oMonitor.rcWork;
	if( rc.bottom > rcWork.bottom ) {
		rc.left = rcOwner.left;
		rc.right = rcOwner.right;
		if( szDrop.cx > 0 )
		{
			if ((uDropBoxAlgin & CComboUI::DROPBOXALIGN_LEFT) != 0)
			{
				rc.right = rc.left + szDrop.cx;
			}
			else
			{
				rc.right = rcOwner.right;
				rc.left = rc.right - szDrop.cx;
			}
		}
		rc.top = rcOwner.top - MIN(cyFixed, szDrop.cy);
		rc.bottom = rcOwner.top;

		::MapWindowRect(pOwner->GetManager()->GetPaintWindow(), HWND_DESKTOP, &rc);
	}
    
    Create(pOwner->GetManager()->GetPaintWindow(), NULL, WS_POPUP, WS_EX_TOOLWINDOW, rc);
    // HACK: Don't deselect the parent's caption
    HWND hWndParent = m_hWnd;
    while( ::GetParent(hWndParent) != NULL ) hWndParent = ::GetParent(hWndParent);
    ::ShowWindow(m_hWnd, SW_SHOW);
#if defined(UI_BUILD_FOR_WIN32) && !defined(UI_BUILD_FOR_WINCE)
    ::SendMessage(hWndParent, WM_NCACTIVATE, TRUE, 0L);
#endif
}

LPCTSTR CComboWnd::GetWindowClassName() const
{
    return _T("ComboWnd");
}

void CComboWnd::BreakLinkage()
{
	m_pOwner = NULL;
}

void CComboWnd::OnFinalMessage(HWND hWnd)
{
	if (m_pOwner != NULL)
	{
		m_pOwner->m_pWindow = NULL;
		m_pOwner->m_uButtonState &= ~ UISTATE_PUSHED;
		m_pOwner->Invalidate();
	}
    delete this;
}

LRESULT CComboWnd::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if( ( m_hWnd != (HWND) wParam ))
		PostMessage(WM_CLOSE);
	bHandled = FALSE;
	return 0;
}

LRESULT CComboWnd::OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;
	int zDelta = (int) (short) HIWORD(wParam);
	TEventUI event = { 0 };
	event.Type = UIEVENT_SCROLLWHEEL;
	event.wParam = MAKELPARAM(zDelta < 0 ? SB_LINEDOWN : SB_LINEUP, 0);
	event.lParam = lParam;
	event.dwTimestamp = ::GetTickCount();
	m_pOwner->DoEvent(event);
	EnsureVisible(m_pOwner->GetCurSel());
	return 0;
}

LRESULT CComboWnd::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	switch( wParam )
	{
	case VK_ESCAPE:
		if (m_pOwner != NULL)
			m_pOwner->SelectItem(m_iOldSel, true);
		EnsureVisible(m_iOldSel);
		// FALL THROUGH...
	case VK_RETURN:			
		if ((m_pOwner != NULL) && (m_pOwner->GetCurSel() >= 0))
		{
			bHandled = TRUE;
			m_pOwner->Activate();
		}
		PostMessage(WM_KILLFOCUS);
		break;
	default:
		TEventUI event;
		event.Type = UIEVENT_KEYDOWN;
		event.chKey = (TCHAR)wParam;
		m_pOwner->DoEvent(event);
		EnsureVisible(m_pOwner->GetCurSel());
		bHandled = TRUE;
		return 0;
	}
	return 0;
}

LRESULT CComboWnd::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	POINT pt = { 0 };
	BOOL bResult = ::GetCursorPos(&pt);
	if (!bResult && GetLastError() == 120)
	{
		// jzebook (MIPSII) does not support this function
		pt.x = GET_X_LPARAM(lParam);
		pt.y = GET_Y_LPARAM(lParam);
	}
	else
		::ScreenToClient(m_paintManager.GetPaintWindow(), &pt);
	CControlUI* pControl = m_paintManager.FindControl(pt);
	if( pControl && _tcsicmp(pControl->GetClass(), kScrollBarUIClassName) != 0 ) PostMessage(WM_KILLFOCUS);
	
	bHandled = FALSE;
	return 0;
}

LRESULT CComboWnd::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE;

	m_paintManager.Init(m_hWnd);
	// The trick is to add the items to the new container. Their owner gets
	// reassigned by this operation - which is why it is important to reassign
	// the items back to the righfull owner/manager when the window closes.
	m_pLayout = new CVerticalLayoutUI;
	m_paintManager.UseParentResource(m_pOwner->GetManager());
	m_pLayout->SetManager(&m_paintManager, NULL, true);
	LPCTSTR pDefaultAttributes = m_pOwner->GetManager()->GetDefaultAttributeList(kVerticalLayoutUIInterfaceName);
	if( pDefaultAttributes ) {
		m_pLayout->ApplyAttributeList(pDefaultAttributes);
	}
	m_pLayout->SetInset(CRect(2, 2, 2, 2));
	m_pLayout->SetBkColor(0xFFFFFFFF);
	m_pLayout->SetBorderColor(0xFF85E4FF);
	m_pLayout->SetBorderSize(2);
	m_pLayout->SetAutoDestroy(false);
	m_pLayout->EnableScrollBar();
	m_pLayout->ApplyAttributeList(m_pOwner->GetDropBoxAttributeList());
	for( int i = 0; i < m_pOwner->GetCount(); i++ ) {
		m_pLayout->Add(static_cast<CControlUI*>(m_pOwner->GetItemAt(i)));
	}
	m_paintManager.AttachDialog(m_pLayout);

#if defined(UI_BUILD_FOR_SKIA)
	this->INHERITED::OnCreate(uMsg, wParam, lParam, bHandled);
#endif

	return 0;
}

LRESULT CComboWnd::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (m_pOwner != NULL)
	{
		m_pOwner->SetManager(m_pOwner->GetManager(), m_pOwner->GetParent(), false);
		m_pOwner->SetPos(m_pOwner->GetPos());
		m_pOwner->SetFocus();
	}

	bHandled = FALSE;

	return 0;
}

#if defined(UI_BUILD_FOR_WINGDI)
LRESULT CComboWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
#elif defined(UI_BUILD_FOR_SKIA)
LRESULT CComboWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
#endif
{
	LRESULT lRes = 0;

#if defined(UI_BUILD_FOR_WINGDI)
	BOOL bHandled = TRUE;

	switch( uMsg ) {
		case WM_CREATE:			lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
		case WM_CLOSE:			lRes = OnClose(uMsg, wParam, lParam, bHandled); break;
		case WM_KILLFOCUS:		lRes = OnKillFocus(uMsg, wParam, lParam, bHandled); break;
		case WM_MOUSEWHEEL:		lRes = OnMouseWheel(uMsg, wParam, lParam, bHandled); break;
		case WM_KEYDOWN:		lRes = OnKeyDown(uMsg, wParam, lParam, bHandled); break;
		case WM_LBUTTONUP:		lRes = OnLButtonUp(uMsg, wParam, lParam, bHandled); break;
		default:
			bHandled = FALSE;
			break;
	}

	if( bHandled ) return lRes;
    if( m_paintManager.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
    return CWindowWnd::HandleMessage(uMsg, wParam, lParam);

#elif defined(UI_BUILD_FOR_SKIA)
#endif
	return 0;
}

void CComboWnd::EnsureVisible(int iIndex)
{
    if( m_pOwner->GetCurSel() < 0 ) return;
    m_pLayout->FindSelectable(m_pOwner->GetCurSel(), false);
    RECT rcItem = m_pLayout->GetItemAt(iIndex)->GetPos();
    RECT rcList = m_pLayout->GetPos();
    CScrollBarUI* pHorizontalScrollBar = m_pLayout->GetHorizontalScrollBar();
    if( pHorizontalScrollBar && pHorizontalScrollBar->IsVisible() ) rcList.bottom -= pHorizontalScrollBar->GetFixedHeight();
    int iPos = m_pLayout->GetScrollPos().cy;
    if( rcItem.top >= rcList.top && rcItem.bottom < rcList.bottom ) return;
    int dx = 0;
    if( rcItem.top < rcList.top ) dx = rcItem.top - rcList.top;
    if( rcItem.bottom > rcList.bottom ) dx = rcItem.bottom - rcList.bottom;
    Scroll(0, dx);
}

void CComboWnd::Scroll(int dx, int dy)
{
    if( dx == 0 && dy == 0 ) return;
    SIZE sz = m_pLayout->GetScrollPos();
    m_pLayout->SetScrollPos(CSize(sz.cx + dx, sz.cy + dy));
}

////////////////////////////////////////////////////////


CComboUI::CComboUI() : m_pWindow(NULL), m_iCurSel(-1), m_uButtonState(0), m_pEditWnd(NULL), m_uDropBoxAlign(DROPBOXALIGN_LEFT | DROPBOXALIGN_BOTTOM)
{
    m_szDropBox = CSize(0, 150);
	::ZeroMemory(&m_rcTextPadding, sizeof(m_rcTextPadding));
	m_rcDropBtn.left = 0;
	m_rcDropBtn.right = 0;
	m_rcDropBtn.top = 0;
	m_rcDropBtn.bottom = 0;

	m_uDropType = COMBODROP_DOWN;

    m_ListInfo.nColumns = 0;
    m_ListInfo.nFont = -1;
    m_ListInfo.uTextStyle = DT_VCENTER;
    m_ListInfo.dwTextColor = 0xFF000000;
    m_ListInfo.dwBkColor = 0;
    m_ListInfo.bAlternateBk = false;
    m_ListInfo.dwSelectedTextColor = 0xFF000000;
    m_ListInfo.dwSelectedBkColor = 0xFFC1E3FF;
    m_ListInfo.dwHotTextColor = 0xFF000000;
    m_ListInfo.dwHotBkColor = 0xFFE9F5FF;
    m_ListInfo.dwDisabledTextColor = 0xFFCCCCCC;
    m_ListInfo.dwDisabledBkColor = 0xFFFFFFFF;
    m_ListInfo.dwLineColor = 0;
    m_ListInfo.bShowHtml = false;
    m_ListInfo.bMultiExpandable = false;
    ::ZeroMemory(&m_ListInfo.rcTextPadding, sizeof(m_ListInfo.rcTextPadding));
    ::ZeroMemory(&m_ListInfo.rcColumn, sizeof(m_ListInfo.rcColumn));

	//SetBorderSize(1);
	//SetBorderColor(0xFF4EA0D1);
	//SetFocusBorderColor(0xFF85E4FF);
	//SetBkColor(0xFFFFFFFF);
}

CComboUI::~CComboUI()
{
	if (m_pWindow != NULL)
	{
		m_pWindow->BreakLinkage();
		m_pWindow->Close();
		m_pWindow = NULL;
	}

	if (m_pEditWnd != NULL)
	{
		m_pEditWnd->BreakLinkage();
		m_pEditWnd->Close();
		m_pEditWnd = NULL;
	}
}

LPCTSTR CComboUI::GetClass() const
{
    return kComboUIClassName;
}

LPVOID CComboUI::GetInterface(LPCTSTR pstrName)
{
	if( _tcsicmp(pstrName, kComboUIInterfaceName) == 0 ) return static_cast<CComboUI*>(this);
	else if( _tcsicmp(pstrName, kIEditUIInterfaceName) == 0 ) return static_cast<IEditUI*>(this);
    if( _tcsicmp(pstrName, kIListOwnerUIInterfaceName) == 0 ) return static_cast<IListOwnerUI*>(this);
    return CContainerUI::GetInterface(pstrName);
}

UINT CComboUI::GetControlFlags() const
{
	if( !IsEnabled() ) return CControlUI::GetControlFlags();

    return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
}

void CComboUI::DoInit()
{
	if( m_iCurSel < 0 ) SelectItem(0);
}

int CComboUI::GetCurSel() const
{
    return m_iCurSel;
}

bool CComboUI::Activate()
{
	// 对普通的list，鼠标单击响应itemclick 双击或者enter键需要响应itemactivate
	// 而对Combox，如果通过上下键或者鼠标滚轮选中某个item时不会接收到消息，只有当按enter键捉着鼠标单击或者双击时，需响应itemselect
	if ( m_pManager != NULL ) m_pManager->SendNotify(this, kItemSelect);
	return true;
}

bool CComboUI::SelectItem(int iIndex, bool bTakeFocus, bool bSendNofitied)
{
	m_uSelectRangeStart = 0;
	if( m_pWindow != NULL ) m_pWindow->Close();
	if(GetManager() == NULL) return false;

    if( iIndex == m_iCurSel ) return true;
    int iOldSel = m_iCurSel;
    if( m_iCurSel >= 0 ) {
        CControlUI* pControl = static_cast<CControlUI*>(m_items[m_iCurSel]);
        if( !pControl ) return false;
        IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(kIListItemUIInterfaceName));
        if( pListItem != NULL ) pListItem->Select(false);
        m_iCurSel = -1;
    }
    if( iIndex < 0 ) return false;
    if( m_items.GetSize() == 0 ) return false;
    if( iIndex >= m_items.GetSize() ) iIndex = m_items.GetSize() - 1;
    CControlUI* pControl = static_cast<CControlUI*>(m_items[iIndex]);
    if( !pControl || !pControl->IsVisible() || !pControl->IsEnabled() ) return false;
    IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(kIListItemUIInterfaceName));
    if( pListItem == NULL ) return false;
    m_iCurSel = iIndex;
    if( m_pWindow != NULL || bTakeFocus ) pControl->SetFocus();
    pListItem->Select(true);

    if (bSendNofitied && m_pManager != NULL) m_pManager->SendNotify(this, kItemSelect, m_iCurSel, iOldSel);
    Invalidate();

    return true;
}

bool CComboUI::SetItemIndex(CControlUI* pControl, int iIndex)
{
    int iOrginIndex = GetItemIndex(pControl);
    if( iOrginIndex == -1 ) return false;
    if( iOrginIndex == iIndex ) return true;

    IListItemUI* pSelectedListItem = NULL;
    if( m_iCurSel >= 0 ) pSelectedListItem = 
        static_cast<IListItemUI*>(GetItemAt(m_iCurSel)->GetInterface(kIListItemUIInterfaceName));
    if( !CContainerUI::SetItemIndex(pControl, iIndex) ) return false;
    int iMinIndex = min(iOrginIndex, iIndex);
    int iMaxIndex = max(iOrginIndex, iIndex);
    for(int i = iMinIndex; i < iMaxIndex + 1; ++i) {
        CControlUI* p = GetItemAt(i);
        IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(kIListItemUIInterfaceName));
        if( pListItem != NULL ) {
            pListItem->SetIndex(i);
        }
    }
    if( m_iCurSel >= 0 && pSelectedListItem != NULL ) m_iCurSel = pSelectedListItem->GetIndex();
    return true;
}

bool CComboUI::Add(CControlUI* pControl)
{
    IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(kIListItemUIInterfaceName));
    if( pListItem != NULL ) 
    {
        pListItem->SetOwner(this);
        pListItem->SetIndex(m_items.GetSize());
    }
    return CContainerUI::Add(pControl);
}

bool CComboUI::AddAt(CControlUI* pControl, int iIndex)
{
    if (!CContainerUI::AddAt(pControl, iIndex)) return false;

    // The list items should know about us
    IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(kIListItemUIInterfaceName));
    if( pListItem != NULL ) {
        pListItem->SetOwner(this);
        pListItem->SetIndex(iIndex);
    }

    for(int i = iIndex + 1; i < GetCount(); ++i) {
        CControlUI* p = GetItemAt(i);
        pListItem = static_cast<IListItemUI*>(p->GetInterface(kIListItemUIInterfaceName));
        if( pListItem != NULL ) {
            pListItem->SetIndex(i);
        }
    }
    if( m_iCurSel >= iIndex ) m_iCurSel += 1;
    return true;
}

bool CComboUI::Remove(CControlUI* pControl)
{
    int iIndex = GetItemIndex(pControl);
    if (iIndex == -1) return false;

    if (!CContainerUI::RemoveAt(iIndex)) return false;

    for(int i = iIndex; i < GetCount(); ++i) {
        CControlUI* p = GetItemAt(i);
        IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(kIListItemUIInterfaceName));
        if( pListItem != NULL ) {
            pListItem->SetIndex(i);
        }
    }

    if( iIndex == m_iCurSel && m_iCurSel >= 0 ) {
        int iSel = m_iCurSel;
        m_iCurSel = -1;
        SelectItem(FindSelectable(iSel, false));
    }
    else if( iIndex < m_iCurSel ) m_iCurSel -= 1;
    return true;
}

bool CComboUI::RemoveAt(int iIndex)
{
    if (!CContainerUI::RemoveAt(iIndex)) return false;

    for(int i = iIndex; i < GetCount(); ++i) {
        CControlUI* p = GetItemAt(i);
        IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(kIListItemUIInterfaceName));
        if( pListItem != NULL ) pListItem->SetIndex(i);
    }

    if( iIndex == m_iCurSel && m_iCurSel >= 0 ) {
        int iSel = m_iCurSel;
        m_iCurSel = -1;
        SelectItem(FindSelectable(iSel, false));
    }
    else if( iIndex < m_iCurSel ) m_iCurSel -= 1;
    return true;
}

void CComboUI::RemoveAll()
{
    m_iCurSel = -1;
    CContainerUI::RemoveAll();
}

void CComboUI::DoEvent(TEventUI& event)
{
#ifdef UI_BUILD_FOR_DESIGNER
	if (IsUsedForPitchUpon() && (m_pManager != NULL) && m_pManager->IsUsedAsUIDesigner() && ((GetKeyState(VK_CONTROL) < 0) || (GetKeyState(VK_SHIFT) < 0)))
	{
		CControlUI::DoEvent(event);
		return;
	}
#endif

	if(GetManager() == NULL) return;

    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_pParent != NULL ) m_pParent->DoEvent(event);
        else CContainerUI::DoEvent(event);
        return;
    }

	if( event.Type == UIEVENT_SETCURSOR && IsEnabled() )
	{
		if( m_uDropType != COMBODROP_LIST && !::PtInRect(&m_rcDropBtn,event.ptMouse) && ::PtInRect(&m_rcItem, event.ptMouse) )
			::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_IBEAM)));
		else
			::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
		return;
	}

	if( event.Type == UIEVENT_WINDOWSIZE )
	{
		if( m_pEditWnd != NULL ) m_pManager->SetFocusNeeded(this);
	}

	if( event.Type == UIEVENT_KILLFOCUS && IsEnabled() ) 
	{
        Invalidate();
	}
    if( event.Type == UIEVENT_SETFOCUS && IsEnabled()) 
    {
		if( m_uDropType != COMBODROP_LIST && !::PtInRect(&m_rcDropBtn,event.ptMouse) && ::PtInRect(&m_rcItem, event.ptMouse) )
		{
			if( m_pEditWnd ) return;
			m_pEditWnd = new CEditWndEx();
			ASSERT(m_pEditWnd);
			m_pEditWnd->Init(this);
		}
        Invalidate();
    }
	if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK || event.Type == UIEVENT_RBUTTONDOWN) 
	{
		if( IsEnabled() )
		{
			if( event.Type == UIEVENT_BUTTONDOWN )
				m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;

			if( m_uDropType != COMBODROP_LIST && !::PtInRect(&m_rcDropBtn,event.ptMouse) && ::PtInRect(&m_rcItem, event.ptMouse) )
			{
				GetManager()->ReleaseCapture();
				if (m_pEditWnd==NULL)
				{
					m_pEditWnd = new CEditWndEx();
					ASSERT(m_pEditWnd);
					m_pEditWnd->Init(this);
				}
				if( PtInRect(&m_rcItem, event.ptMouse) )
				{
					int nSize = GetWindowTextLength(*m_pEditWnd);
					if( nSize == 0 )
						nSize = 1;

					if (m_uSelectRangeStart > nSize)
						m_uSelectRangeStart = 0;

					Edit_SetSel(*m_pEditWnd, m_uSelectRangeStart, nSize);
				}
				else if( m_pEditWnd != NULL )
				{
					int nSize = GetWindowTextLength(*m_pEditWnd);
					if( nSize == 0 )
						nSize = 1;

					if (m_uSelectRangeStart > nSize)
						m_uSelectRangeStart = 0;

					Edit_SetSel(*m_pEditWnd, m_uSelectRangeStart, nSize);
				}
				return;
			}
			else if (::PtInRect(&m_rcDropBtn,event.ptMouse))
				ActivateDropWnd();
		}
		return;
	}

    if( event.Type == UIEVENT_BUTTONUP )
	{
		if( ::PtInRect(&m_rcDropBtn,event.ptMouse) ){
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
				m_uButtonState &= ~ UISTATE_CAPTURED;
				Invalidate();
			}
		}
        return;
    }
    if( event.Type == UIEVENT_MOUSEMOVE )
    {
        return;
    }	
    if( event.Type == UIEVENT_KEYDOWN )
    {
        switch( event.chKey ) {
        case VK_F4:
            ActivateDropWnd();
            return;
        case VK_UP:
            SelectItem(FindSelectable(m_iCurSel - 1, false), false);
            return;
        case VK_DOWN:
            SelectItem(FindSelectable(m_iCurSel + 1, true), false);
            return;
        case VK_PRIOR:
            SelectItem(FindSelectable(m_iCurSel - 1, false), false);
            return;
        case VK_NEXT:
            SelectItem(FindSelectable(m_iCurSel + 1, true), false);
            return;
        case VK_HOME:
            SelectItem(FindSelectable(0, false), false);
            return;
        case VK_END:
            SelectItem(FindSelectable(GetCount() - 1, true), false);
            return;
        }
    }
    if( event.Type == UIEVENT_SCROLLWHEEL )
    {
		if( m_pEditWnd != NULL ) return;
        bool bDownward = LOWORD(event.wParam) == SB_LINEDOWN;
        SelectItem(FindSelectable(m_iCurSel + (bDownward ? 1 : -1), bDownward), false);
        return;
    }
    if( event.Type == UIEVENT_CONTEXTMENU )
    {
        return;
    }
	if( event.Type == UIEVENT_MOUSEENTER )
	{
		if( (m_uButtonState & UISTATE_HOT) == 0  )
			m_uButtonState |= UISTATE_HOT;
		Invalidate();
		return;
	}
	if( event.Type == UIEVENT_MOUSELEAVE )
	{
		if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			m_uButtonState &= ~UISTATE_HOT;
			Invalidate();
		}
		return;
	}

    CControlUI::DoEvent(event);
}

SIZE CComboUI::EstimateSize(SIZE szAvailable)
{
    if( m_cxyFixed.cy == 0 ) return CSize(m_cxyFixed.cx, m_pManager->GetDefaultFontInfo()->tm.tmHeight + 12);
    return CControlUI::EstimateSize(szAvailable);
}

bool CComboUI::ActivateDropWnd()
{
    if( !CControlUI::Activate() ) return false;
	if( GetCount() <= 0 ) return false;
    if( m_pWindow ) return true;
    m_pWindow = new CComboWnd();
    ASSERT(m_pWindow);
    m_pWindow->Init(this);
    if( m_pManager != NULL ) m_pManager->SendNotify(this, kDropDown);
    Invalidate();
    return true;
}

CStdString CComboUI::GetText() const
{
    if( m_iCurSel < 0 ) return m_sText;
    CControlUI* pControl = static_cast<CControlUI*>(m_items[m_iCurSel]);
    return pControl->GetText();
}

void CComboUI::SetEnabled(bool bEnable)
{
    CContainerUI::SetEnabled(bEnable);
    if( !IsEnabled() ) m_uButtonState = 0;
}

CStdString CComboUI::GetDropBoxAttributeList()
{
    return m_sDropBoxAttributes;
}

void CComboUI::SetDropBoxAttributeList(LPCTSTR pstrList)
{
    m_sDropBoxAttributes = pstrList;
}

SIZE CComboUI::GetDropBoxSize() const
{
    return m_szDropBox;
}

void CComboUI::SetDropBoxSize(SIZE szDropBox)
{
    m_szDropBox = szDropBox;
}

void CComboUI::SetDropType(UINT uDropType)
{
	m_uDropType = uDropType;
}

UINT CComboUI::GetDropType() const
{
	return m_uDropType;
}

UINT CComboUI::GetDropBoxAlign() const
{
	return m_uDropBoxAlign;
}

void CComboUI::SetDropBoxAlign(UINT align)
{
	m_uDropBoxAlign = align;
}

RECT CComboUI::GetTextPadding() const
{
    return m_rcTextPadding;
}

void CComboUI::SetTextPadding(RECT rc)
{
    m_rcTextPadding = rc;
    Invalidate();
}

LPCTSTR CComboUI::GetNormalImage() const
{
    return m_sNormalImage;
}

void CComboUI::SetNormalImage(LPCTSTR pStrImage)
{
    m_sNormalImage = pStrImage;
    Invalidate();
}

LPCTSTR CComboUI::GetHotImage() const
{
    return m_sHotImage;
}

void CComboUI::SetHotImage(LPCTSTR pStrImage)
{
    m_sHotImage = pStrImage;
    Invalidate();
}

LPCTSTR CComboUI::GetPushedImage() const
{
    return m_sPushedImage;
}

void CComboUI::SetPushedImage(LPCTSTR pStrImage)
{
    m_sPushedImage = pStrImage;
    Invalidate();
}

LPCTSTR CComboUI::GetFocusedImage() const
{
    return m_sFocusedImage;
}

void CComboUI::SetFocusedImage(LPCTSTR pStrImage)
{
    m_sFocusedImage = pStrImage;
    Invalidate();
}

LPCTSTR CComboUI::GetDisabledImage() const
{
    return m_sDisabledImage;
}

void CComboUI::SetDisabledImage(LPCTSTR pStrImage)
{
    m_sDisabledImage = pStrImage;
    Invalidate();
}

LPCTSTR CComboUI::GetDropBtnImage() const
{
    return m_sDropBtnImage;
}

void CComboUI::SetDropBtnImage(LPCTSTR pStrImage)
{
    m_sDropBtnImage = pStrImage;
    Invalidate();
}

TListInfoUI* CComboUI::GetListInfo()
{
    return &m_ListInfo;
}

void CComboUI::SetItemFont(int index)
{
    m_ListInfo.nFont = index;
    Invalidate();
}

void CComboUI::SetItemTextStyle(UINT uStyle)
{
	m_ListInfo.uTextStyle = uStyle;
	Invalidate();
}

RECT CComboUI::GetItemTextPadding() const
{
	return m_ListInfo.rcTextPadding;
}

void CComboUI::SetItemTextPadding(RECT rc)
{
    m_ListInfo.rcTextPadding = rc;
    Invalidate();
}

void CComboUI::SetItemTextColor(DWORD dwTextColor)
{
    m_ListInfo.dwTextColor = dwTextColor;
    Invalidate();
}

void CComboUI::SetItemBkColor(DWORD dwBkColor)
{
    m_ListInfo.dwBkColor = dwBkColor;
}

void CComboUI::SetItemBkImage(LPCTSTR pStrImage)
{
    m_ListInfo.sBkImage = pStrImage;
}

DWORD CComboUI::GetItemTextColor() const
{
	return m_ListInfo.dwTextColor;
}

DWORD CComboUI::GetItemBkColor() const
{
	return m_ListInfo.dwBkColor;
}

LPCTSTR CComboUI::GetItemBkImage() const
{
	return m_ListInfo.sBkImage;
}

bool CComboUI::IsAlternateBk() const
{
    return m_ListInfo.bAlternateBk;
}

void CComboUI::SetAlternateBk(bool bAlternateBk)
{
    m_ListInfo.bAlternateBk = bAlternateBk;
}

void CComboUI::SetSelectedItemTextColor(DWORD dwTextColor)
{
    m_ListInfo.dwSelectedTextColor = dwTextColor;
}

void CComboUI::SetSelectedItemBkColor(DWORD dwBkColor)
{
    m_ListInfo.dwSelectedBkColor = dwBkColor;
}

void CComboUI::SetSelectedItemImage(LPCTSTR pStrImage)
{
	m_ListInfo.sSelectedImage = pStrImage;
}

DWORD CComboUI::GetSelectedItemTextColor() const
{
	return m_ListInfo.dwSelectedTextColor;
}

DWORD CComboUI::GetSelectedItemBkColor() const
{
	return m_ListInfo.dwSelectedBkColor;
}

LPCTSTR CComboUI::GetSelectedItemImage() const
{
	return m_ListInfo.sSelectedImage;
}

void CComboUI::SetHotItemTextColor(DWORD dwTextColor)
{
    m_ListInfo.dwHotTextColor = dwTextColor;
}

void CComboUI::SetHotItemBkColor(DWORD dwBkColor)
{
    m_ListInfo.dwHotBkColor = dwBkColor;
}

void CComboUI::SetHotItemImage(LPCTSTR pStrImage)
{
    m_ListInfo.sHotImage = pStrImage;
}

DWORD CComboUI::GetHotItemTextColor() const
{
	return m_ListInfo.dwHotTextColor;
}
DWORD CComboUI::GetHotItemBkColor() const
{
	return m_ListInfo.dwHotBkColor;
}

LPCTSTR CComboUI::GetHotItemImage() const
{
	return m_ListInfo.sHotImage;
}

void CComboUI::SetDisabledItemTextColor(DWORD dwTextColor)
{
    m_ListInfo.dwDisabledTextColor = dwTextColor;
}

void CComboUI::SetDisabledItemBkColor(DWORD dwBkColor)
{
    m_ListInfo.dwDisabledBkColor = dwBkColor;
}

void CComboUI::SetDisabledItemImage(LPCTSTR pStrImage)
{
    m_ListInfo.sDisabledImage = pStrImage;
}

DWORD CComboUI::GetDisabledItemTextColor() const
{
	return m_ListInfo.dwDisabledTextColor;
}

DWORD CComboUI::GetDisabledItemBkColor() const
{
	return m_ListInfo.dwDisabledBkColor;
}

LPCTSTR CComboUI::GetDisabledItemImage() const
{
	return m_ListInfo.sDisabledImage;
}

DWORD CComboUI::GetItemLineColor() const
{
	return m_ListInfo.dwLineColor;
}

void CComboUI::SetItemLineColor(DWORD dwLineColor)
{
    m_ListInfo.dwLineColor = dwLineColor;
}

bool CComboUI::IsItemShowHtml()
{
    return m_ListInfo.bShowHtml;
}

void CComboUI::SetItemShowHtml(bool bShowHtml)
{
    if( m_ListInfo.bShowHtml == bShowHtml ) return;

    m_ListInfo.bShowHtml = bShowHtml;
    Invalidate();
}

LPCTSTR CComboUI::GetHitText()
{
	return m_sHitText;
}

void CComboUI::SetHitText(LPCTSTR pStrText)
{
	m_sHitText = pStrText;
	Invalidate();
}

DWORD CComboUI::GetHitTextColor()
{
	return m_dwHitTextColor;
}

void CComboUI::SetHitTextColor(DWORD dwColor)
{
	m_dwHitTextColor = dwColor;
}
void CComboUI::SetPos(RECT rc)
{
    // Put all elements out of sight
    RECT rcNull = { 0 };
    for( int i = 0; i < m_items.GetSize(); i++ ) static_cast<CControlUI*>(m_items[i])->SetPos(rcNull);
    // Position this control
    CControlUI::SetPos(rc);

	RECT rcImage = {0};
	CRenderEngine::CaculateImageRect(m_sDropBtnImage, this, m_pManager, rcImage);
	if (!IsRectEmpty(&rcImage))
	{
		m_rcDropBtn.right = m_rcItem.right - 2;
		m_rcDropBtn.left = m_rcDropBtn.right - (rcImage.right - rcImage.left);
		m_rcDropBtn.top = m_rcItem.top + ((m_rcItem.bottom - m_rcItem.top) - (rcImage.bottom - rcImage.top))/2;
		m_rcDropBtn.bottom = m_rcDropBtn.top + (rcImage.bottom - rcImage.top);
	}
	else
	{
		m_rcDropBtn.left = m_rcItem.right-22;
		m_rcDropBtn.right = m_rcItem.right;
		m_rcDropBtn.top = m_rcItem.top;
		m_rcDropBtn.bottom = m_rcItem.bottom;
	}

	if(m_pEditWnd){
		CRect rc = GetEditPos();
		RECT rcPadding = GetEditTextPadding();
		rc.left += rcPadding.left;
		rc.top += rcPadding.top;
		rc.right -= rcPadding.right;
		rc.bottom -= rcPadding.bottom;

		m_pEditWnd->MoveEditWnd(rc);
	}
}

void CComboUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if( _tcsicmp(pstrName, _T("dropboxsize")) == 0 ) {
		SIZE szDropBox = { 0 };
		LPTSTR pstr = NULL;
		szDropBox.cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
		szDropBox.cy = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
		SetDropBoxSize(szDropBox);
	}
    else if( _tcsicmp(pstrName, _T("textpadding")) == 0 ) {
        RECT rcTextPadding = { 0 };
        LPTSTR pstr = NULL;
        rcTextPadding.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
        rcTextPadding.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
        rcTextPadding.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
        rcTextPadding.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
        SetTextPadding(rcTextPadding);
    }
	else if(_tcsicmp(pstrName, _T("droptype")) == 0 ){
		if( _tcsstr(pstrValue, _T("dropsimple")) != NULL ) 	SetDropType(COMBODROP_SIMPLE);
		else if( _tcsstr(pstrValue, _T("dropdown")) != NULL ) SetDropType(COMBODROP_DOWN);
		else if( _tcsstr(pstrValue, _T("droplist")) != NULL ) SetDropType(COMBODROP_LIST);
	}
    else if( _tcsicmp(pstrName, _T("normalimage")) == 0 ) SetNormalImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("hotimage")) == 0 ) SetHotImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("pushedimage")) == 0 ) SetPushedImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("focusedimage")) == 0 ) SetFocusedImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("disabledimage")) == 0 ) SetDisabledImage(pstrValue);
	else if( _tcsicmp(pstrName, _T("dropbtnimage")) == 0 ) SetDropBtnImage(pstrValue);
	else if( _tcsicmp(pstrName, _T("dropbox")) == 0 ) SetDropBoxAttributeList(pstrValue);
	else if( _tcsicmp(pstrName, _T("dropboxalign")) == 0)
	{
        if( _tcsstr(pstrValue, _T("left")) != NULL ) {
            m_uDropBoxAlign &= ~DROPBOXALIGN_RIGHT;
            m_uDropBoxAlign |= DROPBOXALIGN_LEFT;
        }
        if( _tcsstr(pstrValue, _T("right")) != NULL ) {
            m_uDropBoxAlign &= ~DROPBOXALIGN_LEFT;
            m_uDropBoxAlign |= DROPBOXALIGN_RIGHT;
        }
		if( _tcsstr(pstrValue, _T("top")) != NULL ) {
			m_uDropBoxAlign &= ~DROPBOXALIGN_BOTTOM;
			m_uDropBoxAlign |= DROPBOXALIGN_TOP;
		}
		if( _tcsstr(pstrValue, _T("bottom")) != NULL ) {
			m_uDropBoxAlign &= ~DROPBOXALIGN_TOP;
			m_uDropBoxAlign |= DROPBOXALIGN_BOTTOM;
		}
		if (!((m_uDropBoxAlign & 0xFFFF) & DROPBOXALIGN_TOP) || ((m_uDropBoxAlign & 0xFFFF) & DROPBOXALIGN_BOTTOM))
		{
			m_uDropBoxAlign &= ~DROPBOXALIGN_TOP;
			m_uDropBoxAlign |= DROPBOXALIGN_BOTTOM;
		}
    }
    else if( _tcsicmp(pstrName, _T("itemfont")) == 0 ) m_ListInfo.nFont = _ttoi(pstrValue);
    else if( _tcsicmp(pstrName, _T("itemalign")) == 0 ) {
        if( _tcsstr(pstrValue, _T("left")) != NULL ) {
            m_ListInfo.uTextStyle &= ~(DT_CENTER | DT_RIGHT);
            m_ListInfo.uTextStyle |= DT_LEFT;
        }
        if( _tcsstr(pstrValue, _T("center")) != NULL ) {
            m_ListInfo.uTextStyle &= ~(DT_LEFT | DT_RIGHT);
            m_ListInfo.uTextStyle |= DT_CENTER;
        }
        if( _tcsstr(pstrValue, _T("right")) != NULL ) {
            m_ListInfo.uTextStyle &= ~(DT_LEFT | DT_CENTER);
            m_ListInfo.uTextStyle |= DT_RIGHT;
        }
    }
    if( _tcsicmp(pstrName, _T("itemtextpadding")) == 0 ) {
        RECT rcTextPadding = { 0 };
        LPTSTR pstr = NULL;
        rcTextPadding.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
        rcTextPadding.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
        rcTextPadding.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
        rcTextPadding.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
        SetItemTextPadding(rcTextPadding);
    }
    else if( _tcsicmp(pstrName, _T("itemtextcolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetItemTextColor(clrColor);
    }
    else if( _tcsicmp(pstrName, _T("itembkcolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetItemBkColor(clrColor);
    }
    else if( _tcsicmp(pstrName, _T("itembkimage")) == 0 ) SetItemBkImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("itemaltbk")) == 0 ) SetAlternateBk(_tcsicmp(pstrValue, _T("true")) == 0);
    else if( _tcsicmp(pstrName, _T("itemselectedtextcolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetSelectedItemTextColor(clrColor);
    }
    else if( _tcsicmp(pstrName, _T("itemselectedbkcolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetSelectedItemBkColor(clrColor);
    }
    else if( _tcsicmp(pstrName, _T("itemselectedimage")) == 0 ) SetSelectedItemImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("itemhottextcolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetHotItemTextColor(clrColor);
    }
    else if( _tcsicmp(pstrName, _T("itemhotbkcolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetHotItemBkColor(clrColor);
    }
    else if( _tcsicmp(pstrName, _T("itemhotimage")) == 0 ) SetHotItemImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("itemdisabledtextcolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetDisabledItemTextColor(clrColor);
    }
    else if( _tcsicmp(pstrName, _T("itemdisabledbkcolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetDisabledItemBkColor(clrColor);
    }
    else if( _tcsicmp(pstrName, _T("itemdisabledimage")) == 0 ) SetDisabledItemImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("itemlinecolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetItemLineColor(clrColor);
    }
    else if( _tcsicmp(pstrName, _T("itemshowhtml")) == 0 ) SetItemShowHtml(_tcsicmp(pstrValue, _T("true")) == 0);
	if( _tcsicmp(pstrName, _T("hittext")) == 0 ) SetHitText(pstrValue);
	else if( _tcsicmp(pstrName, _T("hitttextcolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetHitTextColor(clrColor);
	}
    else CContainerUI::SetAttribute(pstrName, pstrValue);
}

void CComboUI::DoPaint(void* ctx, const RECT& rcPaint)
{
	if (ctx == NULL) return;
    CControlUI::DoPaint(ctx, rcPaint);
}

void CComboUI::PaintDropdownButton(void* ctx)
{
	if (ctx == NULL) return;

	if( !m_sDropBtnImage.IsEmpty() ) {
		CStdString sImageDropBtn;
		sImageDropBtn.SmallFormat(_T("dest='%d,%d,%d,%d'"), m_rcDropBtn.left - m_rcItem.left, m_rcDropBtn.top - m_rcItem.top, m_rcDropBtn.right - m_rcItem.left, m_rcDropBtn.bottom - m_rcItem.top);
        if( !DrawImage(ctx, (LPCTSTR)m_sDropBtnImage , (LPCTSTR)sImageDropBtn)) m_sDropBtnImage.Empty();
        else return;
	}
}

void CComboUI::PaintBorder(void* ctx)
{
	if (ctx == NULL) return;

    DWORD dwBorderColor = GetBorderColor();
    int nBorderSize = GetBorderSize();
	if (nBorderSize == 0) return;
    if( (m_uButtonState & UISTATE_HOT) != 0 )
		dwBorderColor = GetFocusBorderColor();

	CRenderEngine::DrawRect(ctx, m_rcItem, nBorderSize, dwBorderColor);
}

void CComboUI::PaintStatusImage(void* ctx)
{
	if (ctx == NULL) return;

	if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
	else m_uButtonState &= ~ UISTATE_FOCUSED;
	if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
	else m_uButtonState &= ~ UISTATE_DISABLED;

	if( (m_uButtonState & UISTATE_DISABLED) != 0 ) {
		if( !m_sDisabledImage.IsEmpty() ) {
			if( !DrawImage(ctx, (LPCTSTR)m_sDisabledImage) ) m_sDisabledImage.Empty();
			else return PaintDropdownButton(ctx);
		}
		else
			return PaintDropdownButton(ctx);
	}
	else if( (m_uButtonState & UISTATE_PUSHED) != 0 ) {
		if( !m_sPushedImage.IsEmpty() ) {
			if( !DrawImage(ctx, (LPCTSTR)m_sPushedImage) ) m_sPushedImage.Empty();
			else return PaintDropdownButton(ctx);
		}
		else
			return PaintDropdownButton(ctx);
	}
	else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
		if( !m_sHotImage.IsEmpty() ) {
			if( !DrawImage(ctx, (LPCTSTR)m_sHotImage) ) m_sHotImage.Empty();
			else return PaintDropdownButton(ctx);
		}
		else
			return PaintDropdownButton(ctx);
	}
	else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) {
		if( !m_sFocusedImage.IsEmpty() ) {
			if( !DrawImage(ctx, (LPCTSTR)m_sFocusedImage) ) m_sFocusedImage.Empty();
			else return PaintDropdownButton(ctx);
		}
		else
			return PaintDropdownButton(ctx);
	}

	if( !m_sNormalImage.IsEmpty() ) {
		if( !DrawImage(ctx, (LPCTSTR)m_sNormalImage) ) m_sNormalImage.Empty();
		else return PaintDropdownButton(ctx);
	}
	else
		return PaintDropdownButton(ctx);
}

void CComboUI::PaintText(void* ctx)
{
	if (ctx == NULL) return;

	if(m_pEditWnd == NULL){
		CRect rcText = m_rcItem;
		rcText.left += GetEditTextPadding().left;
		rcText.right -= GetEditTextPadding().right;
		rcText.top += GetEditTextPadding().top;
		rcText.bottom -= GetEditTextPadding().bottom;
		rcText.right -= m_rcDropBtn.right - m_rcDropBtn.left;

		if( m_iCurSel >= 0 ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[m_iCurSel]);
			IListItemUI* pElement = static_cast<IListItemUI*>(pControl->GetInterface(kIListItemUIInterfaceName));
			if( pElement != NULL ) {
				pElement->DrawItemText(ctx, rcText);
			}
			else {
				RECT rcOldPos = pControl->GetPos();
				pControl->SetPos(rcText);
				pControl->DoPaint(ctx, rcText);
				pControl->SetPos(rcOldPos);
			}
		}
		else
		{
			CStdString sText = m_sText;
			if (sText.IsEmpty())
				sText = m_sHitText;

			RECT rcItemPadding = GetItemTextPadding();
			rcText.left += rcItemPadding.left;
			rcText.right -= rcItemPadding.right;
			rcText.top += rcItemPadding.top;
			rcText.bottom -= rcItemPadding.bottom;

			LONG lEditHeight = GetManager()->GetDefaultFontInfo()->tm.tmHeight;
			if( lEditHeight < rcText.GetHeight() ) {
				rcText.top += (rcText.GetHeight() - lEditHeight) / 2;
				rcText.bottom = rcText.top + lEditHeight;
			}
			CRenderEngine::DrawText(ctx, GetManager(), rcText, m_sText, m_ListInfo.dwTextColor, \
            m_ListInfo.nFont, DT_SINGLELINE | m_ListInfo.uTextStyle);
		}
	}
}

CControlUI* const CComboUI::GetHostedControl()
{
	return this;
}

LPCTSTR CComboUI::GetEditClass() const
{
	return GetClass();
}

CRect CComboUI::GetEditPos()
{
	RECT rcEdit;
	rcEdit.left = m_rcItem.left;
	rcEdit.right = m_rcItem.right - (m_rcDropBtn.right - m_rcDropBtn.left);
	rcEdit.top = m_rcItem.top;
	rcEdit.bottom = m_rcItem.bottom;

	RECT rcItemPadding = GetItemTextPadding();
	rcEdit.left += rcItemPadding.left;
	rcEdit.top += rcItemPadding.top;
	rcEdit.right -= rcItemPadding.right;
	rcEdit.bottom -= rcItemPadding.bottom;
	return rcEdit;
}

RECT CComboUI::GetEditTextPadding() const
{
	return m_rcTextPadding;
}

DWORD CComboUI::GetEditTextColor()
{
	return m_pManager->GetDefaultFontColor();	
}

bool CComboUI::IsPasswordMode() const
{
	return false;
}

TCHAR CComboUI::GetPasswordChar() const
{
	return NULL;
}

CPaintManagerUI* CComboUI::GetManager() const
{
	return CContainerUI::GetManager();
}

UINT CComboUI::GetMaxChar()
{
	return 1024;
}

void CComboUI::SetText(LPCTSTR pstrText)
{
	SelectItem(-1);
	CContainerUI::SetText(pstrText);

	bool bHasMatched = false;
	for( int i = 0; ( i < GetCount() ) && ( m_pEditWnd != NULL ); ++i )
	{
		CControlUI* pControl = static_cast<CControlUI*>(GetItemAt(i));
		if( pControl != NULL && _tcslen(pControl->GetText().GetData()) > 0)
		{
			if( _tcsicmp(pControl->GetText().GetData(), pstrText) == 0 )
				break;
			else if(_tcsicmp(pControl->GetText().Left(_tcslen(pstrText)), pstrText) == 0 )
			{
				bHasMatched = true;
				CContainerUI::SetText(pControl->GetText().GetData());
				m_uSelectRangeStart = _tcslen(pstrText);
				SetWindowText(*m_pEditWnd, pControl->GetText().GetData());
				int nSize = GetWindowTextLength(*m_pEditWnd);
				Edit_SetSel(*m_pEditWnd, _tcslen(pstrText), nSize);
				break;
			}
		}
	}

	if (!bHasMatched)
		m_uSelectRangeStart = 0;
}

bool CComboUI::IsEnabled() const
{
	return 	CContainerUI::IsEnabled();
}

bool CComboUI::IsReadOnly() const
{
	return false;
}

CStdString CComboUI::GetName() const
{
	return	CContainerUI::GetName();
}

void CComboUI::SetEidtWndNull()
{
	m_pEditWnd = NULL;
}

bool CComboUI::IsMultiLine() const
{
	return false;
}

#if(WINVER >= 0x0400)
bool CComboUI::IsDigitalNumber() const
{
	return false;
}
#endif

DWORD CComboUI::GetBkColor() const
{
	DWORD dwColor = CContainerUI::GetBkColor();
	if ((dwColor == 0) && (GetParent() != NULL))
	{
		dwColor = GetParent()->GetBkColor();
	}
	return dwColor;
}

} // namespace DuiLib
