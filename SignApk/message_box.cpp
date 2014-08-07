#include "stdafx.h"
#include <windows.h>
#include <shellapi.h>

#include "resource.h"
#include "win_impl_base.hpp"
#include "message_box.hpp"

int DUIMessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
	CMessageBox msg_box(hWnd, lpCaption, lpText, uType);
	msg_box.CenterWindow();
	return msg_box.ShowModal(hWnd);
}

const TCHAR* const kTitleControlName = _T("apptitle");
const TCHAR* const kCloseButtonControlName = _T("closebtn");
const TCHAR* const kMinButtonControlName = _T("minbtn");
const TCHAR* const kMaxButtonControlName = _T("maxbtn");
const TCHAR* const kRestoreButtonControlName = _T("restorebtn");
const int kMessageBoxMaxHeight = 250;
const TCHAR* const kDefaultButtonAttribute = _T("name=\"%s\" text=\"%s\" width=\"70\" height=\"26\" bkimage=\"res='115' restype='png' corner='5,5,5,5'\" hotimage=\"res='116' restype='png' corner='2,3,2,2'\" pushedimage=\"res='117' restype='png' corner='2,5,2,2'\"");

CMessageBox::CMessageBox(HWND hParent, const tString& caption, const tString& message, UINT uType)
: caption_(caption)
, message_(message)
, type_(uType)
, parent_(hParent)
{
	Create(parent_, NULL, WS_POPUP, WS_EX_TOOLWINDOW, CRect());
}

CMessageBox::~CMessageBox()
{}

LPCTSTR CMessageBox::GetWindowClassName() const
{
	return _T("CMessageBox");
}

CControlUI* CMessageBox::CreateControl(LPCTSTR pstrClass)
{
	return NULL;
}

void CMessageBox::OnFinalMessage(HWND hWnd)
{
	WindowImplBase::OnFinalMessage(hWnd);
}

tString CMessageBox::GetSkinFile()
{
	TCHAR szBuf[MAX_PATH] = {0};
	_stprintf_s(szBuf, MAX_PATH - 1, _T("%d"), IDR_XML_MSGBOX);
	return szBuf;
}

tString CMessageBox::GetSkinFolder()
{
	return tString(CPaintManagerUI::GetInstancePath());
}

LONG CMessageBox::GetStyle()
{
	LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
	styleValue &= ~( WS_CAPTION | WS_MAXIMIZEBOX | WS_SIZEBOX );

	return styleValue;
}

LRESULT CMessageBox::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return __super::OnSysCommand(uMsg, wParam, lParam, bHandled);
}

LRESULT CMessageBox::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return __super::HandleMessage(uMsg, wParam, lParam);
}

LRESULT CMessageBox::ResponseDefaultKeyEvent(WPARAM wParam)
{
	if (wParam == VK_RETURN)
	{
		return FALSE;
	}
	else if (wParam == VK_ESCAPE)
	{
		return TRUE;
	}
	return FALSE;
}


// 窗口刚创建完成时调用该函数
void CMessageBox::Init()
{}

