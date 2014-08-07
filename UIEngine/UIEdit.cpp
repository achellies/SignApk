#include "StdAfx.h"
#include "UIlib.h"

namespace DirectUI {

#if 0
/////////////////////////////////////////////////////////////////////////////////////
//
//
class CMultiLineEditCanvasWnd : public CWindowWnd
{
public:
	void Init(CMultiLineEditUI* pOwner);

	LPCTSTR GetWindowClassName() const;
	LPCTSTR GetSuperClassName() const;

	void OnFinalMessage(HWND hWnd);

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);	
	LRESULT OnEditChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);	
	LRESULT OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void BreakLinkage();

protected:
	CMultiLineEditUI*	m_pOwner;
};


// 因技术能力有限，无法实现多行的文本框，特用系统的文本框来输入多行文字，
// 但因系统的多行文本框的滚动条效果较难看，所以特使用两个CScrollbarUI来模拟横向和纵向的滚动条
// 这两个滚动条必须命名为vEditScrollbar 和 hEditScrollbar
class CMultiLineEditWnd : public CWindowWnd
{
public:
	CMultiLineEditWnd()
		: m_pOwner(NULL)
		, m_pVScrollBar(NULL)
		, m_pHScrollBar(NULL)
	{}
	void Init(CMultiLineEditUI* pOwner);

	LPCTSTR GetWindowClassName() const;
	LPCTSTR GetSuperClassName() const;
	void OnFinalMessage(HWND hWnd);

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);	
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	CScrollbarUI* GetVerticalScrollbar() const {return m_pVScrollBar;}
    CScrollbarUI* GetHorizontalScrollbar() const {return m_pHScrollBar;}

	void BreakLinkage();

protected:
	CMultiLineEditUI*	m_pOwner;
	CScrollbarUI*		m_pVScrollBar;
	CScrollbarUI*		m_pHScrollBar;
};

void CMultiLineEditCanvasWnd::Init(CMultiLineEditUI* pOwner)
{
   CRect rcPos = pOwner->GetPos();
   Create(pOwner->m_pManager->GetPaintWindow(), NULL, WS_CHILD | WS_VISIBLE, 0, rcPos);
   if( pOwner->IsVisible() ) ::ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
   m_pOwner = pOwner;
}

void CMultiLineEditCanvasWnd::BreakLinkage()
{
	m_pOwner = NULL;
}

LPCTSTR CMultiLineEditCanvasWnd::GetWindowClassName() const
{
   return _T("MultiLineEditCanvasWnd");
}

LPCTSTR CMultiLineEditCanvasWnd::GetSuperClassName() const
{
   return NULL;
}

void CMultiLineEditCanvasWnd::OnFinalMessage(HWND /*hWnd*/)
{
	if (m_pOwner != NULL)
		m_pOwner->m_pCanvas = NULL;
	delete this;
}


LRESULT CMultiLineEditCanvasWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	BOOL bHandled = TRUE;
	if ( uMsg == WM_CTLCOLOREDIT )
	{
		::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
		HDC hDC = (HDC) wParam;
		::SetTextColor(hDC,RGB(0,0,0));
		::SetBkColor(hDC, RGB(255,255,255));
		lRes = (LRESULT) ::CreateSolidBrush(RGB(255,255,255));
	}
	else if( uMsg == WM_COMMAND && GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE ) lRes = OnEditChanged(uMsg, wParam, lParam, bHandled);
	else if( uMsg == WM_COMMAND && GET_WM_COMMAND_CMD(wParam, lParam) == EN_HSCROLL ) lRes = OnHScroll(uMsg, wParam, lParam, bHandled);
	else if( uMsg == WM_COMMAND && GET_WM_COMMAND_CMD(wParam, lParam) == EN_VSCROLL ) lRes = OnVScroll(uMsg, wParam, lParam, bHandled);
	else bHandled = FALSE;
	if( !bHandled ) return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	return lRes;
}

