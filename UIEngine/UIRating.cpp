#include "StdAfx.h"
#include "UIlib.h"
#include "UIRating.h"
#include "UICrack.h"

namespace DuiLib {

/////////////////////////////////////////////////////////////////////////////////////
//

// RatingUI
const TCHAR* const kRatingUIClassName = _T("RatingUI");
const TCHAR* const kRatingUIInterfaceName = _T("Rating");

CRatingUI::CRatingUI() : m_bHorizontal(true), m_uButtonState(0), m_nMin(0), m_nMax(5), m_nValue(0)
{
    m_uTextStyle = DT_SINGLELINE | DT_CENTER;
    SetFixedHeight(24);
}

CRatingUI::~CRatingUI()
{}

LPCTSTR CRatingUI::GetClass() const
{
    return kRatingUIClassName;
}

LPVOID CRatingUI::GetInterface(LPCTSTR pstrName)
{
	if( _tcsicmp(pstrName, kRatingUIInterfaceName) == 0 ) return static_cast<CRatingUI*>(this);
    return CLabelUI::GetInterface(pstrName);
}

UINT CRatingUI::GetControlFlags() const
{
	if( !IsEnabled() ) return CControlUI::GetControlFlags();

    return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
}

bool CRatingUI::IsHorizontal()
{
    return m_bHorizontal;
}

void CRatingUI::SetHorizontal(bool bHorizontal)
{
    if( m_bHorizontal == bHorizontal ) return;

    m_bHorizontal = bHorizontal;
    Invalidate();
}

int CRatingUI::GetMinValue() const
{
    return m_nMin;
}

void CRatingUI::SetMinValue(int nMin)
{
    m_nMin = nMin;
    Invalidate();
}

int CRatingUI::GetMaxValue() const
{
    return m_nMax;
}

void CRatingUI::SetMaxValue(int nMax)
{
    m_nMax = nMax;
    Invalidate();
}

int CRatingUI::GetValue() const
{
    return m_nValue;
}

void CRatingUI::SetValue(int nValue)
{
    m_nValue = nValue;
    Invalidate();
}

LPCTSTR CRatingUI::GetNormalImage() const
{
    return m_sNormalImage;
}

void CRatingUI::SetNormalImage(LPCTSTR pStrImage)
{
    if( m_sNormalImage == pStrImage ) return;

    m_sNormalImage = pStrImage;
    Invalidate();
}

LPCTSTR CRatingUI::GetDisabledImage() const
{
    return m_sDisabledImage;
}

void CRatingUI::SetDisabledImage(LPCTSTR pStrImage)
{
    if( m_sDisabledImage == pStrImage ) return;

    m_sDisabledImage = pStrImage;
    Invalidate();
}

void CRatingUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if( _tcsicmp(pstrName, _T("normalimage")) == 0 ) SetNormalImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("disabledimage")) == 0 ) SetDisabledImage(pstrValue);
    else if( _tcsicmp(pstrName, _T("hor")) == 0 ) SetHorizontal(_tcsicmp(pstrValue, _T("true")) == 0);
    else if( _tcsicmp(pstrName, _T("min")) == 0 ) SetMinValue(_ttoi(pstrValue));
    else if( _tcsicmp(pstrName, _T("max")) == 0 ) SetMaxValue(_ttoi(pstrValue));
    else if( _tcsicmp(pstrName, _T("value")) == 0 ) SetValue(_ttoi(pstrValue));
    else CLabelUI::SetAttribute(pstrName, pstrValue);
}

