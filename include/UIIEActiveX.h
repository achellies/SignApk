#ifndef __UIIEACTIVEX_H__
#define __UIIEACTIVEX_H__

#pragma once
#include <mshtmhst.h>
#include "UIActiveX.h"
struct IOleObject;


namespace DuiLib {
/////////////////////////////////////////////////////////////////////////////////////
//

class CIEActiveXCtrl;

/////////////////////////////////////////////////////////////////////////////////////
//

class UILIB_API CIEActiveXUI : public CControlUI, public IMessageFilterUI
{
    friend CIEActiveXCtrl;
public:
    CIEActiveXUI();
    virtual ~CIEActiveXUI();

    LPCTSTR GetClass() const;
	LPVOID GetInterface(LPCTSTR pstrName);

    HWND GetHostWindow() const;

    bool IsDelayCreate() const;
    void SetDelayCreate(bool bDelayCreate = true);

    bool CreateControl(const CLSID clsid);
    bool CreateControl(LPCTSTR pstrCLSID);
    HRESULT GetControl(const IID iid, LPVOID* ppRet);
	CLSID GetClisd() const;
    CStdString GetModuleName() const;
    void SetModuleName(LPCTSTR pstrText);

    void SetVisible(bool bVisible = true);
    void SetInternVisible(bool bVisible = true);
    void SetPos(RECT rc);
    void DoPaint(HDC hDC, const RECT& rcPaint);

    void SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);
	void SetExternalUIHandler(IDocHostUIHandler* handler);

    LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

	BOOL Refresh();
	BOOL Stop();
	BOOL GoForward();
	BOOL GoBack();

	BOOL NavigateToURL(const WCHAR * pcszURL);
protected:
    void ReleaseControl();
    bool DoCreateControl();

protected:
    CLSID m_clsid;
    CStdString m_sModuleName;
    bool m_bCreated;
    bool m_bDelayCreate;
    IOleObject* m_pUnk;
    CIEActiveXCtrl* m_pControl;
    HWND m_hwndHost;
	IDocHostUIHandler* m_HostUIHandler;
};

} // namespace DuiLib

#endif // __UIIEACTIVEX_H__