// 窗口将要显示前调用该函数
void CMessageBox::OnPrepare(TNotifyUI& msg)
{
	TCHAR szBuf[MAX_PATH] = {0};

	CLabelUI* caption = static_cast<CLabelUI*>(paint_manager_.FindControl(kTitleControlName));
	if (caption != NULL)
	{
		_stprintf_s(szBuf, MAX_PATH - 1, _T("%s"), caption_.c_str());
		caption->SetText(szBuf);
	}

	CRichEditUI* message = static_cast<CRichEditUI*>(paint_manager_.FindControl(_T("message")));
	if (message != NULL)
	{
		message->SetText(message_.c_str());
		CRect rcText = message->GetPos();

		CRect rcAvailable;
		rcAvailable.right = rcText.GetWidth();
		rcAvailable.bottom = 800;
		HFONT hOldFont = static_cast<HFONT>(SelectObject(paint_manager_.GetPaintDC(), paint_manager_.GetFont(1)));
		DrawText(paint_manager_.GetPaintDC(), message_.c_str(), -1, &rcAvailable, DT_LEFT | DT_WORDBREAK | DT_CALCRECT); 
		SelectObject(paint_manager_.GetPaintDC(), hOldFont);

		if (rcAvailable.GetHeight() > rcText.GetHeight())
		{
			CRect rcWindow;
			GetWindowRect(m_hWnd, &rcWindow);
			MoveWindow(m_hWnd, 0, 0, rcWindow.GetWidth(), (rcWindow.GetHeight() + rcAvailable.GetHeight() - rcText.GetHeight() < kMessageBoxMaxHeight) ? (rcWindow.GetHeight() + rcAvailable.GetHeight() - rcText.GetHeight()) : kMessageBoxMaxHeight, FALSE);
			CenterWindow();
		}
	}

	CContainerUI* pContainer = static_cast<CContainerUI*>(paint_manager_.FindControl(_T("buttons")));
	if (pContainer != NULL)
	{
		switch (type_)
		{
		case MB_OK:
			{
				CLabelUI* pLabel = new CLabelUI();
				ASSERT(pLabel != NULL);
				pContainer->Add(pLabel);

				_stprintf_s(szBuf, MAX_PATH - 1, kDefaultButtonAttribute, _T("btn_ok"), _T("确定"));
				CButtonUI* pButton = new CButtonUI();
				ASSERT(pButton != NULL);
				pButton->ApplyAttributeList(szBuf);
				pContainer->Add(pButton);

				pLabel = new CLabelUI();
				ASSERT(pLabel != NULL);
				pContainer->Add(pLabel);
			}
			break;
		case MB_OKCANCEL:
			{
				CRect rcParent = pContainer->GetPos();
				int nMargin = static_cast<int>( (rcParent.GetWidth() - 2 * 70) / 3 );

				CLabelUI* pLabel = new CLabelUI();
				ASSERT(pLabel != NULL);
				_stprintf_s(szBuf, MAX_PATH - 1, _T("width=\"%d\""), nMargin);
				pLabel->ApplyAttributeList(szBuf);
				pContainer->Add(pLabel);

				_stprintf_s(szBuf, MAX_PATH - 1, kDefaultButtonAttribute, _T("btn_ok"), _T("确定"));
				CButtonUI* pButton = new CButtonUI();
				ASSERT(pButton != NULL);
				pButton->ApplyAttributeList(szBuf);
				pContainer->Add(pButton);

				pLabel = new CLabelUI();
				ASSERT(pLabel != NULL);
				_stprintf_s(szBuf, MAX_PATH - 1, _T("width=\"%d\""), nMargin);
				pLabel->ApplyAttributeList(szBuf);
				pContainer->Add(pLabel);

				_stprintf_s(szBuf, MAX_PATH - 1, kDefaultButtonAttribute, _T("btn_cancel"), _T("取消"));
				pButton = new CButtonUI();
				ASSERT(pButton != NULL);
				pButton->ApplyAttributeList(szBuf);
				pContainer->Add(pButton);

				pLabel = new CLabelUI();
				ASSERT(pLabel != NULL);
				_stprintf_s(szBuf, MAX_PATH - 1, _T("width=\"%d\""), nMargin);
				pLabel->ApplyAttributeList(szBuf);
				pContainer->Add(pLabel);
			}
		case MB_YESNO:
			{
				CRect rcParent = pContainer->GetPos();
				int nMargin = static_cast<int>( (rcParent.GetWidth() - 2 * 70) / 3 );

				CLabelUI* pLabel = new CLabelUI();
				ASSERT(pLabel != NULL);
				_stprintf_s(szBuf, MAX_PATH - 1, _T("width=\"%d\""), nMargin);
				pLabel->ApplyAttributeList(szBuf);
				pContainer->Add(pLabel);

				_stprintf_s(szBuf, MAX_PATH - 1, kDefaultButtonAttribute, _T("btn_yes"), _T("是"));
				CButtonUI* pButton = new CButtonUI();
				ASSERT(pButton != NULL);
				pButton->ApplyAttributeList(szBuf);
				pContainer->Add(pButton);

				pLabel = new CLabelUI();
				ASSERT(pLabel != NULL);
				_stprintf_s(szBuf, MAX_PATH - 1, _T("width=\"%d\""), nMargin);
				pLabel->ApplyAttributeList(szBuf);
				pContainer->Add(pLabel);

				_stprintf_s(szBuf, MAX_PATH - 1, kDefaultButtonAttribute, _T("btn_no"), _T("否"));
				pButton = new CButtonUI();
				ASSERT(pButton != NULL);
				pButton->ApplyAttributeList(szBuf);
				pContainer->Add(pButton);

				pLabel = new CLabelUI();
				ASSERT(pLabel != NULL);
				_stprintf_s(szBuf, MAX_PATH - 1, _T("width=\"%d\""), nMargin);
				pLabel->ApplyAttributeList(szBuf);
				pContainer->Add(pLabel);
			}
			break;
		case MB_RETRYCANCEL:
			{
				CRect rcParent = pContainer->GetPos();
				int nMargin = static_cast<int>( (rcParent.GetWidth() - 2 * 70) / 3 );

				CLabelUI* pLabel = new CLabelUI();
				ASSERT(pLabel != NULL);
				_stprintf_s(szBuf, MAX_PATH - 1, _T("width=\"%d\""), nMargin);
				pLabel->ApplyAttributeList(szBuf);
				pContainer->Add(pLabel);

				_stprintf_s(szBuf, MAX_PATH - 1, kDefaultButtonAttribute, _T("btn_retry"), _T("重试"));
				CButtonUI* pButton = new CButtonUI();
				ASSERT(pButton != NULL);
				pButton->ApplyAttributeList(szBuf);
				pContainer->Add(pButton);

				pLabel = new CLabelUI();
				ASSERT(pLabel != NULL);
				_stprintf_s(szBuf, MAX_PATH - 1, _T("width=\"%d\""), nMargin);
				pLabel->ApplyAttributeList(szBuf);
				pContainer->Add(pLabel);

				_stprintf_s(szBuf, MAX_PATH - 1, kDefaultButtonAttribute, _T("btn_cancel"), _T("取消"));
				pButton = new CButtonUI();
				ASSERT(pButton != NULL);
				pButton->ApplyAttributeList(szBuf);
				pContainer->Add(pButton);

				pLabel = new CLabelUI();
				ASSERT(pLabel != NULL);
				_stprintf_s(szBuf, MAX_PATH - 1, _T("width=\"%d\""), nMargin);
				pLabel->ApplyAttributeList(szBuf);
				pContainer->Add(pLabel);
			}
			break;
		case MB_YESNOCANCEL:
			{
				CRect rcParent = pContainer->GetPos();
				int nMargin = static_cast<int>( (rcParent.GetWidth() - 3 * 70) / 4 );

				CLabelUI* pLabel = new CLabelUI();
				ASSERT(pLabel != NULL);
				_stprintf_s(szBuf, MAX_PATH - 1, _T("width=\"%d\""), nMargin);
				pLabel->ApplyAttributeList(szBuf);
				pContainer->Add(pLabel);

				_stprintf_s(szBuf, MAX_PATH - 1, kDefaultButtonAttribute, _T("btn_yes"), _T("是"));
				CButtonUI* pButton = new CButtonUI();
				ASSERT(pButton != NULL);
				pButton->ApplyAttributeList(szBuf);
				pContainer->Add(pButton);

				pLabel = new CLabelUI();
				ASSERT(pLabel != NULL);
				_stprintf_s(szBuf, MAX_PATH - 1, _T("width=\"%d\""), nMargin);
				pLabel->ApplyAttributeList(szBuf);
				pContainer->Add(pLabel);

				_stprintf_s(szBuf, MAX_PATH - 1, kDefaultButtonAttribute, _T("btn_no"), _T("否"));
				pButton = new CButtonUI();
				ASSERT(pButton != NULL);
				pButton->ApplyAttributeList(szBuf);
				pContainer->Add(pButton);

				pLabel = new CLabelUI();
				ASSERT(pLabel != NULL);
				_stprintf_s(szBuf, MAX_PATH - 1, _T("width=\"%d\""), nMargin);
				pLabel->ApplyAttributeList(szBuf);
				pContainer->Add(pLabel);

				_stprintf_s(szBuf, MAX_PATH - 1, kDefaultButtonAttribute, _T("btn_cancel"), _T("取消"));
				pButton = new CButtonUI();
				ASSERT(pButton != NULL);
				pButton->ApplyAttributeList(szBuf);
				pContainer->Add(pButton);

				pLabel = new CLabelUI();
				ASSERT(pLabel != NULL);
				_stprintf_s(szBuf, MAX_PATH - 1, _T("width=\"%d\""), nMargin);
				pLabel->ApplyAttributeList(szBuf);
				pContainer->Add(pLabel);
			}
			break;
		case MB_ABORTRETRYIGNORE:
			{
				CRect rcParent = pContainer->GetPos();
				int nMargin = static_cast<int>( (rcParent.GetWidth() - 3 * 70) / 4 );

				CLabelUI* pLabel = new CLabelUI();
				ASSERT(pLabel != NULL);
				_stprintf_s(szBuf, MAX_PATH - 1, _T("width=\"%d\""), nMargin);
				pLabel->ApplyAttributeList(szBuf);
				pContainer->Add(pLabel);

				_stprintf_s(szBuf, MAX_PATH - 1, kDefaultButtonAttribute, _T("btn_abort"), _T("终止"));
				CButtonUI* pButton = new CButtonUI();
				ASSERT(pButton != NULL);
				pButton->ApplyAttributeList(szBuf);
				pContainer->Add(pButton);

				pLabel = new CLabelUI();
				ASSERT(pLabel != NULL);
				_stprintf_s(szBuf, MAX_PATH - 1, _T("width=\"%d\""), nMargin);
				pLabel->ApplyAttributeList(szBuf);
				pContainer->Add(pLabel);

				_stprintf_s(szBuf, MAX_PATH - 1, kDefaultButtonAttribute, _T("btn_retry"), _T("重试"));
				pButton = new CButtonUI();
				ASSERT(pButton != NULL);
				pButton->ApplyAttributeList(szBuf);
				pContainer->Add(pButton);

				pLabel = new CLabelUI();
				ASSERT(pLabel != NULL);
				_stprintf_s(szBuf, MAX_PATH - 1, _T("width=\"%d\""), nMargin);
				pLabel->ApplyAttributeList(szBuf);
				pContainer->Add(pLabel);

				_stprintf_s(szBuf, MAX_PATH - 1, kDefaultButtonAttribute, _T("btn_ignore"), _T("忽略"));
				pButton = new CButtonUI();
				ASSERT(pButton != NULL);
				pButton->ApplyAttributeList(szBuf);
				pContainer->Add(pButton);

				pLabel = new CLabelUI();
				ASSERT(pLabel != NULL);
				_stprintf_s(szBuf, MAX_PATH - 1, _T("width=\"%d\""), nMargin);
				pLabel->ApplyAttributeList(szBuf);
				pContainer->Add(pLabel);
			}
			break;
		}
	}
}