void CRatingUI::DoEvent(TEventUI& event)
{
    if( event.Type == UIEVENT_MOUSEMOVE )
    {
		if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			if( ::PtInRect(&m_rcItem, event.ptMouse) ) {
				LONG nImageWidth = static_cast<LONG>( (m_rcItem.right - m_rcItem.left) / m_nMax );
				LONG nImageHeight = static_cast<LONG>( (m_rcItem.bottom - m_rcItem.top) / m_nMax );
				if( m_bHorizontal ) {
					m_nValue = static_cast<int>( ( event.ptMouse.x - m_rcItem.left ) / nImageWidth ) + \
						( ( ( event.ptMouse.x - m_rcItem.left ) % nImageWidth ) ? 1 : 0);
				}
				else {
					m_nValue = static_cast<int>( ( m_rcItem.bottom - event.ptMouse.y ) / nImageHeight ) + \
						( ( m_rcItem.bottom - event.ptMouse.y > 0 ) && ( ( m_rcItem.bottom - event.ptMouse.y ) % nImageHeight ) ? 1 : 0);
					if( m_rcItem.bottom - event.ptMouse.y == 1 )
						m_nValue = 0;
				}
				Invalidate();
			}
        }
        return;
    }
    if( event.Type == UIEVENT_BUTTONUP )
    {
        if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			if( ::PtInRect(&m_rcItem, event.ptMouse) ) {
				m_uButtonState &= ~UISTATE_HOT;

				LONG nImageWidth = static_cast<LONG>( (m_rcItem.right - m_rcItem.left) / m_nMax );
				LONG nImageHeight = static_cast<LONG>( (m_rcItem.bottom - m_rcItem.top) / m_nMax );
				if( m_bHorizontal ) {
					m_nValue = static_cast<int>( ( event.ptMouse.x - m_rcItem.left ) / nImageWidth ) + \
						( ( ( event.ptMouse.x - m_rcItem.left ) % nImageWidth ) ? 1 : 0);
				}
				else {
					m_nValue = static_cast<int>( ( m_rcItem.bottom - event.ptMouse.y ) / nImageHeight ) + \
						( ( m_rcItem.bottom - event.ptMouse.y > 0 ) && ( ( m_rcItem.bottom - event.ptMouse.y ) % nImageHeight ) ? 1 : 0);
					if( m_rcItem.bottom - event.ptMouse.y == 1 )
						m_nValue = 0;
				}

				Invalidate();
			}
        }
        return;
    }
    if( event.Type == UIEVENT_MOUSEENTER )
    {
        if( IsEnabled() ) {
            m_uButtonState |= UISTATE_HOT;
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
	return CLabelUI::DoEvent(event);
}

void CRatingUI::DoPaint(void* ctx, const RECT& rcPaint)
{
	return CLabelUI::DoPaint(ctx, rcPaint);
}

void CRatingUI::PaintStatusImage(void* ctx)
{
    if( m_nMax <= m_nMin ) m_nMax = m_nMin + 1;
    if( m_nValue > m_nMax ) m_nValue = m_nMax;
    if( m_nValue < m_nMin ) m_nValue = m_nMin;

	int nValue = m_nValue;

	LONG nImageWidth = static_cast<LONG>( (m_rcItem.right - m_rcItem.left) / m_nMax );
	LONG nImageHeight = static_cast<LONG>( (m_rcItem.bottom - m_rcItem.top) / m_nMax );
	for( int index = 0; ( index < nValue ) && !m_sNormalImage.IsEmpty(); ++index )
	{
		RECT rc = {0};
		if( m_bHorizontal ) {
			rc.left = index * nImageWidth;
			rc.right = rc.left + nImageWidth;
			rc.bottom = m_rcItem.bottom - m_rcItem.top;
		}
		else {			
			rc.right = m_rcItem.right - m_rcItem.left;
			rc.bottom = m_rcItem.bottom - m_rcItem.top - index * nImageHeight;
			rc.top = m_rcItem.bottom - m_rcItem.top - ( index + 1 ) * nImageHeight;
		}

		if( !m_sNormalImage.IsEmpty() ) {
			CStdString sImageModify;
			sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), rc.left, rc.top, rc.right, rc.bottom);

			if( !DrawImage(ctx, (LPCTSTR)m_sNormalImage, (LPCTSTR)sImageModify) ) {
				m_sNormalImage.Empty();
				return;
			}
		}
	}

	for( int index = 0; ( index < m_nMax - nValue ) && !m_sDisabledImage.IsEmpty(); ++index )
	{
		RECT rc = {0};
		if( m_bHorizontal ) {
			rc.left = ( index + nValue ) * nImageWidth;
			rc.right = rc.left + nImageWidth;
			rc.bottom = m_rcItem.bottom - m_rcItem.top;
		}
		else {			
			rc.right = m_rcItem.right - m_rcItem.left;
			rc.bottom = m_rcItem.bottom - m_rcItem.top - ( index + nValue ) * nImageHeight;
			rc.top = m_rcItem.bottom - m_rcItem.top - ( index + nValue + 1 ) * nImageHeight;
		}

		if( !m_sDisabledImage.IsEmpty() ) {
			CStdString sImageModify;
			sImageModify.SmallFormat(_T("dest='%d,%d,%d,%d'"), rc.left, rc.top, rc.right, rc.bottom);

			if( !DrawImage(ctx, (LPCTSTR)m_sDisabledImage, (LPCTSTR)sImageModify) ) {
				m_sDisabledImage.Empty();
				return;
			}
		}
	}
}


} // namespace DuiLib
