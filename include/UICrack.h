//
//
// DirectUI - UI Library
//
// Written by Bjarke Viksoe (bjarke@viksoe.dk)
// Copyright (c) 2006-2007 Bjarke Viksoe.
//
// This code may be used in compiled form in any way you desire. These
// source files may be redistributed by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever. It's free, so don't hassle me about it.
//
////
// Acknowledgements :
// Bjarke Viksoe (http://www.viksoe.dk/code/windowless1.htm)
//
//
//
// Beware of bugs.
//
//
//
////////////////////////////////////////////////////////
#ifndef _UICRACK_H_
#define _UICRACK_H_

#ifdef _MSC_VER
#pragma once
#endif

namespace DuiLib {

/////////////////////////////////////////////////////////////////////////////////////
//
//
// constrol class name and interface name

// UIActiveX
extern UILIB_API const TCHAR* const kUIActiveXClassName;// = _T("ActiveXUI");
extern UILIB_API const TCHAR* const kUIActiveXInterfaceName;// = _T("ActiveX");

// ComboUI
extern UILIB_API const TCHAR* const kComboUIClassName;// = _T("ComboUI");
extern UILIB_API const TCHAR* const kComboUIInterfaceName;// = _T("Combo");

// LabelUI
extern UILIB_API const TCHAR* const kLabelUIClassName;// = _T("LabelUI");
extern UILIB_API const TCHAR* const kLabelUIInterfaceName;// = _T("Label");

// GifLabelUI
extern UILIB_API const TCHAR* const kGifLabelUIClassName;// = _T("GifLabelUI");
extern UILIB_API const TCHAR* const kGifLabelUIInterfaceName;// = _T("GifLabel");

// ButtonUI
extern UILIB_API const TCHAR* const kButtonUIClassName;// = _T("ButtonUI");
extern UILIB_API const TCHAR* const kButtonUIInterfaceName;// = _T("Button");

// GifButtonUI
extern UILIB_API const TCHAR* const kGifButtonUIClassName;// = _T("GifButtonUI");
extern UILIB_API const TCHAR* const kGifButtonUIInterfaceName;// = _T("GifButton");

// OptionUI
extern UILIB_API const TCHAR* const kOptionUIClassName;// = _T("OptionUI");
extern UILIB_API const TCHAR* const kOptionUIInterfaceName;// = _T("Option");

// TextUI
extern UILIB_API const TCHAR* const kTextUIClassName;// = _T("TextUI");
extern UILIB_API const TCHAR* const kTextUIInterfaceName;// = _T("Text");

// ProgressUI
extern UILIB_API const TCHAR* const kProgressUIClassName;// = _T("ProgressUI");
extern UILIB_API const TCHAR* const kProgressUIInterfaceName;// = _T("Progress");

// SliderUI
extern UILIB_API const TCHAR* const kSliderUIClassName;// = _T("SliderUI");
extern UILIB_API const TCHAR* const kSliderUIInterfaceName;// = _T("Slider");

// EditUI
extern UILIB_API const TCHAR* const kEditUIClassName;// = _T("EditUI");
extern UILIB_API const TCHAR* const kEditUIInterfaceName;// = _T("Edit");

// IEditUI
extern UILIB_API const TCHAR* const kIEditUIInterfaceName;// = _T("Edit");

// ScrollBarUI
extern UILIB_API const TCHAR* const kScrollBarUIClassName;// = _T("ScrollBarUI");
extern UILIB_API const TCHAR* const kScrollBarUIInterfaceName;// = _T("ScrollBar");

// ContainerUI
extern UILIB_API const TCHAR* const kContainerUIClassName;// = _T("ContainerUI");
extern UILIB_API const TCHAR* const kContainerUIInterfaceName;// = _T("Container");

// IContainerUI
extern UILIB_API const TCHAR* const kIContainerUIInterfaceName;// = _T("IContainer");

// VerticalLayoutUI
extern UILIB_API const TCHAR* const kVerticalLayoutUIClassName;// = _T("VerticalLayoutUI");
extern UILIB_API const TCHAR* const kVerticalLayoutUIInterfaceName;// = _T("VerticalLayout");

// HorizontalLayoutUI
extern UILIB_API const TCHAR* const kHorizontalLayoutUIClassName;// = _T("HorizontalLayoutUI");
extern UILIB_API const TCHAR* const kHorizontalLayoutUIInterfaceName;// = _T("HorizontalLayout");

// TileLayoutUI
extern UILIB_API const TCHAR* const kTileLayoutUIClassName;// = _T("TileLayoutUI");
extern UILIB_API const TCHAR* const kTileLayoutUIInterfaceName;// = _T("TileLayout");

// DialogLayoutUI
extern UILIB_API const TCHAR* const kDialogLayoutUIClassName;// = _T("DialogLayoutUI");
extern UILIB_API const TCHAR* const kDialogLayoutUIInterfaceName;// = _T("DialogLayout");

// TabLayoutUI
extern UILIB_API const TCHAR* const kTabLayoutUIClassName;// = _T("TabLayoutUI");
extern UILIB_API const TCHAR* const kTabLayoutUIInterfaceName;// = _T("TabLayout");

// ControlUI
extern UILIB_API const TCHAR* const kControlUIClassName;// = _T("ControlUI");
extern UILIB_API const TCHAR* const kControlUIInterfaceName;// = _T("Control");

// ListUI
extern UILIB_API const TCHAR* const kListUIClassName;// = _T("ListUI");
extern UILIB_API const TCHAR* const kListUIInterfaceName;// = _T("List");

// IListUI
extern UILIB_API const TCHAR* const kIListUIInterfaceName;// = _T("IList");

// IListOwnerUI
extern UILIB_API const TCHAR* const kIListOwnerUIInterfaceName;// = _T("IListOwner");

// ListHeaderUI
extern UILIB_API const TCHAR* const kListHeaderUIClassName;// = _T("ListHeaderUI");
extern UILIB_API const TCHAR* const kListHeaderUIInterfaceName;// = _T("ListHeader");

// ListHeaderItemUI
extern UILIB_API const TCHAR* const kListHeaderItemUIClassName;// = _T("ListHeaderItemUI");
extern UILIB_API const TCHAR* const kListHeaderItemUIInterfaceName;// = _T("ListHeaderItem");

// ListElementUI
extern UILIB_API const TCHAR* const kListElementUIClassName;// = _T("ListElementUI");
extern UILIB_API const TCHAR* const kListElementUIInterfaceName;// = _T("ListElement");

// IListItemUI
extern UILIB_API const TCHAR* const kIListItemUIInterfaceName;// = _T("ListItem");

// ListLabelElementUI
extern UILIB_API const TCHAR* const kListLabelElementUIClassName;// = _T("ListLabelElementUI");
extern UILIB_API const TCHAR* const kListLabelElementUIInterfaceName;// = _T("ListLabelElement");

// ListTextElementUI
extern UILIB_API const TCHAR* const kListTextElementUIClassName;// = _T("ListTextElementUI");
extern UILIB_API const TCHAR* const kListTextElementUIInterfaceName;// = _T("ListTextElement");

// ListExpandElementUI
//extern UILIB_API const TCHAR* const kListExpandElementUIClassName;// = _T("ListExpandElementUI");
//extern UILIB_API const TCHAR* const kListExpandElementUIInterfaceName;// = _T("ListExpandElement");

// ListContainerElementUI
extern UILIB_API const TCHAR* const kListContainerElementUIClassName;// = _T("ListContainerElementUI");
extern UILIB_API const TCHAR* const kListContainerElementUIInterfaceName;// = _T("ListContainerElement");

// RichEditUI
extern UILIB_API const TCHAR* const kRichEditUIClassName;// = _T("RichEditUI");
extern UILIB_API const TCHAR* const kRichEditUIInterfaceName;// = _T("RichEdit");

/////////////////////////////////////////////////////////////////////////////////////
//
//
// control related message
extern UILIB_API const TCHAR* const kWindowInit;// = _T("windowinit");
extern UILIB_API const TCHAR* const kClick;// = _T("click");
extern UILIB_API const TCHAR* const kSelectChanged;// = _T("selectchanged");
extern UILIB_API const TCHAR* const kItemSelect;// = _T("itemselect");
extern UILIB_API const TCHAR* const kItemActivate;// = _T("itemactivate");
extern UILIB_API const TCHAR* const kItemClick;// = _T("itemclick");
extern UILIB_API const TCHAR* const kDropDown;// = _T("dropdown");
extern UILIB_API const TCHAR* const kTimer;// = _T("timer");
extern UILIB_API const TCHAR* const kMenu;// = _T("menu");
extern UILIB_API const TCHAR* const kReturn;// = _T("return");
extern UILIB_API const TCHAR* const kTextChanged;// = _T("textchanged");
extern UILIB_API const TCHAR* const kKillFocus; // = _T("killfocus");
extern UILIB_API const TCHAR* const kSetFocus; // = _T("setfocus");
extern UILIB_API const TCHAR* const kValueChanged; // = _T("valuechanged");

}; // namespace DuiLib

#endif // _UICRACK_H_