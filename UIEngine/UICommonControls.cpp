#include "StdAfx.h"
#include "UIlib.h"

namespace DuiLib {

/////////////////////////////////////////////////////////////////////////////////////
//
//

CLabelUI::CLabelUI() : m_uTextStyle(DT_SINGLELINE | DT_VCENTER | DT_WORD_ELLIPSIS | DT_END_ELLIPSIS), m_dwTextColor(0), 
    m_dwDisabledTextColor(0), m_iFont(-1), m_bShowHtml(false)
{
    ::ZeroMemory(&m_rcTextPadding, sizeof(m_rcTextPadding));
}

LPCTSTR CLabelUI::GetClass() const
{
    return kLabelUIClassName;
}

LPVOID CLabelUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcsicmp(pstrName, kLabelUIInterfaceName) == 0 ) return static_cast<CLabelUI*>(this);
    return CControlUI::GetInterface(pstrName);
}

void CLabelUI::SetTextStyle(UINT uStyle)
{
    m_uTextStyle = uStyle;
    Invalidate();
}

UINT CLabelUI::GetTextStyle() const
{
	return m_uTextStyle;
}

void CLabelUI::SetTextColor(DWORD dwTextColor)
{
    m_dwTextColor = dwTextColor;
}

DWORD CLabelUI::GetTextColor() const
{
	return m_dwTextColor;
}

void CLabelUI::SetDisabledTextColor(DWORD dwTextColor)
{
    m_dwDisabledTextColor = dwTextColor;
}

DWORD CLabelUI::GetDisabledTextColor() const
{
	return m_dwDisabledTextColor;
}

void CLabelUI::SetFont(int index)
{
    m_iFont = index;
}

int CLabelUI::GetFont() const
{
	return m_iFont;
}

RECT CLabelUI::GetTextPadding() const
{
    return m_rcTextPadding;
}

void CLabelUI::SetTextPadding(RECT rc)
{
    m_rcTextPadding = rc;
    Invalidate();
}

bool CLabelUI::IsShowHtml()
{
    return m_bShowHtml;
}

void CLabelUI::SetShowHtml(bool bShowHtml)
{
    if( m_bShowHtml == bShowHtml ) return;

    m_bShowHtml = bShowHtml;
    Invalidate();
}

SIZE CLabelUI::EstimateSize(SIZE szAvailable)
{
	// 没有指定固定的宽高
    if (m_cxyFixed.cy == 0)
	{
		if (m_uTextStyle & DT_WORDBREAK)
		{
			// 显示多行
			CRect rcTemp;
			HFONT hOldFont = (HFONT)::SelectObject(m_pManager->GetPaintDC(), m_pManager->GetFont(m_iFont));
			::DrawText(m_pManager->GetPaintDC(), m_sText, m_sText.GetLength(), &rcTemp, m_uTextStyle | DT_CALCRECT);
			::SelectObject(m_pManager->GetPaintDC(), hOldFont);
			return CSize(m_cxyFixed.cx, rcTemp.Height() + m_rcTextPadding.bottom + m_rcTextPadding.top);
		}
		else
		{
			// 显示单独一行
			//return CSize(m_cxyFixed.cx, m_pManager->GetFontInfo(GetFont())->tm.tmHeight + 4);
			return CSize(m_cxyFixed.cx, m_pManager->GetFontInfo(GetFont())->tm.tmHeight + m_rcTextPadding.bottom + m_rcTextPadding.top);
		}
	}
    return CControlUI::EstimateSize(szAvailable);
}

void CLabelUI::DoEvent(TEventUI& event)
{
#ifdef UI_BUILD_FOR_DESIGNER
	if (IsUsedForPitchUpon() && (m_pManager != NULL) && m_pManager->IsUsedAsUIDesigner() && ((GetKeyState(VK_CONTROL) < 0) || (GetKeyState(VK_SHIFT) < 0)))
	{
		CControlUI::DoEvent(event);
		return;
	}
#endif

    if( event.Type == UIEVENT_SETFOCUS ) 
    {
        m_bFocused = true;
        return;
    }
    if( event.Type == UIEVENT_KILLFOCUS ) 
    {
        m_bFocused = false;
        return;
    }
    if( event.Type == UIEVENT_MOUSEENTER )
    {
        return;
    }
    if( event.Type == UIEVENT_MOUSELEAVE )
    {
        return;
    }
    CControlUI::DoEvent(event);
}

void CLabelUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if( _tcsicmp(pstrName, _T("align")) == 0 ) {
        if( _tcsstr(pstrValue, _T("left")) != NULL ) {
            m_uTextStyle &= ~(DT_CENTER | DT_RIGHT | DT_TOP | DT_BOTTOM);
            m_uTextStyle |= DT_LEFT;
        }
        if( _tcsstr(pstrValue, _T("center")) != NULL ) {
            m_uTextStyle &= ~(DT_LEFT | DT_RIGHT | DT_TOP | DT_BOTTOM);
            m_uTextStyle |= DT_CENTER;
        }
        if( _tcsstr(pstrValue, _T("right")) != NULL ) {
            m_uTextStyle &= ~(DT_LEFT | DT_CENTER | DT_TOP | DT_BOTTOM);
            m_uTextStyle |= DT_RIGHT;
        }
		if( _tcsstr(pstrValue, _T("top")) != NULL ) {
			m_uTextStyle &= ~(DT_BOTTOM | DT_VCENTER | DT_LEFT | DT_RIGHT);
			m_uTextStyle |= DT_TOP;
		}
		if( _tcsstr(pstrValue, _T("bottom")) != NULL ) {
			m_uTextStyle &= ~(DT_TOP | DT_VCENTER | DT_LEFT | DT_RIGHT);
			m_uTextStyle |= DT_BOTTOM;
		}
		if( _tcsstr(pstrValue, _T("wrap")) != NULL ) {
			m_uTextStyle &= ~DT_SINGLELINE;
			m_uTextStyle |= DT_WORDBREAK;
		}
		else {
			m_uTextStyle |= DT_SINGLELINE;
		}
    }
    else if( _tcsicmp(pstrName, _T("endellipsis")) == 0 ) {
        if( _tcsicmp(pstrValue, _T("true")) == 0 ) m_uTextStyle |= DT_END_ELLIPSIS;
        else m_uTextStyle &= ~DT_END_ELLIPSIS;
    }    	
    else if( _tcsicmp(pstrName, _T("font")) == 0 ) SetFont(_ttoi(pstrValue));
    else if( _tcsicmp(pstrName, _T("textcolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetTextColor(clrColor);
    }
    else if( _tcsicmp(pstrName, _T("disabledtextcolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetDisabledTextColor(clrColor);
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
    else if( _tcsicmp(pstrName, _T("showhtml")) == 0 ) SetShowHtml(_tcsicmp(pstrValue, _T("true")) == 0);
    else CControlUI::SetAttribute(pstrName, pstrValue);
}

void CLabelUI::PaintText(void* ctx)
{
	if (ctx == NULL) return;

    if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
    if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

    if( m_sText.IsEmpty() ) return;
    int nLinks = 0;
    RECT rc = m_rcItem;
    rc.left += m_rcTextPadding.left;
    rc.right -= m_rcTextPadding.right;
    rc.top += m_rcTextPadding.top;
    rc.bottom -= m_rcTextPadding.bottom;

    if( m_bShowHtml )
		CRenderEngine::DrawHtmlText(ctx, this, m_pManager, rc, m_sText, IsEnabled()?m_dwTextColor:m_dwDisabledTextColor, \
        NULL, NULL, nLinks, m_uTextStyle);
    else
        CRenderEngine::DrawText(ctx, m_pManager, rc, m_sText, IsEnabled()?m_dwTextColor:m_dwDisabledTextColor, \
        m_iFont, m_uTextStyle);
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

CButtonUI::CButtonUI() : m_uButtonState(0), m_dwHotTextColor(0), m_dwPushedTextColor(0), m_dwFocusedTextColor(0)

{
    m_uTextStyle = DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_WORD_ELLIPSIS | DT_END_ELLIPSIS;
}

LPCTSTR CButtonUI::GetClass() const
{
    return kButtonUIClassName;
}

LPVOID CButtonUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcsicmp(pstrName, kButtonUIInterfaceName) == 0 ) return static_cast<CButtonUI*>(this);
    return CLabelUI::GetInterface(pstrName);
}

UINT CButtonUI::GetControlFlags() const
{
    return (IsKeyboardEnabled() ? UIFLAG_TABSTOP : 0) | (IsEnabled() ? UIFLAG_SETCURSOR : 0);
}

void CButtonUI::DoEvent(TEventUI& event)
{
#ifdef UI_BUILD_FOR_DESIGNER
	if (IsUsedForPitchUpon() && (m_pManager != NULL) && m_pManager->IsUsedAsUIDesigner() && ((GetKeyState(VK_CONTROL) < 0) || (GetKeyState(VK_SHIFT) < 0)))
	{
		CControlUI::DoEvent(event);
		return;
	}
#endif

    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_pParent != NULL ) m_pParent->DoEvent(event);
        else CLabelUI::DoEvent(event);
        return;
    }

    if( event.Type == UIEVENT_SETFOCUS ) 
    {
        Invalidate();
    }
    if( event.Type == UIEVENT_KILLFOCUS ) 
    {
        Invalidate();
    }
    if( event.Type == UIEVENT_KEYDOWN )
    {
		if (IsKeyboardEnabled()) {
			if( event.chKey == VK_SPACE || event.chKey == VK_RETURN ) {
				Activate();
				return;
			}
		}
    }
    if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK )
    {
        if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() ) {
            m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_MOUSEMOVE )
    {
        if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
            if( ::PtInRect(&m_rcItem, event.ptMouse) ) m_uButtonState |= UISTATE_PUSHED;
            else m_uButtonState &= ~UISTATE_PUSHED;
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_BUTTONUP )
    {
        if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
            if( ::PtInRect(&m_rcItem, event.ptMouse) ) Activate();
            m_uButtonState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_CONTEXTMENU )
    {
        if( IsContextMenuUsed() ) {
            m_pManager->SendNotify(this, _T("menu"), event.wParam, event.lParam);
        }
        return;
    }
    if( event.Type == UIEVENT_MOUSEENTER )
    {
        if( IsEnabled() ) {
            m_uButtonState |= UISTATE_HOT;
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_MOUSELEAVE )
    {
        if( IsEnabled() ) {
            m_uButtonState &= ~UISTATE_HOT;
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_SETCURSOR ) {
        ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
        return;
    }
    CLabelUI::DoEvent(event);
}

bool CButtonUI::Activate()
{
    if( !CControlUI::Activate() ) return false;
    if( m_pManager != NULL ) m_pManager->SendNotify(this, kClick);
    return true;
}

void CButtonUI::SetEnabled(bool bEnable)
{
    CControlUI::SetEnabled(bEnable);
    if( !IsEnabled() ) {
        m_uButtonState = 0;
    }
}

void CButtonUI::SetHotTextColor(DWORD dwColor)
{
	m_dwHotTextColor = dwColor;
}

DWORD CButtonUI::GetHotTextColor() const
{
	return m_dwHotTextColor;
}

void CButtonUI::SetPushedTextColor(DWORD dwColor)
{
	m_dwPushedTextColor = dwColor;
}

DWORD CButtonUI::GetPushedTextColor() const
{
	return m_dwPushedTextColor;
}

void CButtonUI::SetFocusedTextColor(DWORD dwColor)
{
	m_dwFocusedTextColor = dwColor;
}

DWORD CButtonUI::GetFocusedTextColor() const
{
	return m_dwFocusedTextColor;
}

LPCTSTR CButtonUI::GetNormalImage()
{
    return m_sNormalImage;
}

void CButtonUI::SetNormalImage(LPCTSTR pStrImage)
{
    m_sNormalImage = pStrImage;
    Invalidate();
}

LPCTSTR CButtonUI::GetHotImage()
{
    return m_sHotImage;
}

void CButtonUI::SetHotImage(LPCTSTR pStrImage)
{
    m_sHotImage = pStrImage;
    Invalidate();
}

LPCTSTR CButtonUI::GetPushedImage()
{
    return m_sPushedImage;
}

void CButtonUI::SetPushedImage(LPCTSTR pStrImage)
{
    m_sPushedImage = pStrImage;
    Invalidate();
}

LPCTSTR CButtonUI::GetFocusedImage()
{
    return m_sFocusedImage;
}

void CButtonUI::SetFocusedImage(LPCTSTR pStrImage)
{
    m_sFocusedImage = pStrImage;
    Invalidate();
}

LPCTSTR CButtonUI::GetDisabledImage()
{
    return m_sDisabledImage;
}

void CButtonUI::SetDisabledImage(LPCTSTR pStrImage)
{
    m_sDisabledImage = pStrImage;
    Invalidate();
}

LPCTSTR CButtonUI::GetForeImage()
{
	return m_sForeImage;
}

void CButtonUI::SetForeImage(LPCTSTR pStrImage)
{
	m_sForeImage=pStrImage;
	Invalidate();
}

SIZE CButtonUI::EstimateSize(SIZE szAvailable)
{
    if( m_cxyFixed.cy == 0 ) return CSize(m_cxyFixed.cx, m_pManager->GetFontInfo(GetFont())->tm.tmHeight + 8);
    return CControlUI::EstimateSize(szAvailable);
}

void CButtonUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if( _tcsicmp(pstrName, _T("normalimage")) == 0 ) SetNormalImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("hotimage")) == 0 ) SetHotImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("pushedimage")) == 0 ) SetPushedImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("focusedimage")) == 0 ) SetFocusedImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("disabledimage")) == 0 ) SetDisabledImage(pstrValue);
	else if( _tcsicmp(pstrName, _T("foreimage")) == 0 ) SetForeImage(pstrValue);
	else if( _tcsicmp(pstrName, _T("fitallArea")) == 0 ) SetImageFitAllArea((_tcsicmp(pstrValue, _T("true")) == 0)?true:false);
    else if( _tcsicmp(pstrName, _T("hottextcolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetHotTextColor(clrColor);
    }
    else if( _tcsicmp(pstrName, _T("pushedtextcolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetPushedTextColor(clrColor);
    }
    else if( _tcsicmp(pstrName, _T("focusedtextcolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetFocusedTextColor(clrColor);
    }
    else CLabelUI::SetAttribute(pstrName, pstrValue);
}

void CButtonUI::PaintBorder(void* ctx)
{
	if (ctx == NULL) return;
	//if (m_dwBorderColor != 0 && m_nBorderSize > 0)
	//{
	//	DWORD dwBorderColor = m_dwBorderColor;
	//	int nBorderSize = m_nBorderSize;
	//	if (((m_uButtonState & UISTATE_HOT) != 0) || ((m_uButtonState & UISTATE_FOCUSED) != 0)) {
	//		dwBorderColor = 0xFF85E4FF;
	//		nBorderSize += 1;
	//	}
	//	CRenderEngine::DrawRect(hDC, m_rcItem, nBorderSize, dwBorderColor);
	//}

	__super::PaintBorder(ctx);
}

void CButtonUI::PaintText(void* ctx)
{
	if (ctx == NULL) return;

	if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
	else m_uButtonState &= ~ UISTATE_FOCUSED;
	if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
	else m_uButtonState &= ~ UISTATE_DISABLED;

    if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
    if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

    if( m_sText.IsEmpty() ) return;
    int nLinks = 0;
    RECT rc = m_rcItem;
    rc.left += m_rcTextPadding.left;
    rc.right -= m_rcTextPadding.right;
    rc.top += m_rcTextPadding.top;
    rc.bottom -= m_rcTextPadding.bottom;

	if (!m_bImageFitallArea && !m_sNormalImage.IsEmpty())
	{
		RECT rcImg = {0};
		CRenderEngine::CaculateImageRect(m_sNormalImage, this, m_pManager, rcImg);
		rc.left += rcImg.right - rcImg.left;
	}

	DWORD clrColor = IsEnabled()?m_dwTextColor:m_dwDisabledTextColor;

	if( ((m_uButtonState & UISTATE_PUSHED) != 0) && (GetPushedTextColor() != 0) )
		clrColor = GetPushedTextColor();
	else if( ((m_uButtonState & UISTATE_HOT) != 0) && (GetHotTextColor() != 0) )
		clrColor = GetHotTextColor();
	else if( ((m_uButtonState & UISTATE_FOCUSED) != 0) && (GetFocusedTextColor() != 0) )
		clrColor = GetFocusedTextColor();

    if( m_bShowHtml )
		CRenderEngine::DrawHtmlText(ctx, this, m_pManager, rc, m_sText, clrColor, \
        NULL, NULL, nLinks, m_uTextStyle);
    else
        CRenderEngine::DrawText(ctx, m_pManager, rc, m_sText, clrColor, \
        m_iFont, m_uTextStyle);
}

void CButtonUI::PaintStatusImage(void* ctx)
{
	if (ctx == NULL) return;

	if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
	else m_uButtonState &= ~ UISTATE_FOCUSED;
	if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
	else m_uButtonState &= ~ UISTATE_DISABLED;

	if( (m_uButtonState & UISTATE_DISABLED) != 0 ) {
		if( !m_sDisabledImage.IsEmpty() ) {
			if( !DrawImage(ctx, (LPCTSTR)m_sDisabledImage) ) m_sDisabledImage.Empty();
			else
			{
				PaintForeImage(ctx);
				return;
			}
		}
	}
	else if( (m_uButtonState & UISTATE_PUSHED) != 0 ) {
		if( !m_sPushedImage.IsEmpty() ) {
			if( !DrawImage(ctx, (LPCTSTR)m_sPushedImage) ) m_sPushedImage.Empty();
			else
			{
				PaintForeImage(ctx);
				return;
			}
		}
	}
	else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
		if( !m_sHotImage.IsEmpty() ) {
			if( !DrawImage(ctx, (LPCTSTR)m_sHotImage) ) m_sHotImage.Empty();
			else
			{
				PaintForeImage(ctx);
				return;
			}
		}
	}
	else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) {
		if( !m_sFocusedImage.IsEmpty() ) {
			if( !DrawImage(ctx, (LPCTSTR)m_sFocusedImage) ) m_sFocusedImage.Empty();
			else
			{
				PaintForeImage(ctx);
				return;
			}
		}
	}

	if( !m_sNormalImage.IsEmpty() ) {
		if( !DrawImage(ctx, (LPCTSTR)m_sNormalImage) ) m_sNormalImage.Empty();
	}

	PaintForeImage(ctx);
}

void CButtonUI::PaintForeImage(void* ctx)
{
	if (ctx == NULL) return;

	if( !m_sForeImage.IsEmpty() ) {
		if( !DrawImage(ctx, (LPCTSTR)m_sForeImage) ) m_sForeImage.Empty();
	}
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

COptionUI::COptionUI() : m_bSelected(false), m_dwSelectedTextColor(0)
{
}

COptionUI::~COptionUI()
{
    if( !m_sGroupName.IsEmpty() && m_pManager ) m_pManager->RemoveOptionGroup(m_sGroupName, this);
}

LPCTSTR COptionUI::GetClass() const
{
    return kOptionUIClassName;
}

LPVOID COptionUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcsicmp(pstrName, kOptionUIInterfaceName) == 0 ) return static_cast<COptionUI*>(this);
    return CButtonUI::GetInterface(pstrName);
}

void COptionUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
{
    CControlUI::SetManager(pManager, pParent, bInit);
    if( bInit && !m_sGroupName.IsEmpty() ) {
        if (m_pManager) m_pManager->AddOptionGroup(m_sGroupName, this);
    }
}

LPCTSTR COptionUI::GetGroup() const
{
    return m_sGroupName;
}

void COptionUI::SetGroup(LPCTSTR pStrGroupName)
{
    if( pStrGroupName == NULL ) {
        if( m_sGroupName.IsEmpty() ) return;
        m_sGroupName.Empty();
    }
    else {
        if( m_sGroupName == pStrGroupName ) return;
        if (!m_sGroupName.IsEmpty() && m_pManager) m_pManager->RemoveOptionGroup(m_sGroupName, this);
        m_sGroupName = pStrGroupName;
    }

    if( !m_sGroupName.IsEmpty() ) {
        if (m_pManager) m_pManager->AddOptionGroup(m_sGroupName, this);
    }
    else {
        if (m_pManager) m_pManager->RemoveOptionGroup(m_sGroupName, this);
    }

    Selected(m_bSelected);
}

bool COptionUI::IsSelected() const
{
    return m_bSelected;
}

void COptionUI::Selected(bool bSelected)
{
    if( m_bSelected == bSelected ) return;
    m_bSelected = bSelected;
    if( m_bSelected ) m_uButtonState |= UISTATE_SELECTED;
    else m_uButtonState &= ~UISTATE_SELECTED;

    if( m_pManager != NULL ) {
        if( !m_sGroupName.IsEmpty() ) {
            if( m_bSelected ) {
                CStdPtrArray* aOptionGroup = m_pManager->GetOptionGroup(m_sGroupName);
                for( int i = 0; i < aOptionGroup->GetSize(); i++ ) {
                    COptionUI* pControl = static_cast<COptionUI*>(aOptionGroup->GetAt(i));
                    if( pControl != this ) {
                        pControl->Selected(false);
                    }
                }
                m_pManager->SendNotify(this, kSelectChanged);
            }
        }
        else {
            m_pManager->SendNotify(this, kSelectChanged);
        }
    }

    Invalidate();
}

bool COptionUI::Activate()
{
    if( !CButtonUI::Activate() ) return false;
    if( !m_sGroupName.IsEmpty() ) Selected(true);
    else Selected(!m_bSelected);

    return true;
}

void COptionUI::SetEnabled(bool bEnable)
{
    CControlUI::SetEnabled(bEnable);
    if( !IsEnabled() ) {
        if( m_bSelected ) m_uButtonState = UISTATE_SELECTED;
        else m_uButtonState = 0;
    }
}

LPCTSTR COptionUI::GetSelectedImage()
{
    return m_sSelectedImage;
}

void COptionUI::SetSelectedImage(LPCTSTR pStrImage)
{
    m_sSelectedImage = pStrImage;
    Invalidate();
}

void COptionUI::SetSelectedTextColor(DWORD dwTextColor)
{
	m_dwSelectedTextColor = dwTextColor;
}

DWORD COptionUI::GetSelectedTextColor()
{
	if (m_dwSelectedTextColor == 0) m_dwSelectedTextColor = m_pManager->GetDefaultFontColor();
	return m_dwSelectedTextColor;
}

SIZE COptionUI::EstimateSize(SIZE szAvailable)
{
    if( m_cxyFixed.cy == 0 ) return CSize(m_cxyFixed.cx, m_pManager->GetFontInfo(GetFont())->tm.tmHeight + 8);
    return CControlUI::EstimateSize(szAvailable);
}

void COptionUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if( _tcsicmp(pstrName, _T("group")) == 0 ) SetGroup(pstrValue);
    else if( _tcsicmp(pstrName, _T("selected")) == 0 ) Selected(_tcsicmp(pstrValue, _T("true")) == 0);
    else if( _tcsicmp(pstrName, _T("selectedimage")) == 0 ) SetSelectedImage(pstrValue);	
	else if( _tcsicmp(pstrName, _T("selectedtextcolor")) == 0 ) {
		if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
		LPTSTR pstr = NULL;
		DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
		SetSelectedTextColor(clrColor);
	}
    else CButtonUI::SetAttribute(pstrName, pstrValue);
}

void COptionUI::PaintStatusImage(void* ctx)
{
	if (ctx == NULL) return;
    m_uButtonState &= ~UISTATE_PUSHED;

	if( (m_uButtonState & UISTATE_SELECTED) != 0 ) {
		m_uButtonState &= ~UISTATE_HOT;
		if( !m_sSelectedImage.IsEmpty() ) {
			if( !DrawImage(ctx, (LPCTSTR)m_sSelectedImage) ) m_sSelectedImage.Empty();
			else
			{
				PaintForeImage(ctx);
				return;
			}
		}
	}

	CButtonUI::PaintStatusImage(ctx);

	return;
}

void COptionUI::PaintText(void* ctx)
{
	if (ctx == NULL) return;

	if( (m_uButtonState & UISTATE_SELECTED) != 0 )
	{

		DWORD oldTextColor = m_dwTextColor;
		if( m_dwSelectedTextColor != 0 ) m_dwTextColor = m_dwSelectedTextColor;

		if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
		if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

		if( m_sText.IsEmpty() ) return;
		int nLinks = 0;
		RECT rc = m_rcItem;
		rc.left += m_rcTextPadding.left;
		rc.right -= m_rcTextPadding.right;
		rc.top += m_rcTextPadding.top;
		rc.bottom -= m_rcTextPadding.bottom;

		if (!m_bImageFitallArea && !m_sSelectedImage.IsEmpty())
		{
			RECT rcImg = {0};
			CRenderEngine::CaculateImageRect(m_sSelectedImage, this, m_pManager, rcImg);
			rc.left += rcImg.right - rcImg.left;
		}

		if( m_bShowHtml )
			CRenderEngine::DrawHtmlText(ctx, this, m_pManager, rc, m_sText, IsEnabled()?m_dwTextColor:m_dwDisabledTextColor, \
			NULL, NULL, nLinks, DT_SINGLELINE | m_uTextStyle);
		else
			CRenderEngine::DrawText(ctx, m_pManager, rc, m_sText, IsEnabled()?m_dwTextColor:m_dwDisabledTextColor, \
			m_iFont, m_uTextStyle);

		m_dwTextColor = oldTextColor;
	}
	else
		CButtonUI::PaintText(ctx);
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

CTextUI::CTextUI() : m_nLinks(0), m_nHoverLink(-1)
{
    m_uTextStyle = DT_WORDBREAK;
    m_rcTextPadding.left = 2;
    m_rcTextPadding.right = 2;
    ::ZeroMemory(m_rcLinks, sizeof(m_rcLinks));
}

CTextUI::~CTextUI()
{
}

LPCTSTR CTextUI::GetClass() const
{
    return kTextUIClassName;
}

LPVOID CTextUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcsicmp(pstrName, kTextUIInterfaceName) == 0 ) return static_cast<CTextUI*>(this);
    return CLabelUI::GetInterface(pstrName);
}

UINT CTextUI::GetControlFlags() const
{
#ifdef UI_BUILD_FOR_DESIGNER
	return UIFLAG_SETCURSOR;
#else
    if( IsEnabled() && m_nLinks > 0 ) return UIFLAG_SETCURSOR;
    else return 0;
#endif
}

CStdString* CTextUI::GetLinkContent(int iIndex)
{
    if( iIndex >= 0 && iIndex < m_nLinks ) return &m_sLinks[iIndex];
    return NULL;
}

