#include "StdAfx.h"

namespace DuiLib {

static TCHAR cNullTerminated = _T('\0');

CControlUI::CControlUI() : 
m_pManager(NULL), 
m_pParent(NULL), 
m_bUpdateNeeded(true),
m_bMenuUsed(false),
m_bVisible(true), 
m_bInternVisible(true),
m_bFocused(false),
m_bEnabled(true),
m_bMouseEnabled(true),
m_bKeyboardEnabled(true),
m_bFloat(false),
m_bSetPos(false),
m_bImageFitallArea(true),
#ifdef UI_BUILD_FOR_DESIGNER
m_bPitchUpon(false),
m_bUsedForPitchUpon(true),
m_bPitchUponContinousTwice(false),
m_bUsedAbsolutePath(false),
#endif
m_chShortcut(cNullTerminated),
m_pTag(NULL),
m_dwBackColor(0),
m_dwBackColor2(0),
m_dwBackColor3(0),
m_dwBorderColor(0),
m_dwFocusBorderColor(0),
m_bColorHSL(false),
m_nBorderSize(1)
{
    m_cXY.cx = m_cXY.cy = 0;
    m_cxyFixed.cx = m_cxyFixed.cy = 0;
    m_cxyMin.cx = m_cxyMin.cy = 0;
    m_cxyMax.cx = m_cxyMax.cy = 9999;
    m_cxyBorderRound.cx = m_cxyBorderRound.cy = 0;

    ::ZeroMemory(&m_rcPadding, sizeof(m_rcPadding));
    ::ZeroMemory(&m_rcItem, sizeof(RECT));
    ::ZeroMemory(&m_rcPaint, sizeof(RECT));

	::ZeroMemory(&m_tRelativePos,sizeof(TRelativePosUI));
	m_tRelativePos.bRelative = false;
}

CControlUI::~CControlUI()
{
    if( OnDestroy ) OnDestroy(this);
    if( m_pManager != NULL ) m_pManager->ReapObjects(this);
}

CStdString CControlUI::GetName() const
{
    return m_sName;
}

void CControlUI::SetName(LPCTSTR pstrName)
{
    m_sName = pstrName;
}

LPVOID CControlUI::GetInterface(LPCTSTR pstrName)
{
    if( _tcsicmp(pstrName, kControlUIInterfaceName) == 0 ) return this;
    return NULL;
}

LPCTSTR CControlUI::GetClass() const
{
    return kControlUIClassName;
}

UINT CControlUI::GetControlFlags() const
{
#ifdef UI_BUILD_FOR_DESIGNER
	return (IsUsedForPitchUpon() && (m_pManager != NULL) && m_pManager->IsUsedAsUIDesigner())?UIFLAG_SETCURSOR:0;
#else
	return 0;
#endif
}

bool CControlUI::Activate()
{
    if( !IsVisible() ) return false;
    if( !IsEnabled() ) return false;
    return true;
}

CPaintManagerUI* CControlUI::GetManager() const
{
    return m_pManager;
}

void CControlUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
{
    m_pManager = pManager;
    m_pParent = pParent;
    if( bInit && m_pParent ) Init();
}

CControlUI* CControlUI::GetParent() const
{
    return m_pParent;
}

CStdString CControlUI::GetText() const
{
    return m_sText;
}

void CControlUI::SetText(LPCTSTR pstrText)
{
    if( m_sText == pstrText ) return;

    m_sText = pstrText;
    Invalidate();
}

DWORD CControlUI::GetBkColor() const
{
    return m_dwBackColor;
}

void CControlUI::SetBkColor(DWORD dwBackColor)
{
    if( m_dwBackColor == dwBackColor ) return;

    m_dwBackColor = dwBackColor;
    Invalidate();
}

DWORD CControlUI::GetBkColor2() const
{
    return m_dwBackColor2;
}

void CControlUI::SetBkColor2(DWORD dwBackColor)
{
    if( m_dwBackColor2 == dwBackColor ) return;

    m_dwBackColor2 = dwBackColor;
    Invalidate();
}

DWORD CControlUI::GetBkColor3() const
{
    return m_dwBackColor3;
}

void CControlUI::SetBkColor3(DWORD dwBackColor)
{
    if( m_dwBackColor3 == dwBackColor ) return;

    m_dwBackColor3 = dwBackColor;
    Invalidate();
}

LPCTSTR CControlUI::GetBkImage()
{
    return m_sBkImage;
}

void CControlUI::SetBkImage(LPCTSTR pStrImage)
{
    if( m_sBkImage == pStrImage ) return;

    m_sBkImage = pStrImage;
    Invalidate();
}

DWORD CControlUI::GetBorderColor() const
{
    return m_dwBorderColor;
}

void CControlUI::SetBorderColor(DWORD dwBorderColor)
{
    if( m_dwBorderColor == dwBorderColor ) return;

    m_dwBorderColor = dwBorderColor;
    Invalidate();
}

DWORD CControlUI::GetFocusBorderColor() const
{
    return m_dwFocusBorderColor;
}

void CControlUI::SetFocusBorderColor(DWORD dwBorderColor)
{
    if( m_dwFocusBorderColor == dwBorderColor ) return;

    m_dwFocusBorderColor = dwBorderColor;
    Invalidate();
}

bool CControlUI::IsColorHSL() const
{
    return m_bColorHSL;
}

void CControlUI::SetColorHSL(bool bColorHSL)
{
    if( m_bColorHSL == bColorHSL ) return;

    m_bColorHSL = bColorHSL;
    Invalidate();
}

int CControlUI::GetBorderSize() const
{
    return m_nBorderSize;
}

void CControlUI::SetBorderSize(int nSize)
{
    if( m_nBorderSize == nSize ) return;

    m_nBorderSize = nSize;
    Invalidate();
}

SIZE CControlUI::GetBorderRound() const
{
    return m_cxyBorderRound;
}

void CControlUI::SetBorderRound(SIZE cxyRound)
{
    m_cxyBorderRound = cxyRound;
    Invalidate();
}

bool CControlUI::DrawImage(void* ctx, LPCTSTR pStrImage, LPCTSTR pStrModify)
{
	if (ctx == NULL) return false;

	//if (m_bImageFitallArea)
	{
		return CRenderEngine::DrawImageString(ctx, this, m_pManager, m_rcItem, m_rcPaint, pStrImage, pStrModify);
	}
	//else
	//{
	//	return CRenderEngine::DrawImage(ctx, this, m_pManager, m_rcItem, m_rcPaint,pStrImage);
	//}
	return false;
}

const RECT& CControlUI::GetPos() const
{
    return m_rcItem;
}

void CControlUI::SetPos(RECT rc)
{
    if( rc.right < rc.left ) rc.right = rc.left;
    if( rc.bottom < rc.top ) rc.bottom = rc.top;

    CRect invalidateRc = m_rcItem;
    if( ::IsRectEmpty(&invalidateRc) ) invalidateRc = rc;

    m_rcItem = rc;
    if( m_pManager == NULL ) return;

    if( !m_bSetPos ) {
        m_bSetPos = true;
        if( OnSize ) OnSize(this);
        m_bSetPos = false;
    }
    
    if( m_bFloat ) {
        CControlUI* pParent = GetParent();
        if( pParent != NULL ) {
            RECT rcParentPos = pParent->GetPos();
            if( m_cXY.cx >= 0 ) m_cXY.cx = m_rcItem.left - rcParentPos.left;
            else m_cXY.cx = m_rcItem.right - rcParentPos.right;
            if( m_cXY.cy >= 0 ) m_cXY.cy = m_rcItem.top - rcParentPos.top;
            else m_cXY.cy = m_rcItem.bottom - rcParentPos.bottom;
            m_cxyFixed.cx = m_rcItem.right - m_rcItem.left;
            m_cxyFixed.cy = m_rcItem.bottom - m_rcItem.top;
        }
    }

    m_bUpdateNeeded = false;
    invalidateRc.Join(m_rcItem);

    CControlUI* pParent = this;
    RECT rcTemp;
    RECT rcParent;
    while( pParent = pParent->GetParent() )
    {
        rcTemp = invalidateRc;
        rcParent = pParent->GetPos();
        if( !::IntersectRect(&invalidateRc, &rcTemp, &rcParent) ) 
        {
            return;
        }
    }
    m_pManager->Invalidate(invalidateRc);
}

int CControlUI::GetWidth() const
{
    return m_rcItem.right - m_rcItem.left;
}

int CControlUI::GetHeight() const
{
    return m_rcItem.bottom - m_rcItem.top;
}

int CControlUI::GetX() const
{
    return m_rcItem.left;
}

int CControlUI::GetY() const
{
    return m_rcItem.top;
}

RECT CControlUI::GetPadding() const
{
    return m_rcPadding;
}

void CControlUI::SetPadding(RECT rcPadding)
{
    m_rcPadding = rcPadding;
    NeedParentUpdate();
}

SIZE CControlUI::GetFixedXY() const
{
    return m_cXY;
}

void CControlUI::SetFixedXY(SIZE szXY)
{
    m_cXY.cx = szXY.cx;
    m_cXY.cy = szXY.cy;
    if( !m_bFloat ) NeedParentUpdate();
    else NeedUpdate();
}

int CControlUI::GetFixedWidth() const
{
    return m_cxyFixed.cx;
}

void CControlUI::SetFixedWidth(int cx)
{
    if( cx < 0 ) return; 
    m_cxyFixed.cx = cx;
    if( !m_bFloat ) NeedParentUpdate();
    else NeedUpdate();
}

int CControlUI::GetFixedHeight() const
{
    return m_cxyFixed.cy;
}

void CControlUI::SetFixedHeight(int cy)
{
    if( cy < 0 ) return; 
    m_cxyFixed.cy = cy;
    if( !m_bFloat ) NeedParentUpdate();
    else NeedUpdate();
}

int CControlUI::GetMinWidth() const
{
    return m_cxyMin.cx;
}

void CControlUI::SetMinWidth(int cx)
{
    if( m_cxyMin.cx == cx ) return;

    if( cx < 0 ) return; 
    m_cxyMin.cx = cx;
    if( !m_bFloat ) NeedParentUpdate();
    else NeedUpdate();
}

int CControlUI::GetMaxWidth() const
{
    return m_cxyMax.cx;
}

void CControlUI::SetMaxWidth(int cx)
{
    if( m_cxyMax.cx == cx ) return;

    if( cx < 0 ) return; 
    m_cxyMax.cx = cx;
    if( !m_bFloat ) NeedParentUpdate();
    else NeedUpdate();
}

int CControlUI::GetMinHeight() const
{
    return m_cxyMin.cy;
}

void CControlUI::SetMinHeight(int cy)
{
    if( m_cxyMin.cy == cy ) return;

    if( cy < 0 ) return; 
    m_cxyMin.cy = cy;
    if( !m_bFloat ) NeedParentUpdate();
    else NeedUpdate();
}

int CControlUI::GetMaxHeight() const
{
    return m_cxyMax.cy;
}

void CControlUI::SetMaxHeight(int cy)
{
    if( m_cxyMax.cy == cy ) return;

    if( cy < 0 ) return; 
    m_cxyMax.cy = cy;
    if( !m_bFloat ) NeedParentUpdate();
    else NeedUpdate();
}

void CControlUI::SetRelativePos(SIZE szMove,SIZE szZoom)
{
    m_tRelativePos.bRelative = TRUE;
    m_tRelativePos.nMoveXPercent = szMove.cx;
    m_tRelativePos.nMoveYPercent = szMove.cy;
    m_tRelativePos.nZoomXPercent = szZoom.cx;
    m_tRelativePos.nZoomYPercent = szZoom.cy;
}

void CControlUI::SetRelativeParentSize(SIZE sz)
{
	m_tRelativePos.szParent=sz;
}

TRelativePosUI CControlUI::GetRelativePos() const
{
	return m_tRelativePos;
}

bool CControlUI::IsRelativePos() const
{
	return m_tRelativePos.bRelative;
}

CStdString CControlUI::GetToolTip() const
{
    return m_sToolTip;
}

void CControlUI::SetToolTip(LPCTSTR pstrText)
{
    m_sToolTip = pstrText;
}


TCHAR CControlUI::GetShortcut() const
{
    return m_chShortcut;
}

void CControlUI::SetShortcut(TCHAR ch)
{
    m_chShortcut = ch;
}

bool CControlUI::IsContextMenuUsed() const
{
    return m_bMenuUsed;
}

void CControlUI::SetContextMenuUsed(bool bMenuUsed)
{
    m_bMenuUsed = bMenuUsed;
}

const CStdString& CControlUI::GetUserData()
{
    return m_sUserData;
}

void CControlUI::SetUserData(LPCTSTR pstrText)
{
    m_sUserData = pstrText;
}

UINT_PTR CControlUI::GetTag() const
{
    return m_pTag;
}

void CControlUI::SetTag(UINT_PTR pTag)
{
    m_pTag = pTag;
}

bool CControlUI::IsImageFitallArea() const
{
	return m_bImageFitallArea;
}

void CControlUI::SetImageFitAllArea(bool bFit)
{
	m_bImageFitallArea = bFit;
}

bool CControlUI::IsVisible() const
{
    return m_bVisible && m_bInternVisible;
}

void CControlUI::SetVisible(bool bVisible)
{
    if( m_bVisible == bVisible ) return;

    bool v = IsVisible();
    m_bVisible = bVisible;
    if( m_bFocused ) m_bFocused = false;
	if (!bVisible && m_pManager && m_pManager->GetFocus() == this) {
		m_pManager->SetFocus(NULL) ;
	}
    if( IsVisible() != v ) {
        NeedParentUpdate();
    }
}

void CControlUI::SetInternVisible(bool bVisible)
{
    m_bInternVisible = bVisible;
	if (!bVisible && m_pManager && m_pManager->GetFocus() == this) {
		m_pManager->SetFocus(NULL) ;
	}
}

bool CControlUI::IsEnabled() const
{
    return m_bEnabled;
}

void CControlUI::SetEnabled(bool bEnabled)
{
    if( m_bEnabled == bEnabled ) return;

    m_bEnabled = bEnabled;
    Invalidate();
}

bool CControlUI::IsMouseEnabled() const
{
    return m_bMouseEnabled;
}

void CControlUI::SetMouseEnabled(bool bEnabled)
{
    m_bMouseEnabled = bEnabled;
}

bool CControlUI::IsKeyboardEnabled() const
{
	return m_bKeyboardEnabled ;
}
void CControlUI::SetKeyboardEnabled(bool bEnabled)
{
	m_bKeyboardEnabled = bEnabled ; 
}

bool CControlUI::IsFocused() const
{
    return m_bFocused;
}
void CControlUI::SetFocus()
{
    if( m_pManager != NULL ) m_pManager->SetFocus(this);
}

bool CControlUI::IsFloat() const
{
    return m_bFloat;
}

void CControlUI::SetFloat(bool bFloat)
{
    if( m_bFloat == bFloat ) return;

    m_bFloat = bFloat;
    NeedParentUpdate();
}

#ifdef UI_BUILD_FOR_DESIGNER
bool CControlUI::IsPitchUpon() const
{
	return m_bPitchUpon;
}

void CControlUI::SetPitchUpon(bool bPitchUpon)
{
	if (!m_bUsedForPitchUpon)
		m_bPitchUpon = false;
	else
	{
		if (m_bPitchUpon != bPitchUpon)
			Invalidate();
		m_bPitchUpon = bPitchUpon;
	}
}

bool CControlUI::IsPitchUponContinousTwice() const
{
	return m_bPitchUponContinousTwice;
}

void CControlUI::SetPitchUponContinousTwice(bool bPitchUponContinousTwice)
{
	if (!m_bUsedForPitchUpon)
		m_bPitchUponContinousTwice = false;
	else
	{
		if (m_bPitchUponContinousTwice != bPitchUponContinousTwice)
			Invalidate();
		m_bPitchUponContinousTwice = bPitchUponContinousTwice;
	}
}

bool CControlUI::IsUsedForPitchUpon() const
{
	return m_bUsedForPitchUpon;
}

void CControlUI::SetUsedForPitchUpon(bool bUsedForPitchUpon)
{
	m_bUsedForPitchUpon = bUsedForPitchUpon;
}

void CControlUI::Move(LONG xOffset, LONG yOffset)
{
	if (IsUsedForPitchUpon() && (m_pManager != NULL) && m_pManager->IsUsedAsUIDesigner() && m_bPitchUpon)
	{
		RECT rc = m_rcItem;

		rc.left = rc.left + xOffset;
		//if (rc.left < 0)
		//	rc.left = 0;
		rc.right = rc.left + m_rcItem.right - m_rcItem.left;

		rc.top = rc.top + yOffset;
		//if (rc.top < 0)
		//	rc.top = 0;
		rc.bottom = rc.top + m_rcItem.bottom - m_rcItem.top;

		RECT rcParent = {0};
		if (GetParent()) 
			rcParent = GetParent()->GetPos();

		if (rc.left < rcParent.left)
			rc.left = rcParent.left;
		if (rc.top < rcParent.top)
			rc.top = rcParent.top;

		if ((rc.left < rcParent.right) && (rc.top < rcParent.bottom))
		{
			SetFixedXY(CSize(rc.left - rcParent.left, rc.top - rcParent.top));
			if( GetParent() ) 
				GetParent()->NeedParentUpdate();
		}
	}
}

bool CControlUI::IsImageAbsolutePath() const
{
	return m_bUsedAbsolutePath;
}

void CControlUI::SetImageAbsolutePath(bool bUseAbsolutePath)
{
	m_bUsedAbsolutePath = bUseAbsolutePath;
}
#endif

CControlUI* CControlUI::FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags)
{
    if( (uFlags & UIFIND_VISIBLE) != 0 && !IsVisible() ) return NULL;
    if( (uFlags & UIFIND_ENABLED) != 0 && !IsEnabled() ) return NULL;
    if( (uFlags & UIFIND_HITTEST) != 0 && (!m_bMouseEnabled || !::PtInRect(&m_rcItem, * static_cast<LPPOINT>(pData))) ) return NULL;
    return Proc(this, pData);
}

