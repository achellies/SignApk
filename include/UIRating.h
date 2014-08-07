#ifndef __UIRATING_H__
#define __UIRATING_H__

#ifdef _MSC_VER
#pragma once
#endif

namespace DuiLib {
/////////////////////////////////////////////////////////////////////////////////////
//

// RatingUI
extern const TCHAR* const kRatingUIClassName;// = _T("RatingUI");
extern const TCHAR* const kRatingUIInterfaceName;// = _T("Rating");

class CRatingUI : public CLabelUI
{
public:
    CRatingUI();

	~CRatingUI();

    LPCTSTR GetClass() const;
    LPVOID GetInterface(LPCTSTR pstrName);
    UINT GetControlFlags() const;

    bool IsHorizontal();
    void SetHorizontal(bool bHorizontal = true);
    int GetMinValue() const;
    void SetMinValue(int nMin);
    int GetMaxValue() const;
    void SetMaxValue(int nMax);
    int GetValue() const;
    void SetValue(int nValue);
    LPCTSTR GetNormalImage() const;
    void SetNormalImage(LPCTSTR pStrImage);
    LPCTSTR GetDisabledImage() const;
    void SetDisabledImage(LPCTSTR pStrImage);

    void DoEvent(TEventUI& event);
    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
    
    void DoPaint(void* ctx, const RECT& rcPaint);
    void PaintStatusImage(void* ctx);

protected:
    bool m_bHorizontal;
    int m_nMax;
    int m_nMin;
    int m_nValue;

	UINT m_uButtonState;

    CStdString m_sNormalImage;
    CStdString m_sDisabledImage;
};

} // namespace DuiLib

#endif // __UICOMBOEX_H__
