#include "StdAfx.h"

#include <ocidl.h>
#include <ExDisp.h>
#include <exdispid.h>
#include <Mshtml.h>
#include <atlbase.h>
#include <mshtmdid.h>

/*
DEFINE_GUID(CLSID_WebBrowser,       0x8856F961L, 0x340A, 0x11D0, 0xA9, 0x6B, 0x00, 0xC0, 0x4F, 0xD7, 0x05, 0xA2);
DEFINE_GUID(IID_IWebBrowser,        0xEAB22AC1L, 0x30C1, 0x11CF, 0xA7, 0xEB, 0x00, 0x00, 0xC0, 0x5B, 0xAE, 0x0B);
DEFINE_GUID(IID_IWebBrowser2,       0xD30C1661L, 0xCDAF, 0x11D0, 0x8A, 0x3E, 0x00, 0xC0, 0x4F, 0xC9, 0xE2, 0x6E);
DEFINE_GUID(DIID_DWebBrowserEvents, 0xEAB22AC2L, 0x30C1, 0x11CF, 0xA7, 0xEB, 0x00, 0x00, 0xC0, 0x5B, 0xAE, 0x0B);
DEFINE_GUID(DIID_DWebBrowserEvents2, 0x34A715A0L, 0x6587, 0x11D0, 0x92, 0x4A, 0x00, 0x20, 0xAF, 0xC7, 0xAC, 0x4D);
DEFINE_GUID(IID_IWebBrowserApp,     0x0002DF05L, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);

const GUID SID_SDocHost = { 0xc6504990, 0xd43e, 0x11cf, { 0x89, 0x3b, 0x00, 0xaa, 0x00, 0xbd, 0xce, 0x1a}};
*/
//------------------------------------------------------------------------
//Macro define
#define MAX_URL  2048

namespace DuiLib {

/////////////////////////////////////////////////////////////////////////////////////
//
//

class CIEActiveXCtrl;


/////////////////////////////////////////////////////////////////////////////////////
//
//

class CIEActiveXWnd : public CWindowWnd
{
public:
    HWND Init(CIEActiveXCtrl* pOwner, HWND hWndParent);

    LPCTSTR GetWindowClassName() const;
    void OnFinalMessage(HWND hWnd);

    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
    void DoVerb(LONG iVerb);

    LRESULT OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

protected:
    CIEActiveXCtrl* m_pOwner;
};


/////////////////////////////////////////////////////////////////////////////////////
//
//

class CIEActiveXEnum : public IEnumUnknown
{
public:
    CIEActiveXEnum(IUnknown* pUnk) : m_pUnk(pUnk), m_dwRef(1), m_iPos(0)
    {
        m_pUnk->AddRef();
    }
    ~CIEActiveXEnum()
    {
        m_pUnk->Release();
    }

    LONG m_iPos;
    ULONG m_dwRef;
    IUnknown* m_pUnk;

    STDMETHOD_(ULONG,AddRef)()
    {
        return ++m_dwRef;
    }
    STDMETHOD_(ULONG,Release)()
    {
        LONG lRef = --m_dwRef;
        if( lRef == 0 ) delete this;
        return lRef;
    }
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppvObject)
    {
        *ppvObject = NULL;
        if( riid == IID_IUnknown ) *ppvObject = static_cast<IEnumUnknown*>(this);
        else if( riid == IID_IEnumUnknown ) *ppvObject = static_cast<IEnumUnknown*>(this);
        if( *ppvObject != NULL ) AddRef();
        return *ppvObject == NULL ? E_NOINTERFACE : S_OK;
    }
    STDMETHOD(Next)(ULONG celt, IUnknown **rgelt, ULONG *pceltFetched)
    {
        if( pceltFetched != NULL ) *pceltFetched = 0;
        if( ++m_iPos > 1 ) return S_FALSE;
        *rgelt = m_pUnk;
        (*rgelt)->AddRef();
        if( pceltFetched != NULL ) *pceltFetched = 1;
        return S_OK;
    }
    STDMETHOD(Skip)(ULONG celt)
    {
        m_iPos += celt;
        return S_OK;
    }
    STDMETHOD(Reset)(void)
    {
        m_iPos = 0;
        return S_OK;
    }
    STDMETHOD(Clone)(IEnumUnknown **ppenum)
    {
        return E_NOTIMPL;
    }
};


/////////////////////////////////////////////////////////////////////////////////////
//
//

class CIEActiveXFrameWnd : public IOleInPlaceFrame
{
public:
    CIEActiveXFrameWnd(CIEActiveXUI* pOwner) : m_dwRef(1), m_pOwner(pOwner), m_pActiveObject(NULL)
    {
    }
    ~CIEActiveXFrameWnd()
    {
        if( m_pActiveObject != NULL ) m_pActiveObject->Release();
    }

    ULONG m_dwRef;
    CIEActiveXUI* m_pOwner;
    IOleInPlaceActiveObject* m_pActiveObject;

    // IUnknown
    STDMETHOD_(ULONG,AddRef)()
    {
        return ++m_dwRef;
    }
    STDMETHOD_(ULONG,Release)()
    {
        ULONG lRef = --m_dwRef;
        if( lRef == 0 ) delete this;
        return lRef;
    }
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppvObject)
    {
        *ppvObject = NULL;
        if( riid == IID_IUnknown ) *ppvObject = static_cast<IOleInPlaceFrame*>(this);
        else if( riid == IID_IOleWindow ) *ppvObject = static_cast<IOleWindow*>(this);
        else if( riid == IID_IOleInPlaceFrame ) *ppvObject = static_cast<IOleInPlaceFrame*>(this);
        else if( riid == IID_IOleInPlaceUIWindow ) *ppvObject = static_cast<IOleInPlaceUIWindow*>(this);
        if( *ppvObject != NULL ) AddRef();
        return *ppvObject == NULL ? E_NOINTERFACE : S_OK;
    }  
    // IOleInPlaceFrameWindow
    STDMETHOD(InsertMenus)(HMENU /*hmenuShared*/, LPOLEMENUGROUPWIDTHS /*lpMenuWidths*/)
    {
        return S_OK;
    }
    STDMETHOD(SetMenu)(HMENU /*hmenuShared*/, HOLEMENU /*holemenu*/, HWND /*hwndActiveObject*/)
    {
        return S_OK;
    }
    STDMETHOD(RemoveMenus)(HMENU /*hmenuShared*/)
    {
        return S_OK;
    }
    STDMETHOD(SetStatusText)(LPCOLESTR /*pszStatusText*/)
    {
        return S_OK;
    }
    STDMETHOD(EnableModeless)(BOOL /*fEnable*/)
    {
        return S_OK;
    }
    STDMETHOD(TranslateAccelerator)(LPMSG /*lpMsg*/, WORD /*wID*/)
    {
        return S_FALSE;
    }
    // IOleWindow
    STDMETHOD(GetWindow)(HWND* phwnd)
    {
        if( m_pOwner == NULL ) return E_UNEXPECTED;
        *phwnd = m_pOwner->GetManager()->GetPaintWindow();
        return S_OK;
    }
    STDMETHOD(ContextSensitiveHelp)(BOOL /*fEnterMode*/)
    {
        return S_OK;
    }
    // IOleInPlaceUIWindow
    STDMETHOD(GetBorder)(LPRECT /*lprectBorder*/)
    {
        return S_OK;
    }
    STDMETHOD(RequestBorderSpace)(LPCBORDERWIDTHS /*pborderwidths*/)
    {
        return INPLACE_E_NOTOOLSPACE;
    }
    STDMETHOD(SetBorderSpace)(LPCBORDERWIDTHS /*pborderwidths*/)
    {
        return S_OK;
    }
    STDMETHOD(SetActiveObject)(IOleInPlaceActiveObject* pActiveObject, LPCOLESTR /*pszObjName*/)
    {
        if( pActiveObject != NULL ) pActiveObject->AddRef();
        if( m_pActiveObject != NULL ) m_pActiveObject->Release();
        m_pActiveObject = pActiveObject;
        return S_OK;
    }
};

/////////////////////////////////////////////////////////////////////////////////////
//
class CHtmlDocumentEventSink : public HTMLDocumentEvents2, public HTMLFormElementEvents2
{
public:
    CHtmlDocumentEventSink();
    ~CHtmlDocumentEventSink();

	// IUnknown
    STDMETHOD_(ULONG,AddRef)();
    STDMETHOD_(ULONG,Release)();
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppvObject);

	// IDispatch methods
	// HTMLDocumentEvents2
	STDMETHOD(GetTypeInfoCount)(UINT FAR* pctinfo);
	STDMETHOD(GetTypeInfo)(UINT itinfo,LCID lcid,ITypeInfo FAR* FAR* pptinfo);
	STDMETHOD(GetIDsOfNames)(REFIID riid,OLECHAR FAR* FAR* rgszNames,UINT cNames,LCID lcid, DISPID FAR* rgdispid);
	STDMETHOD(Invoke)(DISPID dispidMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS FAR* pdispparams, VARIANT FAR* pvarResult,EXCEPINFO FAR* pexcepinfo,UINT FAR* puArgErr);

	HRESULT DefHtmlDocumentEventsInvokeProc(DISPID dispidMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS FAR* pdispparams, VARIANT FAR* pvarResult,EXCEPINFO FAR* pexcepinfo,UINT FAR* puArgErr);

private:
	LONG m_dwRef;
	DWORD m_dwFormEventCookie;
	IConnectionPoint* m_pCPFormEventSink;
};

class CIEActiveXCtrl :
    public IOleClientSite,
    public IOleInPlaceSiteWindowless,
    public IOleControlSite,
    public IObjectWithSite,
    public IOleContainer,
	public IDocHostUIHandler,
	public DWebBrowserEvents2
{
    friend CIEActiveXUI;
    friend CIEActiveXWnd;
public:
    CIEActiveXCtrl();
    ~CIEActiveXCtrl();

    // IUnknown
    STDMETHOD_(ULONG,AddRef)();
    STDMETHOD_(ULONG,Release)();
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppvObject);

    // IObjectWithSite
    STDMETHOD(SetSite)(IUnknown *pUnkSite);
    STDMETHOD(GetSite)(REFIID riid, LPVOID* ppvSite);

    // IOleClientSite
    STDMETHOD(SaveObject)(void);       
    STDMETHOD(GetMoniker)(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker** ppmk);
    STDMETHOD(GetContainer)(IOleContainer** ppContainer);        
    STDMETHOD(ShowObject)(void);        
    STDMETHOD(OnShowWindow)(BOOL fShow);        
    STDMETHOD(RequestNewObjectLayout)(void);

    // IOleInPlaceSiteWindowless
    STDMETHOD(CanWindowlessActivate)(void);
    STDMETHOD(GetCapture)(void);
    STDMETHOD(SetCapture)(BOOL fCapture);
    STDMETHOD(GetFocus)(void);
    STDMETHOD(SetFocus)(BOOL fFocus);
    STDMETHOD(GetDC)(LPCRECT pRect, DWORD grfFlags, HDC* phDC);
    STDMETHOD(ReleaseDC)(HDC hDC);
    STDMETHOD(InvalidateRect)(LPCRECT pRect, BOOL fErase);
    STDMETHOD(InvalidateRgn)(HRGN hRGN, BOOL fErase);
    STDMETHOD(ScrollRect)(INT dx, INT dy, LPCRECT pRectScroll, LPCRECT pRectClip);
    STDMETHOD(AdjustRect)(LPRECT prc);
    STDMETHOD(OnDefWindowMessage)(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* plResult);

    // IOleInPlaceSiteEx
    STDMETHOD(OnInPlaceActivateEx)(BOOL *pfNoRedraw, DWORD dwFlags);        
    STDMETHOD(OnInPlaceDeactivateEx)(BOOL fNoRedraw);       
    STDMETHOD(RequestUIActivate)(void);

    // IOleInPlaceSite
    STDMETHOD(CanInPlaceActivate)(void);       
    STDMETHOD(OnInPlaceActivate)(void);        
    STDMETHOD(OnUIActivate)(void);
    STDMETHOD(GetWindowContext)(IOleInPlaceFrame** ppFrame, IOleInPlaceUIWindow** ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo);
    STDMETHOD(Scroll)(SIZE scrollExtant);
    STDMETHOD(OnUIDeactivate)(BOOL fUndoable);
    STDMETHOD(OnInPlaceDeactivate)(void);
    STDMETHOD(DiscardUndoState)( void);
    STDMETHOD(DeactivateAndUndo)( void);
    STDMETHOD(OnPosRectChange)(LPCRECT lprcPosRect);

    // IOleWindow
    STDMETHOD(GetWindow)(HWND* phwnd);
    STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode);

    // IOleControlSite
    STDMETHOD(OnControlInfoChanged)(void);      
    STDMETHOD(LockInPlaceActive)(BOOL fLock);       
    STDMETHOD(GetExtendedControl)(IDispatch** ppDisp);        
    STDMETHOD(TransformCoords)(POINTL* pPtlHimetric, POINTF* pPtfContainer, DWORD dwFlags);       
    STDMETHOD(TranslateAccelerator)(MSG* pMsg, DWORD grfModifiers);
    STDMETHOD(OnFocus)(BOOL fGotFocus);
    STDMETHOD(ShowPropertyFrame)(void);

    // IOleContainer
    STDMETHOD(EnumObjects)(DWORD grfFlags, IEnumUnknown** ppenum);
    STDMETHOD(LockContainer)(BOOL fLock);

    // IParseDisplayName
    STDMETHOD(ParseDisplayName)(IBindCtx* pbc, LPOLESTR pszDisplayName, ULONG* pchEaten, IMoniker** ppmkOut);

	// IDocHostUIHandler
	STDMETHOD(ShowContextMenu)(DWORD dwID, POINT* pptPosition, IUnknown* pCommandTarget, IDispatch* pDispatchObjectHit);
	STDMETHOD(GetHostInfo)(DOCHOSTUIINFO* pInfo);
	STDMETHOD(ShowUI)(DWORD dwID, IOleInPlaceActiveObject* pActiveObject, IOleCommandTarget* pCommandTarget, IOleInPlaceFrame* pFrame, IOleInPlaceUIWindow* pDoc);
	STDMETHOD(HideUI)();
	STDMETHOD(UpdateUI)();
	STDMETHOD(EnableModeless)(BOOL fEnable);
	STDMETHOD(OnDocWindowActivate)(BOOL fActivate);
	STDMETHOD(OnFrameWindowActivate)(BOOL fActivate);
	STDMETHOD(ResizeBorder)(LPCRECT prcBorder, IOleInPlaceUIWindow* pUIWindow, BOOL fFrameWindow);
	STDMETHOD(TranslateAccelerator)(LPMSG lpMsg, const GUID* pguidCmdGroup, DWORD nCmdID);
	STDMETHOD(GetOptionKeyPath)(LPOLESTR* pchKey, DWORD dwReserved);
	STDMETHOD(GetDropTarget)(IDropTarget* pDropTarget, IDropTarget** ppDropTarget);
	STDMETHOD(GetExternal)(IDispatch** ppDispatch);
	STDMETHOD(TranslateUrl)(DWORD dwTranslate, OLECHAR* pchURLIn, OLECHAR** ppchURLOut);
	STDMETHOD(FilterDataObject)(IDataObject* pDO, IDataObject** ppDORet);
	
	//DWebBrowserEvents2
	//IDispatch methods
	STDMETHOD(GetTypeInfoCount)(UINT FAR* pctinfo);
	STDMETHOD(GetTypeInfo)(UINT itinfo,LCID lcid,ITypeInfo FAR* FAR* pptinfo);
	STDMETHOD(GetIDsOfNames)(REFIID riid,OLECHAR FAR* FAR* rgszNames,UINT cNames,LCID lcid, DISPID FAR* rgdispid);
	STDMETHOD(Invoke)(DISPID dispidMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS FAR* pdispparams, VARIANT FAR* pvarResult,EXCEPINFO FAR* pexcepinfo,UINT FAR* puArgErr);

	HRESULT DefDWebBrowserEventInvokeProc(DISPID dispidMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS FAR* pdispparams, VARIANT FAR* pvarResult,EXCEPINFO FAR* pexcepinfo,UINT FAR* puArgErr);