void CTextUI::DoEvent(TEventUI& event)
{
#ifdef UI_BUILD_FOR_DESIGNER
	if (IsUsedForPitchUpon() && (m_pManager != NULL) && m_pManager->IsUsedAsUIDesigner() && ((GetKeyState(VK_CONTROL) < 0) || (GetKeyState(VK_SHIFT) < 0)))
	{
		CControlUI::DoEvent(event);
		return;
	}
#endif

	if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_pParent != NULL ) m_pParent->DoEvent(event);
        else CLabelUI::DoEvent(event);
        return;
    }

    if( event.Type == UIEVENT_SETCURSOR ) {
        for( int i = 0; i < m_nLinks; i++ ) {
            if( ::PtInRect(&m_rcLinks[i], event.ptMouse) ) {
                ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
                return;
            }
        }
    }
    if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK && IsEnabled() ) {
        for( int i = 0; i < m_nLinks; i++ ) {
            if( ::PtInRect(&m_rcLinks[i], event.ptMouse) ) {
                Invalidate();
                return;
            }
        }
    }
    if( event.Type == UIEVENT_BUTTONUP && IsEnabled() ) {
        for( int i = 0; i < m_nLinks; i++ ) {
            if( ::PtInRect(&m_rcLinks[i], event.ptMouse) ) {
                m_pManager->SendNotify(this, _T("link"), i);
                return;
            }
        }
    }
    if( event.Type == UIEVENT_CONTEXTMENU )
    {
        return;
    }
    // When you move over a link
    if( m_nLinks > 0 && event.Type == UIEVENT_MOUSEMOVE && IsEnabled() ) {
        int nHoverLink = -1;
        for( int i = 0; i < m_nLinks; i++ ) {
            if( ::PtInRect(&m_rcLinks[i], event.ptMouse) ) {
                nHoverLink = i;
                break;
            }
        }

        if(m_nHoverLink != nHoverLink) {
            m_nHoverLink = nHoverLink;
            Invalidate();
            return;
        }      
    }
    if( event.Type == UIEVENT_MOUSELEAVE ) {
        if( m_nLinks > 0 && IsEnabled() ) {
            if(m_nHoverLink != -1) {
                m_nHoverLink = -1;
                Invalidate();
                return;
            }
        }
    }

    CLabelUI::DoEvent(event);
}

SIZE CTextUI::EstimateSize(SIZE szAvailable)
{
    RECT rcText = { 0, 0, MAX(szAvailable.cx, m_cxyFixed.cx), 9999 };
    rcText.left += m_rcTextPadding.left;
    rcText.right -= m_rcTextPadding.right;
    if( m_bShowHtml ) {   
        int nLinks = 0;
#if defined(UI_BUILD_FOR_WINGDI)
        CRenderEngine::DrawHtmlText(m_pManager->GetPaintDC(), this, m_pManager, rcText, m_sText, m_dwTextColor, NULL, NULL, nLinks, DT_CALCRECT | m_uTextStyle);
#elif defined(UI_BUILD_FOR_SKIA)
		CRenderEngine::DrawHtmlText(m_pManager->GetSkiaContext(), this, m_pManager, rcText, m_sText, m_dwTextColor, NULL, NULL, nLinks, DT_CALCRECT | m_uTextStyle);
#endif
    }
    else {
#if defined(UI_BUILD_FOR_WINGDI)
        CRenderEngine::DrawText(m_pManager->GetPaintDC(), m_pManager, rcText, m_sText, m_dwTextColor, m_iFont, DT_CALCRECT | m_uTextStyle);
#elif defined(UI_BUILD_FOR_SKIA)
		CRenderEngine::DrawText(m_pManager->GetSkiaContext(), m_pManager, rcText, m_sText, m_dwTextColor, m_iFont, DT_CALCRECT | m_uTextStyle);
#endif
    }
    SIZE cXY = {rcText.right - rcText.left + m_rcTextPadding.left + m_rcTextPadding.right,
        rcText.bottom - rcText.top + m_rcTextPadding.top + m_rcTextPadding.bottom};

    if( m_cxyFixed.cy != 0 ) cXY.cy = m_cxyFixed.cy;
    return cXY;
}

void CTextUI::PaintText(void* ctx)
{
	if (ctx == NULL) return;

    if( m_sText.IsEmpty() ) {
        m_nLinks = 0;
        return;
    }

    if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
    if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

    if( m_sText.IsEmpty() ) return;

    m_nLinks = lengthof(m_rcLinks);
    RECT rc = m_rcItem;
    rc.left += m_rcTextPadding.left;
    rc.right -= m_rcTextPadding.right;
    rc.top += m_rcTextPadding.top;
    rc.bottom -= m_rcTextPadding.bottom;
    if( IsEnabled() ) {
        if( m_bShowHtml )
            CRenderEngine::DrawHtmlText(ctx, this, m_pManager, rc, m_sText, m_dwTextColor, \
            m_rcLinks, m_sLinks, m_nLinks, m_uTextStyle);
        else
            CRenderEngine::DrawText(ctx, m_pManager, rc, m_sText, m_dwTextColor, \
            m_iFont, m_uTextStyle);
    }
    else {
        if( m_bShowHtml )
            CRenderEngine::DrawHtmlText(ctx, this, m_pManager, rc, m_sText, m_dwDisabledTextColor, \
            m_rcLinks, m_sLinks, m_nLinks, m_uTextStyle);
        else
            CRenderEngine::DrawText(ctx, m_pManager, rc, m_sText, m_dwDisabledTextColor, \
            m_iFont, m_uTextStyle);
    }
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

CProgressUI::CProgressUI() : m_bHorizontal(true), m_nMin(0), m_nMax(100), m_nValue(0)
{
	m_dwBorderColor = 0xFF4EA0D1;
	m_nBorderSize = 1;
    m_uTextStyle = DT_SINGLELINE | DT_CENTER | DT_WORD_ELLIPSIS | DT_END_ELLIPSIS;
    SetFixedHeight(12);
}

LPCTSTR CProgressUI::GetClass() const
{
    return kProgressUIClassName;
}

LPVOID CProgressUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcsicmp(pstrName, kProgressUIInterfaceName) == 0 ) return static_cast<CProgressUI*>(this);
    return CLabelUI::GetInterface(pstrName);
}

bool CProgressUI::IsHorizontal()
{
    return m_bHorizontal;
}

void CProgressUI::SetHorizontal(bool bHorizontal)
{
    if( m_bHorizontal == bHorizontal ) return;

    m_bHorizontal = bHorizontal;
    Invalidate();
}

int CProgressUI::GetMinValue() const
{
    return m_nMin;
}

void CProgressUI::SetMinValue(int nMin)
{
    m_nMin = nMin;
    Invalidate();
}

int CProgressUI::GetMaxValue() const
{
    return m_nMax;
}

void CProgressUI::SetMaxValue(int nMax)
{
    m_nMax = nMax;
    Invalidate();
}

int CProgressUI::GetValue() const
{
    return m_nValue;
}

void CProgressUI::SetValue(int nValue)
{
    m_nValue = nValue;
    Invalidate();
}

LPCTSTR CProgressUI::GetForeImage() const
{
    return m_sForeImage;
}

void CProgressUI::SetForeImage(LPCTSTR pStrImage)
{
    if( m_sForeImage == pStrImage ) return;

    m_sForeImage = pStrImage;
    Invalidate();
}

void CProgressUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if( _tcsicmp(pstrName, _T("foreimage")) == 0 ) SetForeImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("hor")) == 0 ) SetHorizontal(_tcsicmp(pstrValue, _T("true")) == 0);
    else if( _tcsicmp(pstrName, _T("min")) == 0 ) SetMinValue(_ttoi(pstrValue));
    else if( _tcsicmp(pstrName, _T("max")) == 0 ) SetMaxValue(_ttoi(pstrValue));
    else if( _tcsicmp(pstrName, _T("value")) == 0 ) SetValue(_ttoi(pstrValue));
    else CLabelUI::SetAttribute(pstrName, pstrValue);
}

void CProgressUI::PaintStatusImage(void* ctx)
{
	if (ctx == NULL) return;

	if( m_nMax <= m_nMin ) m_nMax = m_nMin + 1;
	if( m_nValue > m_nMax ) m_nValue = m_nMax;
	if( m_nValue < m_nMin ) m_nValue = m_nMin;

	RECT rc = {0};
	if( m_bHorizontal ) {
		rc.right = (m_nValue - m_nMin) * (m_rcItem.right - m_rcItem.left) / (m_nMax - m_nMin);
		rc.bottom = m_rcItem.bottom - m_rcItem.top;
	}
	else {
		rc.top = (m_rcItem.bottom - m_rcItem.top) * (m_nMax - m_nValue) / (m_nMax - m_nMin);
		rc.right = m_rcItem.right - m_rcItem.left;
		rc.bottom = m_rcItem.bottom - m_rcItem.top;
	}

	if( !m_sForeImage.IsEmpty() ) {
		m_sForeImageModify.Empty();
		m_sForeImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), rc.left, rc.top, rc.right, rc.bottom);

		if( !DrawImage(ctx, (LPCTSTR)m_sForeImage, (LPCTSTR)m_sForeImageModify) ) m_sForeImage.Empty();
		else return;
	}
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

CSliderUI::CSliderUI() : m_uButtonState(0), m_nStep(1)
{
    m_uTextStyle = DT_SINGLELINE | DT_CENTER | DT_WORD_ELLIPSIS | DT_END_ELLIPSIS;
    m_szThumb.cx = m_szThumb.cy = 10;
}

LPCTSTR CSliderUI::GetClass() const
{
    return kSliderUIClassName;
}

UINT CSliderUI::GetControlFlags() const
{
    if( IsEnabled() ) return UIFLAG_SETCURSOR;
    else return 0;
}

LPVOID CSliderUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcsicmp(pstrName, kSliderUIInterfaceName) == 0 ) return static_cast<CSliderUI*>(this);
    return CProgressUI::GetInterface(pstrName);
}

void CSliderUI::SetEnabled(bool bEnable)
{
    CControlUI::SetEnabled(bEnable);
    if( !IsEnabled() ) {
        m_uButtonState = 0;
    }
}

int CSliderUI::GetChangeStep() const
{
    return m_nStep;
}

void CSliderUI::SetChangeStep(int step)
{
    m_nStep = step;
}

void CSliderUI::SetThumbSize(SIZE szXY)
{
    m_szThumb = szXY;
}

RECT CSliderUI::GetThumbRect() const
{
    if( m_bHorizontal ) {
        int left = m_rcItem.left + (m_rcItem.right - m_rcItem.left - m_szThumb.cx) * (m_nValue - m_nMin) / (m_nMax - m_nMin);
        int top = (m_rcItem.bottom + m_rcItem.top - m_szThumb.cy) / 2;
        return CRect(left, top, left + m_szThumb.cx, top + m_szThumb.cy); 
    }
    else {
        int left = (m_rcItem.right + m_rcItem.left - m_szThumb.cx) / 2;
        int top = m_rcItem.bottom - m_szThumb.cy - (m_rcItem.bottom - m_rcItem.top - m_szThumb.cy) * (m_nValue - m_nMin) / (m_nMax - m_nMin);
        return CRect(left, top, left + m_szThumb.cx, top + m_szThumb.cy); 
    }
}

LPCTSTR CSliderUI::GetThumbImage() const
{
    return m_sThumbImage;
}

void CSliderUI::SetThumbImage(LPCTSTR pStrImage)
{
    m_sThumbImage = pStrImage;
    Invalidate();
}

LPCTSTR CSliderUI::GetThumbHotImage() const
{
    return m_sThumbHotImage;
}

void CSliderUI::SetThumbHotImage(LPCTSTR pStrImage)
{
    m_sThumbHotImage = pStrImage;
    Invalidate();
}

LPCTSTR CSliderUI::GetThumbPushedImage() const
{
    return m_sThumbPushedImage;
}

void CSliderUI::SetThumbPushedImage(LPCTSTR pStrImage)
{
    m_sThumbPushedImage = pStrImage;
    Invalidate();
}

void CSliderUI::DoEvent(TEventUI& event)
{
#ifdef UI_BUILD_FOR_DESIGNER
	if (IsUsedForPitchUpon() && (m_pManager != NULL) && m_pManager->IsUsedAsUIDesigner() && ((GetKeyState(VK_CONTROL) < 0) || (GetKeyState(VK_SHIFT) < 0)))
	{
		CControlUI::DoEvent(event);
		return;
	}
#endif

    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_pParent != NULL ) m_pParent->DoEvent(event);
        else CProgressUI::DoEvent(event);
        return;
    }

    if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK )
    {
        if( IsEnabled() ) {
            RECT rcThumb = GetThumbRect();
            if( ::PtInRect(&rcThumb, event.ptMouse) ) {
                m_uButtonState |= UISTATE_CAPTURED;
            }
        }
        return;
    }
    if( event.Type == UIEVENT_BUTTONUP )
    {
        if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
            m_uButtonState &= ~UISTATE_CAPTURED;
            m_pManager->SendNotify(this, kValueChanged);
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_CONTEXTMENU )
    {
        return;
    }
    if( event.Type == UIEVENT_SCROLLWHEEL ) 
    {
        switch( LOWORD(event.wParam) ) {
        case SB_LINEUP:
            SetValue(GetValue() + GetChangeStep());
            return;
        case SB_LINEDOWN:
            SetValue(GetValue() - GetChangeStep());
            return;
        }
    }
    if( event.Type == UIEVENT_MOUSEMOVE )
    {
        if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
            if( m_bHorizontal ){
                if( event.ptMouse.x >= m_rcItem.right - m_szThumb.cx / 2 ) {
                    m_nValue = m_nMax;
                }
                else if( event.ptMouse.x <= m_rcItem.left + m_szThumb.cx / 2 ) {
                    m_nValue = m_nMin;
                }
                else {
                    m_nValue = m_nMin + (m_nMax - m_nMin) * (event.ptMouse.x - m_rcItem.left - m_szThumb.cx / 2 ) / (m_rcItem.right - m_rcItem.left - m_szThumb.cx);
                }
            }
            else {
                if( event.ptMouse.y >= m_rcItem.bottom - m_szThumb.cy / 2 ) {
                    m_nValue = m_nMin;
                }
                else if( event.ptMouse.y <= m_rcItem.top + m_szThumb.cy / 2  ) {
                    m_nValue = m_nMax;
                }
                else {
                    m_nValue = m_nMin + (m_nMax - m_nMin) * (m_rcItem.bottom - event.ptMouse.y - m_szThumb.cy / 2 ) / (m_rcItem.bottom - m_rcItem.top - m_szThumb.cy);
                }
            }

            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_SETCURSOR )
    {
        RECT rcThumb = GetThumbRect();
        if( IsEnabled() && ::PtInRect(&rcThumb, event.ptMouse) ) {
            ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND)));
            return;
        }
    }
    if( event.Type == UIEVENT_MOUSEENTER )
    {
        if( IsEnabled() ) {
            m_uButtonState |= UISTATE_HOT;
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_MOUSELEAVE )
    {
        if( IsEnabled() ) {
            m_uButtonState &= ~UISTATE_HOT;
            Invalidate();
        }
        return;
    }
    CControlUI::DoEvent(event);
}


void CSliderUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if( _tcsicmp(pstrName, _T("thumbimage")) == 0 ) SetThumbImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("thumbhotimage")) == 0 ) SetThumbHotImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("thumbpushedimage")) == 0 ) SetThumbPushedImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("thumbsize")) == 0 ) {
        SIZE szXY = {0};
        LPTSTR pstr = NULL;
        szXY.cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
        szXY.cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr); 
        SetThumbSize(szXY);
    }
    else if( _tcsicmp(pstrName, _T("step")) == 0 ) {
        SetChangeStep(_ttoi(pstrValue));
    }
    else CProgressUI::SetAttribute(pstrName, pstrValue);
}

void CSliderUI::PaintStatusImage(void* ctx)
{
	if (ctx == NULL) return;

    CProgressUI::PaintStatusImage(ctx);

    RECT rcThumb = GetThumbRect();
    rcThumb.left -= m_rcItem.left;
    rcThumb.top -= m_rcItem.top;
    rcThumb.right -= m_rcItem.left;
    rcThumb.bottom -= m_rcItem.top;
    if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
        if( !m_sThumbPushedImage.IsEmpty() ) {
            m_sImageModify.Empty();
            m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
            if( !DrawImage(ctx, (LPCTSTR)m_sThumbPushedImage, (LPCTSTR)m_sImageModify) ) m_sThumbPushedImage.Empty();
            else return;
        }
    }
    else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
        if( !m_sThumbHotImage.IsEmpty() ) {
            m_sImageModify.Empty();
            m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
            if( !DrawImage(ctx, (LPCTSTR)m_sThumbHotImage, (LPCTSTR)m_sImageModify) ) m_sThumbHotImage.Empty();
            else return;
        }
    }

    if( !m_sThumbImage.IsEmpty() ) {
        m_sImageModify.Empty();
        m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
        if( !DrawImage(ctx, (LPCTSTR)m_sThumbImage, (LPCTSTR)m_sImageModify) ) m_sThumbImage.Empty();
        else return;
    }
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

CEditWnd::CEditWnd() : m_pOwner(NULL), m_hBkBrush(NULL)
#if defined(UI_BUILD_FOR_WINCE)
, m_hFont(NULL)
#endif
, m_bInit(false)
{
}

CEditWnd::~CEditWnd()
{
#if defined(UI_BUILD_FOR_WINCE)
	if( m_hFont != NULL ) ::DeleteObject(m_hFont);
	m_hFont = NULL;
#endif
}

void CEditWnd::Init(IEditUI* pOwner)
{
	m_pOwner = pOwner;
	RECT rcPos = CalPos();
    UINT uStyle = WS_CHILD | ES_AUTOHSCROLL;
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

#if defined(UI_BUILD_FOR_WINCE)
	::SetCaretBlinkTime((UINT)-1);
#endif

	Create(m_pOwner->GetManager()->GetPaintWindow(), NULL, uStyle, 0, rcPos);
#if defined(UI_BUILD_FOR_WINCE)
	TFontInfo* fontInfo = m_pOwner->GetManager()->GetDefaultFontInfo();
	LOGFONT lf = { 0 };
	_tcscpy(lf.lfFaceName, fontInfo->sFontName);
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfHeight = -fontInfo->iSize;
	if( fontInfo->bBold ) lf.lfWeight += FW_BOLD;
	if( fontInfo->bUnderline ) lf.lfUnderline = TRUE;
	m_hFont = ::CreateFontIndirect(&lf);
	if( fontInfo->bItalic ) lf.lfItalic = TRUE;

	SetWindowFont(m_hWnd, m_hFont, TRUE);
#else
    SetWindowFont(m_hWnd, m_pOwner->GetManager()->GetFontInfo(((CEditUI*)m_pOwner->GetHostedControl())->GetFont())->hFont, TRUE);
#endif
	if (!m_pOwner->IsMultiLine()) Edit_LimitText(m_hWnd, m_pOwner->GetMaxChar());
	if (m_pOwner->IsPasswordMode()) Edit_SetPasswordChar(m_hWnd, m_pOwner->GetPasswordChar());	
	Edit_SetText(m_hWnd, m_pOwner->GetText());
	Edit_SetModify(m_hWnd, FALSE);
	SendMessage(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(0, 0));
	Edit_Enable(m_hWnd, m_pOwner->IsEnabled() == true);
	Edit_SetReadOnly(m_hWnd, m_pOwner->IsReadOnly() == true);
	::ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
	::SetFocus(m_hWnd);
    m_bInit = true;    
}

RECT CEditWnd::CalPos()
{
	CRect rcPos = m_pOwner->GetEditPos();
	RECT rcPadding = m_pOwner->GetEditTextPadding();
	rcPos.left += rcPadding.left;
	rcPos.top += rcPadding.top;
	rcPos.right -= rcPadding.right;
	rcPos.bottom -= rcPadding.bottom;
	if (!m_pOwner->IsMultiLine())
	{
    LONG lEditHeight = m_pOwner->GetManager()->GetFontInfo(((CEditUI*)m_pOwner->GetHostedControl())->GetFont())->tm.tmHeight;
		if( lEditHeight < rcPos.GetHeight() ) {
			rcPos.top += (rcPos.GetHeight() - lEditHeight) / 2;
			rcPos.bottom = rcPos.top + lEditHeight;
		}
	}
    return rcPos;
}

void CEditWnd::MoveEditWnd(RECT rc)
{
	::MoveWindow(m_hWnd,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,FALSE);
}

void CEditWnd::SetEditWndText(LPCTSTR lpsText,bool bSelect)
{
	Edit_SetText(m_hWnd, lpsText);
	Edit_SetModify(m_hWnd, FALSE);

	if(bSelect){
		int cchLen = ::GetWindowTextLength(m_hWnd) + 1;
		Edit_SetSel(m_hWnd,0,cchLen);
	}
}

LPCTSTR CEditWnd::GetWindowClassName() const
{
    return _T("EditWnd");
}

LPCTSTR CEditWnd::GetSuperClassName() const
{
    return WC_EDIT;
}

void CEditWnd::BreakLinkage()
{
	m_pOwner = NULL;
}

DWORD CEditWnd::GetEditTextColor()
{
	LPTSTR pstr = NULL;
	DWORD dwColor = 0xFF000000;
	if (m_pOwner != NULL)
		dwColor = m_pOwner->GetEditTextColor();
	return dwColor;
}

DWORD CEditWnd::GetBkColor()
{
	LPTSTR pstr = NULL;
	DWORD dwColor = 0xFFFFFFFF;
	if (m_pOwner != NULL)
		dwColor = m_pOwner->GetBkColor();
	return dwColor;
}

void CEditWnd::OnFinalMessage(HWND /*hWnd*/)
{
    // Clear reference and die
    if( m_hBkBrush != NULL ) ::DeleteObject(m_hBkBrush);
    if (m_pOwner != NULL) m_pOwner->SetEidtWndNull();
    delete this;
}

#if defined(UI_BUILD_FOR_SKIA)
LRESULT CEditWnd::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}
#endif

#if defined(UI_BUILD_FOR_WINGDI)
LRESULT CEditWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
#elif defined(UI_BUILD_FOR_SKIA)
LRESULT CEditWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
#endif
{
    LRESULT lRes = 0;
#if defined(UI_BUILD_FOR_WINGDI)
	BOOL bHandled = TRUE;
	if( uMsg == WM_KILLFOCUS ) lRes = OnKillFocus(uMsg, wParam, lParam, bHandled);
#endif
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

#if defined(UI_BUILD_FOR_WINGDI)
    if( !bHandled ) return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
#elif defined(UI_BUILD_FOR_SKIA)
#endif
    return lRes;
}

LRESULT CEditWnd::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LRESULT lRes = ::DefWindowProc(m_hWnd, uMsg, wParam, lParam);
    PostMessage(WM_CLOSE);
    return lRes;
}

LRESULT CEditWnd::OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	//switch (wParam) 
	//{ 
	//case 0x08:
	//	// Process a backspace.
	//	break;
	//case 0x0A:
	//	// Process a linefeed.
	//	break;
	//case 0x1B:
	//	// Process an escape.
	//	break;
	//case 0x09:
	//	// Process a tab.
	//	break;
	//case 0x0D:
	//	// Process a carriage return.
	//	break;
	//default:
	//	// Process displayable characters.
	//	{
	//		bHandled = TRUE;
	//		CWindowWnd::HandleMessage(uMsg, wParam, lParam);

	//		if( m_pOwner == NULL ) return 0;
	//		// Copy text back
	//		int cchLen = ::GetWindowTextLength(m_hWnd) + 1;
	//		LPTSTR pstr = static_cast<LPTSTR>(_alloca(cchLen * sizeof(TCHAR)));
	//		ASSERT(pstr);
	//		if( pstr == NULL ) return 0;
	//		::GetWindowText(m_hWnd, pstr, cchLen);	
	//		if (m_pOwner->GetHostedControl() != NULL)
	//		{
	//			m_pOwner->GetHostedControl()->CControlUI::SetText(pstr);
	//			m_pOwner->GetHostedControl()->Invalidate();
	//			m_pOwner->GetManager()->SendNotify(m_pOwner->GetHostedControl(), kTextChanged);
	//		}
	//	}
	//	break;
	//}
	return 0;
}

LRESULT CEditWnd::OnEditChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if( !m_bInit ) return 0;
	if( m_pOwner == NULL ) return 0;
	// Copy text back
	int cchLen = ::GetWindowTextLength(m_hWnd) + 1;
	LPTSTR pstr = static_cast<LPTSTR>(_alloca(cchLen * sizeof(TCHAR)));
	ASSERT(pstr);
	if( pstr == NULL ) return 0;
	::GetWindowText(m_hWnd, pstr, cchLen);	
	if (m_pOwner->GetHostedControl() != NULL)
	{
		m_pOwner->GetHostedControl()->CControlUI::SetText(pstr);
		m_pOwner->GetHostedControl()->Invalidate();
		m_pOwner->GetManager()->SendNotify(m_pOwner->GetHostedControl(), kTextChanged);
	}

	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////
//
//

CEditUI::CEditUI() : m_pWindow(NULL), m_uMaxChar(255), m_bReadOnly(false), 
m_bPasswordMode(false), m_bMultiLine(false), m_cPasswordChar(_T('*')), m_uButtonState(0), m_dwHitTextColor(0)
#if(WINVER >= 0x0400)
, m_bDigitalNumber(false)
#endif
{
	m_dwBorderColor = 0xFF000000;
	m_nBorderSize = 1;

    SetTextPadding(CRect(4, 3, 4, 3));
    SetBkColor(0xFFFFFFFF);
	m_uTextStyle = DT_SINGLELINE | DT_LEFT | DT_VCENTER;
}

CEditUI::~CEditUI()
{
	if (m_pWindow != NULL)
	{
		m_pWindow->BreakLinkage();
		m_pWindow->Close();
		m_pWindow = NULL;
	}
}

HWND CEditUI::GetNativeHWND()
{
	return (m_pWindow != NULL) ? m_pWindow->GetHWND() : NULL;
}

LPCTSTR CEditUI::GetClass() const
{
    return kEditUIClassName;
}

LPVOID CEditUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcsicmp(pstrName, kEditUIInterfaceName) == 0 ) return static_cast<CEditUI*>(this);
    return CLabelUI::GetInterface(pstrName);
}

UINT CEditUI::GetControlFlags() const
{
    if( !IsEnabled() ) return CControlUI::GetControlFlags();

    return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
}

