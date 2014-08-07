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
#ifndef __UIGIFCONTROLS_H__
#define __UIGIFCONTROLS_H__

#include "UICommonControls.h"

#ifdef _MSC_VER
#pragma once
#endif

#if defined(UI_BUILD_FOR_WIN32) && !defined(UI_BUILD_FOR_WINCE)

class CGDIImage;
namespace DuiLib {
/////////////////////////////////////////////////////////////////////////////////////
//
enum eGifUpdateTimer
{
	GifUpdateTimerId = 1000,
	GifUpdateTimerInterval = 360
};


class UILIB_API CGifLabelUI : public CLabelUI
{
public:
	CGifLabelUI();

	~CGifLabelUI();

    LPCTSTR GetClass() const;
    LPVOID GetInterface(LPCTSTR pstrName);

	virtual void DoEvent(TEventUI& event);

	virtual void PaintBkImage(void* ctx);	

private:
	bool timer_started_;
	DWORD frame_count_;
	DWORD frame_index_;

	CStdString m_sPreBkImage;
	CGDIImage* m_pImage;
};

class UILIB_API CGifButtonUI : public CButtonUI
{
public:
	CGifButtonUI();

	~CGifButtonUI();

    LPCTSTR GetClass() const;
    LPVOID GetInterface(LPCTSTR pstrName);

	virtual void DoEvent(TEventUI& event);
	virtual void PaintBkImage(void* ctx);

private:
	bool timer_started_;
	DWORD frame_count_;
	DWORD frame_index_;

	CStdString m_sPreBkImage;
	CGDIImage* m_pImage;
};

} // namespace DuiLib

#endif

#endif // __UIGIFCONTROLS_H__