protected:
    HRESULT CreateActiveXWnd();

	HRESULT RegisterBrowserEventSink();

	HRESULT AutoSubmitForm(IHTMLDocument2* pDocument, LPCTSTR lpstrUserName, LPCTSTR lpstrPassword);

	HRESULT EnumJavascripts(IHTMLDocument2* pDocument);

protected:
    LONG m_dwRef;
    CIEActiveXUI* m_pOwner;
    CIEActiveXWnd* m_pWindow;
    IUnknown* m_pUnkSite;
    IViewObject* m_pViewObject;
    IOleInPlaceObjectWindowless* m_pInPlaceObject;
	IWebBrowser2* m_pWB2;
	IConnectionPoint *m_pCPWebBrowserEventSink;
	IConnectionPoint *m_pCPDocumentEventSink;
	IConnectionPoint *m_pCPFormEventSink;
	IOleObject *m_pOleObj;

	CHtmlDocumentEventSink *m_pHtmlDocEventSink;

	DWORD m_dwWebBrowserEventCookie;
	DWORD m_dwDocumentEventCookie;
	DWORD m_dwFormEventCookie;

    bool m_bLocked;
    bool m_bFocused;
    bool m_bCaptured;
    bool m_bUIActivated;
    bool m_bInPlaceActive;
    bool m_bWindowless;

	CStdString m_navigateURL;
};

CHtmlDocumentEventSink::CHtmlDocumentEventSink() : 
m_dwRef(1),
m_dwFormEventCookie(0),
m_pCPFormEventSink(NULL)
{
}

CHtmlDocumentEventSink::~CHtmlDocumentEventSink()
{
	if( m_pCPFormEventSink != NULL && m_dwFormEventCookie != NULL )
	{
		m_pCPFormEventSink->Unadvise(m_dwFormEventCookie);	
	}
	if( m_pCPFormEventSink != NULL) m_pCPFormEventSink->Release();
}

STDMETHODIMP CHtmlDocumentEventSink::QueryInterface(REFIID riid, LPVOID *ppvObject)
{
    *ppvObject = NULL;
    if( riid == IID_IUnknown )                       *ppvObject = static_cast<HTMLDocumentEvents2*>(this);
	else if( riid == DIID_HTMLDocumentEvents2 )		 *ppvObject = static_cast<HTMLDocumentEvents2*>(this);
	else if( riid == DIID_HTMLFormElementEvents2 )	 *ppvObject = static_cast<HTMLFormElementEvents2*>(this);
	else if( riid == IID_IDispatch )				 *ppvObject = static_cast<HTMLDocumentEvents2*>(this);
    if( *ppvObject != NULL ) AddRef();
    return *ppvObject == NULL ? E_NOINTERFACE : S_OK;
}

STDMETHODIMP_(ULONG) CHtmlDocumentEventSink::AddRef()
{
    return ++m_dwRef;
}

STDMETHODIMP_(ULONG) CHtmlDocumentEventSink::Release()
{
    LONG lRef = --m_dwRef;
    if( lRef == 0 ) delete this;
    return lRef;
}


//---------------------------------------------------------------------------------
//Description:
// HTMLDocumentEvents2 and IDispatch methods
//
//----------------------------------------------------------------------------------
STDMETHODIMP CHtmlDocumentEventSink::GetTypeInfoCount(UINT FAR* pctinfo)          
{ 
	return E_NOTIMPL;
}


//---------------------------------------------------------------------------------
//Description:
// HTMLDocumentEvents2 and IDispatch methods
//
//----------------------------------------------------------------------------------
STDMETHODIMP CHtmlDocumentEventSink::GetTypeInfo(UINT itinfo,LCID lcid,ITypeInfo FAR* FAR* pptinfo)   
{ 
	return E_NOTIMPL;
}


//---------------------------------------------------------------------------------
//Description:
// HTMLDocumentEvents2 and IDispatch methods
//
//----------------------------------------------------------------------------------
STDMETHODIMP CHtmlDocumentEventSink::GetIDsOfNames(REFIID riid,OLECHAR FAR* FAR* rgszNames,UINT cNames,LCID lcid, DISPID FAR* rgdispid)                  
{ 
	return E_NOTIMPL;
}


//---------------------------------------------------------------------------------
//Description:
// HTMLDocumentEvents2 and IDispatch methods
//
//----------------------------------------------------------------------------------
STDMETHODIMP CHtmlDocumentEventSink::Invoke(DISPID dispidMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS FAR* pdispparams, VARIANT FAR* pvarResult,EXCEPINFO FAR* pexcepinfo,UINT FAR* puArgErr)    
{
	return DefHtmlDocumentEventsInvokeProc(dispidMember,riid,lcid,wFlags,pdispparams, pvarResult,pexcepinfo,puArgErr);
}

//---------------------------------------------------------------------
//Description:
// My default DWebBroserEvent Invoke interface process
//
//----------------------------------------------------------------------
HRESULT CHtmlDocumentEventSink::DefHtmlDocumentEventsInvokeProc(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr)
{
	TCHAR szBuf[MAX_PATH];
	_stprintf(szBuf, _T("dispidMember = %d.\n"), dispidMember);
	OutputDebugString(szBuf);

	switch (dispidMember)
	{
	//////////////////////////////////////////////////////////////
	// HTMLFormElementEvents2
	//////////////////////////////////////////////////////////////
	case DISPID_IHTMLFORMELEMENT_ONSUBMIT:
	case DISPID_IHTMLFORMELEMENT_SUBMIT:
	case DISPID_ONSUBMIT:
		{
			int i = 0;
			++i;
			return E_NOTIMPL;
		}
	case DISPID_HTMLELEMENTEVENTS2_ONAFTERUPDATE:
		{
			int i = 0;
			++i;
			return E_NOTIMPL;
		}
	case DISPID_HTMLELEMENTEVENTS2_ONBEFOREUPDATE:
		{
			int i = 0;
			++i;
			return E_NOTIMPL;
		}
	//////////////////////////////////////////////////////////////
	// HTMLDocumentEvents2
	//////////////////////////////////////////////////////////////
	case DISPID_HTMLDOCUMENTEVENTS2_ONCLICK:
		{
			// IHTMLDocument2接口你应该拿到了吧，响应DISPID_HTMLDOCUMENTEVENTS_ONCLICK消息，用IHTMLDocument2的get_activeElement拿到IHTMLElement*，
			// 这个自然是当前active的element，再用IHTMLElement的get_id方法就能拿到id了
			HRESULT hr = S_OK;
			ASSERT(pdispparams->cArgs == 1);
			ASSERT(pdispparams->rgvarg[0].vt == VT_DISPATCH);
			IDispatch* pDispatch = pdispparams->rgvarg[0].pdispVal;
			if (pDispatch != NULL)
			{
				IHTMLEventObj *pEventObj = NULL;
				IHTMLElement *pElement = NULL;

				hr = pDispatch->QueryInterface(IID_IHTMLEventObj, (LPVOID *)&pEventObj);
				ASSERT(SUCCEEDED(hr));

				hr = pEventObj->get_srcElement(&pElement);
				ASSERT(SUCCEEDED(hr));

				BSTR bstrId;
				hr = pElement->get_id(&bstrId);
				ASSERT(SUCCEEDED(hr));
				if (bstrId != NULL && wcsicmp(bstrId, L"loginsubmit") == 0)
				{
					SysFreeString(bstrId);

					IHTMLDocument2* pDocument = NULL;
					IDispatch* pDocumentDispatch = NULL;
					hr = pElement->get_document(&pDocumentDispatch);
					ASSERT(SUCCEEDED(hr));

					hr = pDocumentDispatch->QueryInterface(IID_IHTMLDocument2, (LPVOID *)&pDocument);
					ASSERT(SUCCEEDED(hr));

					BSTR bstrUserName = NULL;
					BSTR bstrPassword = NULL;
					{
						// 获取密码、账号
						HRESULT hr = S_FALSE;
						BSTR bstrName, bstrValue, bstrType;

						bstrName = SysAllocString(L"name");
						bstrValue = SysAllocString(L"value");
						bstrType = SysAllocString(L"type");

						IHTMLElementCollection* pElementCollection;
						hr = pDocument->get_forms(&pElementCollection);
						ASSERT(SUCCEEDED(hr));

						long nFormCount=0;

						//获取表单数目
						hr = pElementCollection->get_length(&nFormCount);
						ASSERT(SUCCEEDED(hr));

						for (long i = 0; i < nFormCount; ++i)
						{
							//取得第 i 项表单
							IDispatch *pDisp = NULL;

							VARIANT vParam;
							vParam.vt = VT_I4;
							vParam.lVal = i;
							hr = pElementCollection->item(vParam, vParam, &pDisp);
							if (FAILED(hr))
								goto LOOP_1_CLEAN_UP;

							IHTMLFormElement * pFormElement = NULL;
							hr = pDisp->QueryInterface(IID_IHTMLFormElement, (LPVOID*)&pFormElement);
							if (FAILED(hr))
								goto LOOP_1_CLEAN_UP;

#pragma region 监听HTMLFormElementEvents
							//{
							//	IConnectionPointContainer	*pCPCont = NULL;
							//	HTMLFormElementEvents2 *pEvents = NULL;

							//	// Get the connection point container for the browser object.
							//	hr = pFormElement->QueryInterface(IID_IConnectionPointContainer, (LPVOID *)&pCPCont);
							//	ASSERT(SUCCEEDED(hr));

							//	if( m_pCPFormEventSink != NULL && m_dwFormEventCookie != NULL )
							//	{
							//		m_pCPFormEventSink->Unadvise(m_dwFormEventCookie);	
							//	}

							//	if( m_pCPFormEventSink != NULL )
							//		m_pCPFormEventSink->Release();

							//	m_pCPFormEventSink = NULL;
							//	m_dwFormEventCookie = 0;

							//	// Look for DWebBrowserEvents2 connection point.
							//	hr = pCPCont->FindConnectionPoint(DIID_HTMLFormElementEvents2, &m_pCPFormEventSink);
							//	ASSERT(SUCCEEDED(hr));

							//	// Get a DWebBrowserEvents2 pointer from the browser.
							//	hr = QueryInterface(DIID_HTMLFormElementEvents2, (LPVOID *)(&pEvents));
							//	ASSERT(SUCCEEDED(hr));

							//	// Add your event sink to the connectionpoint.
							//	hr = m_pCPFormEventSink->Advise(pEvents, &(m_dwFormEventCookie));
							//	ASSERT(SUCCEEDED(hr));

							//	pEvents->Release();
							//	pCPCont->Release();
							//}
#pragma endregion

							//取得表单中 域的数目
							long nElemCount=0;
							hr = pFormElement->get_length(&nElemCount);
							if (FAILED(hr))
								goto LOOP_1_CLEAN_UP;

							for (long j = 0; j < nElemCount; ++j)
							{
								//取得第 j 项表单域
								VARIANT vParam;
								vParam.vt = VT_I4;
								vParam.lVal = j;
								IDispatch* pDispatchElement = NULL;
								hr = pFormElement->item(vParam, vParam, &pDispatchElement);
								if (FAILED(hr))
									goto LOOP_2_CLEAN_UP;

								IHTMLElement* pInputElement = NULL;
								hr = pDispatchElement->QueryInterface(IID_IHTMLElement, (LPVOID*)&pInputElement);
								//取得表单域的 名，值，类型
								VARIANT vName, vVal, vType;

								hr = pInputElement->getAttribute(bstrName, 0, &vName);
								if (FAILED(hr))
									goto LOOP_2_CLEAN_UP;

								hr = pInputElement->getAttribute(bstrValue, 0, &vVal);
								if (FAILED(hr))
									goto LOOP_2_CLEAN_UP;

								hr = pInputElement->getAttribute(bstrType, 0, &vType);
								if (FAILED(hr))
									goto LOOP_2_CLEAN_UP;

								//未知域名
								LPCTSTR lpName = vName.bstrVal ? OLE2CT(vName.bstrVal) : _T("");

								//空值，未输入
								LPCTSTR lpVal  = vVal.bstrVal ? OLE2CT(vVal.bstrVal) : _T("");

								//未知类型
								LPCTSTR lpType = vType.bstrVal ? OLE2CT(vType.bstrVal) : _T("");

								//向用户名文本框填值
								if (wcsicmp(lpName, L"loginname") == 0)
								{
									bstrUserName = SysAllocString(lpVal);
								}
								//向密码文本框填值
								else if (wcsicmp(lpName, L"loginpwd") == 0)
								{
									bstrPassword = SysAllocString(lpVal);
								}
LOOP_2_CLEAN_UP:
								pDispatchElement->Release();
							} // for (long j = 0; j < nElemCount; ++j)
LOOP_1_CLEAN_UP:
							pDisp->Release();
						} // for (long i = 0; i < nFormCount; ++i)

						SysFreeString(bstrName);
						SysFreeString(bstrValue);
						SysFreeString(bstrType);
					}
					if (bstrUserName != NULL)
						SysFreeString(bstrUserName);
					if (bstrPassword != NULL)
						SysFreeString(bstrPassword);


					pDocument->Release();
					pDocumentDispatch->Release();

				}

				pElement->Release();
				pEventObj->Release();
			}
			return E_NOTIMPL;
		}
		break;
	default:
		return E_NOTIMPL;
	}

	return E_NOTIMPL;
}

CIEActiveXCtrl::CIEActiveXCtrl() : 
m_dwRef(1), 
m_pOwner(NULL), 
m_pWindow(NULL),
m_pUnkSite(NULL), 
m_pViewObject(NULL),
m_pInPlaceObject(NULL),
m_pHtmlDocEventSink(NULL),
m_pCPDocumentEventSink(NULL),
m_pCPFormEventSink(NULL),
m_bLocked(false), 
m_bFocused(false),
m_bCaptured(false),
m_bWindowless(true),
m_bUIActivated(false),
m_bInPlaceActive(false),
m_pWB2(NULL),
m_pCPWebBrowserEventSink(NULL),
m_pOleObj(NULL),
m_dwDocumentEventCookie(0),
m_dwWebBrowserEventCookie(0),
m_dwFormEventCookie(0)
{
	m_pHtmlDocEventSink = new CHtmlDocumentEventSink();
}