void CEditUI::DoEvent(TEventUI& event)
{
#ifdef UI_BUILD_FOR_DESIGNER
	if (IsUsedForPitchUpon() && (m_pManager != NULL) && m_pManager->IsUsedAsUIDesigner() && ((GetKeyState(VK_CONTROL) < 0) || (GetKeyState(VK_SHIFT) < 0)))
	{
		CControlUI::DoEvent(event);
		return;
	}
#endif

    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_pParent != NULL ) m_pParent->DoEvent(event);
        else CLabelUI::DoEvent(event);
        return;
    }

    if( event.Type == UIEVENT_SETCURSOR && IsEnabled() )
    {
        ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_IBEAM)));
        return;
    }
    if( event.Type == UIEVENT_WINDOWSIZE )
    {
        if( m_pWindow != NULL ) m_pManager->SetFocusNeeded(this);
    }
    if( event.Type == UIEVENT_SCROLLWHEEL )
    {
        if( m_pWindow != NULL ) return;
    }
    if( event.Type == UIEVENT_SETFOCUS && IsEnabled() ) 
    {
        if( m_pWindow ) return;
        m_pWindow = new CEditWnd();
        ASSERT(m_pWindow);
        m_pWindow->Init(this);
        Invalidate();
    }
    if( event.Type == UIEVENT_KILLFOCUS && IsEnabled() ) 
    {
        Invalidate();
    }
    if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK || event.Type == UIEVENT_RBUTTONDOWN) 
    {
        if( IsEnabled() ) {
            GetManager()->ReleaseCapture();
            if( IsFocused() && m_pWindow == NULL )
            {
                m_pWindow = new CEditWnd();
                ASSERT(m_pWindow);
                m_pWindow->Init(this);

				if( PtInRect(&m_rcItem, event.ptMouse) )
				{
					int nSize = GetWindowTextLength(*m_pWindow);
					if( nSize == 0 )
						nSize = 1;

					Edit_SetSel(*m_pWindow, 0, nSize);
				}
            }
            else if( m_pWindow != NULL )
            {
#if 1
				int nSize = GetWindowTextLength(*m_pWindow);
				if( nSize == 0 )
					nSize = 1;

				Edit_SetSel(*m_pWindow, 0, nSize);
#else
                POINT pt = event.ptMouse;
                pt.x -= m_rcItem.left + m_rcTextPadding.left;
                pt.y -= m_rcItem.top + m_rcTextPadding.top;
                ::SendMessage(*m_pWindow, WM_LBUTTONDOWN, event.wParam, MAKELPARAM(pt.x, pt.y));
#endif
            }
        }
        return;
    }
    if( event.Type == UIEVENT_MOUSEMOVE ) 
    {
        return;
    }
    if( event.Type == UIEVENT_BUTTONUP ) 
    {
        return;
    }
    if( event.Type == UIEVENT_CONTEXTMENU )
    {
        return;
    }
    if( event.Type == UIEVENT_MOUSEENTER )
    {
        if( IsEnabled() ) {
            m_uButtonState |= UISTATE_HOT;
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_MOUSELEAVE )
    {
        if( IsEnabled() ) {
            m_uButtonState &= ~UISTATE_HOT;
            Invalidate();
        }
        return;
    }
    CLabelUI::DoEvent(event);
}

void CEditUI::SetEnabled(bool bEnable)
{
    CControlUI::SetEnabled(bEnable);
    if( !IsEnabled() ) {
        m_uButtonState = 0;
    }
}

void CEditUI::SetText(LPCTSTR pstrText)
{
    m_sText = pstrText;
    if( m_pWindow != NULL ) Edit_SetText(*m_pWindow, m_sText);
    Invalidate();
}

void CEditUI::SetMaxChar(UINT uMax)
{
    m_uMaxChar = uMax;
    if( m_pWindow != NULL ) Edit_LimitText(*m_pWindow, m_uMaxChar);
}

UINT CEditUI::GetMaxChar()
{
    return m_uMaxChar;
}

void CEditUI::SetReadOnly(bool bReadOnly)
{
    if( m_bReadOnly == bReadOnly ) return;

    m_bReadOnly = bReadOnly;
    if( m_pWindow != NULL ) Edit_SetReadOnly(*m_pWindow, m_bReadOnly);
    Invalidate();
}

bool CEditUI::IsReadOnly() const
{
    return m_bReadOnly;
}

void CEditUI::SetPasswordMode(bool bPasswordMode)
{
    if( m_bPasswordMode == bPasswordMode ) return;
    m_bPasswordMode = bPasswordMode;
    Invalidate();
}

bool CEditUI::IsPasswordMode() const
{
    return m_bPasswordMode;
}

void CEditUI::SetPasswordChar(TCHAR cPasswordChar)
{
    if( m_cPasswordChar == cPasswordChar ) return;
    m_cPasswordChar = cPasswordChar;
    if( m_pWindow != NULL ) Edit_SetPasswordChar(*m_pWindow, m_cPasswordChar);
    Invalidate();
}

TCHAR CEditUI::GetPasswordChar() const
{
    return m_cPasswordChar;
}

LPCTSTR CEditUI::GetNormalImage()
{
    return m_sNormalImage;
}

void CEditUI::SetMultiLine(bool bMultiLine)
{
	m_bMultiLine = bMultiLine;
	if (bMultiLine)
	{
		m_uTextStyle = DT_WORDBREAK | DT_LEFT | DT_TOP;
	}
	else
	{
		m_uTextStyle = DT_SINGLELINE | DT_LEFT | DT_VCENTER;
	}
}

#if(WINVER >= 0x0400)
bool CEditUI::IsDigitalNumber() const
{
	return m_bDigitalNumber;
}

void CEditUI::SetDigitalNumber(bool bDigitalNumber)
{
	m_bDigitalNumber = bDigitalNumber;
}
#endif

void CEditUI::SetNormalImage(LPCTSTR pStrImage)
{
    m_sNormalImage = pStrImage;
    Invalidate();
}

LPCTSTR CEditUI::GetHotImage()
{
    return m_sHotImage;
}

void CEditUI::SetHotImage(LPCTSTR pStrImage)
{
    m_sHotImage = pStrImage;
    Invalidate();
}

LPCTSTR CEditUI::GetFocusedImage()
{
    return m_sFocusedImage;
}

void CEditUI::SetFocusedImage(LPCTSTR pStrImage)
{
    m_sFocusedImage = pStrImage;
    Invalidate();
}

LPCTSTR CEditUI::GetDisabledImage()
{
    return m_sDisabledImage;
}

void CEditUI::SetDisabledImage(LPCTSTR pStrImage)
{
    m_sDisabledImage = pStrImage;
    Invalidate();
}

LPCTSTR CEditUI::GetHitText()
{
	return m_sHitText;
}

void CEditUI::SetHitText(LPCTSTR pStrText)
{
	m_sHitText = pStrText;
	Invalidate();
}

DWORD CEditUI::GetHitTextColor()
{
	return m_dwHitTextColor;
}

void CEditUI::SetHitTextColor(DWORD dwColor)
{
	m_dwHitTextColor = dwColor;
}

DWORD CEditUI::GetHitBkColor()
{
	return m_dwHitBkColor;
}

void CEditUI::SetHitBkColor(DWORD dwColor)
{
	m_dwHitBkColor = dwColor;
}

void CEditUI::SetPos(RECT rc)
{
    CControlUI::SetPos(rc);
    if( m_pWindow != NULL ) {
        RECT rcPos = m_pWindow->CalPos();
        ::SetWindowPos(m_pWindow->GetHWND(), NULL, rcPos.left, rcPos.top, rcPos.right - rcPos.left, 
            rcPos.bottom - rcPos.top, SWP_NOZORDER | SWP_NOACTIVATE);        
    }
}

void CEditUI::SetVisible(bool bVisible)
{
    CControlUI::SetVisible(bVisible);
    if( !IsVisible() && m_pWindow != NULL ) m_pManager->SetFocus(NULL);
}

void CEditUI::SetInternVisible(bool bVisible)
{
    if( !IsVisible() && m_pWindow != NULL ) m_pManager->SetFocus(NULL);
}

SIZE CEditUI::EstimateSize(SIZE szAvailable)
{
    if( m_cxyFixed.cy == 0 ) return CSize(m_cxyFixed.cx, m_pManager->GetFontInfo(GetFont())->tm.tmHeight + 6);
    return CControlUI::EstimateSize(szAvailable);
}

void CEditUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if( _tcsicmp(pstrName, _T("readonly")) == 0 ) SetReadOnly(_tcsicmp(pstrValue, _T("true")) == 0);
    else if( _tcsicmp(pstrName, _T("password")) == 0 ) SetPasswordMode(_tcsicmp(pstrValue, _T("true")) == 0);
    else if( _tcsicmp(pstrName, _T("normalimage")) == 0 ) SetNormalImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("hotimage")) == 0 ) SetHotImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("focusedimage")) == 0 ) SetFocusedImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("disabledimage")) == 0 ) SetDisabledImage(pstrValue);
	else if( _tcsicmp(pstrName, _T("multiline")) == 0 ) SetMultiLine(_tcsicmp(pstrValue, _T("true")) == 0);
#if(WINVER >= 0x0400)
	else if( _tcsicmp(pstrName, _T("digitalnumber")) == 0 ) SetDigitalNumber(_tcsicmp(pstrValue, _T("true")) == 0);
#endif
	else if( _tcsicmp(pstrName, _T("maxchar")) == 0 )
	{
        LPTSTR pstr = NULL;
        UINT uMax = _tcstol(pstrValue, &pstr, 10); 
		ASSERT(pstr);
		SetMaxChar(uMax);
	}
	else if( _tcscmp(pstrName, _T("hittext")) == 0 ) SetHitText(pstrValue);
	else if( _tcscmp(pstrName, _T("hitttextcolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetHitTextColor(clrColor);
	}
	else if( _tcscmp(pstrName, _T("hittbkcolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetHitBkColor(clrColor);
	}
    else CLabelUI::SetAttribute(pstrName, pstrValue);
}

void CEditUI::PaintStatusImage(void* ctx)
{
	if (ctx == NULL) return;

    if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
    else m_uButtonState &= ~ UISTATE_FOCUSED;
    if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
    else m_uButtonState &= ~ UISTATE_DISABLED;

    if( (m_uButtonState & UISTATE_DISABLED) != 0 ) {
        if( !m_sDisabledImage.IsEmpty() ) {
            if( !DrawImage(ctx, (LPCTSTR)m_sDisabledImage) ) m_sDisabledImage.Empty();
            else return;
        }
    }
    else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) {
        if( !m_sFocusedImage.IsEmpty() ) {
            if( !DrawImage(ctx, (LPCTSTR)m_sFocusedImage) ) m_sFocusedImage.Empty();
            else return;
        }
    }
    else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
        if( !m_sHotImage.IsEmpty() ) {
            if( !DrawImage(ctx, (LPCTSTR)m_sHotImage) ) m_sHotImage.Empty();
            else return;
        }
    }

    if( !m_sNormalImage.IsEmpty() ) {
        if( !DrawImage(ctx, (LPCTSTR)m_sNormalImage) ) m_sNormalImage.Empty();
        else return;
    }

    //DWORD dwBorderColor = 0xFF4EA0D1;
    //int nBorderSize = 1;
    //if( (m_uButtonState & UISTATE_HOT) != 0 || (m_uButtonState & UISTATE_FOCUSED) != 0) {
    //    dwBorderColor = 0xFF85E4FF;
    //    nBorderSize = 2;
    //}
    //CRenderEngine::DrawRect(ctx, m_rcItem, nBorderSize, dwBorderColor);
}

void CEditUI::PaintBkColor(HDC hDC)
{
	if (m_sText.IsEmpty() && !m_sHitText.IsEmpty() && m_pWindow == NULL)
	{
		CRenderEngine::DrawColor(hDC, m_rcPaint, GetAdjustColor(m_dwHitBkColor));
		return;
	}
	return CLabelUI::PaintBkColor(hDC);
}

void CEditUI::PaintText(void* ctx)
{
	if (ctx == NULL) return;

    if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
    if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();
	if( m_dwHitTextColor == 0 ) m_dwHitTextColor = m_dwTextColor;

	CStdString sText;
	if( m_sText.IsEmpty() ) {
		if( m_sHitText.IsEmpty() ) return;

		sText = m_sHitText;
	} else {
		sText = m_sText;
		if( m_bPasswordMode ) {
			sText.Empty();
			LPCTSTR p = m_sText.GetData();
			while( *p != _T('\0') ) {
				sText += m_cPasswordChar;
				p = ::CharNext(p);
			}
		}
	}

    RECT rc = m_rcItem;
    rc.left += m_rcTextPadding.left;
    rc.right -= m_rcTextPadding.right;
    rc.top += m_rcTextPadding.top;
    rc.bottom -= m_rcTextPadding.bottom;

	if (m_sText.IsEmpty())
		CRenderEngine::DrawText(ctx, m_pManager, rc, sText, m_dwHitTextColor, \
			m_iFont, m_uTextStyle);
	else
		CRenderEngine::DrawText(ctx, m_pManager, rc, sText, IsEnabled()?m_dwTextColor:m_dwDisabledTextColor, \
			m_iFont, m_uTextStyle);
}

CControlUI* const CEditUI::GetHostedControl()
{
	return this;
}

LPCTSTR CEditUI::GetEditClass() const
{
	return GetClass();
}

CRect CEditUI::GetEditPos()
{
	return GetPos();
}

RECT CEditUI::GetEditTextPadding() const
{
	return GetTextPadding();
}

DWORD CEditUI::GetEditTextColor()
{
	return CLabelUI::GetTextColor();
}

CPaintManagerUI* CEditUI::GetManager() const
{
	return CLabelUI::GetManager();
}

CStdString CEditUI::GetText() const
{
	return CLabelUI::GetText();
}

bool CEditUI::IsEnabled() const
{
	return CLabelUI::IsEnabled();
}

bool CEditUI::IsMultiLine() const
{
	return m_bMultiLine;
}

CStdString CEditUI::GetName() const
{
	return CLabelUI::GetName();
}

void CEditUI::SetEidtWndNull()
{
	m_pWindow = NULL;
}

DWORD CEditUI::GetBkColor() const
{
	DWORD dwColor = CLabelUI::GetBkColor();
	if ((dwColor == 0) && (GetParent() != NULL))
	{
		dwColor = GetParent()->GetBkColor();
	}
	return dwColor;
}

/////////////////////////////////////////////////////////////////////////////////////
//
//

CScrollBarUI::CScrollBarUI(bool bHorizontal) : m_bHorizontal(false), m_nRange(100), m_nScrollPos(0), m_nLineSize(8), 
m_pOwner(NULL), m_nLastScrollPos(0), m_nLastScrollOffset(0), m_nScrollRepeatDelay(0), m_uButton1State(0), \
m_uButton2State(0), m_uThumbState(0), m_bShowButton1(true), m_bShowButton2(true)
{
    m_cxyFixed.cx = DEFAULT_SCROLLBAR_SIZE;
    ptLastMouse.x = ptLastMouse.y = 0;
    ::ZeroMemory(&m_rcThumb, sizeof(m_rcThumb));
    ::ZeroMemory(&m_rcButton1, sizeof(m_rcButton1));
    ::ZeroMemory(&m_rcButton2, sizeof(m_rcButton2));

	if (bHorizontal)
		SetHorizontal();
}

LPCTSTR CScrollBarUI::GetClass() const
{
    return kScrollBarUIClassName;
}

LPVOID CScrollBarUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcsicmp(pstrName, kScrollBarUIInterfaceName) == 0 ) return static_cast<CScrollBarUI*>(this);
    return CControlUI::GetInterface(pstrName);
}