void CControlUI::Invalidate()
{
    if( !IsVisible() ) return;

    RECT invalidateRc = m_rcItem;

    CControlUI* pParent = this;
    RECT rcTemp;
    RECT rcParent;
    while( pParent = pParent->GetParent() )
    {
        rcTemp = invalidateRc;
        rcParent = pParent->GetPos();
        if( !::IntersectRect(&invalidateRc, &rcTemp, &rcParent) ) 
        {
            return;
        }
    }

    if( m_pManager != NULL ) m_pManager->Invalidate(invalidateRc);
}

bool CControlUI::IsUpdateNeeded() const
{
    return m_bUpdateNeeded;
}

void CControlUI::NeedUpdate()
{
    if( !IsVisible() ) return;
    m_bUpdateNeeded = true;
    Invalidate();

    if( m_pManager != NULL ) m_pManager->NeedUpdate();
}

void CControlUI::NeedParentUpdate()
{
    if( GetParent() ) {
        GetParent()->NeedUpdate();
        GetParent()->Invalidate();
    }
    else {
        NeedUpdate();
    }

    if( m_pManager != NULL ) m_pManager->NeedUpdate();
}

DWORD CControlUI::GetAdjustColor(DWORD dwColor)
{
    if( !m_bColorHSL ) return dwColor;
    short H, S, L;
    CPaintManagerUI::GetHSL(&H, &S, &L);
    return CRenderEngine::AdjustColor(dwColor, H, S, L);
}

