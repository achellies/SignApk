#if !defined(AFX_ATLDATAOBJ_H__20040706_4F49_248D_7C5E_0080AD509054__INCLUDED_)
#define AFX_ATLDATAOBJ_H__20040706_4F49_248D_7C5E_0080AD509054__INCLUDED_

#pragma once

/////////////////////////////////////////////////////////////////////////////
// IDataObject and OLE clipboard wrappers
//
// Written by Bjarke Viksoe (bjarke@viksoe.dk)
// Copyright (c) 2004 - 2009 Bjarke Viksoe.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever. It's free, so don't hassle me about it.
//
// Beware of bugs.
//


#ifndef __cplusplus
  #error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLCOM_H__
  #error atldispa.h requires atlcom.h to be included first
#endif


/////////////////////////////////////////////////////////////////////////
// Standard Clipboard

class CClipboard
{
public:
   BOOL m_bOpened;

   CClipboard() : m_bOpened(FALSE)
   {
   }

   CClipboard(HWND hWnd) : m_bOpened(FALSE)
   {
      Open(hWnd);
   }

   ~CClipboard()
   {
      Close();
   }

   BOOL Open(HWND hWnd)
   {
      ATLASSERT(hWnd==NULL || ::IsWindow(hWnd));  // See Q92530
      Close();
      BOOL bRes = m_bOpened = ::OpenClipboard(hWnd);
      return bRes;
   }

   void Close()
   {
      if( !m_bOpened ) return;
      ::CloseClipboard();
      m_bOpened = FALSE;
   }

   BOOL Empty()
   {
      ATLASSERT(m_bOpened);
      return ::EmptyClipboard();
   }

   BOOL IsOpen()
   {
      return m_bOpened;
   }

   int GetFormatCount() const
   {
      return ::CountClipboardFormats();
   }

   UINT EnumFormats(UINT uFormat)
   {
      ATLASSERT(m_bOpened);
      return ::EnumClipboardFormats(uFormat);
   }

   void EnumFormats(CSimpleValArray<UINT>& aFormats)
   {
      ATLASSERT(m_bOpened);
      UINT uFormat = 0;
      while( true ) {
         uFormat = ::EnumClipboardFormats(uFormat);
         if( uFormat == 0 ) break;
         aFormats.Add(uFormat);
      }
   }

   BOOL IsFormatAvailable(UINT uFormat) const
   {
      return ::IsClipboardFormatAvailable(uFormat);
   }

   int GetFormatName(UINT uFormat, LPTSTR pstrName, int cchMax) const
   {
      // NOTE: Doesn't return names of predefined cf!
      return ::GetClipboardFormatName(uFormat, pstrName, cchMax);
   }

   HANDLE GetData(UINT uFormat) const
   {
      ATLASSERT(m_bOpened);
      return ::GetClipboardData(uFormat);
   }

#if defined(_WTL_USE_CSTRING) || defined(__ATLSTR_H__)

   BOOL GetTextData(_CSTRING_NS::CString& sText) const
   {
      ATLASSERT(m_bOpened);
      // Look for UNICODE version first because there's a better
      // chance to convert to the correct locale.
      HGLOBAL hMem = ::GetClipboardData(CF_UNICODETEXT);
      if( hMem != NULL ) {
         sText = (LPCWSTR) GlobalLock(hMem);
         return GlobalUnlock(hMem);
      }
      hMem = ::GetClipboardData(CF_TEXT);
      if( hMem != NULL ) {
         sText = (LPCSTR) GlobalLock(hMem);
         return GlobalUnlock(hMem);
      }
      return FALSE;
   }

   BOOL GetFormatName(UINT uFormat, _CSTRING_NS::CString& sName) const
   {
      for( int nSize = 256; ; nSize *= 2 ) {
         int nLen = ::GetClipboardFormatName(uFormat, sName.GetBufferSetLength(nSize), nSize);
         sName.ReleaseBuffer();
         if( nLen < nSize - 1 ) return TRUE;
      }
      return TRUE;  // Hmm, unreachable!
   }

#endif // _WTL_USE_CSTRING

   HANDLE SetData(UINT uFormat, HANDLE hMem)
   {
      ATLASSERT(m_bOpened);
      ATLASSERT(hMem!=NULL);   // No support for WM_RENDERFORMAT here! 
                               // Enjoy the ASSERT for NULL!
      return ::SetClipboardData(uFormat, hMem);
   }