LRESULT CMultiLineEditCanvasWnd::OnHScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	if ((m_pOwner != NULL) && (m_pOwner->m_pWindow->GetHorizontalScrollbar() != NULL))
	{
		SCROLLINFO si = {0};
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_ALL;
		if (GetScrollInfo(m_pOwner->m_pWindow->GetHWND(), SB_HORZ, &si))
		{
			m_pOwner->m_pWindow->GetHorizontalScrollbar()->SetEnabled((si.nMax >= static_cast<int>(si.nPage)));
			m_pOwner->m_pWindow->GetHorizontalScrollbar()->SetScrollRange(si.nMax - si.nMin);
			m_pOwner->m_pWindow->GetHorizontalScrollbar()->SetScrollPos(si.nPos);
		}
	}
	return 0;
}

LRESULT CMultiLineEditCanvasWnd::OnVScroll(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	int nLineCount = Edit_GetLineCount(m_pOwner->m_pWindow->GetHWND());
	if ((nLineCount > 0) && (m_pOwner != NULL) && (m_pOwner->m_pWindow->GetVerticalScrollbar() != NULL))
	{
		SCROLLINFO si = {0};
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_ALL;
		if (GetScrollInfo(m_pOwner->m_pWindow->GetHWND(), SB_VERT, &si))
		{
			m_pOwner->m_pWindow->GetVerticalScrollbar()->SetEnabled((si.nMax >= static_cast<int>(si.nPage)));
			m_pOwner->m_pWindow->GetVerticalScrollbar()->SetScrollRange(si.nMax - si.nMin);
			m_pOwner->m_pWindow->GetVerticalScrollbar()->SetScrollPos(si.nPos);
		}
	}
	return 0;
}

LRESULT CMultiLineEditCanvasWnd::OnEditChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if( m_pOwner == NULL ) return 0;
	// Rearrange the scrollbar
	int nLineCount = Edit_GetLineCount(m_pOwner->m_pWindow->GetHWND());
	if ((nLineCount > 0) && (m_pOwner->m_pWindow->GetVerticalScrollbar() != NULL))
	{
		SCROLLINFO si = {0};
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_ALL;
		if (GetScrollInfo(m_pOwner->m_pWindow->GetHWND(), SB_VERT, &si))
		{
			m_pOwner->m_pWindow->GetVerticalScrollbar()->SetEnabled((si.nMax >= static_cast<int>(si.nPage)));
			m_pOwner->m_pWindow->GetVerticalScrollbar()->SetScrollRange(si.nMax - si.nMin);
			m_pOwner->m_pWindow->GetVerticalScrollbar()->SetScrollPos(si.nPos);
		}
	}

	// Copy text back
	int cchLen = ::GetWindowTextLength(m_pOwner->m_pWindow->GetHWND()) + 1;
	LPTSTR pstr = static_cast<LPTSTR>(_alloca(cchLen * sizeof(TCHAR)));
	ASSERT(pstr);
	::GetWindowText(m_pOwner->m_pWindow->GetHWND(), pstr, cchLen);
	m_pOwner->m_sText = pstr;
	m_pOwner->GetManager()->SendNotify(m_pOwner, _T("textchanged"));
	return 0;
}

void CMultiLineEditWnd::Init(CMultiLineEditUI* pOwner)
{
	CRect rcPos = pOwner->GetPos();
#if 1
	Create(pOwner->m_pCanvas->GetHWND(), NULL, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL, 0, rcPos);
#else
	Create(pOwner->m_pManager->GetPaintWindow(), NULL, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL, 0, rcPos);
#endif
	SetWindowFont(m_hWnd, pOwner->GetManager()->GetDefaultFont(), TRUE);
	Edit_SetText(m_hWnd, pOwner->m_sText);
	Edit_SetModify(m_hWnd, FALSE);
	SendMessage(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(2, 2));
	Edit_SetReadOnly(m_hWnd, pOwner->IsReadOnly() == true);
	Edit_Enable(m_hWnd, pOwner->IsEnabled() == true);
	if( pOwner->IsVisible() ) ::ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
	m_pOwner = pOwner;
}