void CControlUI::Init()
{
    DoInit();
    if( OnInit ) OnInit(this);
}

void CControlUI::DoInit()
{}

enum eClickArea
{
	eClickUnknown	= -1,
	eClickCenter	= 0,
	eClickLeft		= 1 << 1,	
	eClickBottom	= 1 << 2,
	eClickTop		= 1 << 3,
	eClickRight		= 1 << 4,
	eClickLeftTop	= (1 << 1) | (1 << 3),
	eClickLeftBottom = (1 << 1) | (1 << 2),
	eClickRightBottom = (1 << 4) | (1 << 2),
	eClickRightTop = (1 << 4) | (1 << 3)
};

eClickArea HitTestControlArea(const CControlUI* pControl, const TEventUI& event)
{
	eClickArea clickArea = eClickUnknown;
	if (pControl != NULL)
	{
		int nMargin = 7;
        CRect rcLeftTop, rcRightTop, rcLeftBottom, rcRightBottom;
		CRect rcLeft, rcTop, rcRight, rcBottom, rcCenter;
		CRect rcControl = pControl->GetPos();

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

		rcLeftBottom = rcLeft;
		rcLeftBottom.top = rcLeftBottom.bottom - nMargin;

		rcLeft.top = rcLeftTop.bottom;
		rcLeft.bottom = rcLeftBottom.top;

		rcRightBottom = rcBottom;
		rcRightBottom.left = rcRightBottom.right - nMargin;

		rcBottom.left = rcLeftBottom.right;
		rcBottom.right = rcRightBottom.left;

		rcRightTop = rcRight;
		rcRightTop.bottom = rcRightTop.top + nMargin;

		rcRight.top = rcRightTop.bottom;
		rcRight.bottom = rcRightBottom.top;

		rcTop.left = rcLeftTop.right;
		rcTop.right = rcRightTop.left;

		rcCenter.left = rcLeft.right;
		rcCenter.top = rcTop.bottom;
		rcCenter.right = rcRight.left;
		rcCenter.bottom = rcBottom.top;

		rcLeft.top = rcLeft.top + static_cast<LONG>((rcLeft.Height() - nMargin) / 2);
		rcLeft.bottom = rcLeft.top + nMargin;

		rcBottom.left = rcBottom.left + static_cast<LONG>((rcBottom.Width() - nMargin) / 2);
		rcBottom.right = rcBottom.left + nMargin;

		rcRight.top = rcRight.top + static_cast<LONG>((rcRight.Height() - nMargin) / 2);
		rcRight.bottom = rcRight.top + nMargin;

		rcTop.left = rcTop.left + static_cast<LONG>((rcTop.Width() - nMargin) / 2);
		rcTop.right = rcTop.left + nMargin;

		if (rcControl.PtInRect(event.ptMouse))
		{
			if (rcCenter.PtInRect(event.ptMouse))
				clickArea = eClickCenter;
			else if (rcLeftTop.PtInRect(event.ptMouse))
				clickArea = eClickLeftTop;
			else if (rcLeft.PtInRect(event.ptMouse))
				clickArea = eClickLeft;
			else if (rcLeftBottom.PtInRect(event.ptMouse))
				clickArea = eClickLeftBottom;
			else if (rcBottom.PtInRect(event.ptMouse))
				clickArea = eClickBottom;
			else if (rcRightBottom.PtInRect(event.ptMouse))
				clickArea = eClickRightBottom;
			else if (rcRight.PtInRect(event.ptMouse))
				clickArea = eClickRight;
			else if (rcRightTop.PtInRect(event.ptMouse))
				clickArea = eClickRightTop;
			else if (rcTop.PtInRect(event.ptMouse))
				clickArea = eClickTop;
		}
	}

	return clickArea;
}