   HANDLE SetData(UINT uFormat, LPCVOID pData, int iSize)
   {
      HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE, (SIZE_T) iSize);
      if( hGlobal == NULL ) return NULL;
      memcpy(GlobalLock(hGlobal), pData, (size_t) iSize);
      GlobalUnlock(hGlobal);
      HANDLE hHandle = ::SetClipboardData(uFormat, hGlobal);
      if( hHandle == NULL ) GlobalFree(hGlobal);
      return hHandle;
   }

   BOOL SetTextData(LPCSTR pstrText)
   {
      return SetData(CF_TEXT, pstrText, lstrlenA(pstrText) + 1) != NULL;
   }

   BOOL SetUnicodeTextData(LPCWSTR pstrText)
   {
      return SetData(CF_UNICODETEXT, pstrText, (lstrlenW(pstrText) + 1) * sizeof(WCHAR)) != NULL;
   }

   static HWND GetOwner()
   {
      return ::GetClipboardOwner();
   }

#if (WINVER >= 0x0500)

   static DWORD GetSequenceNumber()
   {
      return ::GetClipboardSequenceNumber();
   }

#endif // WINVER

   static UINT RegisterFormat(LPCTSTR pstrFormat)
   {
      return ::RegisterClipboardFormat(pstrFormat);
   }
};


/////////////////////////////////////////////////////////////////////////
// Misc clipboard functions

#ifdef __ATLCONV_H__

inline BOOL AtlSetClipboardText(HWND hWnd, LPCTSTR pstrText)
{
   USES_CONVERSION;
   CClipboard cb;
   if( !cb.Open(hWnd) ) return FALSE;
   cb.Empty();
   cb.SetTextData(T2CA(pstrText));
   cb.SetUnicodeTextData(T2CW(pstrText));
   cb.Close();
   return TRUE;
}

#endif // __ATLCONV_H__


/////////////////////////////////////////////////////////////////////////
// CRawDropSource

class CRawDropSource : public IDropSource
{
public:
   // IUnknown

   STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppvObject)
   {
      if( riid == __uuidof(IDropSource) || riid == IID_IUnknown ) {
         *ppvObject = static_cast<IDropSource*>(this);
         return S_OK;
      }
      *ppvObject = NULL;
      return E_NOINTERFACE;
   }

   ULONG STDMETHODCALLTYPE AddRef()
   {
      return 1;
   }

   ULONG STDMETHODCALLTYPE Release()
   {
      return 1;
   }
   
   // IDropSource
   
   STDMETHOD(QueryContinueDrag)(BOOL bEsc, DWORD dwKeyState)
   {
      if( bEsc ) return ResultFromScode(DRAGDROP_S_CANCEL);
      if( (dwKeyState & MK_LBUTTON) == 0 ) return ResultFromScode(DRAGDROP_S_DROP);
      return S_OK;
   }

   STDMETHOD(GiveFeedback)(DWORD)
   {
      return ResultFromScode(DRAGDROP_S_USEDEFAULTCURSORS);
   }
};


//////////////////////////////////////////////////////////////////////////////
// CRawDropTarget

template< class T >
class CRawDropTarget : public IDropTarget
{
public:
   CRawDropTarget() : m_hwndTarget(NULL), m_fAcceptFmt(false), nFormats(0)
   { 
   }

   HRESULT RegisterDropTarget(HWND hWnd)
   {
      m_hwndTarget = hWnd;
      ::RegisterDragDrop(m_hwndTarget, this);
      return S_OK;
   }

   void RevokeDropTarget()
   {
      ::RevokeDragDrop(m_hwndTarget);
   }

   void AddDropFormat(FORMATETC fe)
   {
      if( nFormats >= sizeof(m_Formats) / sizeof(FORMATETC) ) return;
      m_Formats[nFormats++] = fe;
   }

   FORMATETC GetDropFormat(int iIndex) const
   {
      FORMATETC fe = { 0 };
      if( iIndex < 0 || iIndex >= nFormats) return fe;
      return m_Formats[iIndex];
   }

   // Attributes

   HWND m_hwndTarget;
   bool m_fAcceptFmt;
   FORMATETC m_Formats[8];
   int nFormats;

   // IUnknown

   STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppvObject)
   {
      if( riid == __uuidof(IDropTarget) || riid == IID_IUnknown ) {
         *ppvObject = static_cast<IDropTarget*>(this);
         return S_OK;
      }
      *ppvObject = NULL;
      return E_NOINTERFACE;
   }

   ULONG STDMETHODCALLTYPE AddRef()
   {
      return 1;
   }

   ULONG STDMETHODCALLTYPE Release()
   {
      return 1;
   }

   // IDropTarget

   STDMETHOD(DragEnter)(LPDATAOBJECT pDataObj, DWORD dwKeyState, POINTL /*ptl*/, LPDWORD pdwEffect)
   {
      // Does the drag source provide our CF types?
      m_fAcceptFmt = false;
      for( int i = 0; !m_fAcceptFmt && i < nFormats; i++ ) {
         FORMATETC fe = m_Formats[i];
         m_fAcceptFmt = (S_OK == pDataObj->QueryGetData(&fe));
      }
      *pdwEffect = _QueryDrop(dwKeyState, *pdwEffect);
      return S_OK;
   }

   STDMETHOD(DragOver)(DWORD dwKeyState, POINTL /*pt*/, LPDWORD pdwEffect)
   {
      *pdwEffect = _QueryDrop(dwKeyState, *pdwEffect);
      return S_OK;
   } 

   STDMETHOD(DragLeave)(VOID)
   {
      m_fAcceptFmt = false;
      return S_OK;
   }

   STDMETHOD(Drop)(LPDATAOBJECT pDataObj, DWORD dwKeyState, POINTL /*ptl*/, LPDWORD pdwEffect)
   {
      // Determine drop effect...
      DWORD dwDropEffect = _QueryDrop(dwKeyState, *pdwEffect);
      *pdwEffect = DROPEFFECT_NONE; // Default to failed/cancelled
      // Did we accept this drop effect?
      if( dwDropEffect == DROPEFFECT_NONE ) return S_OK;
      // Do the drop...
      T* pT = static_cast<T*>(this); pT;
      BOOL bRes = pT->DoDrop(pDataObj);
      *pdwEffect = bRes ? dwDropEffect : DROPEFFECT_NONE;
      return bRes ? S_OK : E_FAIL;
   }

   // Implementation

   DWORD _QueryDrop(DWORD dwKeyState, DWORD dwEffect) const
   {
      if( !m_fAcceptFmt ) return DROPEFFECT_NONE;
      // Test key-state
      DWORD dwMask = _GetDropEffectFromKeyState(dwKeyState);
      if( (dwEffect & dwMask) != 0 ) return dwEffect & dwMask;
      // Map common alternatives
      if( (dwEffect & DROPEFFECT_COPY) != 0 ) return DROPEFFECT_COPY;
      // Drop-effect is determined by keys or default
      return dwMask;
   }

   DWORD _GetDropEffectFromKeyState(DWORD dwKeyState) const
   {
      // We don't support DROPEFFECT_LINK nor DROPEFFECT_MOVE operations
      DWORD dwDropEffect = DROPEFFECT_COPY;
      if( (dwKeyState & MK_CONTROL) != 0 ) dwDropEffect = DROPEFFECT_COPY;
      return dwDropEffect;
   }
};


#ifdef __ATLCOM_H__

//////////////////////////////////////////////////////////////////////////////
// CEnumFORMATETC

class ATL_NO_VTABLE CEnumFORMATETC : 
   public CComObjectRootEx<CComSingleThreadModel>,
   public IEnumFORMATETC
{
public:
   ULONG m_iCur;                              // Current index
   CSimpleValArray<FORMATETC> m_aFmtEtc;      // List for clipboard formats

   BEGIN_COM_MAP(CEnumFORMATETC)
      COM_INTERFACE_ENTRY(IEnumFORMATETC)
   END_COM_MAP()

public:
   CEnumFORMATETC() : m_iCur(0)
   {
   }

   BOOL Add(const FORMATETC& fmtc)
   {
      FORMATETC fm = fmtc;
      return m_aFmtEtc.Add(fm);
   }

   // Implementation

   FORMATETC _CopyFormatEtc(const FORMATETC& fmtc) const
   {
      FORMATETC fm = fmtc;
      if( fm.ptd != NULL ) {
         fm.ptd = (DVTARGETDEVICE*) ::CoTaskMemAlloc(sizeof(DVTARGETDEVICE));
         if( fm.ptd != NULL ) ::CopyMemory(fm.ptd, fmtc.ptd, sizeof(DVTARGETDEVICE));
      }
      return fm;
   }

   // IEnumFORMATETC

   STDMETHOD(Next)(ULONG celt, LPFORMATETC lpFormatEtc, ULONG* pceltFetched)
   {
      if( pceltFetched != NULL) *pceltFetched = 0;
      if( lpFormatEtc == NULL ) return E_INVALIDARG;
      if( celt <= 0 || m_iCur >= (ULONG) m_aFmtEtc.GetSize() ) return S_FALSE;
      if( pceltFetched == NULL && celt != 1 ) return S_FALSE;
      ULONG nCount = 0;
      while( m_iCur < (ULONG) m_aFmtEtc.GetSize() && celt > 0 ) {
         *lpFormatEtc++ = _CopyFormatEtc(m_aFmtEtc[m_iCur++]);
         --celt;
         ++nCount;
      }
      if( pceltFetched != NULL ) *pceltFetched = nCount;
      return celt == 0 ? S_OK : S_FALSE;
   }

   STDMETHOD(Skip)(ULONG celt)
   {
      if( m_iCur + celt >= (ULONG) m_aFmtEtc.GetSize() ) return S_FALSE;
      m_iCur += celt;
      return S_OK;
   }

   STDMETHOD(Reset)(void)
   {
      m_iCur = 0;
      return S_OK;
   }

   STDMETHOD(Clone)(IEnumFORMATETC**)
   {
      ATLTRACENOTIMPL(_T("CEnumFORMATETC::Clone"));
   }
};