CIEActiveXCtrl::~CIEActiveXCtrl()
{
    if( m_pWindow != NULL ) {
        ::DestroyWindow(*m_pWindow);
        delete m_pWindow;
	}

	if( m_pCPFormEventSink != NULL && m_dwFormEventCookie != NULL )
	{
		m_pCPFormEventSink->Unadvise(m_dwFormEventCookie);
	}

	if( m_pCPDocumentEventSink != NULL && m_dwDocumentEventCookie != NULL )
	{
		m_pCPDocumentEventSink->Unadvise(m_dwDocumentEventCookie);
	}

	if( m_pCPWebBrowserEventSink != NULL && m_dwWebBrowserEventCookie != NULL )
	{
		m_pCPWebBrowserEventSink->Unadvise(m_dwWebBrowserEventCookie);
	}

	if (m_pHtmlDocEventSink != NULL)
	{
		delete m_pHtmlDocEventSink;
		m_pHtmlDocEventSink = NULL;
	}

    if( m_pUnkSite != NULL ) m_pUnkSite->Release();
    if( m_pViewObject != NULL ) m_pViewObject->Release();
    if( m_pInPlaceObject != NULL ) m_pInPlaceObject->Release();
	if( m_pWB2 != NULL ) m_pWB2->Release();
	if( m_pCPWebBrowserEventSink != NULL ) m_pCPWebBrowserEventSink->Release();
	if( m_pCPDocumentEventSink != NULL) m_pCPDocumentEventSink->Release();
	if( m_pCPFormEventSink != NULL) m_pCPFormEventSink->Release();
}

STDMETHODIMP CIEActiveXCtrl::QueryInterface(REFIID riid, LPVOID *ppvObject)
{
    *ppvObject = NULL;
    if( riid == IID_IUnknown )                       *ppvObject = static_cast<IOleWindow*>(this);
    else if( riid == IID_IOleClientSite )            *ppvObject = static_cast<IOleClientSite*>(this);
    else if( riid == IID_IOleInPlaceSiteWindowless ) *ppvObject = static_cast<IOleInPlaceSiteWindowless*>(this);
    else if( riid == IID_IOleInPlaceSiteEx )         *ppvObject = static_cast<IOleInPlaceSiteEx*>(this);
    else if( riid == IID_IOleInPlaceSite )           *ppvObject = static_cast<IOleInPlaceSite*>(this);
    else if( riid == IID_IOleWindow )                *ppvObject = static_cast<IOleWindow*>(this);
    else if( riid == IID_IOleControlSite )           *ppvObject = static_cast<IOleControlSite*>(this);
    else if( riid == IID_IOleContainer )             *ppvObject = static_cast<IOleContainer*>(this);
    else if( riid == IID_IObjectWithSite )           *ppvObject = static_cast<IObjectWithSite*>(this);
	else if( riid == IID_IDocHostUIHandler)			 *ppvObject = static_cast<IDocHostUIHandler*>(this);
	else if( riid == DIID_DWebBrowserEvents2 )		 *ppvObject = static_cast<DWebBrowserEvents2*>(this); 
	else if( riid == IID_IDispatch )				 *ppvObject = static_cast<DWebBrowserEvents2*>(this);
    if( *ppvObject != NULL ) AddRef();
    return *ppvObject == NULL ? E_NOINTERFACE : S_OK;
}

STDMETHODIMP_(ULONG) CIEActiveXCtrl::AddRef()
{
    return ++m_dwRef;
}

STDMETHODIMP_(ULONG) CIEActiveXCtrl::Release()
{
    LONG lRef = --m_dwRef;
    if( lRef == 0 ) delete this;
    return lRef;
}

STDMETHODIMP CIEActiveXCtrl::SetSite(IUnknown *pUnkSite)
{
    TRACE(_T("AX: CIEActiveXCtrl::SetSite"));
    if( m_pUnkSite != NULL ) {
        m_pUnkSite->Release();
        m_pUnkSite = NULL;
    }
    if( pUnkSite != NULL ) {
        m_pUnkSite = pUnkSite;
        m_pUnkSite->AddRef();
    }
    return S_OK;
}

STDMETHODIMP CIEActiveXCtrl::GetSite(REFIID riid, LPVOID* ppvSite)
{
    TRACE(_T("AX: CIEActiveXCtrl::GetSite"));
    if( ppvSite == NULL ) return E_POINTER;
    *ppvSite = NULL;
    if( m_pUnkSite == NULL ) return E_FAIL;
    return m_pUnkSite->QueryInterface(riid, ppvSite);
}

STDMETHODIMP CIEActiveXCtrl::SaveObject(void)
{
    TRACE(_T("AX: CIEActiveXCtrl::SaveObject"));
    return E_NOTIMPL;
}

STDMETHODIMP CIEActiveXCtrl::GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker** ppmk)
{
    TRACE(_T("AX: CIEActiveXCtrl::GetMoniker"));
    if( ppmk != NULL ) *ppmk = NULL;
    return E_NOTIMPL;
}

STDMETHODIMP CIEActiveXCtrl::GetContainer(IOleContainer** ppContainer)
{
    TRACE(_T("AX: CIEActiveXCtrl::GetContainer"));
    if( ppContainer == NULL ) return E_POINTER;
    *ppContainer = NULL;
    HRESULT Hr = E_NOTIMPL;
    if( m_pUnkSite != NULL ) Hr = m_pUnkSite->QueryInterface(IID_IOleContainer, (LPVOID*) ppContainer);
    if( FAILED(Hr) ) Hr = QueryInterface(IID_IOleContainer, (LPVOID*) ppContainer);
    return Hr;
}

STDMETHODIMP CIEActiveXCtrl::ShowObject(void)
{
    TRACE(_T("AX: CIEActiveXCtrl::ShowObject"));
    if( m_pOwner == NULL ) return E_UNEXPECTED;
    HDC hDC = ::GetDC(m_pOwner->m_hwndHost);
    if( hDC == NULL ) return E_FAIL;
    if( m_pViewObject != NULL ) m_pViewObject->Draw(DVASPECT_CONTENT, -1, NULL, NULL, NULL, hDC, (RECTL*) &m_pOwner->m_rcItem, (RECTL*) &m_pOwner->m_rcItem, NULL, NULL);
    ::ReleaseDC(m_pOwner->m_hwndHost, hDC);
    return S_OK;
}

STDMETHODIMP CIEActiveXCtrl::OnShowWindow(BOOL fShow)
{
    TRACE(_T("AX: CIEActiveXCtrl::OnShowWindow"));
    return E_NOTIMPL;
}

STDMETHODIMP CIEActiveXCtrl::RequestNewObjectLayout(void)
{
    TRACE(_T("AX: CIEActiveXCtrl::RequestNewObjectLayout"));
    return E_NOTIMPL;
}

STDMETHODIMP CIEActiveXCtrl::CanWindowlessActivate(void)
{
    TRACE(_T("AX: CIEActiveXCtrl::CanWindowlessActivate"));
    return S_OK;  // Yes, we can!!
}

STDMETHODIMP CIEActiveXCtrl::GetCapture(void)
{
    TRACE(_T("AX: CIEActiveXCtrl::GetCapture"));
    if( m_pOwner == NULL ) return E_UNEXPECTED;
    return m_bCaptured ? S_OK : S_FALSE;
}

STDMETHODIMP CIEActiveXCtrl::SetCapture(BOOL fCapture)
{
    TRACE(_T("AX: CIEActiveXCtrl::SetCapture"));
    if( m_pOwner == NULL ) return E_UNEXPECTED;
    m_bCaptured = (fCapture == TRUE);
    if( fCapture ) ::SetCapture(m_pOwner->m_hwndHost); else ::ReleaseCapture();
    return S_OK;
}

STDMETHODIMP CIEActiveXCtrl::GetFocus(void)
{
    TRACE(_T("AX: CIEActiveXCtrl::GetFocus"));
    if( m_pOwner == NULL ) return E_UNEXPECTED;
    return m_bFocused ? S_OK : S_FALSE;
}

STDMETHODIMP CIEActiveXCtrl::SetFocus(BOOL fFocus)
{
    TRACE(_T("AX: CIEActiveXCtrl::SetFocus"));
    if( m_pOwner == NULL ) return E_UNEXPECTED;
    if( fFocus ) m_pOwner->SetFocus();
    m_bFocused = (fFocus == TRUE);
    return S_OK;
}

STDMETHODIMP CIEActiveXCtrl::GetDC(LPCRECT pRect, DWORD grfFlags, HDC* phDC)
{
    TRACE(_T("AX: CIEActiveXCtrl::GetDC"));
    if( phDC == NULL ) return E_POINTER;
    if( m_pOwner == NULL ) return E_UNEXPECTED;
    *phDC = ::GetDC(m_pOwner->m_hwndHost);
    if( (grfFlags & OLEDC_PAINTBKGND) != 0 ) {
        CRect rcItem = m_pOwner->GetPos();
        if( !m_bWindowless ) rcItem.ResetOffset();
        ::FillRect(*phDC, &rcItem, (HBRUSH) (COLOR_WINDOW + 1));
    }
    return S_OK;
}

STDMETHODIMP CIEActiveXCtrl::ReleaseDC(HDC hDC)
{
    TRACE(_T("AX: CIEActiveXCtrl::ReleaseDC"));
    if( m_pOwner == NULL ) return E_UNEXPECTED;
    ::ReleaseDC(m_pOwner->m_hwndHost, hDC);
    return S_OK;
}

STDMETHODIMP CIEActiveXCtrl::InvalidateRect(LPCRECT pRect, BOOL fErase)
{
    TRACE(_T("AX: CIEActiveXCtrl::InvalidateRect"));
    if( m_pOwner == NULL ) return E_UNEXPECTED;
    if( m_pOwner->m_hwndHost == NULL ) return E_FAIL;
    return ::InvalidateRect(m_pOwner->m_hwndHost, pRect, fErase) ? S_OK : E_FAIL;
}

STDMETHODIMP CIEActiveXCtrl::InvalidateRgn(HRGN hRGN, BOOL fErase)
{
    TRACE(_T("AX: CIEActiveXCtrl::InvalidateRgn"));
    if( m_pOwner == NULL ) return E_UNEXPECTED;
    return ::InvalidateRgn(m_pOwner->m_hwndHost, hRGN, fErase) ? S_OK : E_FAIL;
}

STDMETHODIMP CIEActiveXCtrl::ScrollRect(INT dx, INT dy, LPCRECT pRectScroll, LPCRECT pRectClip)
{
    TRACE(_T("AX: CIEActiveXCtrl::ScrollRect"));
    return S_OK;
}

STDMETHODIMP CIEActiveXCtrl::AdjustRect(LPRECT prc)
{
    TRACE(_T("AX: CIEActiveXCtrl::AdjustRect"));
    return S_OK;
}

STDMETHODIMP CIEActiveXCtrl::OnDefWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* plResult)
{
    TRACE(_T("AX: CIEActiveXCtrl::OnDefWindowMessage"));
    if( m_pOwner == NULL ) return E_UNEXPECTED;
    *plResult = ::DefWindowProc(m_pOwner->m_hwndHost, msg, wParam, lParam);
    return S_OK;
}

STDMETHODIMP CIEActiveXCtrl::OnInPlaceActivateEx(BOOL* pfNoRedraw, DWORD dwFlags)        
{
    TRACE(_T("AX: CIEActiveXCtrl::OnInPlaceActivateEx"));
    ASSERT(m_pInPlaceObject==NULL);
    if( m_pOwner == NULL ) return E_UNEXPECTED;
    if( m_pOwner->m_pUnk == NULL ) return E_UNEXPECTED;
    ::OleLockRunning(m_pOwner->m_pUnk, TRUE, FALSE);
    HWND hWndFrame = m_pOwner->GetManager()->GetPaintWindow();
    HRESULT Hr = E_FAIL;
    if( (dwFlags & ACTIVATE_WINDOWLESS) != 0 ) {
        m_bWindowless = true;
        Hr = m_pOwner->m_pUnk->QueryInterface(IID_IOleInPlaceObjectWindowless, (LPVOID*) &m_pInPlaceObject);
        m_pOwner->m_hwndHost = hWndFrame;
        m_pOwner->GetManager()->AddMessageFilter(m_pOwner);
    }
    if( FAILED(Hr) ) {
        m_bWindowless = false;
        Hr = CreateActiveXWnd();
        if( FAILED(Hr) ) return Hr;
        Hr = m_pOwner->m_pUnk->QueryInterface(IID_IOleInPlaceObject, (LPVOID*) &m_pInPlaceObject);
    }
    if( m_pInPlaceObject != NULL ) {
        CRect rcItem = m_pOwner->m_rcItem;
        if( !m_bWindowless ) rcItem.ResetOffset();
        m_pInPlaceObject->SetObjectRects(&rcItem, &rcItem);
    }
    m_bInPlaceActive = SUCCEEDED(Hr);
    return Hr;
}

STDMETHODIMP CIEActiveXCtrl::OnInPlaceDeactivateEx(BOOL fNoRedraw)       
{
    TRACE(_T("AX: CIEActiveXCtrl::OnInPlaceDeactivateEx"));
    m_bInPlaceActive = false;
    if( m_pInPlaceObject != NULL ) {
        m_pInPlaceObject->Release();
        m_pInPlaceObject = NULL;
    }
    if( m_pWindow != NULL ) {
        ::DestroyWindow(*m_pWindow);
        delete m_pWindow;
        m_pWindow = NULL;
    }
    return S_OK;
}

STDMETHODIMP CIEActiveXCtrl::RequestUIActivate(void)
{
    TRACE(_T("AX: CIEActiveXCtrl::RequestUIActivate"));
    return S_OK;
}

STDMETHODIMP CIEActiveXCtrl::CanInPlaceActivate(void)       
{
    TRACE(_T("AX: CIEActiveXCtrl::CanInPlaceActivate"));
    return S_OK;
}

STDMETHODIMP CIEActiveXCtrl::OnInPlaceActivate(void)
{
    TRACE(_T("AX: CIEActiveXCtrl::OnInPlaceActivate"));
    BOOL bDummy = FALSE;
    return OnInPlaceActivateEx(&bDummy, 0);
}

STDMETHODIMP CIEActiveXCtrl::OnUIActivate(void)
{
    TRACE(_T("AX: CIEActiveXCtrl::OnUIActivate"));
    m_bUIActivated = true;
    return S_OK;
}

STDMETHODIMP CIEActiveXCtrl::GetWindowContext(IOleInPlaceFrame** ppFrame, IOleInPlaceUIWindow** ppDoc, LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
    TRACE(_T("AX: CIEActiveXCtrl::GetWindowContext"));
    if( ppDoc == NULL ) return E_POINTER;
    if( ppFrame == NULL ) return E_POINTER;
    if( lprcPosRect == NULL ) return E_POINTER;
    if( lprcClipRect == NULL ) return E_POINTER;
    *ppFrame = new CIEActiveXFrameWnd(m_pOwner);
    *ppDoc = NULL;
    ACCEL ac = { 0 };
    HACCEL hac = ::CreateAcceleratorTable(&ac, 1);
    lpFrameInfo->cb = sizeof(OLEINPLACEFRAMEINFO);
    lpFrameInfo->fMDIApp = FALSE;
    lpFrameInfo->hwndFrame = m_pOwner->GetManager()->GetPaintWindow();
    lpFrameInfo->haccel = hac;
    lpFrameInfo->cAccelEntries = 1;
    return S_OK;
}

