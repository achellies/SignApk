#include "StdAfx.h"
#include "UIlib.h"

namespace DuiLib {

CDialogBuilder::CDialogBuilder() : m_pCallback(NULL), m_pstrtype(NULL)
{

}

CControlUI* CDialogBuilder::Create(STRINGorID xml, LPCTSTR type, IDialogBuilderCallback* pCallback, 
                                   CPaintManagerUI* pManager, CControlUI* pParent)
{
	if (pManager == NULL)
		return NULL;
#if defined(UI_BUILD_FOR_WINCE)
	/*
	The resources in an application, whether from an executable or a DLL, 
	are always mapped to the address space of a process. 
	LoadResource simply retrieves the address of the resource that is already in memory. 
	LoadResource does not allocate any additional memory. 
	The pointer returned by LockResource identifies the address of the block of memory, 
	in the executable or in the DLL, containing the specified resource. 
	The system automatically deletes these resources when the process that loaded them terminates, 
	or when FreeLibrary is used to unload a DLL that was loaded with LoadLibrary.
	*/
#endif
	m_pCallback = pCallback;
	if( HIWORD(xml.m_lpstr) != NULL ) {
		if( *(xml.m_lpstr) == _T('<') ) {
			if( !m_xml.Load(xml.m_lpstr) ) return NULL;
		}
		else {
			if( !m_xml.LoadFromFile(xml.m_lpstr, pManager) ) return NULL;
		}
	}
	else {

		HRSRC hResource = ::FindResource(pManager->GetResourceDll(), xml.m_lpstr, type);
		if( hResource == NULL ) return NULL;
		HGLOBAL hGlobal = ::LoadResource(pManager->GetResourceDll(), hResource);
		if( hGlobal == NULL ) {
#if defined(UI_BUILD_FOR_WIN32) && !defined(UI_BUILD_FOR_WINCE)
			::FreeResource(hResource);
#endif
			return NULL;
		}

		m_pCallback = pCallback;
		if( !m_xml.LoadFromMem((BYTE*)::LockResource(hGlobal), ::SizeofResource(pManager->GetResourceDll(), hResource) )) return NULL;
#if defined(UI_BUILD_FOR_WIN32) && !defined(UI_BUILD_FOR_WINCE)
		::FreeResource(hResource);
#endif
		m_pstrtype = type;
	}
    return Create(pCallback, pManager);
}

CControlUI* CDialogBuilder::Create(IDialogBuilderCallback* pCallback, CPaintManagerUI* pManager, CControlUI* pParent)
{
	m_pCallback = pCallback;
	CMarkupNode root = m_xml.GetRoot();
	if( !root.IsValid() ) return NULL;
	if( pManager ) {
		LPCTSTR pstrClass = NULL;			
		int nAttributes = 0;
		LPCTSTR pstrName = NULL;
		LPCTSTR pstrValue = NULL;
		LPTSTR pstr = NULL;

		for( CMarkupNode node = root.GetChild() ; node.IsValid(); node = node.GetSibling() ) {
			pstrClass = node.GetName();
			/// @brief 图片的来源以及图片文件的名称
			/// example <Image name="tree_expand.png" />
			if( _tcsicmp(pstrClass, _T("Image")) == 0 ) {
				nAttributes = node.GetAttributeCount();
				LPCTSTR pImageName = NULL;
				LPCTSTR pImageResType = NULL;
				DWORD mask = 0;
				for( int i = 0; i < nAttributes; i++ ) {
					pstrName = node.GetAttributeName(i);
					pstrValue = node.GetAttributeValue(i);
					if( _tcsicmp(pstrName, _T("name")) == 0 ) {
						pImageName = pstrValue;
					}
					else if( _tcsicmp(pstrName, _T("restype")) == 0 ) {
						pImageResType = pstrValue;
					}
					else if( _tcsicmp(pstrName, _T("mask")) == 0 ) {
						if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
						mask = _tcstoul(pstrValue, &pstr, 16);
					}
				}
				if( pImageName ) pManager->AddImage(pImageName, pImageResType, mask);
			}
			/// @brief 字体的信息
			/// example : <Font name="微软雅黑" size="18" bold="false" default = "true"/>
			else if( _tcsicmp(pstrClass, _T("Font")) == 0 ) {
				nAttributes = node.GetAttributeCount();
				LPCTSTR pFontName = NULL;
				int size = 12;
				bool bold = false;
				bool underline = false;
				bool italic = false;
				bool defaultfont = false;
				for( int i = 0; i < nAttributes; i++ ) {
					pstrName = node.GetAttributeName(i);
					pstrValue = node.GetAttributeValue(i);
					if( _tcsicmp(pstrName, _T("name")) == 0 ) {
						pFontName = pstrValue;
					}
					else if( _tcsicmp(pstrName, _T("size")) == 0 ) {
						size = _tcstol(pstrValue, &pstr, 10);
					}
					else if( _tcsicmp(pstrName, _T("bold")) == 0 ) {
						bold = (_tcsicmp(pstrValue, _T("true")) == 0);
					}
					else if( _tcsicmp(pstrName, _T("underline")) == 0 ) {
						underline = (_tcsicmp(pstrValue, _T("true")) == 0);
					}
					else if( _tcsicmp(pstrName, _T("italic")) == 0 ) {
						italic = (_tcsicmp(pstrValue, _T("true")) == 0);
					}
					else if( _tcsicmp(pstrName, _T("default")) == 0 ) {
						defaultfont = (_tcsicmp(pstrValue, _T("true")) == 0);
					}

				}
				if( pFontName ) {
					pManager->AddFont(pFontName, size, bold, underline, italic);
					if( defaultfont ) pManager->SetDefaultFont(pFontName, size, bold, underline, italic);

				}
			}
			/// @brief Button VScrollBar HScrollBar 的信息
			/// example : <Default name="Button" value="normalimage=&quot;file='button_nor.bmp' fade='200'&quot; hotimage=&quot;file='button_over.bmp' fade='200'&quot; pushedimage=&quot;file='button_down.bmp' fade='200'&quot; " />	
			else if( _tcsicmp(pstrClass, _T("Default")) == 0 ) {
				nAttributes = node.GetAttributeCount();
				LPCTSTR pControlName = NULL;
				LPCTSTR pControlValue = NULL;
				for( int i = 0; i < nAttributes; i++ ) {
					pstrName = node.GetAttributeName(i);
					pstrValue = node.GetAttributeValue(i);
					if( _tcsicmp(pstrName, _T("name")) == 0 ) {
						pControlName = pstrValue;
					}
					else if( _tcsicmp(pstrName, _T("value")) == 0 ) {
						pControlValue = pstrValue;
					}
				}
				if( pControlName ) {
					pManager->AddDefaultAttributeList(pControlName, pControlValue);
				}
			}
		}

		pstrClass = root.GetName();
		if( _tcsicmp(pstrClass, _T("Window")) == 0 ) {
			if( pManager->GetPaintWindow() ) {
				int nAttributes = root.GetAttributeCount();
				for( int i = 0; i < nAttributes; i++ ) {
					pstrName = root.GetAttributeName(i);
					pstrValue = root.GetAttributeValue(i);
					/// @brief 该窗口的大小
					/// example <Window size="800,600" sizebox="4,4,6,6" caption="0,0,0,90" mininfo="800,570" roundcorner="8,8" showdirty="false" >
					if( _tcsicmp(pstrName, _T("size")) == 0 ) {
						LPTSTR pstr = NULL;
						int cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
						int cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr); 
						pManager->SetInitSize(cx, cy);
					} 
					else if( _tcsicmp(pstrName, _T("sizebox")) == 0 ) {
						RECT rcSizeBox = { 0 };
						LPTSTR pstr = NULL;
						rcSizeBox.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
						rcSizeBox.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
						rcSizeBox.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
						rcSizeBox.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
						pManager->SetSizeBox(rcSizeBox);
					}
					/// 标题栏的宽度高度
					else if( _tcsicmp(pstrName, _T("caption")) == 0 ) {
						RECT rcCaption = { 0 };
						LPTSTR pstr = NULL;
						rcCaption.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
						rcCaption.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
						rcCaption.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
						rcCaption.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
						pManager->SetCaptionRect(rcCaption);
					}
					/// 窗口的圆角矩形的半径
					else if( _tcsicmp(pstrName, _T("roundcorner")) == 0 ) {
						LPTSTR pstr = NULL;
						int cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
						int cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr); 
						pManager->SetRoundCorner(cx, cy);
					} 
					else if( _tcsicmp(pstrName, _T("mininfo")) == 0 ) {
						LPTSTR pstr = NULL;
						int cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
						int cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr); 
						pManager->SetMinInfo(cx, cy);
					}
					else if( _tcsicmp(pstrName, _T("maxinfo")) == 0 ) {
						LPTSTR pstr = NULL;
						int cx = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
						int cy = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr); 
						pManager->SetMaxInfo(cx, cy);
					}						
					// 是否显示需要重画的区域
					else if( _tcsicmp(pstrName, _T("showdirty")) == 0 ) {
						pManager->SetShowUpdateRect(_tcsicmp(pstrValue, _T("true")) == 0);
					}
                    else if( _tcsicmp(pstrName, _T("alpha")) == 0 ) {
                        pManager->SetTransparent(_ttoi(pstrValue));
                    }
					else if( _tcsicmp(pstrName, _T("bktrans")) == 0 ) {
						pManager->SetBackgroundTransparent(_tcsicmp(pstrValue, _T("true")) == 0);
					} 
					else if( _tcsicmp(pstrName, _T("disabledfontcolor")) == 0 ) {
						if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
						LPTSTR pstr = NULL;
						DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
						pManager->SetDefaultDisabledColor(clrColor);
					}
					else if( _tcsicmp(pstrName, _T("defaultfontcolor")) == 0 ) {
						if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
						LPTSTR pstr = NULL;
						DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
						pManager->SetDefaultFontColor(clrColor);
					}
					else if( _tcsicmp(pstrName, _T("linkfontcolor")) == 0 ) {
						if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
						LPTSTR pstr = NULL;
						DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
						pManager->SetDefaultLinkFontColor(clrColor);
					}
					else if( _tcsicmp(pstrName, _T("linkhoverfontcolor")) == 0 ) {
						if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
						LPTSTR pstr = NULL;
						DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
						pManager->SetDefaultLinkHoverFontColor(clrColor);
					}
					else if( _tcsicmp(pstrName, _T("selectedcolor")) == 0 ) {
						if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
						LPTSTR pstr = NULL;
						DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
						pManager->SetDefaultSelectedBkColor(clrColor);
					}					
				}
			}
		}
	}
	return _Parse(&root, NULL, pManager);
}