//////////////////////////////////////////////////////////////////////////////
// CSimpleDataObj

template< class T >
class ATL_NO_VTABLE ISimpleDataObjImpl : public IDataObject
{
public:
   typedef struct tagDATAOBJ
   {
      FORMATETC FmtEtc;
      STGMEDIUM StgMed;
   } DATAOBJ;

   CSimpleArray<DATAOBJ> m_aObjects;

   ~ISimpleDataObjImpl()
   {
      for( int i = 0; i < m_aObjects.GetSize(); i++ ) ::ReleaseStgMedium(&m_aObjects[i].StgMed);
   }

   STDMETHOD(GetData)(FORMATETC* pFormatEtc, STGMEDIUM* pStgMedium)
   {
      ATLTRACE2(atlTraceControls,2,_T("ISimpleDataObjImpl::GetData\n"));
      T* pT = static_cast<T*>(this); pT;
      return pT->IDataObject_GetData(pFormatEtc, pStgMedium);
   }

   STDMETHOD(GetDataHere)(FORMATETC* /*pFormatEtc*/, STGMEDIUM* /*pmedium*/)
   {
      ATLTRACENOTIMPL(_T("ISimpleDataObjImpl::GetDataHere"));
   }

   STDMETHOD(QueryGetData)(FORMATETC* pFormatEtc)
   {
      ATLASSERT(pFormatEtc);
      return _FindFormat(pFormatEtc) >= 0 ? S_OK : DV_E_FORMATETC;
   }

   STDMETHOD(GetCanonicalFormatEtc)(FORMATETC* pFormatEctIn, FORMATETC* /*pFormatEtcOut*/)
   {
      pFormatEctIn->ptd = NULL;
      ATLTRACENOTIMPL(_T("ISimpleDataObjImpl::GetCanonicalFormatEtc"));
   }

   STDMETHOD(SetData)(FORMATETC* pFormatEtc, STGMEDIUM* pStgMedium, BOOL fRelease)
   {
      ATLASSERT(pFormatEtc);
      ATLASSERT(pStgMedium);
      if( pFormatEtc == NULL ) return E_POINTER;
      if( pStgMedium == NULL ) return E_POINTER;
      DATAOBJ obj;
      obj.FmtEtc = *pFormatEtc;
      obj.StgMed = *pStgMedium;
      if( !fRelease ) _CopyStgMedium(&obj.StgMed, pStgMedium, pFormatEtc);
      int iIndex = _FindFormat(pFormatEtc);
      if( iIndex < 0 ) {
         m_aObjects.Add(obj); 
      }
      else {
         ::ReleaseStgMedium(&m_aObjects[iIndex].StgMed);
         m_aObjects.SetAtIndex(iIndex, obj);
      }
      return S_OK;
   }