STDMETHODIMP CIEActiveXCtrl::Scroll(SIZE scrollExtant)
{
    TRACE(_T("AX: CIEActiveXCtrl::Scroll"));
    return E_NOTIMPL;
}

STDMETHODIMP CIEActiveXCtrl::OnUIDeactivate(BOOL fUndoable)
{
    TRACE(_T("AX: CIEActiveXCtrl::OnUIDeactivate"));
    m_bUIActivated = false;
    return S_OK;
}

STDMETHODIMP CIEActiveXCtrl::OnInPlaceDeactivate(void)
{
    TRACE(_T("AX: CIEActiveXCtrl::OnInPlaceDeactivate"));
    return OnInPlaceDeactivateEx(TRUE);
}

STDMETHODIMP CIEActiveXCtrl::DiscardUndoState(void)
{
    TRACE(_T("AX: CIEActiveXCtrl::DiscardUndoState"));
    return E_NOTIMPL;
}

STDMETHODIMP CIEActiveXCtrl::DeactivateAndUndo(void)
{
    TRACE(_T("AX: CIEActiveXCtrl::DeactivateAndUndo"));
    return E_NOTIMPL;
}

STDMETHODIMP CIEActiveXCtrl::OnPosRectChange(LPCRECT lprcPosRect)
{
    TRACE(_T("AX: CIEActiveXCtrl::OnPosRectChange"));
    return E_NOTIMPL;
}

STDMETHODIMP CIEActiveXCtrl::GetWindow(HWND* phwnd)
{
    TRACE(_T("AX: CIEActiveXCtrl::GetWindow"));
    if( m_pOwner == NULL ) return E_UNEXPECTED;
    if( m_pOwner->m_hwndHost == NULL ) CreateActiveXWnd();
    if( m_pOwner->m_hwndHost == NULL ) return E_FAIL;
    *phwnd = m_pOwner->m_hwndHost;
    return S_OK;
}

STDMETHODIMP CIEActiveXCtrl::ContextSensitiveHelp(BOOL fEnterMode)
{
    TRACE(_T("AX: CIEActiveXCtrl::ContextSensitiveHelp"));
    return S_OK;
}

STDMETHODIMP CIEActiveXCtrl::OnControlInfoChanged(void)      
{
    TRACE(_T("AX: CIEActiveXCtrl::OnControlInfoChanged"));
    return S_OK;
}

STDMETHODIMP CIEActiveXCtrl::LockInPlaceActive(BOOL fLock)       
{
    TRACE(_T("AX: CIEActiveXCtrl::LockInPlaceActive"));
    return S_OK;
}

STDMETHODIMP CIEActiveXCtrl::GetExtendedControl(IDispatch** ppDisp)        
{
    TRACE(_T("AX: CIEActiveXCtrl::GetExtendedControl"));
    if( ppDisp == NULL ) return E_POINTER;   
    if( m_pOwner == NULL ) return E_UNEXPECTED;
    if( m_pOwner->m_pUnk == NULL ) return E_UNEXPECTED;
    return m_pOwner->m_pUnk->QueryInterface(IID_IDispatch, (LPVOID*) ppDisp);
}

STDMETHODIMP CIEActiveXCtrl::TransformCoords(POINTL* pPtlHimetric, POINTF* pPtfContainer, DWORD dwFlags)       
{
    TRACE(_T("AX: CIEActiveXCtrl::TransformCoords"));
    return S_OK;
}

STDMETHODIMP CIEActiveXCtrl::TranslateAccelerator(MSG *pMsg, DWORD grfModifiers)
{
    TRACE(_T("AX: CIEActiveXCtrl::TranslateAccelerator"));
    return S_FALSE;
}

STDMETHODIMP CIEActiveXCtrl::OnFocus(BOOL fGotFocus)
{
    TRACE(_T("AX: CIEActiveXCtrl::OnFocus"));
    m_bFocused = (fGotFocus == TRUE);
    return S_OK;
}

STDMETHODIMP CIEActiveXCtrl::ShowPropertyFrame(void)
{
    TRACE(_T("AX: CIEActiveXCtrl::ShowPropertyFrame"));
    return E_NOTIMPL;
}

STDMETHODIMP CIEActiveXCtrl::EnumObjects(DWORD grfFlags, IEnumUnknown** ppenum)
{
    TRACE(_T("AX: CIEActiveXCtrl::EnumObjects"));
    if( ppenum == NULL ) return E_POINTER;
    if( m_pOwner == NULL ) return E_UNEXPECTED;
    *ppenum = new CIEActiveXEnum(m_pOwner->m_pUnk);
    return S_OK;
}

STDMETHODIMP CIEActiveXCtrl::LockContainer(BOOL fLock)
{
    TRACE(_T("AX: CIEActiveXCtrl::LockContainer"));
    m_bLocked = fLock != FALSE;
    return S_OK;
}

STDMETHODIMP CIEActiveXCtrl::ParseDisplayName(IBindCtx *pbc, LPOLESTR pszDisplayName, ULONG* pchEaten, IMoniker** ppmkOut)
{
    TRACE(_T("AX: CIEActiveXCtrl::ParseDisplayName"));
    return E_NOTIMPL;
}

// IDocHostUIHandler
STDMETHODIMP CIEActiveXCtrl::ShowContextMenu(DWORD dwID, POINT* pptPosition, IUnknown* pCommandTarget, IDispatch* pDispatchObjectHit)
{
	//Enables MSHTML to display a shortcut menu.
	//S_OK	Host displayed its UI. MSHTML will not attempt to display its UI.
	//S_FALSE	Host did not display its UI. MSHTML will display its UI.
	//DOCHOST_E_UNKNOWN	Menu identifier is unknown. MSHTML might attempt an alternative identifier from a previous version.
	if( m_pOwner == NULL ) return E_UNEXPECTED;
	HRESULT hr = S_FALSE;
	if (m_pOwner->m_HostUIHandler != NULL)
		hr = m_pOwner->m_HostUIHandler->ShowContextMenu(dwID, pptPosition, pCommandTarget, pDispatchObjectHit);
	return hr;
}

STDMETHODIMP CIEActiveXCtrl::GetHostInfo(DOCHOSTUIINFO* pInfo)
{
	if (pInfo == NULL)
		return E_POINTER;
	if( m_pOwner == NULL ) return E_UNEXPECTED;

	if (m_pOwner->m_HostUIHandler != NULL)
		return m_pOwner->m_HostUIHandler->GetHostInfo(pInfo);
	return S_OK;
}

STDMETHODIMP CIEActiveXCtrl::ShowUI(DWORD dwID, IOleInPlaceActiveObject* pActiveObject, IOleCommandTarget* pCommandTarget, IOleInPlaceFrame* pFrame, IOleInPlaceUIWindow* pDoc)
{
	if( m_pOwner == NULL ) return E_UNEXPECTED;
	HRESULT hr = S_OK;
	if (m_pOwner->m_HostUIHandler != NULL)
		hr = m_pOwner->m_HostUIHandler->ShowUI(dwID, pActiveObject, pCommandTarget, pFrame, pDoc);
	return hr;
}

STDMETHODIMP CIEActiveXCtrl::HideUI()
{
	if( m_pOwner == NULL ) return E_UNEXPECTED;
	HRESULT hr = S_OK;
	if (m_pOwner->m_HostUIHandler != NULL)
		hr = m_pOwner->m_HostUIHandler->HideUI();
	return hr;
}

STDMETHODIMP CIEActiveXCtrl::UpdateUI()
{
	if( m_pOwner == NULL ) return E_UNEXPECTED;
	HRESULT hr = S_OK;
	if (m_pOwner->m_HostUIHandler != NULL)
		hr = m_pOwner->m_HostUIHandler->UpdateUI();
	return hr;
}

STDMETHODIMP CIEActiveXCtrl::EnableModeless(BOOL fEnable)
{
	if( m_pOwner == NULL ) return E_UNEXPECTED;
	HRESULT hr = S_OK;
	if (m_pOwner->m_HostUIHandler != NULL)
		hr = m_pOwner->m_HostUIHandler->EnableModeless(fEnable);
	return hr;
}

STDMETHODIMP CIEActiveXCtrl::OnDocWindowActivate(BOOL fActivate)
{
	if( m_pOwner == NULL ) return E_UNEXPECTED;
	HRESULT hr = S_OK;
	if (m_pOwner->m_HostUIHandler != NULL)
		hr = m_pOwner->m_HostUIHandler->OnDocWindowActivate(fActivate);
	return hr;
}

STDMETHODIMP CIEActiveXCtrl::OnFrameWindowActivate(BOOL fActivate)
{
	if( m_pOwner == NULL ) return E_UNEXPECTED;
	HRESULT hr = S_OK;
	if (m_pOwner->m_HostUIHandler != NULL)
		hr = m_pOwner->m_HostUIHandler->OnFrameWindowActivate(fActivate);
	return hr;
}

STDMETHODIMP CIEActiveXCtrl::ResizeBorder(LPCRECT prcBorder, IOleInPlaceUIWindow* pUIWindow, BOOL fFrameWindow)
{
	if( m_pOwner == NULL ) return E_UNEXPECTED;
	HRESULT hr = S_OK;
	if (m_pOwner->m_HostUIHandler != NULL)
		hr = m_pOwner->m_HostUIHandler->ResizeBorder(prcBorder,	pUIWindow, fFrameWindow);
	return hr;
}

STDMETHODIMP CIEActiveXCtrl::TranslateAccelerator(LPMSG lpMsg, const GUID* pguidCmdGroup, DWORD nCmdID)
{
	if( m_pOwner == NULL ) return E_UNEXPECTED;
	HRESULT hr = S_FALSE;
	if (m_pOwner->m_HostUIHandler != NULL)
		m_pOwner->m_HostUIHandler->TranslateAccelerator(lpMsg, pguidCmdGroup, nCmdID);
	return hr;
}

STDMETHODIMP CIEActiveXCtrl::GetOptionKeyPath(LPOLESTR* pchKey, DWORD dwReserved)
{
	if( m_pOwner == NULL ) return E_UNEXPECTED;

	HRESULT hr = S_FALSE;
	if (pchKey == NULL)
		return E_POINTER;
	*pchKey = NULL;
	if (m_pOwner->m_HostUIHandler != NULL)
	{
		hr = m_pOwner->m_HostUIHandler->GetOptionKeyPath(pchKey, dwReserved);
		if (FAILED(hr) || *pchKey == NULL)
			hr = S_FALSE;
	}
	return hr;
}

STDMETHODIMP CIEActiveXCtrl::GetDropTarget(IDropTarget* pDropTarget, IDropTarget** ppDropTarget)
{
	if( m_pOwner == NULL ) return E_UNEXPECTED;
	if (ppDropTarget == NULL)
		return E_POINTER;
	*ppDropTarget = NULL;

	HRESULT hr = E_NOTIMPL;
	if (m_pOwner->m_HostUIHandler != NULL)
	{
		hr = m_pOwner->m_HostUIHandler->GetDropTarget(pDropTarget, ppDropTarget);
		if (FAILED(hr) || *ppDropTarget == NULL)
			hr = S_FALSE;
	}
	return hr;
}

STDMETHODIMP CIEActiveXCtrl::GetExternal(IDispatch** ppDispatch)
{
	if( m_pOwner == NULL ) return E_UNEXPECTED;
	if (ppDispatch == NULL)
		return E_POINTER;
	*ppDispatch = NULL;

	HRESULT hr = E_NOINTERFACE;
	if (m_pOwner->m_HostUIHandler != NULL)
	{
		hr = m_pOwner->m_HostUIHandler->GetExternal(ppDispatch);
		if (FAILED(hr) || *ppDispatch == NULL)
			hr = E_NOINTERFACE;
	}
	return hr;
}

STDMETHODIMP CIEActiveXCtrl::TranslateUrl(DWORD dwTranslate, OLECHAR* pchURLIn, OLECHAR** ppchURLOut)
{
	if( m_pOwner == NULL ) return E_UNEXPECTED;
	if (ppchURLOut == NULL)
		return E_POINTER;
	*ppchURLOut = NULL;

	m_navigateURL = pchURLIn;
	
	HRESULT hr = S_FALSE;
	if (m_pOwner->m_HostUIHandler != NULL)
	{
		hr = m_pOwner->m_HostUIHandler->TranslateUrl(dwTranslate, pchURLIn, ppchURLOut);
		if(FAILED(hr) || *ppchURLOut == NULL)
			hr = S_FALSE;
	}
	return hr;
}

STDMETHODIMP CIEActiveXCtrl::FilterDataObject(IDataObject* pDO, IDataObject** ppDORet)
{
	if( m_pOwner == NULL ) return E_UNEXPECTED;
	if (ppDORet == NULL)
		return E_POINTER;
	*ppDORet = NULL;

	HRESULT hr = S_FALSE;
	if (m_pOwner->m_HostUIHandler != NULL)
	{
		hr = m_pOwner->m_HostUIHandler->FilterDataObject(pDO, ppDORet);
		if (FAILED(hr) || *ppDORet == NULL)
			hr = S_FALSE;
	}
	return hr;
}


//---------------------------------------------------------------------------------
//Description:
// DWebBrowserEvents and IDispatch methods
//
//----------------------------------------------------------------------------------
STDMETHODIMP CIEActiveXCtrl::GetTypeInfoCount(UINT FAR* pctinfo)          
{ 
	return E_NOTIMPL;
}


//---------------------------------------------------------------------------------
//Description:
// DWebBrowserEvents and IDispatch methods
//
//----------------------------------------------------------------------------------
STDMETHODIMP CIEActiveXCtrl::GetTypeInfo(UINT itinfo,LCID lcid,ITypeInfo FAR* FAR* pptinfo)   
{ 
	return E_NOTIMPL;
}


//---------------------------------------------------------------------------------
//Description:
// DWebBrowserEvents and IDispatch methods
//
//----------------------------------------------------------------------------------
STDMETHODIMP CIEActiveXCtrl::GetIDsOfNames(REFIID riid,OLECHAR FAR* FAR* rgszNames,UINT cNames,LCID lcid, DISPID FAR* rgdispid)                  
{ 
	return E_NOTIMPL;
}


//---------------------------------------------------------------------------------
//Description:
// DWebBrowserEvents and IDispatch methods
//
//----------------------------------------------------------------------------------
STDMETHODIMP CIEActiveXCtrl::Invoke(DISPID dispidMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS FAR* pdispparams, VARIANT FAR* pvarResult,EXCEPINFO FAR* pexcepinfo,UINT FAR* puArgErr)    
{
	return DefDWebBrowserEventInvokeProc(dispidMember,riid,lcid,wFlags,pdispparams, pvarResult,pexcepinfo,puArgErr);
}

