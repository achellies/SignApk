#include "StdAfx.h"
#include "UICrack.h"

namespace DuiLib {


/////////////////////////////////////////////////////////////////////////////////////
//
//
// constrol class name and interface name

// UIActiveX
UILIB_API const TCHAR* const kUIActiveXClassName = _T("ActiveXUI");
UILIB_API const TCHAR* const kUIActiveXInterfaceName = _T("ActiveX");

// ComboUI
UILIB_API const TCHAR* const kComboUIClassName = _T("ComboUI");
UILIB_API const TCHAR* const kComboUIInterfaceName = _T("Combo");

// LabelUI
UILIB_API const TCHAR* const kLabelUIClassName = _T("LabelUI");
UILIB_API const TCHAR* const kLabelUIInterfaceName = _T("Label");

// GifLabelUI
UILIB_API const TCHAR* const kGifLabelUIClassName = _T("GifLabelUI");
UILIB_API const TCHAR* const kGifLabelUIInterfaceName = _T("GifLabel");

// ButtonUI
UILIB_API const TCHAR* const kButtonUIClassName = _T("ButtonUI");
UILIB_API const TCHAR* const kButtonUIInterfaceName = _T("Button");

// GifButtonUI
UILIB_API const TCHAR* const kGifButtonUIClassName = _T("GifButtonUI");
UILIB_API const TCHAR* const kGifButtonUIInterfaceName = _T("GifButton");

// OptionUI
UILIB_API const TCHAR* const kOptionUIClassName = _T("OptionUI");
UILIB_API const TCHAR* const kOptionUIInterfaceName = _T("Option");

// TextUI
UILIB_API const TCHAR* const kTextUIClassName = _T("TextUI");
UILIB_API const TCHAR* const kTextUIInterfaceName = _T("Text");

// ProgressUI
UILIB_API const TCHAR* const kProgressUIClassName = _T("ProgressUI");
UILIB_API const TCHAR* const kProgressUIInterfaceName = _T("Progress");

// SliderUI
UILIB_API const TCHAR* const kSliderUIClassName = _T("SliderUI");
UILIB_API const TCHAR* const kSliderUIInterfaceName = _T("Slider");

// EditUI
UILIB_API const TCHAR* const kEditUIClassName = _T("EditUI");
UILIB_API const TCHAR* const kEditUIInterfaceName = _T("Edit");

// IEditUI
UILIB_API const TCHAR* const kIEditUIInterfaceName = _T("Edit");

// ScrollBarUI
UILIB_API const TCHAR* const kScrollBarUIClassName = _T("ScrollBarUI");
UILIB_API const TCHAR* const kScrollBarUIInterfaceName = _T("ScrollBar");

// ContainerUI
UILIB_API const TCHAR* const kContainerUIClassName = _T("ContainerUI");
UILIB_API const TCHAR* const kContainerUIInterfaceName = _T("Container");

// IContainerUI
UILIB_API const TCHAR* const kIContainerUIInterfaceName = _T("IContainer");

// VerticalLayoutUI
UILIB_API const TCHAR* const kVerticalLayoutUIClassName = _T("VerticalLayoutUI");
UILIB_API const TCHAR* const kVerticalLayoutUIInterfaceName = _T("VerticalLayout");

// HorizontalLayoutUI
UILIB_API const TCHAR* const kHorizontalLayoutUIClassName = _T("HorizontalLayoutUI");
UILIB_API const TCHAR* const kHorizontalLayoutUIInterfaceName = _T("HorizontalLayout");

// TileLayoutUI
UILIB_API const TCHAR* const kTileLayoutUIClassName = _T("TileLayoutUI");
UILIB_API const TCHAR* const kTileLayoutUIInterfaceName = _T("TileLayout");

// DialogLayoutUI
UILIB_API const TCHAR* const kDialogLayoutUIClassName = _T("DialogLayoutUI");
UILIB_API const TCHAR* const kDialogLayoutUIInterfaceName = _T("DialogLayout");

// TabLayoutUI
UILIB_API const TCHAR* const kTabLayoutUIClassName = _T("TabLayoutUI");
UILIB_API const TCHAR* const kTabLayoutUIInterfaceName = _T("TabLayout");

// ControlUI
UILIB_API const TCHAR* const kControlUIClassName = _T("ControlUI");
UILIB_API const TCHAR* const kControlUIInterfaceName = _T("Control");

// ListUI
UILIB_API const TCHAR* const kListUIClassName = _T("ListUI");
UILIB_API const TCHAR* const kListUIInterfaceName = _T("List");

// IListUI
UILIB_API const TCHAR* const kIListUIInterfaceName = _T("IList");

// IListOwnerUI
UILIB_API const TCHAR* const kIListOwnerUIInterfaceName = _T("IListOwner");

// ListHeaderUI
UILIB_API const TCHAR* const kListHeaderUIClassName = _T("ListHeaderUI");
UILIB_API const TCHAR* const kListHeaderUIInterfaceName = _T("ListHeader");

// ListHeaderItemUI
UILIB_API const TCHAR* const kListHeaderItemUIClassName = _T("ListHeaderItemUI");
UILIB_API const TCHAR* const kListHeaderItemUIInterfaceName = _T("ListHeaderItem");

// ListElementUI
UILIB_API const TCHAR* const kListElementUIClassName = _T("ListElementUI");
UILIB_API const TCHAR* const kListElementUIInterfaceName = _T("ListElement");

// IListItemUI
UILIB_API const TCHAR* const kIListItemUIInterfaceName = _T("ListItem");

// ListLabelElementUI
UILIB_API const TCHAR* const kListLabelElementUIClassName = _T("ListLabelElementUI");
UILIB_API const TCHAR* const kListLabelElementUIInterfaceName = _T("ListLabelElement");

// ListTextElementUI
UILIB_API const TCHAR* const kListTextElementUIClassName = _T("ListTextElementUI");
UILIB_API const TCHAR* const kListTextElementUIInterfaceName = _T("ListTextElement");

// ListExpandElementUI
//UILIB_API const TCHAR* const kListExpandElementUIClassName = _T("ListExpandElementUI");
//UILIB_API const TCHAR* const kListExpandElementUIInterfaceName = _T("ListExpandElement");

// ListContainerElementUI
UILIB_API const TCHAR* const kListContainerElementUIClassName = _T("ListContainerElementUI");
UILIB_API const TCHAR* const kListContainerElementUIInterfaceName = _T("ListContainerElement");

// RichEditUI
UILIB_API const TCHAR* const kRichEditUIClassName = _T("RichEditUI");
UILIB_API const TCHAR* const kRichEditUIInterfaceName = _T("RichEdit");

/////////////////////////////////////////////////////////////////////////////////////
//
//
// control related message
UILIB_API const TCHAR* const kWindowInit = _T("windowinit");
UILIB_API const TCHAR* const kClick = _T("click");
UILIB_API const TCHAR* const kSelectChanged = _T("selectchanged");
UILIB_API const TCHAR* const kItemSelect = _T("itemselect");
UILIB_API const TCHAR* const kItemActivate = _T("itemactivate");
UILIB_API const TCHAR* const kItemClick = _T("itemclick");
UILIB_API const TCHAR* const kDropDown = _T("dropdown");
UILIB_API const TCHAR* const kTimer = _T("timer");
UILIB_API const TCHAR* const kMenu = _T("menu");
UILIB_API const TCHAR* const kReturn = _T("return");
UILIB_API const TCHAR* const kTextChanged = _T("textchanged");
UILIB_API const TCHAR* const kKillFocus = _T("killfocus");
UILIB_API const TCHAR* const kSetFocus = _T("setfocus");
UILIB_API const TCHAR* const kValueChanged = _T("valuechanged");

}; // namespace DuiLib