void CMessageBox::Notify(TNotifyUI& msg)
{
	if (_tcsicmp(msg.sType, kWindowInit) == 0)
	{
		// 窗口将要显示前调用该函数
		OnPrepare(msg);
	}
	else if (_tcsicmp(msg.sType, kClick) == 0)
	{
		AnimateWindow(GetHWND(), 800, AW_CENTER | AW_HIDE);
		if (_tcsicmp(msg.pSender->GetName(), kCloseButtonControlName) == 0)
		{
			Close(IDCLOSE);
		}
		else if (_tcsicmp(msg.pSender->GetName(), _T("btn_cancel")) == 0)
		{
			Close(IDCANCEL);
		}
		else if (_tcsicmp(msg.pSender->GetName(), _T("btn_ok")) == 0)
		{
			Close(IDOK);
		}
		else if (_tcsicmp(msg.pSender->GetName(), _T("btn_yes")) == 0)
		{
			Close(IDYES);
		}
		else if (_tcsicmp(msg.pSender->GetName(), _T("btn_no")) == 0)
		{
			Close(IDNO);
		}
		else if (_tcsicmp(msg.pSender->GetName(), _T("btn_retry")) == 0)
		{
			Close(IDRETRY);
		}
		else if (_tcsicmp(msg.pSender->GetName(), _T("btn_abort")) == 0)
		{
			Close(IDABORT);
		}
		else if (_tcsicmp(msg.pSender->GetName(), _T("btn_ignore")) == 0)
		{
			Close(IDIGNORE);
		}
	}
}

LRESULT CMessageBox::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}