CContainerUI* CScrollBarUI::GetOwner() const
{
    return m_pOwner;
}

void CScrollBarUI::SetOwner(CContainerUI* pOwner)
{
    m_pOwner = pOwner;
}

void CScrollBarUI::SetVisible(bool bVisible)
{
    if( m_bVisible == bVisible ) return;

    bool v = IsVisible();
    m_bVisible = bVisible;
    if( m_bFocused ) m_bFocused = false;
}

void CScrollBarUI::SetEnabled(bool bEnable)
{
    CControlUI::SetEnabled(bEnable);
    if( !IsEnabled() ) {
        m_uButton1State = 0;
        m_uButton2State = 0;
        m_uThumbState = 0;
    }
}

void CScrollBarUI::SetFocus()
{
    if( m_pOwner != NULL ) m_pOwner->SetFocus();
    else CControlUI::SetFocus();
}

bool CScrollBarUI::IsHorizontal()
{
    return m_bHorizontal;
}

void CScrollBarUI::SetHorizontal(bool bHorizontal)
{
    if( m_bHorizontal == bHorizontal ) return;

    m_bHorizontal = bHorizontal;
    if( m_bHorizontal ) {
        if( m_cxyFixed.cy == 0 ) {
            m_cxyFixed.cx = 0;
            m_cxyFixed.cy = DEFAULT_SCROLLBAR_SIZE;
        }
    }
    else {
        if( m_cxyFixed.cx == 0 ) {
            m_cxyFixed.cx = DEFAULT_SCROLLBAR_SIZE;
            m_cxyFixed.cy = 0;
        }
    }

    if( m_pOwner != NULL ) m_pOwner->NeedUpdate(); else NeedParentUpdate();
}

int CScrollBarUI::GetScrollRange() const
{
    return m_nRange;
}

void CScrollBarUI::SetScrollRange(int nRange)
{
    if( m_nRange == nRange ) return;
    
    m_nRange = nRange;
    if( m_nRange < 0 ) m_nRange = 0;
    if( m_nScrollPos > m_nRange ) m_nScrollPos = m_nRange;
    SetPos(m_rcItem);
}

int CScrollBarUI::GetScrollPos() const
{
    return m_nScrollPos;
}

void CScrollBarUI::SetScrollPos(int nPos)
{
    if( m_nScrollPos == nPos ) return;

    m_nScrollPos = nPos;
    if( m_nScrollPos < 0 ) m_nScrollPos = 0;
    if( m_nScrollPos > m_nRange ) m_nScrollPos = m_nRange;
    SetPos(m_rcItem);
}

int CScrollBarUI::GetLineSize() const
{
    return m_nLineSize;
}

void CScrollBarUI::SetLineSize(int nSize)
{
    m_nLineSize = nSize;
}

bool CScrollBarUI::GetShowButton1()
{
    return m_bShowButton1;
}

void CScrollBarUI::SetShowButton1(bool bShow)
{
    m_bShowButton1 = bShow;
    SetPos(m_rcItem);
}

LPCTSTR CScrollBarUI::GetButton1NormalImage()
{
    return m_sButton1NormalImage;
}