static bool g_bMouseCaptured = false;
static eClickArea g_eMouseCapteredArea = eClickUnknown;
static POINT g_ptLastMouse = {0};

void CControlUI::Event(TEventUI& event)
{
    if( OnEvent(&event) ) DoEvent(event);
}

void CControlUI::DoEvent(TEventUI& event)
{
	if( event.Type == UIEVENT_SETCURSOR )
	{
#ifdef UI_BUILD_FOR_DESIGNER
		if (m_bUsedForPitchUpon && (m_pManager != NULL) && m_pManager->IsUsedAsUIDesigner() && ((GetKeyState(VK_CONTROL) < 0) || (GetKeyState(VK_SHIFT) < 0)))
		{
			eClickArea clickArea = HitTestControlArea(this, event);
			switch (clickArea)
			{
			case eClickCenter:
				::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEALL)));
				break;
			case eClickLeft:
			case eClickRight:
				::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEWE)));
				break;
			case eClickBottom:
			case eClickTop:
				::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENS)));
				break;
			case eClickLeftTop:
			case eClickRightBottom:
				::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENWSE)));
				break;
			case eClickLeftBottom:
			case eClickRightTop:
				::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENESW)));
				break;
			default:
				::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
				break;
			}
			return;
		}		
#endif
	}