   STDMETHOD(EnumFormatEtc)(DWORD dwDirection, IEnumFORMATETC** ppEnumFormatEtc)
   {
      ATLASSERT(ppEnumFormatEtc);
      if( ppEnumFormatEtc == NULL ) return E_POINTER;
      *ppEnumFormatEtc = NULL;
      if( dwDirection != DATADIR_GET ) return E_NOTIMPL;
      CComObject<CEnumFORMATETC>* pEnum = NULL;
      if( FAILED( CComObject<CEnumFORMATETC>::CreateInstance(&pEnum) ) ) return E_FAIL;
      for( int i = 0; i < m_aObjects.GetSize(); i++ ) pEnum->Add(m_aObjects[i].FmtEtc);
      return pEnum->QueryInterface(ppEnumFormatEtc);
   }

   STDMETHOD(DAdvise)(FORMATETC* /*pFormatEtc*/, DWORD /*advf*/, IAdviseSink* /*pAdvSink*/, DWORD* /*pdwConnection*/)
   {
      ATLTRACENOTIMPL(_T("ISimpleDataObjImpl::DAdvise"));
   }

   STDMETHOD(DUnadvise)(DWORD /*dwConnection*/)
   {
      ATLTRACENOTIMPL(_T("ISimpleDataObjImpl::DUnadvise"));
   }

   STDMETHOD(EnumDAdvise)(IEnumSTATDATA** /*ppenumAdvise*/)
   {
      ATLTRACENOTIMPL(_T("ISimpleDataObjImpl::EnumDAdvise"));
   }

   // Helper functions

   int _FindFormat(const FORMATETC* pFormatEtc) const
   {
      for( int i = 0; i < m_aObjects.GetSize(); i++ ) {
         if( m_aObjects[i].FmtEtc.cfFormat == pFormatEtc->cfFormat
             && (m_aObjects[i].FmtEtc.tymed & pFormatEtc->tymed) != 0
             && m_aObjects[i].FmtEtc.lindex == pFormatEtc->lindex
             && m_aObjects[i].FmtEtc.dwAspect == pFormatEtc->dwAspect )
         {
            return i;
         }
      }
      return -1;
   }

   HRESULT _AddRefStgMedium(STGMEDIUM* pMedDest, const STGMEDIUM* pMedSrc, const FORMATETC* pFmtSrc) const
   {
      switch( pMedSrc->tymed ) {
      case TYMED_GDI:
      case TYMED_FILE:
      case TYMED_ENHMF:
      case TYMED_MFPICT:
      case TYMED_HGLOBAL:
         if( pMedSrc->pUnkForRelease != NULL ) pMedDest->hGlobal = pMedSrc->hGlobal;
         else pMedDest->hGlobal = (HGLOBAL) ::OleDuplicateData(pMedSrc->hGlobal, pFmtSrc->cfFormat, NULL);
         break;
      case TYMED_ISTREAM:
         pMedDest->pstm = pMedSrc->pstm;
         pMedDest->pstm->AddRef();
         break;
      case TYMED_ISTORAGE:
         pMedDest->pstg = pMedSrc->pstg;
         pMedDest->pstg->AddRef();
         break;
      default:
         ATLASSERT(false);
         return DV_E_TYMED;
      }
      pMedDest->tymed = pMedSrc->tymed;
      pMedDest->pUnkForRelease = pMedSrc->pUnkForRelease;
      if( pMedDest->pUnkForRelease != NULL ) pMedDest->pUnkForRelease->AddRef();
      return S_OK;
   }

   HRESULT _CopyStgMedium(STGMEDIUM* pMedDest, const STGMEDIUM* pMedSrc, const FORMATETC* pFmtSrc) const
   {
      switch( pMedSrc->tymed ) {
      case TYMED_GDI:
      case TYMED_FILE:
      case TYMED_ENHMF:
      case TYMED_MFPICT:
      case TYMED_HGLOBAL:
         pMedDest->hGlobal = (HGLOBAL) ::OleDuplicateData(pMedSrc->hGlobal, pFmtSrc->cfFormat, NULL);
         break;
      case TYMED_ISTREAM:
         {
            pMedDest->pstm = NULL;
            LARGE_INTEGER dlibMove = { 0, 0 };
            ULARGE_INTEGER alot = { 0, INT_MAX };
            if( FAILED( ::CreateStreamOnHGlobal(NULL, TRUE, &pMedDest->pstm) ) ) return E_OUTOFMEMORY;
            if( FAILED( pMedSrc->pstm->Seek(dlibMove, STREAM_SEEK_SET, NULL) ) ) return E_FAIL;
            if( FAILED( pMedDest->pstm->Seek(dlibMove, STREAM_SEEK_SET, NULL) ) ) return E_FAIL;
            if( FAILED( pMedSrc->pstm->CopyTo(pMedDest->pstm, alot, NULL, NULL) ) ) return STG_E_MEDIUMFULL;
            if( FAILED( pMedSrc->pstm->Seek(dlibMove, STREAM_SEEK_SET, NULL) ) ) return E_FAIL;
            if( FAILED( pMedDest->pstm->Seek(dlibMove, STREAM_SEEK_SET, NULL) ) ) return E_FAIL;
         }
         break;
      case TYMED_ISTORAGE:
         {
            pMedDest->pstg = NULL;
            if( FAILED( ::StgCreateDocfile(NULL, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_DELETEONRELEASE | STGM_CREATE, 0, &pMedDest->pstg) ) ) return E_OUTOFMEMORY;
            if( FAILED( pMedSrc->pstg->CopyTo(0, NULL, NULL, pMedDest->pstg) ) ) return STG_E_INSUFFICIENTMEMORY;
         }
         break;
      default:
         ATLASSERT(false);
         return DV_E_TYMED;
      }
      pMedDest->tymed = pMedSrc->tymed;
      pMedDest->pUnkForRelease = pMedSrc->pUnkForRelease;
      if( pMedDest->pUnkForRelease != NULL ) pMedDest->pUnkForRelease->AddRef();
      return S_OK;
   }
};