void CScrollBarUI::SetButton1NormalImage(LPCTSTR pStrImage)
{
    m_sButton1NormalImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetButton1HotImage()
{
    return m_sButton1HotImage;
}

void CScrollBarUI::SetButton1HotImage(LPCTSTR pStrImage)
{
    m_sButton1HotImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetButton1PushedImage()
{
    return m_sButton1PushedImage;
}

void CScrollBarUI::SetButton1PushedImage(LPCTSTR pStrImage)
{
    m_sButton1PushedImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetButton1DisabledImage()
{
    return m_sButton1DisabledImage;
}

void CScrollBarUI::SetButton1DisabledImage(LPCTSTR pStrImage)
{
    m_sButton1DisabledImage = pStrImage;
    Invalidate();
}

bool CScrollBarUI::GetShowButton2()
{
    return m_bShowButton2;
}

void CScrollBarUI::SetShowButton2(bool bShow)
{
    m_bShowButton2 = bShow;
    SetPos(m_rcItem);
}

LPCTSTR CScrollBarUI::GetButton2NormalImage()
{
    return m_sButton2NormalImage;
}

void CScrollBarUI::SetButton2NormalImage(LPCTSTR pStrImage)
{
    m_sButton2NormalImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetButton2HotImage()
{
    return m_sButton2HotImage;
}

void CScrollBarUI::SetButton2HotImage(LPCTSTR pStrImage)
{
    m_sButton2HotImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetButton2PushedImage()
{
    return m_sButton2PushedImage;
}

void CScrollBarUI::SetButton2PushedImage(LPCTSTR pStrImage)
{
    m_sButton2PushedImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetButton2DisabledImage()
{
    return m_sButton2DisabledImage;
}

void CScrollBarUI::SetButton2DisabledImage(LPCTSTR pStrImage)
{
    m_sButton2DisabledImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetThumbNormalImage()
{
    return m_sThumbNormalImage;
}

void CScrollBarUI::SetThumbNormalImage(LPCTSTR pStrImage)
{
    m_sThumbNormalImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetThumbHotImage()
{
    return m_sThumbHotImage;
}

void CScrollBarUI::SetThumbHotImage(LPCTSTR pStrImage)
{
    m_sThumbHotImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetThumbPushedImage()
{
    return m_sThumbPushedImage;
}

void CScrollBarUI::SetThumbPushedImage(LPCTSTR pStrImage)
{
    m_sThumbPushedImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetThumbDisabledImage()
{
    return m_sThumbDisabledImage;
}

void CScrollBarUI::SetThumbDisabledImage(LPCTSTR pStrImage)
{
    m_sThumbDisabledImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetRailNormalImage()
{
    return m_sRailNormalImage;
}

void CScrollBarUI::SetRailNormalImage(LPCTSTR pStrImage)
{
    m_sRailNormalImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetRailHotImage()
{
    return m_sRailHotImage;
}

void CScrollBarUI::SetRailHotImage(LPCTSTR pStrImage)
{
    m_sRailHotImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetRailPushedImage()
{
    return m_sRailPushedImage;
}

void CScrollBarUI::SetRailPushedImage(LPCTSTR pStrImage)
{
    m_sRailPushedImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetRailDisabledImage()
{
    return m_sRailDisabledImage;
}

void CScrollBarUI::SetRailDisabledImage(LPCTSTR pStrImage)
{
    m_sRailDisabledImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetBkNormalImage()
{
    return m_sBkNormalImage;
}

void CScrollBarUI::SetBkNormalImage(LPCTSTR pStrImage)
{
    m_sBkNormalImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetBkHotImage()
{
    return m_sBkHotImage;
}

void CScrollBarUI::SetBkHotImage(LPCTSTR pStrImage)
{
    m_sBkHotImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetBkPushedImage()
{
    return m_sBkPushedImage;
}

void CScrollBarUI::SetBkPushedImage(LPCTSTR pStrImage)
{
    m_sBkPushedImage = pStrImage;
    Invalidate();
}

LPCTSTR CScrollBarUI::GetBkDisabledImage()
{
    return m_sBkDisabledImage;
}

void CScrollBarUI::SetBkDisabledImage(LPCTSTR pStrImage)
{
    m_sBkDisabledImage = pStrImage;
    Invalidate();
}

void CScrollBarUI::SetPos(RECT rc)
{
    CControlUI::SetPos(rc);
    rc = m_rcItem;

    if( m_bHorizontal ) {
        int cx = rc.right - rc.left;
        if( m_bShowButton1 ) cx -= m_cxyFixed.cy;
        if( m_bShowButton2 ) cx -= m_cxyFixed.cy;
        if( cx > m_cxyFixed.cy ) {
            m_rcButton1.left = rc.left;
            m_rcButton1.top = rc.top;
            if( m_bShowButton1 ) {
                m_rcButton1.right = rc.left + m_cxyFixed.cy;
                m_rcButton1.bottom = rc.top + m_cxyFixed.cy;
            }
            else {
                m_rcButton1.right = m_rcButton1.left;
                m_rcButton1.bottom = m_rcButton1.top;
            }

            m_rcButton2.top = rc.top;
            m_rcButton2.right = rc.right;
            if( m_bShowButton2 ) {
                m_rcButton2.left = rc.right - m_cxyFixed.cy;
                m_rcButton2.bottom = rc.top + m_cxyFixed.cy;
            }
            else {
                m_rcButton2.left = m_rcButton2.right;
                m_rcButton2.bottom = m_rcButton2.top;
            }

            m_rcThumb.top = rc.top;
            m_rcThumb.bottom = rc.top + m_cxyFixed.cy;
            if( m_nRange > 0 ) {
                int cxThumb = cx * (rc.right - rc.left) / (m_nRange + rc.right - rc.left);
                if( cxThumb < m_cxyFixed.cy ) cxThumb = m_cxyFixed.cy;

                m_rcThumb.left = m_nScrollPos * (cx - cxThumb) / m_nRange + m_rcButton1.right;
                m_rcThumb.right = m_rcThumb.left + cxThumb;
                if( m_rcThumb.right > m_rcButton2.left ) {
                    m_rcThumb.left = m_rcButton2.left - cxThumb;
                    m_rcThumb.right = m_rcButton2.left;
                }
            }
            else {
                m_rcThumb.left = m_rcButton1.right;
                m_rcThumb.right = m_rcButton2.left;
            }
        }
        else {
            int cxButton = (rc.right - rc.left) / 2;
            if( cxButton > m_cxyFixed.cy ) cxButton = m_cxyFixed.cy;
            m_rcButton1.left = rc.left;
            m_rcButton1.top = rc.top;
            if( m_bShowButton1 ) {
                m_rcButton1.right = rc.left + cxButton;
                m_rcButton1.bottom = rc.top + m_cxyFixed.cy;
            }
            else {
                m_rcButton1.right = m_rcButton1.left;
                m_rcButton1.bottom = m_rcButton1.top;
            }

            m_rcButton2.top = rc.top;
            m_rcButton2.right = rc.right;
            if( m_bShowButton2 ) {
                m_rcButton2.left = rc.right - cxButton;
                m_rcButton2.bottom = rc.top + m_cxyFixed.cy;
            }
            else {
                m_rcButton2.left = m_rcButton2.right;
                m_rcButton2.bottom = m_rcButton2.top;
            }

            ::ZeroMemory(&m_rcThumb, sizeof(m_rcThumb));
        }
    }
    else {
        int cy = rc.bottom - rc.top;
        if( m_bShowButton1 ) cy -= m_cxyFixed.cx;
        if( m_bShowButton2 ) cy -= m_cxyFixed.cx;
        if( cy > m_cxyFixed.cx ) {
            m_rcButton1.left = rc.left;
            m_rcButton1.top = rc.top;
            if( m_bShowButton1 ) {
                m_rcButton1.right = rc.left + m_cxyFixed.cx;
                m_rcButton1.bottom = rc.top + m_cxyFixed.cx;
            }
            else {
                m_rcButton1.right = m_rcButton1.left;
                m_rcButton1.bottom = m_rcButton1.top;
            }

            m_rcButton2.left = rc.left;
            m_rcButton2.bottom = rc.bottom;
            if( m_bShowButton2 ) {
                m_rcButton2.top = rc.bottom - m_cxyFixed.cx;
                m_rcButton2.right = rc.left + m_cxyFixed.cx;
            }
            else {
                m_rcButton2.top = m_rcButton2.bottom;
                m_rcButton2.right = m_rcButton2.left;
            }

            m_rcThumb.left = rc.left;
            m_rcThumb.right = rc.left + m_cxyFixed.cx;
            if( m_nRange > 0 ) {
                int cyThumb = cy * (rc.bottom - rc.top) / (m_nRange + rc.bottom - rc.top);
                if( cyThumb < m_cxyFixed.cx ) cyThumb = m_cxyFixed.cx;

                m_rcThumb.top = m_nScrollPos * (cy - cyThumb) / m_nRange + m_rcButton1.bottom;
                m_rcThumb.bottom = m_rcThumb.top + cyThumb;
                if( m_rcThumb.bottom > m_rcButton2.top ) {
                    m_rcThumb.top = m_rcButton2.top - cyThumb;
                    m_rcThumb.bottom = m_rcButton2.top;
                }
            }
            else {
                m_rcThumb.top = m_rcButton1.bottom;
                m_rcThumb.bottom = m_rcButton2.top;
            }
        }
        else {
            int cyButton = (rc.bottom - rc.top) / 2;
            if( cyButton > m_cxyFixed.cx ) cyButton = m_cxyFixed.cx;
            m_rcButton1.left = rc.left;
            m_rcButton1.top = rc.top;
            if( m_bShowButton1 ) {
                m_rcButton1.right = rc.left + m_cxyFixed.cx;
                m_rcButton1.bottom = rc.top + cyButton;
            }
            else {
                m_rcButton1.right = m_rcButton1.left;
                m_rcButton1.bottom = m_rcButton1.top;
            }

            m_rcButton2.left = rc.left;
            m_rcButton2.bottom = rc.bottom;
            if( m_bShowButton2 ) {
                m_rcButton2.top = rc.bottom - cyButton;
                m_rcButton2.right = rc.left + m_cxyFixed.cx;
            }
            else {
                m_rcButton2.top = m_rcButton2.bottom;
                m_rcButton2.right = m_rcButton2.left;
            }

            ::ZeroMemory(&m_rcThumb, sizeof(m_rcThumb));
        }
    }
}

void CScrollBarUI::DoEvent(TEventUI& event)
{
#ifdef UI_BUILD_FOR_DESIGNER
	if (IsUsedForPitchUpon() && (m_pManager != NULL) && m_pManager->IsUsedAsUIDesigner() && ((GetKeyState(VK_CONTROL) < 0) || (GetKeyState(VK_SHIFT) < 0)))
	{
		CControlUI::DoEvent(event);
		return;
	}
#endif

    if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
        if( m_pOwner != NULL ) m_pOwner->DoEvent(event);
        else CControlUI::DoEvent(event);
        return;
    }

    if( event.Type == UIEVENT_SETFOCUS ) 
    {
        return;
    }
    if( event.Type == UIEVENT_KILLFOCUS ) 
    {
        return;
    }
    if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK )
    {
        if( !IsEnabled() ) return;

        m_nLastScrollOffset = 0;
        m_nScrollRepeatDelay = 0;
        m_pManager->SetTimer(this, DEFAULT_TIMERID, 50U);

        if( ::PtInRect(&m_rcButton1, event.ptMouse) ) {
            m_uButton1State |= UISTATE_PUSHED;
            if( !m_bHorizontal ) {
                if( m_pOwner != NULL ) m_pOwner->LineUp(); 
                else SetScrollPos(m_nScrollPos - m_nLineSize);
            }
            else {
                if( m_pOwner != NULL ) m_pOwner->LineLeft(); 
                else SetScrollPos(m_nScrollPos - m_nLineSize);
            }
        }
        else if( ::PtInRect(&m_rcButton2, event.ptMouse) ) {
            m_uButton2State |= UISTATE_PUSHED;
            if( !m_bHorizontal ) {
                if( m_pOwner != NULL ) m_pOwner->LineDown(); 
                else SetScrollPos(m_nScrollPos + m_nLineSize);
            }
            else {
                if( m_pOwner != NULL ) m_pOwner->LineRight(); 
                else SetScrollPos(m_nScrollPos + m_nLineSize);
            }
        }
        else if( ::PtInRect(&m_rcThumb, event.ptMouse) ) {
            m_uThumbState |= UISTATE_CAPTURED | UISTATE_PUSHED;
            ptLastMouse = event.ptMouse;
            m_nLastScrollPos = m_nScrollPos;
        }
        else {
            if( !m_bHorizontal ) {
                if( event.ptMouse.y < m_rcThumb.top ) {
                    if( m_pOwner != NULL ) m_pOwner->PageUp(); 
                    else SetScrollPos(m_nScrollPos + m_rcItem.top - m_rcItem.bottom);
                }
                else if ( event.ptMouse.y > m_rcThumb.bottom ){
                    if( m_pOwner != NULL ) m_pOwner->PageDown(); 
                    else SetScrollPos(m_nScrollPos - m_rcItem.top + m_rcItem.bottom);                    
                }
            }
            else {
                if( event.ptMouse.x < m_rcThumb.left ) {
                    if( m_pOwner != NULL ) m_pOwner->PageLeft(); 
                    else SetScrollPos(m_nScrollPos + m_rcItem.left - m_rcItem.right);
                }
                else if ( event.ptMouse.x > m_rcThumb.right ){
                    if( m_pOwner != NULL ) m_pOwner->PageRight(); 
                    else SetScrollPos(m_nScrollPos - m_rcItem.left + m_rcItem.right);                    
                }
            }
        }
        if( m_pManager != NULL && m_pOwner == NULL ) m_pManager->SendNotify(this, _T("scroll"));
        return;
    }
    if( event.Type == UIEVENT_BUTTONUP )
    {
        m_nScrollRepeatDelay = 0;
        m_nLastScrollOffset = 0;
        m_pManager->KillTimer(this, DEFAULT_TIMERID);

        if( (m_uThumbState & UISTATE_CAPTURED) != 0 ) {
            m_uThumbState &= ~( UISTATE_CAPTURED | UISTATE_PUSHED );
            Invalidate();
        }
        else if( (m_uButton1State & UISTATE_PUSHED) != 0 ) {
            m_uButton1State &= ~UISTATE_PUSHED;
            Invalidate();
        }
        else if( (m_uButton2State & UISTATE_PUSHED) != 0 ) {
            m_uButton2State &= ~UISTATE_PUSHED;
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_MOUSEMOVE )
    {
        if( (m_uThumbState & UISTATE_CAPTURED) != 0 ) {
            if( !m_bHorizontal ) {
                 m_nLastScrollOffset = (event.ptMouse.y - ptLastMouse.y) * m_nRange / \
                    (m_rcItem.bottom - m_rcItem.top - m_rcThumb.bottom + m_rcThumb.top - 2 * m_cxyFixed.cx);
            }
            else {
                m_nLastScrollOffset = (event.ptMouse.x - ptLastMouse.x) * m_nRange / \
                    (m_rcItem.right - m_rcItem.left - m_rcThumb.right + m_rcThumb.left - 2 * m_cxyFixed.cy);
            }
        }
        else {
            if( (m_uThumbState & UISTATE_HOT) != 0 ) {
                if( !::PtInRect(&m_rcThumb, event.ptMouse) ) {
                    m_uThumbState &= ~UISTATE_HOT;
                    Invalidate();
                }
            }
            else {
                if( !IsEnabled() ) return;
                if( ::PtInRect(&m_rcThumb, event.ptMouse) ) {
                    m_uThumbState |= UISTATE_HOT;
                    Invalidate();
                }
            }
        }
        return;
    }
    if( event.Type == UIEVENT_CONTEXTMENU )
    {
        return;
    }
    if( event.Type == UIEVENT_TIMER && event.wParam == DEFAULT_TIMERID )
    {
        ++m_nScrollRepeatDelay;
        if( (m_uThumbState & UISTATE_CAPTURED) != 0 ) {
            if( !m_bHorizontal ) {
                if( m_pOwner != NULL ) m_pOwner->SetScrollPos(CSize(m_pOwner->GetScrollPos().cx, \
                    m_nLastScrollPos + m_nLastScrollOffset)); 
                else SetScrollPos(m_nLastScrollPos + m_nLastScrollOffset);
            }
            else {
                if( m_pOwner != NULL ) m_pOwner->SetScrollPos(CSize(m_nLastScrollPos + m_nLastScrollOffset, \
                    m_pOwner->GetScrollPos().cy)); 
                else SetScrollPos(m_nLastScrollPos + m_nLastScrollOffset);
            }
            Invalidate();
        }
        else if( (m_uButton1State & UISTATE_PUSHED) != 0 ) {
            if( m_nScrollRepeatDelay <= 5 ) return;
            if( !m_bHorizontal ) {
                if( m_pOwner != NULL ) m_pOwner->LineUp(); 
                else SetScrollPos(m_nScrollPos - m_nLineSize);
            }
            else {
                if( m_pOwner != NULL ) m_pOwner->LineLeft(); 
                else SetScrollPos(m_nScrollPos - m_nLineSize);
            }
        }
        else if( (m_uButton2State & UISTATE_PUSHED) != 0 ) {
            if( m_nScrollRepeatDelay <= 5 ) return;
            if( !m_bHorizontal ) {
                if( m_pOwner != NULL ) m_pOwner->LineDown(); 
                else SetScrollPos(m_nScrollPos + m_nLineSize);
            }
            else {
                if( m_pOwner != NULL ) m_pOwner->LineRight(); 
                else SetScrollPos(m_nScrollPos + m_nLineSize);
            }
        }
        else {
            if( m_nScrollRepeatDelay <= 5 ) return;
            POINT pt = { 0 };
            ::GetCursorPos(&pt);
            ::ScreenToClient(m_pManager->GetPaintWindow(), &pt);
            if( !m_bHorizontal ) {
                if( pt.y < m_rcThumb.top ) {
                    if( m_pOwner != NULL ) m_pOwner->PageUp(); 
                    else SetScrollPos(m_nScrollPos + m_rcItem.top - m_rcItem.bottom);
                }
                else if ( pt.y > m_rcThumb.bottom ){
                    if( m_pOwner != NULL ) m_pOwner->PageDown(); 
                    else SetScrollPos(m_nScrollPos - m_rcItem.top + m_rcItem.bottom);                    
                }
            }
            else {
                if( pt.x < m_rcThumb.left ) {
                    if( m_pOwner != NULL ) m_pOwner->PageLeft(); 
                    else SetScrollPos(m_nScrollPos + m_rcItem.left - m_rcItem.right);
                }
                else if ( pt.x > m_rcThumb.right ){
                    if( m_pOwner != NULL ) m_pOwner->PageRight(); 
                    else SetScrollPos(m_nScrollPos - m_rcItem.left + m_rcItem.right);                    
                }
            }
        }
        if( m_pManager != NULL && m_pOwner == NULL ) m_pManager->SendNotify(this, _T("scroll"));
        return;
    }
    if( event.Type == UIEVENT_MOUSEENTER )
    {
        if( IsEnabled() ) {
            m_uButton1State |= UISTATE_HOT;
            m_uButton2State |= UISTATE_HOT;
            if( ::PtInRect(&m_rcThumb, event.ptMouse) ) m_uThumbState |= UISTATE_HOT;
            Invalidate();
        }
        return;
    }
    if( event.Type == UIEVENT_MOUSELEAVE )
    {
        if( IsEnabled() ) {
            m_uButton1State &= ~UISTATE_HOT;
            m_uButton2State &= ~UISTATE_HOT;
            m_uThumbState &= ~UISTATE_HOT;
            Invalidate();
        }
        return;
    }

    if( m_pOwner != NULL ) m_pOwner->DoEvent(event); else CControlUI::DoEvent(event);
}

void CScrollBarUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if( _tcsicmp(pstrName, _T("button1normalimage")) == 0 ) SetButton1NormalImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("button1hotimage")) == 0 ) SetButton1HotImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("button1pushedimage")) == 0 ) SetButton1PushedImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("button1disabledimage")) == 0 ) SetButton1DisabledImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("button2normalimage")) == 0 ) SetButton2NormalImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("button2hotimage")) == 0 ) SetButton2HotImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("button2pushedimage")) == 0 ) SetButton2PushedImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("button2disabledimage")) == 0 ) SetButton2DisabledImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("thumbnormalimage")) == 0 ) SetThumbNormalImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("thumbhotimage")) == 0 ) SetThumbHotImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("thumbpushedimage")) == 0 ) SetThumbPushedImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("thumbdisabledimage")) == 0 ) SetThumbDisabledImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("railnormalimage")) == 0 ) SetRailNormalImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("railhotimage")) == 0 ) SetRailHotImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("railpushedimage")) == 0 ) SetRailPushedImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("raildisabledimage")) == 0 ) SetRailDisabledImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("bknormalimage")) == 0 ) SetBkNormalImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("bkhotimage")) == 0 ) SetBkHotImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("bkpushedimage")) == 0 ) SetBkPushedImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("bkdisabledimage")) == 0 ) SetBkDisabledImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("hor")) == 0 ) SetHorizontal(_tcsicmp(pstrValue, _T("true")) == 0);
    else if( _tcsicmp(pstrName, _T("linesize")) == 0 ) SetLineSize(_ttoi(pstrValue));
    else if( _tcsicmp(pstrName, _T("range")) == 0 ) SetScrollRange(_ttoi(pstrValue));
    else if( _tcsicmp(pstrName, _T("value")) == 0 ) SetScrollPos(_ttoi(pstrValue));
    else if( _tcsicmp(pstrName, _T("showbutton1")) == 0 ) SetShowButton1(_tcsicmp(pstrValue, _T("true")) == 0);
    else if( _tcsicmp(pstrName, _T("showbutton2")) == 0 ) SetShowButton2(_tcsicmp(pstrValue, _T("true")) == 0);
    else CControlUI::SetAttribute(pstrName, pstrValue);
}

void CScrollBarUI::DoPaint(void* ctx, const RECT& rcPaint)
{
	if (ctx == NULL) return;

    if( !::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem) ) return;
    PaintBk(ctx);
    PaintButton1(ctx);
    PaintButton2(ctx);
    PaintThumb(ctx);
    PaintRail(ctx);