#ifdef UI_BUILD_FOR_DESIGNER
	if (m_bUsedForPitchUpon && (m_pManager != NULL) && m_pManager->IsUsedAsUIDesigner() && ((GetKeyState(VK_CONTROL) < 0) || (GetKeyState(VK_SHIFT) < 0)))
	{
		if( event.Type == UIEVENT_BUTTONDOWN )
		{
			eClickArea clickArea = HitTestControlArea(this, event);
			if (clickArea != eClickUnknown)
			{
				bool bPitchUpon = IsPitchUpon();
				if (GetKeyState(VK_CONTROL) < 0)
				{
					m_pManager->RemoveAllPitchUpon();
				}

				if (GetKeyState(VK_SHIFT) < 0)
				{
					m_pManager->RemoveAllPitchUpon(true);
				}

				//if (GetParent()) 
				{
					m_bPitchUpon = true;
					if (bPitchUpon && (m_pManager->FindPitchUponControlCount() > 1))
						m_bPitchUponContinousTwice = true;

					NeedParentUpdate();
				}
				g_bMouseCaptured = true;
				g_eMouseCapteredArea = clickArea;
				g_ptLastMouse = event.ptMouse;
			}
			return;
		}
		if( event.Type == UIEVENT_BUTTONUP )
		{
			g_eMouseCapteredArea = eClickUnknown;
			g_bMouseCaptured = false;
			return;
		}
		if( event.Type == UIEVENT_MOUSEMOVE )
		{
			if (g_bMouseCaptured)
			{
				switch (g_eMouseCapteredArea)
				{
				case eClickCenter:
					if (GetParent()) 
					{
						LONG xOffset = event.ptMouse.x - g_ptLastMouse.x;
						LONG yOffset = event.ptMouse.y - g_ptLastMouse.y;
						m_pManager->MoveAllPitchUponControls(xOffset, yOffset);
						break;
					}
				case eClickLeft:
				case eClickRight:
				case eClickBottom:
				case eClickTop:
				case eClickLeftTop:
				case eClickRightBottom:
				case eClickLeftBottom:
				case eClickRightTop:
					{
						RECT rc = m_rcItem;
						if (eClickBottom & g_eMouseCapteredArea) {
							rc.bottom += event.ptMouse.y - g_ptLastMouse.y;
						}
						else if (eClickTop & g_eMouseCapteredArea){
							rc.top += event.ptMouse.y - g_ptLastMouse.y;
							if (rc.top < 0)
								rc.top = 0;
						}

						if (eClickRight & g_eMouseCapteredArea) {
							rc.right += event.ptMouse.x - g_ptLastMouse.x;
						}
						else if (eClickLeft & g_eMouseCapteredArea){
							rc.left += event.ptMouse.x - g_ptLastMouse.x;
							if (rc.left < 0)
								rc.left = 0;
						}

						if ((rc.top < rc.bottom) && ( rc.bottom - rc.top > GetMinHeight() )) {
							m_cxyFixed.cy = rc.bottom - rc.top;
							m_cxyFixed.cx = rc.right - rc.left;
							RECT rcParent = {0};
							if( GetParent() ) 
								rcParent = GetParent()->GetPos();
							if ((eClickTop & g_eMouseCapteredArea)  || (eClickLeft & g_eMouseCapteredArea))
								SetFixedXY(CSize(rc.left - rcParent.left, rc.top - rcParent.top));
							if( GetParent() ) 
								GetParent()->NeedParentUpdate();
						}
						break;
					}
				default:
					break;
				}
				g_ptLastMouse = event.ptMouse;
			}
			return;
		}
		return;
	}

	if (m_bUsedForPitchUpon && (m_pManager != NULL) && m_pManager->IsUsedAsUIDesigner() && IsPitchUpon())
	{
		if( event.Type == UIEVENT_KEYDOWN )
		{
			if ((event.wParam == VK_LEFT) || (event.wParam == VK_RIGHT) || (event.wParam == VK_UP) || (event.wParam == VK_DOWN))
			{
				LONG xOffset = 0;
				LONG yOffset = 0;
				LONG nMoveStep = 1;
				switch (event.wParam)
				{
				case VK_LEFT:
					xOffset -= nMoveStep;
					break;
				case VK_RIGHT:
					xOffset += nMoveStep;
					break;
				case VK_UP:
					yOffset -= nMoveStep;
					break;
				case VK_DOWN:
					yOffset += nMoveStep;
					break;
				}
				if ((xOffset != 0) || (yOffset != 0))
					m_pManager->MoveAllPitchUponControls(xOffset, yOffset);
			}
		}
		return;
	}