//---------------------------------------------------------------------
//Description:
// My default DWebBroserEvent Invoke interface process
//
//----------------------------------------------------------------------
HRESULT CIEActiveXCtrl::DefDWebBrowserEventInvokeProc(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr)
{
	TCHAR szBuf[MAX_PATH];
	_stprintf(szBuf, _T("dispidMember = %d.\n"), dispidMember);
	OutputDebugString(szBuf);

	static bool bLoginFormSubmitting = false;

	switch (dispidMember)
	{
	case DISPID_BEFORENAVIGATE     ://100   // this is sent before navigation to give a chance to abort
		{
			ASSERT(pdispparams->cArgs == 7);
			ASSERT(pdispparams->rgvarg[6].vt == VT_DISPATCH);
			ASSERT(pdispparams->rgvarg[5].vt == VT_BSTR);
			ASSERT(pdispparams->rgvarg[4].vt == VT_BSTR);
			ASSERT(pdispparams->rgvarg[3].vt == VT_BSTR);
			ASSERT(pdispparams->rgvarg[2].vt == (VT_BYREF | VT_VARIANT));
			ASSERT(pdispparams->rgvarg[1].vt == VT_BSTR);
			ASSERT(pdispparams->rgvarg[0].vt == (VT_BYREF | VT_BOOL));
			return E_NOTIMPL;
		}
	case DISPID_NAVIGATECOMPLETE   ://101   // in async, this is sent when we have enough to show
		{
			ASSERT(pdispparams->cArgs == 2);
			ASSERT(pdispparams->rgvarg[1].vt == (VT_BYREF | VT_DISPATCH));
			ASSERT(pdispparams->rgvarg[0].vt == (VT_BYREF | VT_BOOL));
			return E_NOTIMPL;
		}
	case DISPID_STATUSTEXTCHANGE   ://102
		{
			ASSERT(pdispparams->cArgs == 1);
			ASSERT(pdispparams->rgvarg[0].vt == VT_BSTR);
			return E_NOTIMPL;
		}
	case DISPID_BEFORENAVIGATE2     ://250   // hyperlink clicked on
		{
			ASSERT(pdispparams->cArgs == 7);
			ASSERT(pdispparams->rgvarg[6].vt == VT_DISPATCH);
			ASSERT(pdispparams->rgvarg[5].vt == (VT_BYREF | VT_VARIANT)); 
			ASSERT(pdispparams->rgvarg[4].vt == (VT_BYREF | VT_VARIANT));
			ASSERT(pdispparams->rgvarg[3].vt == (VT_BYREF | VT_VARIANT));
			ASSERT(pdispparams->rgvarg[2].vt == (VT_BYREF | VT_VARIANT));
			ASSERT(pdispparams->rgvarg[1].vt == (VT_BYREF | VT_VARIANT));
			ASSERT(pdispparams->rgvarg[5].pvarVal->vt == (VT_BSTR));
			ASSERT(pdispparams->rgvarg[4].pvarVal->vt == (VT_I4));
			ASSERT(pdispparams->rgvarg[3].pvarVal->vt == (VT_BSTR));
			ASSERT(pdispparams->rgvarg[2].pvarVal->vt == (VT_BYREF|VT_VARIANT));
			ASSERT(pdispparams->rgvarg[1].pvarVal->vt == (VT_BSTR));
			ASSERT(pdispparams->rgvarg[0].vt == (VT_BYREF | VT_BOOL));

			bLoginFormSubmitting = false;

			HRESULT hr = S_OK;
			IDispatch* pDispatch = pdispparams->rgvarg[6].pdispVal;
			if (pDispatch != NULL)
			{
				IWebBrowser2* pWebBrowser = NULL;
				IDispatch* pDocumentDispatch = NULL;				
				IHTMLDocument2* pDocument = NULL;

				hr = pDispatch->QueryInterface(IID_IWebBrowser2, (LPVOID*)&pWebBrowser);
				ASSERT(SUCCEEDED(hr));

				hr = pWebBrowser->get_Document(&pDocumentDispatch);
				//ASSERT(SUCCEEDED(hr));

				if (pDocumentDispatch != NULL)
				{
					BSTR bstrLocationURL = NULL;
					pWebBrowser->get_LocationURL(&bstrLocationURL);
					if (bstrLocationURL != NULL) {
						if (wcsstr(bstrLocationURL, L"https://passport.360buy.com/new/login.aspx") != NULL)
							bLoginFormSubmitting = true;
						SysFreeString(bstrLocationURL);
					}

					hr = pDocumentDispatch->QueryInterface(IID_IHTMLDocument2, (LPVOID*)&pDocument);
					//ASSERT(SUCCEEDED(hr));

					if (pDocument != NULL)
					{

						BSTR bstrUserName = NULL;
						BSTR bstrPassword = NULL;
						{
							// 获取密码、账号
							HRESULT hr = S_FALSE;
							BSTR bstrName, bstrValue, bstrType;

							bstrName = SysAllocString(L"name");
							bstrValue = SysAllocString(L"value");
							bstrType = SysAllocString(L"type");

							IHTMLElementCollection* pElementCollection;
							hr = pDocument->get_forms(&pElementCollection);
							ASSERT(SUCCEEDED(hr));

							long nFormCount=0;

							//获取表单数目
							hr = pElementCollection->get_length(&nFormCount);
							ASSERT(SUCCEEDED(hr));

							for (long i = 0; i < nFormCount; ++i)
							{
								//取得第 i 项表单
								IDispatch *pDisp = NULL;

								VARIANT vParam;
								vParam.vt = VT_I4;
								vParam.lVal = i;
								hr = pElementCollection->item(vParam, vParam, &pDisp);
								if (FAILED(hr))
									goto LOOP_1_CLEAN_UP;

								IHTMLFormElement * pFormElement = NULL;
								hr = pDisp->QueryInterface(IID_IHTMLFormElement, (LPVOID*)&pFormElement);
								if (FAILED(hr))
									goto LOOP_1_CLEAN_UP;

								//取得表单中 域的数目
								long nElemCount=0;
								hr = pFormElement->get_length(&nElemCount);
								if (FAILED(hr))
									goto LOOP_1_CLEAN_UP;

								for (long j = 0; j < nElemCount; ++j)
								{
									//取得第 j 项表单域
									VARIANT vParam;
									vParam.vt = VT_I4;
									vParam.lVal = j;
									IDispatch* pDispatchElement = NULL;
									hr = pFormElement->item(vParam, vParam, &pDispatchElement);
									if (FAILED(hr))
										goto LOOP_2_CLEAN_UP;

									IHTMLElement* pInputElement = NULL;
									hr = pDispatchElement->QueryInterface(IID_IHTMLElement, (LPVOID*)&pInputElement);
									//取得表单域的 名，值，类型
									VARIANT vName, vVal, vType;

									hr = pInputElement->getAttribute(bstrName, 0, &vName);
									if (FAILED(hr))
										goto LOOP_2_CLEAN_UP;

									hr = pInputElement->getAttribute(bstrValue, 0, &vVal);
									if (FAILED(hr))
										goto LOOP_2_CLEAN_UP;

									hr = pInputElement->getAttribute(bstrType, 0, &vType);
									if (FAILED(hr))
										goto LOOP_2_CLEAN_UP;

									//未知域名
									LPCTSTR lpName = vName.bstrVal ? OLE2CT(vName.bstrVal) : _T("");

									//空值，未输入
									LPCTSTR lpVal  = vVal.bstrVal ? OLE2CT(vVal.bstrVal) : _T("");

									//未知类型
									LPCTSTR lpType = vType.bstrVal ? OLE2CT(vType.bstrVal) : _T("");

									//向用户名文本框填值
									if (wcsicmp(lpName, L"loginname") == 0)
									{
										bstrUserName = SysAllocString(lpVal);
									}
									//向密码文本框填值
									else if (wcsicmp(lpName, L"loginpwd") == 0)
									{
										bstrPassword = SysAllocString(lpVal);
									}
LOOP_2_CLEAN_UP:
									pDispatchElement->Release();
								} // for (long j = 0; j < nElemCount; ++j)
LOOP_1_CLEAN_UP:
								pDisp->Release();
							} // for (long i = 0; i < nFormCount; ++i)

							SysFreeString(bstrName);
							SysFreeString(bstrValue);
							SysFreeString(bstrType);
						}
						if (bstrUserName != NULL)
							SysFreeString(bstrUserName);
						if (bstrPassword != NULL)
							SysFreeString(bstrPassword);

						pDocument->Release();
					}
					pDocumentDispatch->Release();
				}

				pWebBrowser->Release();
				pDispatch->Release();
			}

			return E_NOTIMPL;  
		}
	case DISPID_NEWWINDOW2           ://251
		{
			ASSERT(pdispparams->cArgs == 2);
			ASSERT(pdispparams->rgvarg[1].vt == (VT_BYREF | VT_DISPATCH));
			ASSERT(pdispparams->rgvarg[0].vt == (VT_BYREF | VT_BOOL));
			*(pdispparams->rgvarg[0].pboolVal) = TRUE;
			if (m_pWB2 != NULL && m_navigateURL.GetLength() > 0)
			{
				// Package the URL as a BSTR for Navigate.
				VARIANT varURL;
				V_VT(&varURL) = VT_EMPTY; //It's the same as : varURL.vt = VT_EMPTY;
				if(m_navigateURL.GetData() && m_navigateURL.GetData()[0] != '/0')
				{
					V_VT(&varURL) = VT_BSTR; //It's the same as : varURL.vt = VT_BSTR;
					varURL.bstrVal = SysAllocString(m_navigateURL.GetData());
					if(varURL.bstrVal)
					{
						// Call IWebBrowser2::Navigate2 with no special options.
						m_pWB2->Navigate2(&varURL, NULL, NULL, NULL, NULL);  
					}
				}
				// Clean up the BSTR if it exists.
				VariantClear(&varURL);
			}
			return S_OK;
		}
	case DISPID_NAVIGATECOMPLETE2    ://252   // UIActivate new document
		{
			ASSERT(pdispparams->cArgs == 2);
			ASSERT(pdispparams->rgvarg[1].vt == VT_DISPATCH);
			ASSERT(pdispparams->rgvarg[0].vt == (VT_BYREF | VT_VARIANT));
			ASSERT(pdispparams->rgvarg[0].pvarVal->vt == (VT_BSTR));

			if (bLoginFormSubmitting)
			{
				// 登录成功
			}

			//HRESULT hr = S_OK;
			//IDispatch* pDispatch = NULL;

			//hr = m_pWB2->get_Document(&pDispatch);
			//if (pDispatch != NULL)
			//{
			//	ASSERT(SUCCEEDED(hr));

			//	IHTMLDocument2* pDocument = NULL;

			//	hr = pDispatch->QueryInterface(IID_IHTMLDocument2, (LPVOID*)&pDocument);
			//	ASSERT(SUCCEEDED(hr));

			//	if (pDocument != NULL)
			//	{
			//		OutputDebugString(L"EnumJavascripts().\n");

			//		hr = EnumJavascripts(pDocument);

			//		pDocument->Release();
			//	}

			//	pDispatch->Release();

			//}
			return E_NOTIMPL;   
		}
	case DISPID_HTMLFORMELEMENTEVENTS2_ONSUBMIT:
		{
			int i = 0;
			++i;
			return E_NOTIMPL;
		}
	case DISPID_DOCUMENTCOMPLETE    ://259   // new document goes ReadyState_Complete
		{
			ASSERT(pdispparams->cArgs == 2);
			ASSERT(pdispparams->rgvarg[1].vt == (VT_DISPATCH));
			ASSERT(pdispparams->rgvarg[0].vt == (VT_BYREF | VT_VARIANT));
			ASSERT(pdispparams->rgvarg[0].pvarVal->vt == (VT_BSTR));

			HRESULT hr = S_OK;
			IDispatch* pDispatch = NULL;

			hr = m_pWB2->get_Document(&pDispatch);
			ASSERT(SUCCEEDED(hr));

			IHTMLDocument2* pDocument = NULL;
			
			hr = pDispatch->QueryInterface(IID_IHTMLDocument2, (LPVOID*)&pDocument);
			ASSERT(SUCCEEDED(hr));

			BSTR bstrCookie;
			hr = pDocument->get_cookie(&bstrCookie);
			ASSERT(SUCCEEDED(hr));
			SysFreeString(bstrCookie);

			BSTR bstrJavascript = SysAllocString(L"SaveAs");
			VARIANT vValue;
			VARIANT_BOOL vResult;

			vValue.vt = VT_BSTR;
			vValue.bstrVal = bstrJavascript;
			vValue.bstrVal = SysAllocString(L"javascript");
			//hr = pDocument->execCommand(bstrJavascript, VARIANT_FALSE, vValue, &vResult);
			SysFreeString(bstrJavascript);
			SysFreeString(vValue.bstrVal);

			//IHTMLWindow2 *pHtmlWindow = NULL;

			//VARIANT vResult2;
			//vResult2.vt = VT_EMPTY;

			//vValue.bstrVal = SysAllocString(L"javascript");

			//bstrJavascript = SysAllocString(L"alert(\"我是警告框！！\")");

			//hr = pDocument->get_parentWindow(&pHtmlWindow);
			//pHtmlWindow->execScript(bstrJavascript, vValue.bstrVal, &vResult2);
			//SysFreeString(bstrJavascript);
			//SysFreeString(vValue.bstrVal);

			//hr = AutoSubmitForm(pDocument, _T("jingdong_abc"), _T("123456"));

			//hr = EnumJavascripts(pDocument);

#pragma region 获取IHTMLWindow2
			//IHTMLWindow2 *pHtmlWindow = NULL;
			//hr = pDocument->get_parentWindow(&pHtmlWindow);
			//ASSERT(SUCCEEDED(hr));
			//if (pHtmlWindow != NULL)
			//{
			//	IOmHistory *pHistory = NULL;
			//	hr = pHtmlWindow->get_history(&pHistory);

			//	short history_length = 0;
			//	hr = pHistory->get_length(&history_length);
			//	ASSERT(SUCCEEDED(hr));

			//	IOmNavigator *pNavigator = NULL;
			//	hr = pHtmlWindow->get_navigator(&pNavigator);
			//	ASSERT(SUCCEEDED(hr));

			//	BSTR bstrUserAgent;
			//	hr = pNavigator->get_userAgent(&bstrUserAgent);
			//	ASSERT(SUCCEEDED(hr));
			//	SysFreeString(bstrUserAgent);

			//	IHTMLWindow3 *pHtmlWindow3 = NULL;
			//	hr = pHtmlWindow->QueryInterface(IID_IHTMLWindow3, (LPVOID*)&pHtmlWindow3);
			//	ASSERT(SUCCEEDED(hr));

			//	BSTR bstrOnSubmit = SysAllocString(L"onsubmit");
			//	VARIANT_BOOL result;
			//	pHtmlWindow3->attachEvent(bstrOnSubmit, this, &result);
			//	SysFreeString(bstrOnSubmit);
			//}
#pragma endregion

#pragma region 监听HtmlDocumentEvent
			{
				IUnknown* pSrcUnk = NULL;
				hr = pDocument->QueryInterface(IID_IUnknown, (LPVOID*)&pSrcUnk);
				ASSERT(SUCCEEDED(hr));

				IConnectionPointContainer	*pCPCont = NULL;
				HTMLDocumentEvents2 *pEvents = NULL;

				// Get the connection point container for the browser object.
				hr = pDocument->QueryInterface(IID_IConnectionPointContainer, (LPVOID *)&pCPCont);
				ASSERT(SUCCEEDED(hr));

				if( m_pCPDocumentEventSink != NULL && m_dwDocumentEventCookie != NULL )
				{
					m_pCPDocumentEventSink->Unadvise(m_dwDocumentEventCookie);	
				}

				if( m_pCPDocumentEventSink != NULL )
					m_pCPDocumentEventSink->Release();

				m_pCPDocumentEventSink = NULL;
				m_dwDocumentEventCookie = 0;

				// Look for DWebBrowserEvents2 connection point.
				hr = pCPCont->FindConnectionPoint(DIID_HTMLDocumentEvents2, &m_pCPDocumentEventSink);
				ASSERT(SUCCEEDED(hr));

				// Get a DWebBrowserEvents2 pointer from the browser.
				hr = m_pHtmlDocEventSink->QueryInterface(DIID_HTMLDocumentEvents2, (LPVOID *)(&pEvents));
				ASSERT(SUCCEEDED(hr));

				// Add your event sink to the connectionpoint.
				hr = m_pCPDocumentEventSink->Advise(pEvents, &(m_dwDocumentEventCookie));
				ASSERT(SUCCEEDED(hr));

				pEvents->Release();
				pSrcUnk->Release();
				pCPCont->Release();
			}
#pragma endregion

#pragma region 监听HTMLFormElementEvents
			//{
			//	IConnectionPointContainer	*pCPCont = NULL;
			//	HTMLFormElementEvents2 *pEvents = NULL;

			//	// Get the connection point container for the browser object.
			//	hr = pDocument->QueryInterface(IID_IConnectionPointContainer, (LPVOID *)&pCPCont);
			//	ASSERT(SUCCEEDED(hr));

			//	if( m_pCPFormEventSink != NULL && m_dwFormEventCookie != NULL )
			//	{
			//		m_pCPFormEventSink->Unadvise(m_dwFormEventCookie);	
			//	}

			//	if( m_pCPFormEventSink != NULL )
			//		m_pCPFormEventSink->Release();

			//	m_pCPFormEventSink = NULL;
			//	m_dwFormEventCookie = 0;

			//	// Look for DWebBrowserEvents2 connection point.
			//	hr = pCPCont->FindConnectionPoint(DIID_HTMLFormElementEvents2, &m_pCPFormEventSink);
			//	ASSERT(SUCCEEDED(hr));

			//	// Get a DWebBrowserEvents2 pointer from the browser.
			//	hr = QueryInterface(DIID_HTMLFormElementEvents2, (LPVOID *)(&pEvents));
			//	ASSERT(SUCCEEDED(hr));

			//	// Add your event sink to the connectionpoint.
			//	hr = m_pCPFormEventSink->Advise(pEvents, &(m_dwFormEventCookie));
			//	ASSERT(SUCCEEDED(hr));

			//	pEvents->Release();
			//	pCPCont->Release();
			//}
#pragma endregion

			pDocument->Release();

			pDispatch->Release();
			return E_NOTIMPL;
		}
	case DISPID_QUIT               ://103
		{
			ASSERT(pdispparams->cArgs == 0);
			return E_NOTIMPL;
		}
	case DISPID_DOWNLOADCOMPLETE   ://104
		{
			ASSERT(pdispparams->cArgs == 0);
			return E_NOTIMPL;
		}
	case DISPID_COMMANDSTATECHANGE ://105
		{
			ASSERT(pdispparams->cArgs == 2);
			ASSERT(pdispparams->rgvarg[1].vt == VT_I4);
			ASSERT(pdispparams->rgvarg[0].vt == VT_BOOL);


			return E_NOTIMPL;
		}
	case DISPID_DOWNLOADBEGIN      ://106
		{
			ASSERT(pdispparams->cArgs == 0);
			return E_NOTIMPL;
		}
	case DISPID_NEWWINDOW          ://107   // sent when a new window should be created
		{
			ASSERT(pdispparams->cArgs == 2);
			ASSERT(pdispparams->rgvarg[1].vt == (VT_BYREF | VT_DISPATCH));
			ASSERT(pdispparams->rgvarg[0].vt == (VT_BYREF | VT_BOOL));
			return E_NOTIMPL;
		}
	case DISPID_PROGRESSCHANGE     ://108   // sent when download progress is updated
		{
			ASSERT(pdispparams->cArgs == 2);
			ASSERT(pdispparams->rgvarg[1].vt == VT_I4);
			ASSERT(pdispparams->rgvarg[0].vt == VT_I4);
			return E_NOTIMPL;
		}
	case DISPID_WINDOWMOVE         ://109   // sent when main window has been moved
		{
			return E_NOTIMPL;
		}
	case DISPID_WINDOWRESIZE       ://110   // sent when main window has been sized
		{
			return E_NOTIMPL;
		}
	case DISPID_WINDOWACTIVATE     ://111   // sent when main window has been activated 
		{
			return E_NOTIMPL;
		}
	case DISPID_PROPERTYCHANGE     ://112   // sent when the PutProperty method is called
		{
			ASSERT(pdispparams->cArgs == 1);
			ASSERT(pdispparams->rgvarg[0].vt == VT_BSTR);
			return E_NOTIMPL;
		}
	case DISPID_TITLECHANGE        ://113   // sent when the document title changes
		{
			ASSERT(pdispparams->cArgs == 1);
			ASSERT(pdispparams->rgvarg[0].vt == VT_BSTR);
			return E_NOTIMPL;
		}
	case DISPID_TITLEICONCHANGE    ://114   // sent when the top level window icon may have changed.
		{
			return E_NOTIMPL;
		}
	case DISPID_ONQUIT              ://253
		{
			ASSERT(pdispparams->cArgs == 0);
			return E_NOTIMPL;
		}   
	case DISPID_ONVISIBLE           ://254   // sent when the window goes visible/hidden
		{
			ASSERT(pdispparams->cArgs == 1);
			ASSERT(pdispparams->rgvarg[0].vt == VT_BOOL);
			return E_NOTIMPL;  
		}
	case DISPID_ONTOOLBAR            ://255   // sent when the toolbar should be shown/hidden
		{
			ASSERT(pdispparams->cArgs == 1);
			ASSERT(pdispparams->rgvarg[0].vt == VT_BOOL);
			return E_NOTIMPL;
		}
	case DISPID_ONMENUBAR            ://256   // sent when the menubar should be shown/hidden
		{   
			ASSERT(pdispparams->cArgs == 1);   
			ASSERT(pdispparams->rgvarg[0].vt == VT_BOOL);
			return E_NOTIMPL;
		}
	case DISPID_ONSTATUSBAR          ://257   // sent when the statusbar should be shown/hidden
		{
			ASSERT(pdispparams->cArgs == 1);
			ASSERT(pdispparams->rgvarg[0].vt == VT_BOOL);
			return E_NOTIMPL;
		}
	case DISPID_ONFULLSCREEN         ://258   // sent when kiosk mode should be on/off
		{
			ASSERT(pdispparams->cArgs == 1);
			ASSERT(pdispparams->rgvarg[0].vt == VT_BOOL);
			return E_NOTIMPL;
		}
	case DISPID_ONTHEATERMODE        ://260   // sent when theater mode should be on/off
		{
			ASSERT(pdispparams->cArgs == 1);
			ASSERT(pdispparams->rgvarg[0].vt == VT_BOOL);
			return E_NOTIMPL;
		}
	case DISPID_ONADDRESSBAR         ://261   // sent when the address bar should be shown/hidden
		{
			ASSERT(pdispparams->cArgs == 1);
			ASSERT(pdispparams->rgvarg[0].vt == VT_BOOL);
			return E_NOTIMPL;
		}
	case DISPID_WINDOWSETRESIZABLE   ://262   // sent to set the style of the host window frame
		{
			ASSERT(pdispparams->cArgs == 1);
			ASSERT(pdispparams->rgvarg[0].vt == VT_BOOL);
			return E_NOTIMPL;  
		}
	case DISPID_WINDOWCLOSING        ://263   // sent before script window.close closes the window
		{
			ASSERT(pdispparams->cArgs == 2);
			ASSERT(pdispparams->rgvarg[1].vt == VT_BOOL);
			ASSERT(pdispparams->rgvarg[0].vt == (VT_BYREF | VT_BOOL));
			return E_NOTIMPL;
		}
	case DISPID_WINDOWSETLEFT        ://264   // sent when the put_left method is called on the WebOC
		{
			ASSERT(pdispparams->cArgs == 1);
			ASSERT(pdispparams->rgvarg[0].vt == VT_I4);
			return E_NOTIMPL;
		}
	case DISPID_WINDOWSETTOP         ://265   // sent when the put_top method is called on the WebOC
		{
			ASSERT(pdispparams->cArgs == 1);
			ASSERT(pdispparams->rgvarg[0].vt == VT_I4);
			return E_NOTIMPL;
		}   
	case DISPID_WINDOWSETWIDTH       ://266   // sent when the put_width method is called on the WebOC
		{
			ASSERT(pdispparams->cArgs == 1);
			ASSERT(pdispparams->rgvarg[0].vt == VT_I4);
			return E_NOTIMPL;
		}   
	case DISPID_WINDOWSETHEIGHT      ://267   // sent when the put_height method is called on the WebOC
		{
			ASSERT(pdispparams->cArgs == 1);
			ASSERT(pdispparams->rgvarg[0].vt == VT_I4);
			return E_NOTIMPL;
		}
	case DISPID_CLIENTTOHOSTWINDOW   ://268   // sent during window.open to request conversion of dimensions
		{
			ASSERT(pdispparams->cArgs == 2);
			ASSERT(pdispparams->rgvarg[1].vt == (VT_BYREF | VT_I4));
			ASSERT(pdispparams->rgvarg[0].vt == (VT_BYREF | VT_I4));
			return E_NOTIMPL;
		}
	case DISPID_SETSECURELOCKICON    ://269   // sent to suggest the appropriate security icon to show
		{
			ASSERT(pdispparams->cArgs == 1);
			ASSERT(pdispparams->rgvarg[0].vt == VT_I4);
			return E_NOTIMPL;
		}
	case DISPID_FILEDOWNLOAD        ://270   // Fired to indicate the File Download dialog is opening
		{
			ASSERT(pdispparams->cArgs == 2);
			ASSERT(pdispparams->rgvarg[1].vt == VT_BOOL);
			ASSERT(pdispparams->rgvarg[0].vt == (VT_BYREF | VT_BOOL));
			return E_NOTIMPL;
		}   
	case DISPID_NAVIGATEERROR        ://271   // Fired to indicate the a binding error has occured
		{
			ASSERT(pdispparams->cArgs == 5);
			ASSERT(pdispparams->rgvarg[4].vt == VT_DISPATCH);
			ASSERT(pdispparams->rgvarg[3].vt == (VT_BYREF | VT_VARIANT));
			ASSERT(pdispparams->rgvarg[2].vt == (VT_BYREF | VT_VARIANT));
			ASSERT(pdispparams->rgvarg[1].vt == (VT_BYREF | VT_VARIANT) );
			ASSERT(pdispparams->rgvarg[3].pvarVal->vt == VT_BSTR);
			ASSERT(pdispparams->rgvarg[2].pvarVal->vt == VT_BSTR);
			ASSERT(pdispparams->rgvarg[1].pvarVal->vt == VT_I4 );
			ASSERT(pdispparams->rgvarg[0].vt == (VT_BYREF | VT_BOOL));
			//if ((pdispparams->rgvarg[0].vt == (VT_BOOL | VT_BYREF)) && (pdispparams->rgvarg[4].vt == VT_DISPATCH))
			//{
			//	CComQIPtr<IDispatch, &IID_IDispatch> pDispatch = pdispparams->rgvarg[4].pdispVal;
			//	if (pDispatch != NULL)
			//	{
			//		CComQIPtr<IWebBrowser2, &IID_IWebBrowser2> pBrowser = pDispatch;
			//		if ((pBrowser != NULL) && (pBrowser == m_pWebBrowser2))
			//		{
			//			*(pdispparams->rgvarg[0].pboolVal) = TRUE;
			//			if (m_hMainWindowEvent != NULL)
			//			{
			//				if (!InitializeForMainIE())
			//				{
			//					//Wait for 5 seconds then close
			//					StartTimer(TIMERID_CLOSE, 5000, TRUE, TimerProc, this);
			//				}
			//			}
			//			else
			//			{
			//				if (m_hPopupWindowEvent == NULL)
			//				{
			//					//Wait for 1 seconds then close   
			//					StartTimer(TIMERID_CLOSE, 2000, TRUE, TimerProc, this);
			//				}
			//			}
			//		}
			//	}
			//}
			return E_NOTIMPL;
		}
	case DISPID_PRIVACYIMPACTEDSTATECHANGE   ://272  // Fired when the user's browsing experience is impacted   
		{
			ASSERT(pdispparams->cArgs == 1);
			ASSERT(pdispparams->rgvarg[0].vt == VT_BOOL);
			return E_NOTIMPL;
		}
	case DISPID_AMBIENT_DLCONTROL:  //////下载控件
		{
			//pvarResult->vt = VT_I4;
			//pvarResult->lVal = DLCTL_DLIMAGES             ////下载图像
			//	//| DLCTL_VIDEOS             ////下载视频
			//	| DLCTL_NO_DLACTIVEXCTLS   ////不下载Active X控件
			//	| DLCTL_NO_JAVA            ////不执行java程序
			//	| DLCTL_NO_SCRIPTS;         ////不执行任何脚本
			////| DLCTL_BGSOUNDS;          ////下载背景音乐						    

			//// put the browser in download only mode
			//// pvarResult->lVal |= DLCTL_DOWNLOADONLY;
			//return S_OK;
		}
		break;
	default:
		return E_NOTIMPL;
	}

	return E_NOTIMPL;
}