class ATL_NO_VTABLE CSimpleDataObj : 
   public CComObjectRootEx<CComSingleThreadModel>,
   public ISimpleDataObjImpl<CSimpleDataObj>
{
public:
   BEGIN_COM_MAP(CSimpleDataObj)
      COM_INTERFACE_ENTRY(IDataObject)
   END_COM_MAP()

   HRESULT IDataObject_GetData(FORMATETC* pFormatEtc, STGMEDIUM* pStgMedium)
   {
      ATLASSERT(pFormatEtc);
      ATLASSERT(pStgMedium);
      int iIndex = _FindFormat(pFormatEtc);
      if( iIndex < 0 ) return DV_E_FORMATETC;
      return _AddRefStgMedium(pStgMedium, &m_aObjects[iIndex].StgMed, &m_aObjects[iIndex].FmtEtc);
   }

   // Operations

   HRESULT SetGlobalData(CLIPFORMAT cf, LPCVOID pData, DWORD dwSize)
   {
      FORMATETC fmtetc = { cf, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
      STGMEDIUM stgmed = { TYMED_HGLOBAL, { 0 }, 0 };
      stgmed.hGlobal = ::GlobalAlloc(GMEM_MOVEABLE, (SIZE_T) dwSize);
      if( stgmed.hGlobal == NULL ) return E_OUTOFMEMORY;
      memcpy(GlobalLock(stgmed.hGlobal), pData, (size_t) dwSize);
      GlobalUnlock(stgmed.hGlobal);
      return SetData(&fmtetc, &stgmed, TRUE);
   }

#ifdef _SHLOBJ_H_

   // Include shlobj.h first
   HRESULT SetHDropData(LPCSTR pstrFilename)
   {
      FORMATETC fmtetc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
      STGMEDIUM stgmed = { TYMED_HGLOBAL, { 0 }, 0 };
      DWORD dwSize = sizeof(DROPFILES) + ::lstrlenA(pstrFilename) + 2;
      stgmed.hGlobal = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, dwSize);
      if( stgmed.hGlobal == NULL ) return E_OUTOFMEMORY;
      DROPFILES* pDest = (DROPFILES*) GlobalLock(stgmed.hGlobal);
      pDest->pFiles = sizeof(DROPFILES);
      pDest->fWide = FALSE;
      ::lstrcpyA( ((LPSTR) pDest) + sizeof(DROPFILES), pstrFilename );
      GlobalUnlock(stgmed.hGlobal);
      return SetData(&fmtetc, &stgmed, TRUE);
   }

#endif // _SHLOBJ_H_

   HRESULT SetTextData(LPCSTR pstrData)
   {
      return SetGlobalData(CF_TEXT, pstrData, ::lstrlenA(pstrData) + 1);
   }

   HRESULT SetUnicodeTextData(LPCWSTR pstrData)
   {
      return SetGlobalData(CF_UNICODETEXT, pstrData, (::lstrlenW(pstrData) + 1) * sizeof(WCHAR));
   }
};

#endif // __ATLCOM_H__


#endif // !defined(AFX_ATLDATAOBJ_H__20040706_4F49_248D_7C5E_0080AD509054__INCLUDED_)