void CMultiLineEditWnd::BreakLinkage()
{
	m_pOwner = NULL;
	m_pVScrollBar = NULL;
	m_pHScrollBar = NULL;
}

LPCTSTR CMultiLineEditWnd::GetWindowClassName() const
{
   return _T("MultiLineEditWnd");
}

LPCTSTR CMultiLineEditWnd::GetSuperClassName() const
{
   return WC_EDIT;
}

void CMultiLineEditWnd::OnFinalMessage(HWND /*hWnd*/)
{
	if (m_pOwner != NULL)
		m_pOwner->m_pWindow = NULL;
	delete this;
}

LRESULT CMultiLineEditWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = 0;
	BOOL bHandled = TRUE;
	if( uMsg == WM_SIZE ) lRes = OnSize(uMsg, wParam, lParam, bHandled);
	else bHandled = FALSE;
	if( !bHandled ) return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	return lRes;
}

LRESULT CMultiLineEditWnd::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
{
	SCROLLINFO vsi = {0};
	vsi.cbSize = sizeof(SCROLLINFO);
	vsi.fMask = SIF_ALL;
	SCROLLINFO hsi = {0};
	hsi.cbSize = sizeof(SCROLLINFO);
	hsi.fMask = SIF_ALL;
	if ((m_pOwner != NULL) && (m_pOwner->GetManager() != NULL) && GetScrollInfo(m_pOwner->m_pWindow->GetHWND(), SB_VERT, &vsi))
	{
		m_pVScrollBar = static_cast<CScrollbarUI*>(m_pOwner->GetManager()->FindControl(_T("vEditScrollbar")));
		m_pHScrollBar = static_cast<CScrollbarUI*>(m_pOwner->GetManager()->FindControl(_T("hEditScrollbar")));

		if (m_pVScrollBar != NULL)
		{
			m_pVScrollBar->SetNotifier(m_pOwner);
			m_pVScrollBar->SetEnabled(vsi.nMax >= static_cast<int>(vsi.nPage));
			m_pVScrollBar->SetScrollRange(vsi.nMax - vsi.nMin);
			m_pVScrollBar->SetScrollPos(vsi.nPos);			
		}

		if ((m_pHScrollBar != NULL) && GetScrollInfo(m_pOwner->m_pWindow->GetHWND(), SB_VERT, &hsi))
		{
			m_pHScrollBar->SetNotifier(m_pOwner);
			m_pVScrollBar->SetEnabled(hsi.nMax >= static_cast<int>(hsi.nPage));
			m_pHScrollBar->SetScrollRange(hsi.nMax - hsi.nMin);
			m_pHScrollBar->SetScrollPos(hsi.nPos);
		}

		m_pOwner->SetScrollRange(CSize(hsi.nMax - hsi.nMin, vsi.nMax - vsi.nMin));
	}
	
	bHandled = FALSE;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

CMultiLineEditUI::CMultiLineEditUI() : m_pWindow(NULL), m_pCanvas(NULL)
{}

CMultiLineEditUI::~CMultiLineEditUI()
{
   if (m_pCanvas != NULL && ::IsWindow(*m_pCanvas))
   {
	   m_pCanvas->BreakLinkage();
	   m_pCanvas->Close();
   }

   if( m_pWindow != NULL && ::IsWindow(*m_pWindow) )
   {
	   m_pWindow->BreakLinkage();
	   m_pWindow->Close();
   }
}

void CMultiLineEditUI::Init()
{
	m_pCanvas = new CMultiLineEditCanvasWnd();
	ASSERT(m_pCanvas);
	m_pCanvas->Init(this);

	m_pWindow = new CMultiLineEditWnd();
	ASSERT(m_pWindow);
	m_pWindow->Init(this);
}

LPCTSTR CMultiLineEditUI::GetClass() const
{
   return _T("MultiLineEditUI");
}

LPCTSTR CMultiLineEditUI::GetTypeName() const
{
	return _T("MultiLineEdit");
}

UINT CMultiLineEditUI::GetControlFlags() const
{
   return UIFLAG_TABSTOP;
}

void CMultiLineEditUI::SetText(LPCTSTR pstrText)
{
   m_sText = pstrText;
   if( m_pWindow != NULL ) SetWindowText(*m_pWindow, pstrText);
   if( m_pManager != NULL ) m_pManager->SendNotify(this, _T("textchanged"));
   Invalidate();
}

CStdString CMultiLineEditUI::GetText() const
{
   if( m_pWindow != NULL ) {
      int cchLen = ::GetWindowTextLength(*m_pWindow) + 1;
      LPTSTR pstr = static_cast<LPTSTR>(_alloca(cchLen * sizeof(TCHAR)));
      ASSERT(pstr);
      ::GetWindowText(*m_pWindow, pstr, cchLen);
      return CStdString(pstr);
   }
   return m_sText;
}

void CMultiLineEditUI::SetVisible(bool bVisible)
{
   CControlUI::SetVisible(bVisible);
   if( m_pWindow != NULL ) ::ShowWindow(*m_pWindow, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
   if( m_pCanvas != NULL ) ::ShowWindow(*m_pCanvas, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
}

void CMultiLineEditUI::SetEnabled(bool bEnabled)
{
   CControlUI::SetEnabled(bEnabled);
   if( m_pWindow != NULL ) ::EnableWindow(*m_pWindow, bEnabled == true);
   if( m_pCanvas != NULL ) ::EnableWindow(*m_pCanvas, bEnabled == true);
}

void CMultiLineEditUI::SetReadOnly(bool bReadOnly)
{
   if( m_pWindow != NULL ) Edit_SetReadOnly(*m_pWindow, bReadOnly == true);
   Invalidate();
}

bool CMultiLineEditUI::IsReadOnly() const
{
   return (GetWindowStyle(*m_pWindow) & ES_READONLY) != 0;
}

SIZE CMultiLineEditUI::EstimateSize(SIZE /*szAvailable*/)
{
	return CSize(m_rcItem);
}

void CMultiLineEditUI::SetPos(RECT rc)
{
	if( m_pCanvas != NULL ) {
		CRect rcEdit = rc;
		::SetWindowPos(*m_pCanvas, HWND_TOP, rcEdit.left, rcEdit.top, rcEdit.GetWidth(), rcEdit.GetHeight(), SWP_NOOWNERZORDER);
	}

	if( m_pWindow != NULL ) {
		CRect rcEdit = rc;
		rcEdit.right += 20;
		::SetWindowPos(*m_pWindow, HWND_TOP, 0, 0, rcEdit.GetWidth(), rcEdit.GetHeight(), SWP_NOACTIVATE);
	}
	CControlUI::SetPos(rc);
}

void CMultiLineEditUI::SetPos(int left, int top, int right, int bottom)
{
	SetPos(CRect(left, top, right, bottom));
}

void CMultiLineEditUI::Event(TEventUI& event)
{
#ifdef UI_DESIGNER
	if (IsUsedForPitchUpon() && (m_pManager != NULL) && m_pManager->IsUsedAsUIDesigner() && ((GetKeyState(VK_CONTROL) < 0) || (GetKeyState(VK_SHIFT) < 0)))
	{
		CControlUI::Event(event);
		return;
	}
#endif

	if( event.Type == UIEVENT_WINDOWSIZE )
	{
		if( m_pWindow != NULL ) m_pManager->SetFocus(NULL);
	}
	if( event.Type == UIEVENT_SCROLLWHEEL )
	{
		if( m_pWindow != NULL ) m_pManager->SetFocus(NULL);
	}
	if( event.Type == UIEVENT_SETFOCUS && IsEnabled() ) 
	{
		if( m_pWindow != NULL ) ::SetFocus(*m_pWindow);
		Invalidate();
	}

	CControlUI::Event(event);
}


void CMultiLineEditUI::PaintStatusImage(HDC hDC)
{
    DWORD dwBorderColor = 0xFF4EA0D1;
    int nBorderSize = 1;
    CRenderEngine::DrawRect(hDC, m_rcItem, nBorderSize, dwBorderColor);
}

void CMultiLineEditUI::PaintText(HDC hDC)
{
}


void CMultiLineEditUI::LineUp()
{
	SCROLLINFO vsi = {0};
	vsi.cbSize = sizeof(SCROLLINFO);
	vsi.fMask = SIF_ALL;

	if ((m_pWindow != NULL) && (m_pWindow->GetVerticalScrollbar() != NULL) && GetScrollInfo(m_pWindow->GetHWND(), SB_VERT, &vsi))
	{
		if (vsi.nPos > vsi.nMin)
		{
			vsi.cbSize = sizeof(SCROLLINFO);
			vsi.fMask = SIF_POS;
			--vsi.nPos;
			SetScrollInfo(m_pWindow->GetHWND(), SB_VERT, &vsi, TRUE);
			SendMessage(m_pWindow->GetHWND(), WM_VSCROLL, MAKELONG(SB_LINEUP, vsi.nPos), NULL);
		}
	}
}

void CMultiLineEditUI::LineDown()
{
	SCROLLINFO vsi = {0};
	vsi.cbSize = sizeof(SCROLLINFO);
	vsi.fMask = SIF_ALL;

	if ((m_pWindow != NULL) && (m_pWindow->GetVerticalScrollbar() != NULL) && GetScrollInfo(m_pWindow->GetHWND(), SB_VERT, &vsi))
	{
		if (vsi.nPos < vsi.nMax)
		{
			vsi.cbSize = sizeof(SCROLLINFO);
			vsi.fMask = SIF_POS;
			++vsi.nPos;
			SetScrollInfo(m_pWindow->GetHWND(), SB_VERT, &vsi, TRUE);
			SendMessage(m_pWindow->GetHWND(), WM_VSCROLL, MAKELONG(SB_LINEDOWN, vsi.nPos), NULL);
		}
	}
}

void CMultiLineEditUI::PageUp()
{
	SCROLLINFO vsi = {0};
	vsi.cbSize = sizeof(SCROLLINFO);
	vsi.fMask = SIF_ALL;

	if ((m_pWindow != NULL) && (m_pWindow->GetVerticalScrollbar() != NULL) && GetScrollInfo(m_pWindow->GetHWND(), SB_VERT, &vsi))
	{
		vsi.cbSize = sizeof(SCROLLINFO);
		vsi.fMask = SIF_POS;
		vsi.nPos -= vsi.nPage;
		if (vsi.nPos < vsi.nMin)
			vsi.nPos = vsi.nMin;
		SetScrollInfo(m_pWindow->GetHWND(), SB_VERT, &vsi, TRUE);
		SendMessage(m_pWindow->GetHWND(), WM_VSCROLL, MAKELONG(SB_PAGEUP, vsi.nPos), NULL);
	}
}

void CMultiLineEditUI::PageDown()
{
	SCROLLINFO vsi = {0};
	vsi.cbSize = sizeof(SCROLLINFO);
	vsi.fMask = SIF_ALL;

	if ((m_pWindow != NULL) && (m_pWindow->GetVerticalScrollbar() != NULL) && GetScrollInfo(m_pWindow->GetHWND(), SB_VERT, &vsi))
	{
		vsi.cbSize = sizeof(SCROLLINFO);
		vsi.fMask = SIF_POS;

		vsi.nPos += vsi.nPage;
		if (vsi.nPos > vsi.nMax)
			vsi.nPos = vsi.nMax;
		SetScrollInfo(m_pWindow->GetHWND(), SB_VERT, &vsi, TRUE);
		SendMessage(m_pWindow->GetHWND(), WM_VSCROLL, MAKELONG(SB_PAGEDOWN, vsi.nPos), NULL);
	}
}

void CMultiLineEditUI::HomeUp()
{
	SCROLLINFO vsi = {0};
	vsi.cbSize = sizeof(SCROLLINFO);
	vsi.fMask = SIF_ALL;

	if ((m_pWindow != NULL) && (m_pWindow->GetVerticalScrollbar() != NULL) && GetScrollInfo(m_pWindow->GetHWND(), SB_VERT, &vsi))
	{
		vsi.cbSize = sizeof(SCROLLINFO);
		vsi.fMask = SIF_POS;

		vsi.nPos =  vsi.nMin;
		SetScrollInfo(m_pWindow->GetHWND(), SB_VERT, &vsi, TRUE);
		SendMessage(m_pWindow->GetHWND(), WM_VSCROLL, MAKELONG(SB_LINEUP, vsi.nPos), NULL);
	}
}

void CMultiLineEditUI::EndDown()
{
	SCROLLINFO vsi = {0};
	vsi.cbSize = sizeof(SCROLLINFO);
	vsi.fMask = SIF_ALL;

	if ((m_pWindow != NULL) && (m_pWindow->GetVerticalScrollbar() != NULL) && GetScrollInfo(m_pWindow->GetHWND(), SB_VERT, &vsi))
	{
		vsi.cbSize = sizeof(SCROLLINFO);
		vsi.fMask = SIF_POS;

		vsi.nPos = vsi.nMax;
		SetScrollInfo(m_pWindow->GetHWND(), SB_VERT, &vsi, TRUE);
		SendMessage(m_pWindow->GetHWND(), WM_VSCROLL, MAKELONG(SB_LINEDOWN, vsi.nPos), NULL);
	}
}

void CMultiLineEditUI::LineLeft()
{
	SCROLLINFO hsi = {0};
	hsi.cbSize = sizeof(SCROLLINFO);
	hsi.fMask = SIF_ALL;
}

void CMultiLineEditUI::LineRight(){}
void CMultiLineEditUI::PageLeft(){}
void CMultiLineEditUI::PageRight(){}
void CMultiLineEditUI::HomeLeft(){}
void CMultiLineEditUI::EndRight(){}

SIZE CMultiLineEditUI::GetScrollPos() const
{
	return m_szScrollPos;
}

SIZE CMultiLineEditUI::GetScrollRange() const
{
	return m_szScrollRange;
}

void CMultiLineEditUI::SetScrollRange(SIZE szRange)
{
	m_szScrollRange = szRange;
}

void CMultiLineEditUI::SetScrollPos(SIZE szPos)
{
	m_szScrollPos = szPos;

	SCROLLINFO vsi = {0};
	vsi.cbSize = sizeof(SCROLLINFO);
	vsi.fMask = SIF_ALL;

	SCROLLINFO hsi = {0};
	hsi.cbSize = sizeof(SCROLLINFO);
	hsi.fMask = SIF_ALL;

	if ((m_pWindow != NULL) && (m_pWindow->GetVerticalScrollbar() != NULL) && GetScrollInfo(m_pWindow->GetHWND(), SB_VERT, &vsi))
	{
		vsi.cbSize = sizeof(SCROLLINFO);
		vsi.fMask = SIF_POS;
		
		vsi.nPos = m_pWindow->GetVerticalScrollbar()->GetScrollPos() + vsi.nMin;
		SetScrollInfo(m_pWindow->GetHWND(), SB_VERT, &vsi, TRUE);
		SendMessage(m_pWindow->GetHWND(), WM_VSCROLL, MAKELONG(SB_THUMBPOSITION, vsi.nPos), NULL);
	}

	if ((m_pWindow != NULL) && (m_pWindow->GetHorizontalScrollbar() != NULL) && GetScrollInfo(m_pWindow->GetHWND(), SB_HORZ, &hsi))
	{
		hsi.cbSize = sizeof(SCROLLINFO);
		hsi.fMask = SIF_POS;

		hsi.nPos = m_pWindow->GetHorizontalScrollbar()->GetScrollPos() + vsi.nMin;
		SetScrollInfo(m_pWindow->GetHWND(), SB_HORZ, &hsi, TRUE);
		SendMessage(m_pWindow->GetHWND(), WM_VSCROLL, MAKELONG(SB_THUMBPOSITION, hsi.nPos), NULL);
	}
}
#endif

}; // namespace DirectUI