HRESULT CIEActiveXCtrl::EnumJavascripts(IHTMLDocument2* pDocument)
{
	HRESULT hr = S_FALSE;
	if (pDocument != NULL)
	{
		IHTMLElementCollection* pElementCollection;
		hr = pDocument->get_scripts(&pElementCollection);
		ASSERT(SUCCEEDED(hr));

		long nScriptCount = 0;
		hr = pElementCollection->get_length(&nScriptCount);

		for (long i = 0; i < nScriptCount; ++i)
		{
			//取得第i项javascript
			IDispatch *pDisp = NULL;

			VARIANT vParam;
			vParam.vt = VT_I4;
			vParam.lVal = i;
			hr = pElementCollection->item(vParam, vParam, &pDisp);
			if (FAILED(hr))
				goto LOOP_1_CLEAN_UP;

			IHTMLScriptElement * pScriptElement = NULL;
			hr = pDisp->QueryInterface(IID_IHTMLScriptElement, (LPVOID*)&pScriptElement);
			if (FAILED(hr))
				goto LOOP_1_CLEAN_UP;

			BSTR javascript_text;
			hr = pScriptElement->get_text(&javascript_text);
			if (SUCCEEDED(hr))
			{
				OutputDebugString(javascript_text);
				OutputDebugString(L"\n");
				if (javascript_text != NULL && wcslen(javascript_text) > 0 && wcsstr(javascript_text, L"if (screen.width>=1280)") != NULL)
				{
					pScriptElement->put_defer(VARIANT_TRUE);
					BSTR temp = SysAllocString(L"alert(\"京东客户端！\")");
					hr = pScriptElement->put_text(temp);
					SysFreeString(temp);
				}
				SysFreeString(javascript_text);
			}

			BSTR javascript_src;
			hr = pScriptElement->get_src(&javascript_src);
			if (SUCCEEDED(hr))
				SysFreeString(javascript_src);

LOOP_1_CLEAN_UP:
			pScriptElement->Release();
			pDisp->Release();
		}

CLEAN_UP:
		pElementCollection->Release();
	}

	return hr;
}