#endif

    if( event.Type == UIEVENT_SETCURSOR )
    {
        ::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
        return;
    }
    if( event.Type == UIEVENT_SETFOCUS ) 
    {
        m_bFocused = true;
        Invalidate();
        return;
    }
    if( event.Type == UIEVENT_KILLFOCUS ) 
    {
        m_bFocused = false;
        Invalidate();
        return;
    }
    if( event.Type == UIEVENT_TIMER )
    {
        m_pManager->SendNotify(this, kTimer, event.wParam, event.lParam);
        return;
    }
    if( event.Type == UIEVENT_CONTEXTMENU )
    {
        if( IsContextMenuUsed() ) {
            m_pManager->SendNotify(this, kMenu, event.wParam, event.lParam);
            return;
        }
    }
    if( m_pParent != NULL ) m_pParent->DoEvent(event);
}


void CControlUI::SetStretchMode(LPCTSTR pstrStretchMode)
{
	if (pstrStretchMode)
		m_sStretchMode = pstrStretchMode;
}

LPCTSTR CControlUI::GetStretchMode() const
{
	return m_sStretchMode;
}

void CControlUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if( _tcsicmp(pstrName, _T("pos")) == 0 ) {
        RECT rcPos = { 0 };
        LPTSTR pstr = NULL;
        rcPos.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
        rcPos.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
        rcPos.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
        rcPos.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
        SIZE szXY = {rcPos.left >= 0 ? rcPos.left : rcPos.right, rcPos.top >= 0 ? rcPos.top : rcPos.bottom};
        SetFixedXY(szXY);
        SetFixedWidth(rcPos.right - rcPos.left);
        SetFixedHeight(rcPos.bottom - rcPos.top);
    }
	else if( _tcsicmp(pstrName, _T("relativepos")) == 0 ) {
		SIZE szMove,szZoom;
		LPTSTR pstr = NULL;
		szMove.cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
		szMove.cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
		szZoom.cx = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
		szZoom.cy = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr); 
		SetRelativePos(szMove,szZoom);
	}
    else if( _tcsicmp(pstrName, _T("padding")) == 0 ) {
        RECT rcPadding = { 0 };
        LPTSTR pstr = NULL;
        rcPadding.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
        rcPadding.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
        rcPadding.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
        rcPadding.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
        SetPadding(rcPadding);
    }
    else if( _tcsicmp(pstrName, _T("bkcolor")) == 0 || _tcsicmp(pstrName, _T("bkcolor1")) == 0 ) {
        while( *pstrValue > _T('\0') && *pstrValue <= _T(' ') ) pstrValue = ::CharNext(pstrValue);
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetBkColor(clrColor);
    }
    else if( _tcsicmp(pstrName, _T("bkcolor2")) == 0 ) {
        while( *pstrValue > _T('\0') && *pstrValue <= _T(' ') ) pstrValue = ::CharNext(pstrValue);
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetBkColor2(clrColor);
    }
    else if( _tcsicmp(pstrName, _T("bkcolor3")) == 0 ) {
        while( *pstrValue > _T('\0') && *pstrValue <= _T(' ') ) pstrValue = ::CharNext(pstrValue);
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetBkColor3(clrColor);
    }
    else if( _tcsicmp(pstrName, _T("bordercolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetBorderColor(clrColor);
    }
    else if( _tcsicmp(pstrName, _T("focusbordercolor")) == 0 ) {
        if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
        LPTSTR pstr = NULL;
        DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
        SetFocusBorderColor(clrColor);
    }
	else if( _tcsicmp(pstrName, _T("colorhsl")) == 0 ) SetColorHSL(_tcsicmp(pstrValue, _T("true")) == 0);
	else if( _tcsicmp(pstrName, _T("bordersize")) == 0 ) SetBorderSize(_ttoi(pstrValue));
    else if( _tcsicmp(pstrName, _T("borderround")) == 0 ) {
        SIZE cxyRound = { 0 };
        LPTSTR pstr = NULL;
        cxyRound.cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
        cxyRound.cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);     
        SetBorderRound(cxyRound);
    }
    else if( _tcsicmp(pstrName, _T("bkimage")) == 0 ) SetBkImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("width")) == 0 ) SetFixedWidth(_ttoi(pstrValue));
    else if( _tcsicmp(pstrName, _T("height")) == 0 ) SetFixedHeight(_ttoi(pstrValue));
    else if( _tcsicmp(pstrName, _T("minwidth")) == 0 ) SetMinWidth(_ttoi(pstrValue));
    else if( _tcsicmp(pstrName, _T("minheight")) == 0 ) SetMinHeight(_ttoi(pstrValue));
    else if( _tcsicmp(pstrName, _T("maxwidth")) == 0 ) SetMaxWidth(_ttoi(pstrValue));
    else if( _tcsicmp(pstrName, _T("maxheight")) == 0 ) SetMaxHeight(_ttoi(pstrValue));
    else if( _tcsicmp(pstrName, _T("name")) == 0 ) SetName(pstrValue);
    else if( _tcsicmp(pstrName, _T("text")) == 0 ) SetText(pstrValue);
    else if( _tcsicmp(pstrName, _T("tooltip")) == 0 ) SetToolTip(pstrValue);
    else if( _tcsicmp(pstrName, _T("userdata")) == 0 ) SetUserData(pstrValue);
    else if( _tcsicmp(pstrName, _T("enabled")) == 0 ) SetEnabled(_tcsicmp(pstrValue, _T("true")) == 0);
    else if( _tcsicmp(pstrName, _T("mouse")) == 0 ) SetMouseEnabled(_tcsicmp(pstrValue, _T("true")) == 0);
	else if( _tcsicmp(pstrName, _T("keyboard")) == 0 ) SetKeyboardEnabled(_tcsicmp(pstrValue, _T("true")) == 0);
    else if( _tcsicmp(pstrName, _T("visible")) == 0 ) SetVisible(_tcsicmp(pstrValue, _T("true")) == 0);
    else if( _tcsicmp(pstrName, _T("float")) == 0 ) SetFloat(_tcsicmp(pstrValue, _T("true")) == 0);
    else if( _tcsicmp(pstrName, _T("shortcut")) == 0 ) SetShortcut(pstrValue[0]);
    else if( _tcsicmp(pstrName, _T("menu")) == 0 ) SetContextMenuUsed(_tcsicmp(pstrValue, _T("true")) == 0);
}