CMarkup* CDialogBuilder::GetMarkup()
{
    return &m_xml;
}

void CDialogBuilder::GetLastErrorMessage(LPTSTR pstrMessage, SIZE_T cchMax) const
{
	return m_xml.GetLastErrorMessage(pstrMessage, cchMax);
}

void CDialogBuilder::GetLastErrorLocation(LPTSTR pstrSource, SIZE_T cchMax) const
{
	return m_xml.GetLastErrorLocation(pstrSource, cchMax);
}

CControlUI* CDialogBuilder::_Parse(CMarkupNode* pRoot, CControlUI* pParent, CPaintManagerUI* pManager)
{
	CDialogLayoutUI* pDialogLayout = NULL;
	IContainerUI* pContainer = NULL;
	CControlUI* pReturn = NULL;
	for( CMarkupNode node = pRoot->GetChild() ; node.IsValid(); node = node.GetSibling() ) {
		LPCTSTR pstrClass = node.GetName();
		if( _tcsicmp(pstrClass, _T("Image")) == 0 || _tcsicmp(pstrClass, _T("Font")) == 0 \
			|| _tcsicmp(pstrClass, _T("Default")) == 0 ) continue;

		CControlUI* pControl = NULL;
		if( _tcsicmp(pstrClass, _T("Include")) == 0 ) {
			if( !node.HasAttributes() ) continue;
			int count = 1;
			LPTSTR pstr = NULL;
			TCHAR szValue[500] = { 0 };
			SIZE_T cchLen = lengthof(szValue) - 1;
			if ( node.GetAttributeValue(_T("count"), szValue, cchLen) )
				count = _tcstol(szValue, &pstr, 10);
			cchLen = lengthof(szValue) - 1;
			if ( !node.GetAttributeValue(_T("source"), szValue, cchLen) ) continue;
			for ( int i = 0; i < count; i++ ) {
				CDialogBuilder builder;
				if( m_pstrtype != NULL ) { // 使用资源dll，从资源中读取
					WORD id = (WORD)_tcstol(szValue, &pstr, 10); 
					pControl = builder.Create((UINT)id, m_pstrtype, m_pCallback, pManager, pParent);
				}
				else {
					pControl = builder.Create((LPCTSTR)szValue, (UINT)0, m_pCallback, pManager, pParent);
				}
			}
			continue;
		}
		else {
			SIZE_T cchLen = _tcslen(pstrClass);			
			switch( cchLen ) {
		case 4:
			if( _tcsicmp(pstrClass, kEditUIInterfaceName) == 0 )					pControl = new CEditUI;
			else if( _tcsicmp(pstrClass, kListUIInterfaceName) == 0 )				pControl = new CListUI;
			else if( _tcsicmp(pstrClass, kTextUIInterfaceName) == 0 )				pControl = new CTextUI;
			break;
		case 5:
			if( _tcsicmp(pstrClass, kComboUIInterfaceName) == 0 )					pControl = new CComboUI;
			else if( _tcsicmp(pstrClass, kLabelUIInterfaceName) == 0 )				pControl = new CLabelUI;
			break;
		case 6:
			if( _tcsicmp(pstrClass, kButtonUIInterfaceName) == 0 )					pControl = new CButtonUI;
			else if( _tcsicmp(pstrClass, kOptionUIInterfaceName) == 0 )				pControl = new COptionUI;
			else if( _tcsicmp(pstrClass, kSliderUIInterfaceName) == 0 )				pControl = new CSliderUI;
			if( _tcsicmp(pstrClass, kRatingUIInterfaceName) == 0 )					pControl = new CRatingUI;
			break;
		case 7:
			if( _tcsicmp(pstrClass, kControlUIInterfaceName) == 0 )					pControl = new CControlUI;
#if defined(UI_BUILD_FOR_WIN32) && !defined(UI_BUILD_FOR_WINCE)
			else if( _tcsicmp(pstrClass, kUIActiveXInterfaceName) == 0 )			pControl = new CActiveXUI;
#endif
			break;
		case 8:
			if( _tcsicmp(pstrClass, kProgressUIInterfaceName) == 0 )				pControl = new CProgressUI;
//#if defined(UI_BUILD_FOR_WIN32) && !defined(UI_BUILD_FOR_WINCE)
//			else if( _tcsicmp(pstrClass, kGifLabelUIInterfaceName) == 0 )			pControl = new CGifLabelUI;
//#endif
#ifdef SUPPORT_RICHEDIT
			else if(  _tcsicmp(pstrClass, kRichEditUIInterfaceName) == 0 )			pControl = new CRichEditUI;
#endif
			break;
		case 9:
			if( _tcsicmp(pstrClass, kContainerUIInterfaceName) == 0 )				pControl = new CContainerUI;
			else if( _tcsicmp(pstrClass, kTabLayoutUIInterfaceName) == 0 )			pControl = new CTabLayoutUI;
			else if( _tcsicmp(pstrClass, kScrollBarUIInterfaceName) == 0)			pControl = new CScrollBarUI;
//#if defined(UI_BUILD_FOR_WIN32) && !defined(UI_BUILD_FOR_WINCE)
//			else if( _tcsicmp(pstrClass, kGifButtonUIInterfaceName) == 0 )			pControl = new CGifButtonUI;
//#endif
			break;
		case 10:
			if( _tcsicmp(pstrClass, kListHeaderUIInterfaceName) == 0 )				pControl = new CListHeaderUI;
			else if( _tcsicmp(pstrClass, kTileLayoutUIInterfaceName) == 0 )			pControl = new CTileLayoutUI;
			else if( _tcsicmp(pstrClass, _T("VScrollBar")) == 0)					pControl = new CScrollBarUI;
			else if( _tcsicmp(pstrClass, _T("HScrollBar")) == 0)					pControl = new CScrollBarUI(true);
			break;
		case 12:
			if( _tcsicmp(pstrClass, kDialogLayoutUIInterfaceName) == 0 )			pControl = new CDialogLayoutUI;
			break;
		case 14:
			if( _tcsicmp(pstrClass, kVerticalLayoutUIInterfaceName) == 0 )			pControl = new CVerticalLayoutUI;
			else if( _tcsicmp(pstrClass, kListHeaderItemUIInterfaceName) == 0 )		pControl = new CListHeaderItemUI;
			break;
		case 15:
			if( _tcsicmp(pstrClass, kListTextElementUIInterfaceName) == 0 )			pControl = new CListTextElementUI;
			break;
		case 16:
			if( _tcsicmp(pstrClass, kHorizontalLayoutUIInterfaceName) == 0 )		pControl = new CHorizontalLayoutUI;
			else if( _tcsicmp(pstrClass, kListLabelElementUIInterfaceName) == 0 )	pControl = new CListLabelElementUI;
			break;
		//case 17:
		//	if( _tcsicmp(pstrClass, kListExpandElementUIInterfaceName) == 0 )		pControl = new CListExpandElementUI;
		//	break;
		case 20:
			if( _tcsicmp(pstrClass, kListContainerElementUIInterfaceName) == 0 )	pControl = new CListContainerElementUI;
			break;
			}
			// User-supplied control factory
            if( pControl == NULL ) {
                CStdPtrArray* pPlugins = CPaintManagerUI::GetPlugins();
                LPCREATECONTROL lpCreateControl = NULL;
                for( int i = 0; i < pPlugins->GetSize(); ++i ) {
                    lpCreateControl = (LPCREATECONTROL)pPlugins->GetAt(i);
                    if( lpCreateControl != NULL ) {
                        pControl = lpCreateControl(pstrClass);
                        if( pControl != NULL ) break;
                    }
                }
            }
			if( pControl == NULL && m_pCallback != NULL ) {
				pControl = m_pCallback->CreateControl(pstrClass, pManager);
			}
		}
#ifdef UI_BUILD_FOR_DESIGNER
		if (pManager->IsAssertWhenCustomDefinedNull())
			ASSERT(pControl);
#else
		ASSERT(pControl);
#endif
		if( pControl == NULL ) continue;

		// Add children
		if( node.HasChildren() ) {
			_Parse(&node, pControl, pManager);
		}
		// Attach to parent
		// 因为某些属性和父窗口相关，比如selected，必须先Add到父窗口
		if( pParent != NULL ) {
			if( pContainer == NULL ) pContainer = static_cast<IContainerUI*>(pParent->GetInterface(kIContainerUIInterfaceName));
			ASSERT(pContainer);
			if( pContainer == NULL ) return NULL;
			if( !pContainer->Add(pControl) ) {
				delete pControl;
				continue;
			}
		}
		// Init default attributes
		if( pManager ) {
			pControl->SetManager(pManager, NULL, false);
			LPCTSTR pDefaultAttributes = pManager->GetDefaultAttributeList(pstrClass);
			if( pDefaultAttributes ) {
				pControl->ApplyAttributeList(pDefaultAttributes);
			}
		}
		// Process attributes
		if( node.HasAttributes() ) {
			TCHAR szValue[500] = { 0 };
			SIZE_T cchLen = lengthof(szValue) - 1;
			// Set ordinary attributes
			int nAttributes = node.GetAttributeCount();
			for( int i = 0; i < nAttributes; i++ ) {
				pControl->SetAttribute(node.GetAttributeName(i), node.GetAttributeValue(i));
			}

			// Very custom attributes
			if ( node.GetAttributeValue(_T("stretch"), szValue, cchLen) ) {
				if( pParent == NULL ) continue;

				CStdString sStretchMode(szValue);
				pControl->SetStretchMode(sStretchMode);

				if( pDialogLayout == NULL ) pDialogLayout = static_cast<CDialogLayoutUI*>(pParent->GetInterface(kDialogLayoutUIInterfaceName));
				ASSERT(pDialogLayout);
				if( pDialogLayout == NULL ) continue;

				UINT uMode = 0;
				if( _tcsstr(szValue, _T("move_x")) != NULL ) uMode |= UISTRETCH_MOVE_X;
				if( _tcsstr(szValue, _T("move_y")) != NULL ) uMode |= UISTRETCH_MOVE_Y;
				if( _tcsstr(szValue, _T("move_xy")) != NULL ) uMode |= UISTRETCH_MOVE_X | UISTRETCH_MOVE_Y;
				if( _tcsstr(szValue, _T("size_x")) != NULL ) uMode |= UISTRETCH_SIZE_X;
				if( _tcsstr(szValue, _T("size_y")) != NULL ) uMode |= UISTRETCH_SIZE_Y;
				if( _tcsstr(szValue, _T("size_xy")) != NULL ) uMode |= UISTRETCH_SIZE_X | UISTRETCH_SIZE_Y;
				if( _tcsstr(szValue, _T("group")) != NULL ) uMode |= UISTRETCH_NEWGROUP;
				if( _tcsstr(szValue, _T("line")) != NULL ) uMode |= UISTRETCH_NEWLINE;
				pDialogLayout->SetStretchMode(pControl, uMode);
			}
		}
		if( pManager ) {
			pControl->SetManager(NULL, NULL, false);
		}
		// Return first item
		if( pReturn == NULL ) pReturn = pControl;
	}
	return pReturn;
}

} // namespace DuiLib