HRESULT CIEActiveXCtrl::AutoSubmitForm(IHTMLDocument2* pDocument, LPCTSTR lpstrUserName, LPCTSTR lpstrPassword)
{
	HRESULT hr = S_FALSE;

	if (pDocument == NULL || lpstrUserName == NULL || lpstrPassword == NULL)
		return hr;

	BSTR bstrTitle, bstrName, bstrValue, bstrType, bstrHref;
    
    //获取页面标题
    pDocument->get_title(&bstrTitle);
	SysFreeString(bstrTitle);

	bstrName = SysAllocString(L"name");
	bstrValue = SysAllocString(L"value");
	bstrType = SysAllocString(L"type");
	bstrHref = SysAllocString(L"href");

    IHTMLElementCollection* pElementCollection;
	hr = pDocument->get_forms(&pElementCollection);
	ASSERT(SUCCEEDED(hr));
    
    long nFormCount=0;

    //获取表单数目
    hr = pElementCollection->get_length(&nFormCount);
	ASSERT(SUCCEEDED(hr));
    
    for (long i = 0; i < nFormCount; ++i)
    {
		//取得第 i 项表单
		IDispatch *pDisp = NULL;

		VARIANT vParam;
		vParam.vt = VT_I4;
		vParam.lVal = i;
		hr = pElementCollection->item(vParam, vParam, &pDisp);
		if (FAILED(hr))
			goto LOOP_1_CLEAN_UP;
		
		IHTMLFormElement * pFormElement = NULL;
		hr = pDisp->QueryInterface(IID_IHTMLFormElement, (LPVOID*)&pFormElement);
		if (FAILED(hr))
			goto LOOP_1_CLEAN_UP;

		//取得表单中 域的数目
		long nElemCount=0;
		hr = pFormElement->get_length(&nElemCount);
		if (FAILED(hr))
			goto LOOP_1_CLEAN_UP;

		for (long j = 0; j < nElemCount; ++j)
		{
			//取得第 j 项表单域
			VARIANT vParam;
			vParam.vt = VT_I4;
			vParam.lVal = j;
			IDispatch* pDispatchElement = NULL;
			hr = pFormElement->item(vParam, vParam, &pDispatchElement);
			if (FAILED(hr))
				goto LOOP_2_CLEAN_UP;

			IHTMLElement* pInputElement = NULL;
			hr = pDispatchElement->QueryInterface(IID_IHTMLElement, (LPVOID*)&pInputElement);
			//取得表单域的 名，值，类型
			VARIANT vName, vVal, vType;

			hr = pInputElement->getAttribute(bstrName, 0, &vName);
			if (FAILED(hr))
				goto LOOP_2_CLEAN_UP;

			hr = pInputElement->getAttribute(bstrValue, 0, &vVal);
			if (FAILED(hr))
				goto LOOP_2_CLEAN_UP;

			hr = pInputElement->getAttribute(bstrType, 0, &vType);
			if (FAILED(hr))
				goto LOOP_2_CLEAN_UP;

			//未知域名
			LPCTSTR lpName = vName.bstrVal ? OLE2CT(vName.bstrVal) : _T("");

			//空值，未输入
			LPCTSTR lpVal  = vVal.bstrVal ? OLE2CT(vVal.bstrVal) : _T("");

			//未知类型
			LPCTSTR lpType = vType.bstrVal ? OLE2CT(vType.bstrVal) : _T("");

			//向用户名文本框填值
			if (wcsicmp(lpName, L"loginname") == 0)
			{
				VARIANT vInput;
				vInput.vt = VT_BSTR;
				vInput.bstrVal = SysAllocString(lpstrUserName);
				pInputElement->setAttribute(bstrValue, vInput);
				SysFreeString(vInput.bstrVal);
			}
			//向密码文本框填值
			else if (wcsicmp(lpName, L"loginpwd") == 0)
			{
				VARIANT vInput;
				vInput.vt = VT_BSTR;
				vInput.bstrVal = SysAllocString(lpstrPassword);
				pInputElement->setAttribute(bstrValue, vInput);
				SysFreeString(vInput.bstrVal);
			}
			// 点击登录按钮
			else if ((wcsicmp(lpVal, L"登录") == 0) && (wcsicmp(lpType, L"button") == 0))
			{
				pInputElement->click();                
			}
LOOP_2_CLEAN_UP:
			pDispatchElement->Release();
		} // for (long j = 0; j < nElemCount; ++j)
LOOP_1_CLEAN_UP:
		pDisp->Release();
	} // for (long i = 0; i < nFormCount; ++i)

	SysFreeString(bstrName);
	SysFreeString(bstrValue);
	SysFreeString(bstrType);
	SysFreeString(bstrHref);

	return hr;
}


//------------------------------------------------------------------------
//Description:
//  Connect up the event sink
//
//------------------------------------------------------------------------
HRESULT CIEActiveXCtrl::RegisterBrowserEventSink()
{
	HRESULT     hr = S_FALSE;
	if (m_pOwner == NULL)
		return hr;

	IConnectionPointContainer	*pCPCont = NULL;
	DWebBrowserEvents2          *pEvents = NULL;

	hr = m_pOwner->GetControl(IID_IWebBrowser2, (void**)&m_pWB2);
	if (FAILED(hr))
	{
		hr = S_FALSE;
		goto CLEANUP;
	}

	// Get the connection point container for the browser object.
	hr = m_pWB2->QueryInterface(IID_IConnectionPointContainer, (LPVOID *)&pCPCont);
	if (FAILED(hr))
	{
		hr = S_FALSE;
		goto CLEANUP;
	}

	if( m_pCPWebBrowserEventSink != NULL && m_dwWebBrowserEventCookie != NULL )
	{
		m_pCPWebBrowserEventSink->Unadvise(m_dwWebBrowserEventCookie);	
	}

	if( m_pCPWebBrowserEventSink != NULL )
		m_pCPWebBrowserEventSink->Release();

	m_pCPWebBrowserEventSink = NULL;
	m_dwWebBrowserEventCookie = 0;

	// Look for DWebBrowserEvents2 connection point.
	hr = pCPCont->FindConnectionPoint(DIID_DWebBrowserEvents2, &m_pCPWebBrowserEventSink);
	if (FAILED(hr))
	{
		m_pCPWebBrowserEventSink = NULL;
		goto CLEANUP;
	}

	// Get a DWebBrowserEvents2 pointer from the browser.
	hr = QueryInterface(DIID_DWebBrowserEvents2, (LPVOID *)(&pEvents));
	if (FAILED(hr))
	{
		goto CLEANUP;
	}

	// Add your event sink to the connectionpoint.
	hr = m_pCPWebBrowserEventSink->Advise(pEvents, &(m_dwWebBrowserEventCookie));
	if (FAILED(hr))
	{
		goto CLEANUP;
	}

CLEANUP:
	if (pCPCont)
	{
		pCPCont->Release();
	}

	if (pEvents)
	{
		pEvents->Release();
	}

	return hr;

}

HRESULT CIEActiveXCtrl::CreateActiveXWnd()
{
    if( m_pWindow != NULL ) return S_OK;
    m_pWindow = new CIEActiveXWnd;
    if( m_pWindow == NULL ) return E_OUTOFMEMORY;
    m_pOwner->m_hwndHost = m_pWindow->Init(this, m_pOwner->GetManager()->GetPaintWindow());
    return S_OK;
}



/////////////////////////////////////////////////////////////////////////////////////
//
//

HWND CIEActiveXWnd::Init(CIEActiveXCtrl* pOwner, HWND hWndParent)
{
    m_pOwner = pOwner;
    UINT uStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    Create(hWndParent, _T("UIActiveX"), uStyle, 0L, 0,0,0,0, NULL);
    return m_hWnd;
}

LPCTSTR CIEActiveXWnd::GetWindowClassName() const
{
    return _T("ActiveXWnd");
}

void CIEActiveXWnd::OnFinalMessage(HWND hWnd)
{
    //delete this; // 这里不需要清理，CIEActiveXUI会清理的
}

void CIEActiveXWnd::DoVerb(LONG iVerb)
{
    if( m_pOwner == NULL ) return;
    if( m_pOwner->m_pOwner == NULL ) return;
    IOleObject* pUnk = NULL;
    m_pOwner->m_pOwner->GetControl(IID_IOleObject, (LPVOID*) &pUnk);
    if( pUnk == NULL ) return;
    CSafeRelease<IOleObject> RefOleObject = pUnk;
    IOleClientSite* pOleClientSite = NULL;
    m_pOwner->QueryInterface(IID_IOleClientSite, (LPVOID*) &pOleClientSite);
    CSafeRelease<IOleClientSite> RefOleClientSite = pOleClientSite;
    pUnk->DoVerb(iVerb, NULL, pOleClientSite, 0, m_hWnd, &m_pOwner->m_pOwner->GetPos());
}

LRESULT CIEActiveXWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRes;
    BOOL bHandled = TRUE;
    switch( uMsg ) {
    case WM_PAINT:         lRes = OnPaint(uMsg, wParam, lParam, bHandled); break;
    case WM_SETFOCUS:      lRes = OnSetFocus(uMsg, wParam, lParam, bHandled); break;
    case WM_KILLFOCUS:     lRes = OnKillFocus(uMsg, wParam, lParam, bHandled); break;
    case WM_ERASEBKGND:    lRes = OnEraseBkgnd(uMsg, wParam, lParam, bHandled); break;
    case WM_MOUSEACTIVATE: lRes = OnMouseActivate(uMsg, wParam, lParam, bHandled); break;
    default:
        bHandled = FALSE;
    }
    if( !bHandled ) return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
    return lRes;
}

LRESULT CIEActiveXWnd::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if( m_pOwner->m_pViewObject == NULL ) bHandled = FALSE;
    return 1;
}

LRESULT CIEActiveXWnd::OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    IOleObject* pUnk = NULL;
    m_pOwner->m_pOwner->GetControl(IID_IOleObject, (LPVOID*) &pUnk);
    if( pUnk == NULL ) return 0;
    CSafeRelease<IOleObject> RefOleObject = pUnk;
    DWORD dwMiscStatus = 0;
    pUnk->GetMiscStatus(DVASPECT_CONTENT, &dwMiscStatus);
    if( (dwMiscStatus & OLEMISC_NOUIACTIVATE) != 0 ) return 0;
    if( !m_pOwner->m_bInPlaceActive ) DoVerb(OLEIVERB_INPLACEACTIVATE);
    bHandled = FALSE;
    return 0;
}

LRESULT CIEActiveXWnd::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    bHandled = FALSE;
    m_pOwner->m_bFocused = true;
    if( !m_pOwner->m_bUIActivated ) DoVerb(OLEIVERB_UIACTIVATE);
    return 0;
}

LRESULT CIEActiveXWnd::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    bHandled = FALSE;
    m_pOwner->m_bFocused = false;
    return 0;
}

LRESULT CIEActiveXWnd::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    PAINTSTRUCT ps = { 0 };
    ::BeginPaint(m_hWnd, &ps);
    ::EndPaint(m_hWnd, &ps);
    return 1;
}


/////////////////////////////////////////////////////////////////////////////////////
//
//
LPCTSTR FEATURE_SCRIPTURL_MITIGATION = L"Software\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\FEATURE_SCRIPTURL_MITIGATION";
LPCTSTR FEATURE_WARN_ON_SEC_CERT_REV_FAILED = L"SOFTWARE\\Microsoft\\Internet Explorer\\MAIN\\FeatureControl\\FEATURE_WARN_ON_SEC_CERT_REV_FAILED";
CIEActiveXUI::CIEActiveXUI() : m_pUnk(NULL), m_pControl(NULL), m_hwndHost(NULL), m_bCreated(false), m_bDelayCreate(true), m_HostUIHandler(NULL)
{
    m_clsid = IID_NULL;

	// http://msdn.microsoft.com/en-us/library/ee330735(v=vs.85).aspx#url_mitigate
	//HKEY_LOCAL_MACHINE (or HKEY_CURRENT_USER) 
	//     SOFTWARE
	//          Microsoft
	//               Internet Explorer
	//                    Main
	//                         FeatureControl
	//                              FEATURE_SCRIPTURL_MITIGATION
	//                                   contoso.exe = (DWORD) 00000000
	HKEY hk;
	LONG lRet;
	DWORD dw;
	lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, FEATURE_SCRIPTURL_MITIGATION, 0,KEY_ALL_ACCESS,&hk);
	if (lRet != ERROR_SUCCESS)
		lRet = RegCreateKeyEx(HKEY_LOCAL_MACHINE, FEATURE_SCRIPTURL_MITIGATION, 0L, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &hk, &dw);

	if (lRet == ERROR_SUCCESS)
	{
		DWORD dwEnable = 00000001;
		lRet = RegSetValueEx(hk, L"MyIE.exe", 0, REG_DWORD,(LPBYTE)&dwEnable, sizeof(DWORD));
		RegCloseKey(hk);
	}
}

CIEActiveXUI::~CIEActiveXUI()
{
	if(m_HostUIHandler)
	{
		m_HostUIHandler->Release();
		m_HostUIHandler = NULL;
	}
    ReleaseControl();
}

LPCTSTR CIEActiveXUI::GetClass() const
{
    return _T("IEUI");
}

LPVOID CIEActiveXUI::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, _T("IE")) == 0 ) return static_cast<CIEActiveXUI*>(this);
	return CControlUI::GetInterface(pstrName);
}

HWND CIEActiveXUI::GetHostWindow() const
{
    return m_hwndHost;
}

static void PixelToHiMetric(const SIZEL* lpSizeInPix, LPSIZEL lpSizeInHiMetric)
{
#define HIMETRIC_PER_INCH   2540
#define MAP_PIX_TO_LOGHIM(x,ppli)   MulDiv(HIMETRIC_PER_INCH, (x), (ppli))
#define MAP_LOGHIM_TO_PIX(x,ppli)   MulDiv((ppli), (x), HIMETRIC_PER_INCH)
    int nPixelsPerInchX;    // Pixels per logical inch along width
    int nPixelsPerInchY;    // Pixels per logical inch along height
    HDC hDCScreen = ::GetDC(NULL);
    nPixelsPerInchX = ::GetDeviceCaps(hDCScreen, LOGPIXELSX);
    nPixelsPerInchY = ::GetDeviceCaps(hDCScreen, LOGPIXELSY);
    ::ReleaseDC(NULL, hDCScreen);
    lpSizeInHiMetric->cx = MAP_PIX_TO_LOGHIM(lpSizeInPix->cx, nPixelsPerInchX);
    lpSizeInHiMetric->cy = MAP_PIX_TO_LOGHIM(lpSizeInPix->cy, nPixelsPerInchY);
}