CControlUI* CControlUI::ApplyAttributeList(LPCTSTR pstrList)
{
    CStdString sItem;
    CStdString sValue;
    while( *pstrList != cNullTerminated ) {
        sItem.Empty();
        sValue.Empty();
        while( *pstrList != cNullTerminated && *pstrList != _T('=') ) {
            LPTSTR pstrTemp = ::CharNext(pstrList);
            while( pstrList < pstrTemp) {
                sItem += *pstrList++;
            }
        }
        ASSERT( *pstrList == _T('=') );
        if( *pstrList++ != _T('=') ) return this;
        ASSERT( *pstrList == _T('\"') );
        if( *pstrList++ != _T('\"') ) return this;
        while( *pstrList != cNullTerminated && *pstrList != _T('\"') ) {
            LPTSTR pstrTemp = ::CharNext(pstrList);
            while( pstrList < pstrTemp) {
                sValue += *pstrList++;
            }
        }
        ASSERT( *pstrList == _T('\"') );
        if( *pstrList++ != _T('\"') ) return this;
        SetAttribute(sItem, sValue);
        if( *pstrList++ != _T(' ') ) return this;
    }
    return this;
}

SIZE CControlUI::EstimateSize(SIZE szAvailable)
{
    return m_cxyFixed;
}