#if defined(UI_BUILD_FOR_DESIGNER) && defined(UI_BUILD_FOR_WINGDI)
	HDC hDC = (HDC)ctx;
	if (IsUsedForPitchUpon() && (m_pManager != NULL) && m_pManager->IsUsedAsUIDesigner() && m_bPitchUpon)
	{
		int nBorderSize = 1;
		int nMargin = 7;
		DWORD dwFillColor = 0xFF00FF00;
		DWORD dwBorderColor = 0xFF000000;
		CRect rcLeftTop, rcRightTop, rcLeftBottom, rcRightBottom;
		CRect rcLeft, rcTop, rcRight, rcBottom;
		CRect rcControl = GetPos();

		rcLeft = rcControl;
		rcLeft.right = rcLeft.left + nMargin;		

		rcBottom = rcControl;
		rcBottom.top = rcBottom.bottom - nMargin;

		rcRight = rcControl;
		rcRight.left = rcRight.right - nMargin;

		rcTop = rcControl;
		rcTop.bottom = rcTop.top + nMargin;

		rcLeftTop = rcLeft;
		rcLeftTop.bottom = rcLeftTop.top + nMargin;
		CRenderEngine::DrawColor(hDC, rcLeftTop, dwFillColor);
		CRenderEngine::DrawRect(hDC, rcLeftTop, nBorderSize, dwBorderColor);

		rcLeftBottom = rcLeft;
		rcLeftBottom.top = rcLeftBottom.bottom - nMargin;
		CRenderEngine::DrawColor(hDC, rcLeftBottom, dwFillColor);
		CRenderEngine::DrawRect(hDC, rcLeftBottom, nBorderSize, dwBorderColor);

		rcLeft.top = rcLeftTop.bottom;
		rcLeft.bottom = rcLeftBottom.top;

		rcRightBottom = rcBottom;
		rcRightBottom.left = rcRightBottom.right - nMargin;
		CRenderEngine::DrawColor(hDC, rcRightBottom, dwFillColor);
		CRenderEngine::DrawRect(hDC, rcRightBottom, nBorderSize, dwBorderColor);

		rcBottom.left = rcLeftBottom.right;
		rcBottom.right = rcRightBottom.left;

		rcRightTop = rcRight;
		rcRightTop.bottom = rcRightTop.top + nMargin;
		CRenderEngine::DrawColor(hDC, rcRightTop, dwFillColor);
		CRenderEngine::DrawRect(hDC, rcRightTop, nBorderSize, dwBorderColor);

		rcRight.top = rcRightTop.bottom;
		rcRight.bottom = rcRightBottom.top;

		rcTop.left = rcLeftTop.right;
		rcTop.right = rcRightTop.left;

		rcLeft.top = rcLeft.top + static_cast<LONG>((rcLeft.Height() - nMargin) / 2);
		rcLeft.bottom = rcLeft.top + nMargin;
		CRenderEngine::DrawColor(hDC, rcLeft, dwFillColor);
		CRenderEngine::DrawRect(hDC, rcLeft, nBorderSize, dwBorderColor);

		rcBottom.left = rcBottom.left + static_cast<LONG>((rcBottom.Width() - nMargin) / 2);
		rcBottom.right = rcBottom.left + nMargin;
		CRenderEngine::DrawColor(hDC, rcBottom, dwFillColor);
		CRenderEngine::DrawRect(hDC, rcBottom, nBorderSize, dwBorderColor);

		rcRight.top = rcRight.top + static_cast<LONG>((rcRight.Height() - nMargin) / 2);
		rcRight.bottom = rcRight.top + nMargin;
		CRenderEngine::DrawColor(hDC, rcRight, dwFillColor);
		CRenderEngine::DrawRect(hDC, rcRight, nBorderSize, dwBorderColor);

		rcTop.left = rcTop.left + static_cast<LONG>((rcTop.Width() - nMargin) / 2);
		rcTop.right = rcTop.left + nMargin;
		CRenderEngine::DrawColor(hDC, rcTop, dwFillColor);
		CRenderEngine::DrawRect(hDC, rcTop, nBorderSize, dwBorderColor);
	}
#endif
}

void CScrollBarUI::PaintBk(void* ctx)
{
	if (ctx == NULL) return;

    if( !IsEnabled() ) m_uThumbState |= UISTATE_DISABLED;
    else m_uThumbState &= ~ UISTATE_DISABLED;

    if( (m_uThumbState & UISTATE_DISABLED) != 0 ) {
        if( !m_sBkDisabledImage.IsEmpty() ) {
            if( !DrawImage(ctx, (LPCTSTR)m_sBkDisabledImage) ) m_sBkDisabledImage.Empty();
            else return;
        }
    }
    else if( (m_uThumbState & UISTATE_PUSHED) != 0 ) {
        if( !m_sBkPushedImage.IsEmpty() ) {
            if( !DrawImage(ctx, (LPCTSTR)m_sBkPushedImage) ) m_sBkPushedImage.Empty();
            else return;
        }
    }
    else if( (m_uThumbState & UISTATE_HOT) != 0 ) {
        if( !m_sBkHotImage.IsEmpty() ) {
            if( !DrawImage(ctx, (LPCTSTR)m_sBkHotImage) ) m_sBkHotImage.Empty();
            else return;
        }
    }

    if( !m_sBkNormalImage.IsEmpty() ) {
        if( !DrawImage(ctx, (LPCTSTR)m_sBkNormalImage) ) m_sBkNormalImage.Empty();
        else return;
    }
}

void CScrollBarUI::PaintButton1(void* ctx)
{
	if( !m_bShowButton1 ) return;
	if (ctx == NULL) return;

    if( !IsEnabled() ) m_uButton1State |= UISTATE_DISABLED;
    else m_uButton1State &= ~ UISTATE_DISABLED;

    m_sImageModify.Empty();
    m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), m_rcButton1.left - m_rcItem.left, \
        m_rcButton1.top - m_rcItem.top, m_rcButton1.right - m_rcItem.left, m_rcButton1.bottom - m_rcItem.top);

    if( (m_uButton1State & UISTATE_DISABLED) != 0 ) {
        if( !m_sButton1DisabledImage.IsEmpty() ) {
            if( !DrawImage(ctx, (LPCTSTR)m_sButton1DisabledImage, (LPCTSTR)m_sImageModify) ) m_sButton1DisabledImage.Empty();
            else return;
        }
    }
    else if( (m_uButton1State & UISTATE_PUSHED) != 0 ) {
        if( !m_sButton1PushedImage.IsEmpty() ) {
            if( !DrawImage(ctx, (LPCTSTR)m_sButton1PushedImage, (LPCTSTR)m_sImageModify) ) m_sButton1PushedImage.Empty();
            else return;
        }
    }
    else if( (m_uButton1State & UISTATE_HOT) != 0 ) {
        if( !m_sButton1HotImage.IsEmpty() ) {
            if( !DrawImage(ctx, (LPCTSTR)m_sButton1HotImage, (LPCTSTR)m_sImageModify) ) m_sButton1HotImage.Empty();
            else return;
        }
    }

    if( !m_sButton1NormalImage.IsEmpty() ) {
        if( !DrawImage(ctx, (LPCTSTR)m_sButton1NormalImage, (LPCTSTR)m_sImageModify) ) m_sButton1NormalImage.Empty();
        else return;
    }

    DWORD dwBorderColor = 0xFF85E4FF;
    int nBorderSize = 2;
    CRenderEngine::DrawRect(ctx, m_rcButton1, nBorderSize, dwBorderColor);
}

void CScrollBarUI::PaintButton2(void* ctx)
{
	if( !m_bShowButton2 ) return;
	if (ctx == NULL) return;

    if( !IsEnabled() ) m_uButton2State |= UISTATE_DISABLED;
    else m_uButton2State &= ~ UISTATE_DISABLED;

    m_sImageModify.Empty();
    m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), m_rcButton2.left - m_rcItem.left, \
        m_rcButton2.top - m_rcItem.top, m_rcButton2.right - m_rcItem.left, m_rcButton2.bottom - m_rcItem.top);

    if( (m_uButton2State & UISTATE_DISABLED) != 0 ) {
        if( !m_sButton2DisabledImage.IsEmpty() ) {
            if( !DrawImage(ctx, (LPCTSTR)m_sButton2DisabledImage, (LPCTSTR)m_sImageModify) ) m_sButton2DisabledImage.Empty();
            else return;
        }
    }
    else if( (m_uButton2State & UISTATE_PUSHED) != 0 ) {
        if( !m_sButton2PushedImage.IsEmpty() ) {
            if( !DrawImage(ctx, (LPCTSTR)m_sButton2PushedImage, (LPCTSTR)m_sImageModify) ) m_sButton2PushedImage.Empty();
            else return;
        }
    }
    else if( (m_uButton2State & UISTATE_HOT) != 0 ) {
        if( !m_sButton2HotImage.IsEmpty() ) {
            if( !DrawImage(ctx, (LPCTSTR)m_sButton2HotImage, (LPCTSTR)m_sImageModify) ) m_sButton2HotImage.Empty();
            else return;
        }
    }

    if( !m_sButton2NormalImage.IsEmpty() ) {
        if( !DrawImage(ctx, (LPCTSTR)m_sButton2NormalImage, (LPCTSTR)m_sImageModify) ) m_sButton2NormalImage.Empty();
        else return;
    }

    DWORD dwBorderColor = 0xFF85E4FF;
    int nBorderSize = 2;
    CRenderEngine::DrawRect(ctx, m_rcButton2, nBorderSize, dwBorderColor);
}

void CScrollBarUI::PaintThumb(void* ctx)
{
	if (ctx == NULL) return;

    if( m_rcThumb.left == 0 && m_rcThumb.top == 0 && m_rcThumb.right == 0 && m_rcThumb.bottom == 0 ) return;
    if( !IsEnabled() ) m_uThumbState |= UISTATE_DISABLED;
    else m_uThumbState &= ~ UISTATE_DISABLED;

    m_sImageModify.Empty();
    m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), m_rcThumb.left - m_rcItem.left, \
        m_rcThumb.top - m_rcItem.top, m_rcThumb.right - m_rcItem.left, m_rcThumb.bottom - m_rcItem.top);

    if( (m_uThumbState & UISTATE_DISABLED) != 0 ) {
        if( !m_sThumbDisabledImage.IsEmpty() ) {
            if( !DrawImage(ctx, (LPCTSTR)m_sThumbDisabledImage, (LPCTSTR)m_sImageModify) ) m_sThumbDisabledImage.Empty();
            else return;
        }
    }
    else if( (m_uThumbState & UISTATE_PUSHED) != 0 ) {
        if( !m_sThumbPushedImage.IsEmpty() ) {
            if( !DrawImage(ctx, (LPCTSTR)m_sThumbPushedImage, (LPCTSTR)m_sImageModify) ) m_sThumbPushedImage.Empty();
            else return;
        }
    }
    else if( (m_uThumbState & UISTATE_HOT) != 0 ) {
        if( !m_sThumbHotImage.IsEmpty() ) {
            if( !DrawImage(ctx, (LPCTSTR)m_sThumbHotImage, (LPCTSTR)m_sImageModify) ) m_sThumbHotImage.Empty();
            else return;
        }
    }

    if( !m_sThumbNormalImage.IsEmpty() ) {
        if( !DrawImage(ctx, (LPCTSTR)m_sThumbNormalImage, (LPCTSTR)m_sImageModify) ) m_sThumbNormalImage.Empty();
        else return;
    }

    DWORD dwBorderColor = 0xFF85E4FF;
    int nBorderSize = 2;
    CRenderEngine::DrawRect(ctx, m_rcThumb, nBorderSize, dwBorderColor);
}

void CScrollBarUI::PaintRail(void* ctx)
{
	if (ctx == NULL) return;

    if( m_rcThumb.left == 0 && m_rcThumb.top == 0 && m_rcThumb.right == 0 && m_rcThumb.bottom == 0 ) return;
    if( !IsEnabled() ) m_uThumbState |= UISTATE_DISABLED;
    else m_uThumbState &= ~ UISTATE_DISABLED;

    m_sImageModify.Empty();
    if( !m_bHorizontal ) {
        m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), m_rcThumb.left - m_rcItem.left, \
            (m_rcThumb.top + m_rcThumb.bottom) / 2 - m_rcItem.top - m_cxyFixed.cx / 2, \
            m_rcThumb.right - m_rcItem.left, \
            (m_rcThumb.top + m_rcThumb.bottom) / 2 - m_rcItem.top + m_cxyFixed.cx - m_cxyFixed.cx / 2);
    }
    else {
        m_sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), \
            (m_rcThumb.left + m_rcThumb.right) / 2 - m_rcItem.left - m_cxyFixed.cy / 2, \
            m_rcThumb.top - m_rcItem.top, \
            (m_rcThumb.left + m_rcThumb.right) / 2 - m_rcItem.left + m_cxyFixed.cy - m_cxyFixed.cy / 2, \
            m_rcThumb.bottom - m_rcItem.top);
    }

    if( (m_uThumbState & UISTATE_DISABLED) != 0 ) {
        if( !m_sRailDisabledImage.IsEmpty() ) {
            if( !DrawImage(ctx, (LPCTSTR)m_sRailDisabledImage, (LPCTSTR)m_sImageModify) ) m_sRailDisabledImage.Empty();
            else return;
        }
    }
    else if( (m_uThumbState & UISTATE_PUSHED) != 0 ) {
        if( !m_sRailPushedImage.IsEmpty() ) {
            if( !DrawImage(ctx, (LPCTSTR)m_sRailPushedImage, (LPCTSTR)m_sImageModify) ) m_sRailPushedImage.Empty();
            else return;
        }
    }
    else if( (m_uThumbState & UISTATE_HOT) != 0 ) {
        if( !m_sRailHotImage.IsEmpty() ) {
            if( !DrawImage(ctx, (LPCTSTR)m_sRailHotImage, (LPCTSTR)m_sImageModify) ) m_sRailHotImage.Empty();
            else return;
        }
    }

    if( !m_sRailNormalImage.IsEmpty() ) {
        if( !DrawImage(ctx, (LPCTSTR)m_sRailNormalImage, (LPCTSTR)m_sImageModify) ) m_sRailNormalImage.Empty();
        else return;
    }
}

} // namespace DuiLib