void CIEActiveXUI::SetVisible(bool bVisible)
{
    CControlUI::SetVisible(bVisible);
    if( m_hwndHost != NULL ) ::ShowWindow(m_hwndHost, IsVisible() ? SW_SHOW : SW_HIDE);
}

void CIEActiveXUI::SetInternVisible(bool bVisible)
{
    CControlUI::SetInternVisible(bVisible);
    if( m_hwndHost != NULL ) ::ShowWindow(m_hwndHost, IsVisible() ? SW_SHOW : SW_HIDE);
}

void CIEActiveXUI::SetPos(RECT rc)
{
    CControlUI::SetPos(rc);

    if( !m_bCreated ) DoCreateControl();

    if( m_pUnk == NULL ) return;
    if( m_pControl == NULL ) return;

    SIZEL hmSize = { 0 };
    SIZEL pxSize = { 0 };
    pxSize.cx = m_rcItem.right - m_rcItem.left;
    pxSize.cy = m_rcItem.bottom - m_rcItem.top;
    PixelToHiMetric(&pxSize, &hmSize);

    if( m_pUnk != NULL ) {
        m_pUnk->SetExtent(DVASPECT_CONTENT, &hmSize);
    }
    if( m_pControl->m_pInPlaceObject != NULL ) {
        CRect rcItem = m_rcItem;
        if( !m_pControl->m_bWindowless ) rcItem.ResetOffset();
        m_pControl->m_pInPlaceObject->SetObjectRects(&rcItem, &rcItem);
    }
    if( !m_pControl->m_bWindowless ) {
        ASSERT(m_pControl->m_pWindow);
        ::MoveWindow(*m_pControl->m_pWindow, m_rcItem.left, m_rcItem.top, m_rcItem.right - m_rcItem.left, m_rcItem.bottom - m_rcItem.top, TRUE);
    }
}

void CIEActiveXUI::DoPaint(HDC hDC, const RECT& rcPaint)
{
    if( !::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem) ) return;

    if( m_pControl != NULL && m_pControl->m_bWindowless && m_pControl->m_pViewObject != NULL )
    {
        m_pControl->m_pViewObject->Draw(DVASPECT_CONTENT, -1, NULL, NULL, NULL, hDC, (RECTL*) &m_rcItem, (RECTL*) &m_rcItem, NULL, NULL); 
    }
}

void CIEActiveXUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
    if( _tcscmp(pstrName, _T("clsid")) == 0 ) CreateControl(pstrValue);
    else if( _tcscmp(pstrName, _T("modulename")) == 0 ) SetModuleName(pstrValue);
    else if( _tcscmp(pstrName, _T("delaycreate")) == 0 ) SetDelayCreate(_tcscmp(pstrValue, _T("true")) == 0);
    else CControlUI::SetAttribute(pstrName, pstrValue);
}

LRESULT CIEActiveXUI::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    if( m_pControl == NULL ) return 0;
    ASSERT(m_pControl->m_bWindowless);
    if( !m_pControl->m_bInPlaceActive ) return 0;
    if( m_pControl->m_pInPlaceObject == NULL ) return 0;
    if( !IsMouseEnabled() && uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST ) return 0;
    bool bWasHandled = true;
    if( (uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST) || uMsg == WM_SETCURSOR ) {
        // Mouse message only go when captured or inside rect
        DWORD dwHitResult = m_pControl->m_bCaptured ? HITRESULT_HIT : HITRESULT_OUTSIDE;
        if( dwHitResult == HITRESULT_OUTSIDE && m_pControl->m_pViewObject != NULL ) {
            IViewObjectEx* pViewEx = NULL;
            m_pControl->m_pViewObject->QueryInterface(IID_IViewObjectEx, (LPVOID*) &pViewEx);
            if( pViewEx != NULL ) {
                POINT ptMouse = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                pViewEx->QueryHitPoint(DVASPECT_CONTENT, &m_rcItem, ptMouse, 0, &dwHitResult);
                pViewEx->Release();
            }
        }
        if( dwHitResult != HITRESULT_HIT ) return 0;
        if( uMsg == WM_SETCURSOR ) bWasHandled = false;
    }
    else if( uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST ) {
        // Keyboard messages just go when we have focus
        if( !IsFocused() ) return 0;
    }
    else {
        switch( uMsg ) {
        case WM_HELP:
        case WM_CONTEXTMENU:
            bWasHandled = false;
            break;
        default:
            return 0;
        }
    }
    LRESULT lResult = 0;
    HRESULT Hr = m_pControl->m_pInPlaceObject->OnWindowMessage(uMsg, wParam, lParam, &lResult);
    if( Hr == S_OK ) bHandled = bWasHandled;
    return lResult;
}

bool CIEActiveXUI::IsDelayCreate() const
{
    return m_bDelayCreate;
}

void CIEActiveXUI::SetDelayCreate(bool bDelayCreate)
{
    if( m_bDelayCreate == bDelayCreate ) return;
    if( bDelayCreate == false ) {
        if( m_bCreated == false && m_clsid != IID_NULL ) DoCreateControl();
    }
    m_bDelayCreate = bDelayCreate;
}

bool CIEActiveXUI::CreateControl(LPCTSTR pstrCLSID)
{
    CLSID clsid = { 0 };
    OLECHAR szCLSID[100] = { 0 };
#ifndef _UNICODE
    ::MultiByteToWideChar(::GetACP(), 0, pstrCLSID, -1, szCLSID, lengthof(szCLSID) - 1);
#else
    _tcsncpy(szCLSID, pstrCLSID, lengthof(szCLSID) - 1);
#endif
    if( pstrCLSID[0] == '{' ) ::CLSIDFromString(szCLSID, &clsid);
    else ::CLSIDFromProgID(szCLSID, &clsid);
    return CreateControl(clsid);
}

bool CIEActiveXUI::CreateControl(const CLSID clsid)
{
    ASSERT(clsid!=IID_NULL);
    if( clsid == IID_NULL ) return false;
    m_bCreated = false;
    m_clsid = clsid;
    if( !m_bDelayCreate ) DoCreateControl();
    return true;
}

void CIEActiveXUI::ReleaseControl()
{
    m_hwndHost = NULL;
    if( m_pUnk != NULL ) {
        IObjectWithSite* pSite = NULL;
        m_pUnk->QueryInterface(IID_IObjectWithSite, (LPVOID*) &pSite);
        if( pSite != NULL ) {
            pSite->SetSite(NULL);
            pSite->Release();
        }
        m_pUnk->Close(OLECLOSE_NOSAVE);
        m_pUnk->SetClientSite(NULL);
        m_pUnk->Release(); 
        m_pUnk = NULL;
    }
    if( m_pControl != NULL ) {
        m_pControl->m_pOwner = NULL;
        m_pControl->Release();
        m_pControl = NULL;
    }
    m_pManager->RemoveMessageFilter(this);
}

typedef HRESULT (__stdcall *DllGetClassObjectFunc)(REFCLSID rclsid, REFIID riid, LPVOID* ppv); 

bool CIEActiveXUI::DoCreateControl()
{
    ReleaseControl();
    // At this point we'll create the ActiveX control
    m_bCreated = true;
    IOleControl* pOleControl = NULL;

    HRESULT Hr = -1;
    if( !m_sModuleName.IsEmpty() ) {
        HMODULE hModule = ::LoadLibrary((LPCTSTR)m_sModuleName);
        if( hModule != NULL ) {
            IClassFactory* aClassFactory = NULL;
            DllGetClassObjectFunc aDllGetClassObjectFunc = (DllGetClassObjectFunc)::GetProcAddress(hModule, "DllGetClassObject");
            Hr = aDllGetClassObjectFunc(m_clsid, IID_IClassFactory, (LPVOID*)&aClassFactory);
            if( SUCCEEDED(Hr) ) {
                Hr = aClassFactory->CreateInstance(NULL, IID_IOleObject, (LPVOID*)&pOleControl);
            }
            aClassFactory->Release();
        }
    }
    if( FAILED(Hr) ) {
        Hr = ::CoCreateInstance(m_clsid, NULL, CLSCTX_ALL, IID_IOleControl, (LPVOID*)&pOleControl);
    }
    ASSERT(SUCCEEDED(Hr));
    if( FAILED(Hr) ) return false;
    pOleControl->QueryInterface(IID_IOleObject, (LPVOID*) &m_pUnk);
    pOleControl->Release();
    if( m_pUnk == NULL ) return false;
    // Create the host too
    m_pControl = new CIEActiveXCtrl();
    m_pControl->m_pOwner = this;
    // More control creation stuff
    DWORD dwMiscStatus = 0;
    m_pUnk->GetMiscStatus(DVASPECT_CONTENT, &dwMiscStatus);
    IOleClientSite* pOleClientSite = NULL;
    m_pControl->QueryInterface(IID_IOleClientSite, (LPVOID*) &pOleClientSite);
    CSafeRelease<IOleClientSite> RefOleClientSite = pOleClientSite;
    // Initialize control
    if( (dwMiscStatus & OLEMISC_SETCLIENTSITEFIRST) != 0 ) m_pUnk->SetClientSite(pOleClientSite);
    IPersistStreamInit* pPersistStreamInit = NULL;
    m_pUnk->QueryInterface(IID_IPersistStreamInit, (LPVOID*) &pPersistStreamInit);
    if( pPersistStreamInit != NULL ) {
        Hr = pPersistStreamInit->InitNew();
        pPersistStreamInit->Release();
    }
    if( FAILED(Hr) ) return false;
    if( (dwMiscStatus & OLEMISC_SETCLIENTSITEFIRST) == 0 ) m_pUnk->SetClientSite(pOleClientSite);
    // Grab the view...
    Hr = m_pUnk->QueryInterface(IID_IViewObjectEx, (LPVOID*) &m_pControl->m_pViewObject);
    if( FAILED(Hr) ) Hr = m_pUnk->QueryInterface(IID_IViewObject2, (LPVOID*) &m_pControl->m_pViewObject);
    if( FAILED(Hr) ) Hr = m_pUnk->QueryInterface(IID_IViewObject, (LPVOID*) &m_pControl->m_pViewObject);
    // Activate and done...
    m_pUnk->SetHostNames(OLESTR("UIActiveX"), NULL);
    if( m_pManager != NULL ) m_pManager->SendNotify((CControlUI*)this, _T("showactivex"), 0, 0, false);
    if( (dwMiscStatus & OLEMISC_INVISIBLEATRUNTIME) == 0 ) {
        Hr = m_pUnk->DoVerb(OLEIVERB_INPLACEACTIVATE, NULL, pOleClientSite, 0, m_pManager->GetPaintWindow(), &m_rcItem);
        //::RedrawWindow(m_pManager->GetPaintWindow(), &m_rcItem, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_INTERNALPAINT | RDW_FRAME);
    }
    IObjectWithSite* pSite = NULL;
    m_pUnk->QueryInterface(IID_IObjectWithSite, (LPVOID*) &pSite);
    if( pSite != NULL ) {
        pSite->SetSite(static_cast<IOleClientSite*>(m_pControl));
        pSite->Release();
    }

	m_pControl->RegisterBrowserEventSink();
	
	IDocHostUIHandler* pHandler = NULL;
	m_pControl->QueryInterface(IID_IDocHostUIHandler, (LPVOID*)&pHandler);
	if (pHandler != NULL)
		pHandler->Release();
    return SUCCEEDED(Hr);
}

HRESULT CIEActiveXUI::GetControl(const IID iid, LPVOID* ppRet)
{
    ASSERT(ppRet!=NULL);
    ASSERT(*ppRet==NULL);
    if( ppRet == NULL ) return E_POINTER;
    if( m_pUnk == NULL ) return E_PENDING;
    return m_pUnk->QueryInterface(iid, (LPVOID*) ppRet);
}

CLSID CIEActiveXUI::GetClisd() const
{
	return m_clsid;
}

CStdString CIEActiveXUI::GetModuleName() const
{
    return m_sModuleName;
}

void CIEActiveXUI::SetModuleName(LPCTSTR pstrText)
{
    m_sModuleName = pstrText;
}

void CIEActiveXUI::SetExternalUIHandler(IDocHostUIHandler* handler)
{
	if(m_HostUIHandler == handler)
		return;
	if(m_HostUIHandler)
	{
		m_HostUIHandler->Release();
		m_HostUIHandler = NULL;
	}

	m_HostUIHandler = handler;

	if(m_HostUIHandler)
	{
		m_HostUIHandler->AddRef();
	}
}


//------------------------------------------------------------------------
//Description:
//  Browse to a URL
//
//------------------------------------------------------------------------
BOOL CIEActiveXUI::NavigateToURL(const WCHAR * pcszURL)
{
	HRESULT hr;
	// Check for the browser.
	if (NULL == m_pControl || NULL == m_pControl->m_pWB2)
	{
		return E_FAIL; 
	}

	// Package the URL as a BSTR for Navigate.
	VARIANT varURL;
	V_VT(&varURL) = VT_EMPTY; //It's the same as : varURL.vt = VT_EMPTY;
	if(pcszURL && pcszURL[0] != '/0')
	{
		V_VT(&varURL) = VT_BSTR; //It's the same as : varURL.vt = VT_BSTR;
		varURL.bstrVal = SysAllocString(pcszURL);
		if(varURL.bstrVal)
		{
			// Call IWebBrowser2::Navigate2 with no special options.
			hr = m_pControl->m_pWB2->Navigate2(&varURL, NULL, NULL, NULL, NULL);  
		}
	}
	else
	{
		// If there is no URL, go to the default homepage. 
		hr = m_pControl->m_pWB2->GoHome();
	}
	// Clean up the BSTR if it exists.
	VariantClear(&varURL);

	return SUCCEEDED(hr);

}

//---------------------------------------------------------------------
//Description:
// Reloads the file that is currently displayed in the object.
//
//----------------------------------------------------------------------
BOOL CIEActiveXUI::Refresh()
{
	if (NULL == m_pControl || NULL == m_pControl->m_pWB2)
	{
		return E_FAIL;
	}

	return SUCCEEDED(m_pControl->m_pWB2->Refresh());
}

//---------------------------------------------------------------------
//Description:
// Cancels any pending navigation or download operation and stops any dynamic page elements, 
//such as background sounds and animations 
//
//----------------------------------------------------------------------
BOOL CIEActiveXUI::Stop()
{
	if (NULL == m_pControl || NULL == m_pControl->m_pWB2)
	{
		return E_FAIL;
	}

	return SUCCEEDED(m_pControl->m_pWB2->Stop());
}


//---------------------------------------------------------------------
//Description:
// Navigates backward one item in the history list.
//
//----------------------------------------------------------------------
BOOL CIEActiveXUI::GoBack()
{
	if (NULL == m_pControl || NULL == m_pControl->m_pWB2)
	{
		return E_FAIL;
	}

	return SUCCEEDED(m_pControl->m_pWB2->GoBack());
}


//---------------------------------------------------------------------
//Description:
// Navigates forward one item in the history list.
//
//----------------------------------------------------------------------
BOOL CIEActiveXUI::GoForward()
{
	if (NULL == m_pControl || NULL == m_pControl->m_pWB2)
	{
		return E_FAIL;
	}

	return SUCCEEDED(m_pControl->m_pWB2->GoForward());
}

} // namespace DuiLib