void CControlUI::DoPaint(void* ctx, const RECT& rcPaint)
{
	if (ctx == NULL)
		return;

    if( !::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem) ) return;

    // »æÖÆÑ­Ðò£º±³¾°ÑÕÉ«->±³¾°Í¼->×´Ì¬Í¼->ÎÄ±¾->±ß¿ò
    if( m_cxyBorderRound.cx > 0 || m_cxyBorderRound.cy > 0 ) {
        CRenderClip roundClip;
        CRenderClip::GenerateRoundClip(ctx, m_rcPaint,  m_rcItem, m_cxyBorderRound.cx, m_cxyBorderRound.cy, roundClip);
        PaintBkColor(ctx);
        PaintBkImage(ctx);
        PaintStatusImage(ctx);
        PaintText(ctx);
        PaintBorder(ctx);
    }
    else {
		PaintBkColor(ctx);
		PaintBkImage(ctx);
		PaintStatusImage(ctx);
		PaintText(ctx);
		PaintBorder(ctx);
    }

#if defined(UI_BUILD_FOR_DESIGNER) && defined(UI_BUILD_FOR_WINGDI)
	HDC hDC = (HDC)ctx;
	if ((m_pManager != NULL) && m_pManager->IsUsedAsUIDesigner() && m_bPitchUpon)
	{
		int nBorderSize = 1;
		int nMargin = 7;
		DWORD dwFillColor = m_bPitchUponContinousTwice?0xFFFF0000:0xFF00FF00;
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

void CControlUI::PaintBkColor(void* ctx)
{
	if (ctx == NULL) return;

    if( m_dwBackColor != 0 ) {
        if( m_dwBackColor2 != 0 ) {
            if( m_dwBackColor3 != 0 ) {
                RECT rc = m_rcItem;
                rc.bottom = (rc.bottom + rc.top) / 2;
                CRenderEngine::DrawGradient(ctx, rc, GetAdjustColor(m_dwBackColor), GetAdjustColor(m_dwBackColor2), true, 8);
                rc.top = rc.bottom;
                rc.bottom = m_rcItem.bottom;
                CRenderEngine::DrawGradient(ctx, rc, GetAdjustColor(m_dwBackColor2), GetAdjustColor(m_dwBackColor3), true, 8);
            }
            else 
                CRenderEngine::DrawGradient(ctx, m_rcItem, GetAdjustColor(m_dwBackColor), GetAdjustColor(m_dwBackColor2), true, 16);
        }
        else if( m_dwBackColor >= 0xFF000000 ) CRenderEngine::DrawColor(ctx, m_rcPaint, GetAdjustColor(m_dwBackColor));
        else CRenderEngine::DrawColor(ctx, m_rcItem, GetAdjustColor(m_dwBackColor));
    }
}

void CControlUI::PaintBkImage(void* ctx)
{
	if (ctx == NULL) return;
    if( m_sBkImage.IsEmpty() ) return;

    if( !DrawImage(ctx, (LPCTSTR)m_sBkImage) ) m_sBkImage.Empty();
}

void CControlUI::PaintStatusImage(void* ctx)
{
    return;
}

void CControlUI::PaintText(void* ctx)
{
    return;
}

void CControlUI::PaintBorder(void* ctx)
{
	if (ctx == NULL) return;

	if( m_nBorderSize > 0 )
	{
        if( m_cxyBorderRound.cx > 0 || m_cxyBorderRound.cy > 0 )
		{
			if (IsFocused() && m_dwFocusBorderColor != 0)
				CRenderEngine::DrawRoundRect(ctx, m_rcItem, m_nBorderSize, m_cxyBorderRound.cx, m_cxyBorderRound.cy, GetAdjustColor(m_dwFocusBorderColor));
			else
				CRenderEngine::DrawRoundRect(ctx, m_rcItem, m_nBorderSize, m_cxyBorderRound.cx, m_cxyBorderRound.cy, GetAdjustColor(m_dwBorderColor));
		}
		else
		{
			if (IsFocused() && m_dwFocusBorderColor != 0)
				CRenderEngine::DrawRect(ctx, m_rcItem, m_nBorderSize, GetAdjustColor(m_dwFocusBorderColor));
			else if (m_dwBorderColor != 0)
				CRenderEngine::DrawRect(ctx, m_rcItem, m_nBorderSize, GetAdjustColor(m_dwBorderColor));
		}
	}
}

void CControlUI::DoPostPaint(void* ctx, const RECT& rcPaint)
{
    return;
}

} // namespace DuiLib
