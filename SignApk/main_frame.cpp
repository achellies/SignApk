//
// main_frame.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2011 achellies (achellies at 163 dot com)
//
// This code may be used in compiled form in any way you desire. This
// source file may be redistributed by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever. It's free, so don't hassle me about it.
//
// Beware of bugs.
//

#include "stdafx.h"
#include <windows.h>
#include <Commdlg.h>
#if !defined(UNDER_CE)
#include <shellapi.h>
#endif
#include <stdlib.h>

#include "resource.h"
#include "win_impl_base.hpp"
#include "main_frame.hpp"
#include "message_box.hpp"
#include "window_util.hpp"
#include "tinyxml.h"
#include "zip.h"
#include "unzip.h"

#include "xdstring.h"
#include <algorithm>
#include <Shlobj.h>

#include <fstream>
#include <iostream>

const TCHAR* const kTitleControlName = _T("apptitle");
const TCHAR* const kCloseButtonControlName = _T("closebtn");
const TCHAR* const kMinButtonControlName = _T("minbtn");
const TCHAR* const kMaxButtonControlName = _T("maxbtn");
const TCHAR* const kRestoreButtonControlName = _T("restorebtn");

MainFrame::MainFrame()
: lock_(_T("lock"), TRUE, TRUE)
, thread_(NULL)
, thread_cancel_(false)
{}

MainFrame::~MainFrame()
{
	PostQuitMessage(0);
}

LPCTSTR MainFrame::GetWindowClassName() const
{
	return _T("TXGuiFoundation");
}

void MainFrame::OnFinalMessage(HWND hWnd)
{
	WindowImplBase::OnFinalMessage(hWnd);
	delete this;
}

tString MainFrame::GetSkinFile()
{
	TCHAR szBuf[MAX_PATH] = {0};
	_stprintf_s(szBuf, MAX_PATH - 1, _T("%d"), IDR_SKINXML);
	return szBuf;
}

tString MainFrame::GetSkinFolder()
{
	return tString(CPaintManagerUI::GetInstancePath());
}

LRESULT MainFrame::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
#if defined(WIN32) && !defined(UNDER_CE)
	if (SC_CLOSE == wParam && thread_ != NULL)
	{
		ShowWindow();
		return 0;
	}

	BOOL bZoomed = ::IsZoomed(m_hWnd);
	LRESULT lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	if (::IsZoomed(m_hWnd) != bZoomed)
	{
		if (!bZoomed)
		{
			CControlUI* pControl = static_cast<CControlUI*>(paint_manager_.FindControl(kMaxButtonControlName));
			if( pControl ) pControl->SetVisible(false);
			pControl = static_cast<CControlUI*>(paint_manager_.FindControl(kRestoreButtonControlName));
			if( pControl ) pControl->SetVisible(true);
		}
		else 
		{
			CControlUI* pControl = static_cast<CControlUI*>(paint_manager_.FindControl(kMaxButtonControlName));
			if( pControl ) pControl->SetVisible(true);
			pControl = static_cast<CControlUI*>(paint_manager_.FindControl(kRestoreButtonControlName));
			if( pControl ) pControl->SetVisible(false);
		}
	}
#else
	return __super::OnSysCommand(uMsg, wParam, lParam, bHandled);
#endif

	return 0;
}

LRESULT MainFrame::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return __super::HandleMessage(uMsg, wParam, lParam);
}

LRESULT MainFrame::ResponseDefaultKeyEvent(WPARAM wParam)
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

void MainFrame::OnTimer(TNotifyUI& msg)
{
}

void MainFrame::OnExit(TNotifyUI& msg)
{
	SaveProperty();
	AnimateWindow(GetHWND(), 1000, AW_SLIDE | AW_HIDE | AW_VER_POSITIVE);
	Close();
}

void MainFrame::Init()
{
	SetIcon(IDI_ICON1);
}

void MainFrame::OnPrepare(TNotifyUI& msg)
{
	LoadProperty();

	::ShowWindow(GetHWND(), SW_HIDE);
	::BringWindowToTop(GetHWND());

	AnimateWindow(GetHWND(), 1000, AW_SLIDE | AW_VER_POSITIVE);

	CLabelUI* pLabel = static_cast<CLabelUI*>(paint_manager_.FindControl(_T("version")));
	if (pLabel != NULL)
	{
		TCHAR szTitle[MAX_PATH] = {0};
		// Get version information from the application
		TCHAR *szBuf = (TCHAR *)malloc(sizeof(TCHAR)*128*7);
		if (szBuf != NULL) 
		{
			TCHAR *szComment = &(szBuf[128*0]);
			TCHAR *szCompanyName = &(szBuf[128*1]); 
			TCHAR *szFileDescription = &(szBuf[128*2]);
			TCHAR *szFileVersion = &(szBuf[128*3]);
			TCHAR *szLegalCopyright = &(szBuf[128*4]);
			TCHAR *szProductName = &(szBuf[128*5]);
			TCHAR *szProductVersion = &(szBuf[128*6]);
			if (!GetVerString(128, szComment,
				szCompanyName, szFileDescription, szFileVersion, szLegalCopyright,
				szProductName, szProductVersion))
			{
				goto allDone;
			}
#if defined(UNDER_CE)
			_stprintf(szTitle, _T("主程序版本：%s"), szProductVersion);
#else
			_stprintf_s(szTitle, MAX_PATH, _T("主程序版本：%s"), szProductVersion);
#endif
			pLabel->SetText(szTitle);
		}
allDone:
		if (szBuf != NULL)
		{
			free(szBuf);
			szBuf = NULL;
		}
	}
}

void MainFrame::UpdateUI()
{
	bool enable = (thread_ == NULL) ? true : false;
	CEditUI* pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("jdk_path")));
	if (pEdit != NULL)
		pEdit->SetEnabled(enable);
	pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("keystore_file")));
	if (pEdit != NULL)
		pEdit->SetEnabled(enable);
	pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("apk_save_path")));
	if (pEdit != NULL)
		pEdit->SetEnabled(enable);
	pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("keystore_password")));
	if (pEdit != NULL)
		pEdit->SetEnabled(enable);
	pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("keystore_alias")));
	if (pEdit != NULL)
		pEdit->SetEnabled(enable);
	pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("host")));
	if (pEdit != NULL)
		pEdit->SetEnabled(enable);

	CButtonUI* pButton = static_cast<CButtonUI*>(paint_manager_.FindControl(_T("jdk_folder")));
	if (pButton != NULL)
		pButton->SetEnabled(enable);

	pButton = static_cast<CButtonUI*>(paint_manager_.FindControl(_T("keystore_folder")));
	if (pButton != NULL)
		pButton->SetEnabled(enable);

	pButton = static_cast<CButtonUI*>(paint_manager_.FindControl(_T("apk_save_folder")));
	if (pButton != NULL)
		pButton->SetEnabled(enable);

	pButton = static_cast<CButtonUI*>(paint_manager_.FindControl(_T("log_group1")));
	if (pButton != NULL)
		pButton->SetEnabled(enable);
	pButton = static_cast<CButtonUI*>(paint_manager_.FindControl(_T("log_group2")));
	if (pButton != NULL)
		pButton->SetEnabled(enable);

	pButton = static_cast<CButtonUI*>(paint_manager_.FindControl(_T("sign_tool_group1")));
	if (pButton != NULL)
		pButton->SetEnabled(enable);
	pButton = static_cast<CButtonUI*>(paint_manager_.FindControl(_T("sign_tool_group2")));
	if (pButton != NULL)
		pButton->SetEnabled(enable);

	pButton = static_cast<CButtonUI*>(paint_manager_.FindControl(_T("package_type_group1")));
	if (pButton != NULL)
		pButton->SetEnabled(enable);
	pButton = static_cast<CButtonUI*>(paint_manager_.FindControl(_T("package_type_group2")));
	if (pButton != NULL)
		pButton->SetEnabled(enable);

	pButton = static_cast<CButtonUI*>(paint_manager_.FindControl(_T("version_type_group1")));
	if (pButton != NULL)
		pButton->SetEnabled(enable);
	pButton = static_cast<CButtonUI*>(paint_manager_.FindControl(_T("version_type_group2")));
	if (pButton != NULL)
		pButton->SetEnabled(enable);
	pButton = static_cast<CButtonUI*>(paint_manager_.FindControl(_T("version_type_group3")));
	if (pButton != NULL)
		pButton->SetEnabled(enable);

	pButton = static_cast<CButtonUI*>(paint_manager_.FindControl(_T("savebtn")));
	if (pButton != NULL)
		pButton->SetEnabled(enable);

	pButton = static_cast<CButtonUI*>(paint_manager_.FindControl(_T("browser_folder")));
	if (pButton != NULL)
		pButton->SetEnabled(enable);

	pButton = static_cast<CButtonUI*>(paint_manager_.FindControl(_T("android_sdk_folder")));
	if (pButton != NULL)
		pButton->SetEnabled(enable);

	pButton = static_cast<CButtonUI*>(paint_manager_.FindControl(_T("debug_switch")));
	if (pButton != NULL)
		pButton->SetEnabled(enable);

	pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("android_sdk")));
	if (pEdit != NULL)
		pEdit->SetEnabled(enable);
}

void MainFrame::Notify(TNotifyUI& msg)
{
	if (_tcsicmp(msg.sType, kWindowInit) == 0)
	{
		OnPrepare(msg);
	}
	else if (_tcsicmp(msg.sType, kSelectChanged) == 0)
	{
		CStdString name = msg.pSender->GetName();
		
		if (_tcsicmp(name, _T("main_tab1")) == 0 || _tcsicmp(name, _T("main_tab2")) == 0) {
			CTabLayoutUI* pTab = static_cast<CTabLayoutUI*>(paint_manager_.FindControl(_T("main_tab")));
			COptionUI* pOption = static_cast<COptionUI*>(paint_manager_.FindControl(_T("main_tab1")));

			if (pOption != NULL && pTab != NULL)
				pTab->SelectItem(pOption->IsSelected() ? 0 : 1);
			UpdateUI();
		}
		else if (_tcsicmp(name, _T("setting_tab1")) == 0 || _tcsicmp(name, _T("setting_tab2")) == 0 || _tcsicmp(name, _T("setting_tab3")) == 0 ) {
			CTabLayoutUI* pTab = static_cast<CTabLayoutUI*>(paint_manager_.FindControl(_T("setting_tab")));

			COptionUI* pSetting1Option = static_cast<COptionUI*>(paint_manager_.FindControl(_T("setting_tab1")));
			COptionUI* pSetting2Option = static_cast<COptionUI*>(paint_manager_.FindControl(_T("setting_tab2")));
			COptionUI* pSetting3Option = static_cast<COptionUI*>(paint_manager_.FindControl(_T("setting_tab3")));

			if (pSetting1Option != NULL && pSetting2Option != NULL && pTab != NULL)
				pTab->SelectItem(pSetting1Option->IsSelected() ? 0 : (pSetting2Option->IsSelected() ? 1 : (pSetting3Option->IsSelected() ? 2 : 3)));
		}
	}
	else if (_tcsicmp(msg.sType, kClick) == 0)
	{
		if (_tcsicmp(msg.pSender->GetName(), kCloseButtonControlName) == 0)
		{
			OnExit(msg);
		}
		else if (_tcsicmp(msg.pSender->GetName(), kMinButtonControlName) == 0)
		{
#if defined(UNDER_CE)
			::ShowWindow(m_hWnd, SW_MINIMIZE);
#else
			SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
#endif
		}
		else if (_tcsicmp(msg.pSender->GetName(), kMaxButtonControlName) == 0)
		{
#if defined(UNDER_CE)
			::ShowWindow(m_hWnd, SW_MAXIMIZE);
			CControlUI* pControl = static_cast<CControlUI*>(paint_manager_.FindControl(kMaxButtonControlName));
			if( pControl ) pControl->SetVisible(false);
			pControl = static_cast<CControlUI*>(paint_manager_.FindControl(kRestoreButtonControlName));
			if( pControl ) pControl->SetVisible(true);
#else
			SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0);
#endif
		}
		else if (_tcsicmp(msg.pSender->GetName(), kRestoreButtonControlName) == 0)
		{
#if defined(UNDER_CE)
			::ShowWindow(m_hWnd, SW_RESTORE);
			CControlUI* pControl = static_cast<CControlUI*>(paint_manager_.FindControl(kMaxButtonControlName));
			if( pControl ) pControl->SetVisible(true);
			pControl = static_cast<CControlUI*>(paint_manager_.FindControl(kRestoreButtonControlName));
			if( pControl ) pControl->SetVisible(false);
#else
			SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0);
#endif
		}
		else if (_tcsicmp(msg.pSender->GetName(), _T("clear_log")) == 0)
		{
			CAutoEvent autoLock(lock_);
			CListUI* pList = static_cast<CListUI*>(paint_manager_.FindControl(_T("log_list")));
			if (pList != NULL)
			{
				pList->RemoveAll();
			}
		}
		else if (_tcsicmp(msg.pSender->GetName(), _T("savebtn")) == 0)
		{
			SaveProperty();
		}
		else if (_tcsicmp(msg.pSender->GetName(), _T("signbtn")) == 0)
		{
			if (thread_ != NULL)
			{
				if (DUIMessageBox(m_hWnd, _T("确定要停止签名吗？"), _T("警告"), MB_OKCANCEL) == IDOK)
				{
					CAutoEvent autoLock(lock_);
					thread_cancel_ = true;
				}

				return;
			}

			tString apkFile;
			CEditUI* pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("apk_file")));
			if (pEdit != NULL)
				apkFile = pEdit->GetText();

			if (apkFile.empty())
			{
				DUIMessageBox(m_hWnd, _T("请选择要进行签名的APK文件!"), _T("警告"));
				return;
			}

			pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("jdk_path")));
			if (pEdit != NULL)
				jdkPath_ = pEdit->GetText();
			pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("keystore_file")));
			if (pEdit != NULL)
				keystore_ = pEdit->GetText();
			pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("apk_save_path")));
			if (pEdit != NULL)
				outputPath_ = pEdit->GetText();
			pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("keystore_password")));
			if (pEdit != NULL)
				password_ = pEdit->GetText();
			pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("keystore_alias")));
			if (pEdit != NULL)
				aliasname_ = pEdit->GetText();
			pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("android_sdk")));
			if (pEdit != NULL)
				android_sdk_path_ = pEdit->GetText();
			pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("host")));
			if (pEdit != NULL)
				host_ = pEdit->GetText();

			if (jdkPath_.empty() || keystore_.empty() || outputPath_.empty() || password_.empty() || android_sdk_path_.empty() || host_.empty())
			{
				DUIMessageBox(m_hWnd, _T("请到设置面板中进行相关设置后在进行签名!"), _T("警告"));
				return;
			}

			COptionUI* pOption = static_cast<COptionUI*>(paint_manager_.FindControl(_T("sign_tool_group1")));
			if (pOption->IsSelected())
			{
				WIN32_FIND_DATA wfd = {0};
				if (jdkPath_.at(jdkPath_.length() - 1) != _T('\\'))
					jdkPath_ += _T("\\");
				tString jarsigner = jdkPath_ + _T("bin\\jarsigner.exe");
				HANDLE hFindFile = FindFirstFile(jarsigner.c_str(), &wfd);
				if (hFindFile == NULL || !((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0))
				{
					DUIMessageBox(m_hWnd, _T("JDK路径设置不正确，请重新设置!"), _T("警告"));
					return;
				}

				if (hFindFile != NULL)
					FindClose(hFindFile);
			}
			else
			{
				tString temp = paint_manager_.GetInstancePath();
				WIN32_FIND_DATA wfd = {0};
				if (temp.at(temp.length() - 1) != _T('\\'))
					temp += _T("\\");

				tString java_bin = temp + _T("bin\\java.exe");
				HANDLE hFindFile = FindFirstFile(java_bin.c_str(), &wfd);
				if (hFindFile == NULL || !((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0))
				{
					DUIMessageBox(m_hWnd, _T("JDK路径设置不正确，请重新设置!"), _T("警告"));
					return;
				}

				if (hFindFile != NULL)
					FindClose(hFindFile);

				tString signapk_jar = temp + _T("signapk.jar");
				hFindFile = FindFirstFile(signapk_jar.c_str(), &wfd);
				if (hFindFile == NULL || !((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0))
				{
					DUIMessageBox(m_hWnd, _T("jarsign.jar路径设置不正确，请重新设置!"), _T("警告"));
					return;
				}

				if (hFindFile != NULL)
					FindClose(hFindFile);

				tString platform_x509_pem = temp + _T("apkpack.x509.pem");
				hFindFile = FindFirstFile(platform_x509_pem.c_str(), &wfd);
				if (hFindFile == NULL || !((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0))
				{
					DUIMessageBox(m_hWnd, _T("apkpack.x509.pem路径设置不正确，请重新设置!"), _T("警告"));
					return;
				}

				if (hFindFile != NULL)
					FindClose(hFindFile);

				tString platform_pk8 = temp + _T("apkpack.pk8");
				hFindFile = FindFirstFile(platform_pk8.c_str(), &wfd);
				if (hFindFile == NULL || !((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0))
				{
					DUIMessageBox(m_hWnd, _T("apkpack.pk8路径设置不正确，请重新设置!"), _T("警告"));
					return;
				}

				if (hFindFile != NULL)
					FindClose(hFindFile);
			}

			msg.pSender->SetText(_T("停止签名"));

			DWORD dwThreadId = 0;
			HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, _T("xxxx"));

			thread_ = CreateThread(NULL, 0, ApkSignThread, (LPVOID)this, 0, &dwThreadId);

			UpdateUI();

			MSG msg;
			bool bEndUserCanceled = false;
			bool bEndLoop = false;
			while (!bEndLoop)
			{
				DWORD nRet = MsgWaitForMultipleObjectsEx(1, &hEvent, 5 * 60 * 1000, QS_ALLINPUT, MWMO_INPUTAVAILABLE);
				switch (nRet)
				{
				case WAIT_OBJECT_0:
					bEndLoop = true;
					break;
				case WAIT_OBJECT_0 + 1:
					if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
					{
						//if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
						//{
						//	if ((msg.message == WM_KEYDOWN) && (msg.wParam == VK_ESCAPE))
						//	{
						//		bEndUserCanceled = true;
						//		bEndLoop = true;
						//		break;
						//	}
						//}
						while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
						{
							TranslateMessage(&msg);
							DispatchMessage(&msg);

							if (msg.message == WM_CLOSE)
							{
								bEndLoop = true;
								break;
							}
						}
						break;
					}
					break;
				case WAIT_TIMEOUT:
					bEndLoop = true;
					break;
				default:
					break;
				}
			}
		}
		else if (_tcsicmp(msg.pSender->GetName(), _T("android_sdk_folder")) == 0)
		{
			LPITEMIDLIST rootLoation;
			SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &rootLoation);
			if (rootLoation == NULL)
				return;

			BROWSEINFO browseInfo;
			ZeroMemory(&browseInfo, sizeof(browseInfo));
			browseInfo.pidlRoot = rootLoation;
			browseInfo.ulFlags = 0;
			browseInfo.lpszTitle = _T("请选择Android安装的路径");
			browseInfo.lpfn = NULL;
			browseInfo.lParam = NULL;

			LPITEMIDLIST targetLocation = SHBrowseForFolder(&browseInfo);
			if (targetLocation == NULL)
				return;

			TCHAR targetPath[MAX_PATH] = {_T('\0')};
			SHGetPathFromIDList(targetLocation, targetPath);
			if (_tcslen(targetPath) > 0)
			{
				CEditUI* pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("android_sdk")));
				if (pEdit != NULL)
					pEdit->SetText(targetPath);
			}
		}
		else if (_tcsicmp(msg.pSender->GetName(), _T("jdk_folder")) == 0)
		{
			LPITEMIDLIST rootLoation;
			SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &rootLoation);
			if (rootLoation == NULL)
				return;

			BROWSEINFO browseInfo;
			ZeroMemory(&browseInfo, sizeof(browseInfo));
			browseInfo.pidlRoot = rootLoation;
			browseInfo.ulFlags = 0;
			browseInfo.lpszTitle = _T("请选择JDK安装的路径");
			browseInfo.lpfn = NULL;
			browseInfo.lParam = NULL;

			LPITEMIDLIST targetLocation = SHBrowseForFolder(&browseInfo);
			if (targetLocation == NULL)
				return;

			TCHAR targetPath[MAX_PATH] = {_T('\0')};
			SHGetPathFromIDList(targetLocation, targetPath);
			if (_tcslen(targetPath) > 0)
			{
				CEditUI* pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("jdk_path")));
				if (pEdit != NULL)
					pEdit->SetText(targetPath);
			}
		}
		else if (_tcsicmp(msg.pSender->GetName(), _T("apk_save_folder")) == 0)
		{
			LPITEMIDLIST rootLoation;
			SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &rootLoation);
			if (rootLoation == NULL)
				return;

			BROWSEINFO browseInfo;
			ZeroMemory(&browseInfo, sizeof(browseInfo));
			browseInfo.pidlRoot = rootLoation;
			browseInfo.ulFlags = 0;
			browseInfo.lpszTitle = _T("请选择打包后APK存放的路径");
			browseInfo.lpfn = NULL;
			browseInfo.lParam = NULL;

			LPITEMIDLIST targetLocation = SHBrowseForFolder(&browseInfo);
			if (targetLocation == NULL)
				return;

			TCHAR targetPath[MAX_PATH] = {_T('\0')};
			SHGetPathFromIDList(targetLocation, targetPath);
			if (_tcslen(targetPath) > 0)
			{
				CEditUI* pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("apk_save_path")));
				if (pEdit != NULL)
					pEdit->SetText(targetPath);
			}
		}
		else if (_tcsicmp(msg.pSender->GetName(), _T("browser_folder")) == 0)	
		{
			OPENFILENAME ofn = {0};
			TCHAR szFileName[MAX_PATH] = {0};

			ZeroMemory(&ofn, sizeof(ofn));

			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = m_hWnd;
			ofn.lpstrFilter = _T("APK Files (*.apk)\0*.apk\0All Files (*.*)\0*.*\0");
			ofn.lpstrFile = szFileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY|OFN_ALLOWMULTISELECT;
			ofn.lpstrDefExt = _T("apk");

			if (GetOpenFileName(&ofn))
			{
				// Do something usefull with the filename stored in szFileName
				CEditUI* pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("apk_file")));
				if (pEdit != NULL)
					pEdit->SetText(szFileName);
			}
		}
		else if (_tcsicmp(msg.pSender->GetName(), _T("keystore_folder")) == 0) {
			OPENFILENAME ofn = {0};
			TCHAR szFileName[MAX_PATH] = {0};

			ZeroMemory(&ofn, sizeof(ofn));

			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = m_hWnd;
			ofn.lpstrFilter = _T("All Files (*.*)\0*.*\0");
			ofn.lpstrFile = szFileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY|OFN_ALLOWMULTISELECT;
			ofn.lpstrDefExt = _T("*.*");

			if (GetOpenFileName(&ofn))
			{
				// Do something usefull with the filename stored in szFileName
				CEditUI* pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("keystore_file")));
				if (pEdit != NULL)
					pEdit->SetText(szFileName);
			}
		}
	}
	else if (_tcsicmp(msg.sType, kTimer) == 0)
	{
		return OnTimer(msg);
	}
}

LRESULT MainFrame::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

CControlUI* MyBuilderCallback::CreateControl(LPCTSTR pstrClass, CPaintManagerUI* pManager)
{
	if (_tcsicmp(pstrClass, _T("ListContainerElementEx")) == 0)
	{
		return new CListContainerElementUIEx();
	}
	return NULL;
}

void MainFrame::LoadProperty()
{
	DWORD log = 0;
	DWORD package_type = 0;
	DWORD version_type = 0;
	DWORD sign_tool = 0;

	tString iniFile = paint_manager_.GetInstancePath();
	iniFile += _T("settings.ini");

	TCHAR szBuf[MAX_PATH] = {0};
	GetPrivateProfileString(_T("jdk"), _T("jdk_path"), _T(""), szBuf, MAX_PATH - 1, iniFile.c_str());
	jdkPath_ = szBuf;

	GetPrivateProfileString (_T("keystore"), _T("keystore_path"), _T(""), szBuf, MAX_PATH - 1, iniFile.c_str());
	keystore_ = szBuf;
	
	GetPrivateProfileString (_T("keystore"), _T("password"), _T(""), szBuf, MAX_PATH - 1, iniFile.c_str());
	password_ = szBuf;

	GetPrivateProfileString (_T("keystore"), _T("alias"), _T(""), szBuf, MAX_PATH - 1, iniFile.c_str());
	aliasname_ = szBuf;
	
	GetPrivateProfileString (_T("keystore"), _T("output_path"), _T(""), szBuf, MAX_PATH - 1, iniFile.c_str());
	outputPath_ = szBuf;

	GetPrivateProfileString (_T("keystore"), _T("android_sdk"), _T(""), szBuf, MAX_PATH - 1, iniFile.c_str());
	android_sdk_path_ = szBuf;

	GetPrivateProfileString(_T("host"), _T("host"), _T(""), szBuf, MAX_PATH - 1, iniFile.c_str());
	host_ = szBuf;

	log = GetPrivateProfileInt(_T("pack"), _T("log"), 0, iniFile.c_str());
	package_type = GetPrivateProfileInt(_T("pack"), _T("package_type"), 0, iniFile.c_str());
	version_type = GetPrivateProfileInt(_T("pack"), _T("version_type"), 0, iniFile.c_str());
	sign_tool = GetPrivateProfileInt(_T("sign_tool"), _T("sign_tool"), 0, iniFile.c_str());

	CEditUI* pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("jdk_path")));
	if (pEdit != NULL)
		pEdit->SetText(jdkPath_.c_str());
	pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("keystore_file")));
	if (pEdit != NULL)
		pEdit->SetText(keystore_.c_str());
	pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("apk_save_path")));
	if (pEdit != NULL)
		pEdit->SetText(outputPath_.c_str());
	pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("keystore_password")));
	if (pEdit != NULL)
		pEdit->SetText(password_.c_str());
	pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("keystore_alias")));
	if (pEdit != NULL)
		pEdit->SetText(aliasname_.c_str());
	pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("android_sdk")));
	if (pEdit != NULL)
		pEdit->SetText(android_sdk_path_.c_str());
	pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("host")));
	if (pEdit != NULL)
		pEdit->SetText(host_.c_str());

	COptionUI* pOption = static_cast<COptionUI*>(paint_manager_.FindControl(_T("log_group1")));
	COptionUI* pOption2 = static_cast<COptionUI*>(paint_manager_.FindControl(_T("log_group2")));
	if (pOption != NULL && pOption2 != NULL)
	{
		pOption->Selected(log == 0 ? true : false);
		pOption2->Selected(version_type == 1 ? true : false);
	}

	pOption = static_cast<COptionUI*>(paint_manager_.FindControl(_T("sign_tool_group1")));
	pOption2 = static_cast<COptionUI*>(paint_manager_.FindControl(_T("sign_tool_group2")));
	if (pOption != NULL && pOption2 != NULL)
	{
		pOption->Selected(sign_tool == 0 ? true : false);
		pOption2->Selected(sign_tool == 1 ? true : false);
	}

	pOption = static_cast<COptionUI*>(paint_manager_.FindControl(_T("package_type_group1")));
	pOption2 = static_cast<COptionUI*>(paint_manager_.FindControl(_T("package_type_group2")));
	if (pOption != NULL && pOption2 != NULL)
	{
		pOption->Selected(package_type == 0 ? true : false);
		pOption2->Selected(version_type == 1 ? true : false);
	}

	pOption = static_cast<COptionUI*>(paint_manager_.FindControl(_T("version_type_group1")));
	pOption2 = static_cast<COptionUI*>(paint_manager_.FindControl(_T("version_type_group2")));
	COptionUI* pOption3 = static_cast<COptionUI*>(paint_manager_.FindControl(_T("version_type_group3")));
	if (pOption != NULL && pOption2 != NULL && pOption3 != NULL)
	{
		pOption->Selected(version_type == 0 ? true : false);
		pOption2->Selected(version_type == 1 ? true : false);
		pOption3->Selected(version_type == 2 ? true : false);
	}
}

void MainFrame::SaveProperty()
{
	CEditUI* pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("jdk_path")));
	if (pEdit != NULL)
		jdkPath_ = pEdit->GetText();
	pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("keystore_file")));
	if (pEdit != NULL)
		keystore_ = pEdit->GetText();
	pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("apk_save_path")));
	if (pEdit != NULL)
		outputPath_ = pEdit->GetText();
	pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("keystore_password")));
	if (pEdit != NULL)
		password_ = pEdit->GetText();
	pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("keystore_alias")));
	if (pEdit != NULL)
		aliasname_ = pEdit->GetText();
	pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("android_sdk")));
	if (pEdit != NULL)
		android_sdk_path_ = pEdit->GetText();
	pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("host")));
	if (pEdit != NULL)
		host_ = pEdit->GetText();

	DWORD log = 0;
	DWORD package_type = 0;
	DWORD version_type = 0;
	DWORD sign_tool = 0;
	COptionUI* pOption = static_cast<COptionUI*>(paint_manager_.FindControl(_T("log_group1")));
	if (pOption != NULL)
		log = pOption->IsSelected() ? 0 : 1;

	pOption = static_cast<COptionUI*>(paint_manager_.FindControl(_T("sign_tool_group1")));
	if (pOption != NULL)
		sign_tool = pOption->IsSelected() ? 0 : 1;

	pOption = static_cast<COptionUI*>(paint_manager_.FindControl(_T("package_type_group1")));
	if (pOption != NULL)
		package_type = pOption->IsSelected() ? 0 : 1;

	pOption = static_cast<COptionUI*>(paint_manager_.FindControl(_T("version_type_group1")));
	COptionUI* pOption2 = static_cast<COptionUI*>(paint_manager_.FindControl(_T("version_type_group2")));
	if (pOption != NULL && pOption2 != NULL)
		version_type = pOption->IsSelected() ? 0 : (pOption2->IsSelected() ? 1 : 2);

	tString iniFile = paint_manager_.GetInstancePath();
	iniFile += _T("settings.ini");

	WritePrivateProfileString(NULL, NULL, NULL, iniFile.c_str());

	WritePrivateProfileString (_T("jdk"), 
		_T("jdk_path"),
		jdkPath_.c_str(), 
		iniFile.c_str());

	WritePrivateProfileString (_T("keystore"), 
		_T("keystore_path"),
		keystore_.c_str(), 
		iniFile.c_str());

	WritePrivateProfileString (_T("keystore"), 
		_T("password"),
		password_.c_str(), 
		iniFile.c_str());

	WritePrivateProfileString (_T("keystore"), 
		_T("alias"),
		aliasname_.c_str(), 
		iniFile.c_str());

	WritePrivateProfileString (_T("keystore"), 
		_T("android_sdk"),
		android_sdk_path_.c_str(), 
		iniFile.c_str());

	WritePrivateProfileString (_T("keystore"), 
		_T("output_path"),
		outputPath_.c_str(), 
		iniFile.c_str());

	WritePrivateProfileString(_T("host"),
		_T("host"),
		host_.c_str(),
		iniFile.c_str());

	TCHAR szBuf[MAX_PATH] = {0};
	_stprintf(szBuf, _T("%d"), log);
	WritePrivateProfileString (_T("pack"), 
		_T("log"),
		szBuf,
		iniFile.c_str());

	_stprintf(szBuf, _T("%d"), package_type);
	WritePrivateProfileString (_T("pack"), 
		_T("package_type"),
		szBuf,
		iniFile.c_str());

	_stprintf(szBuf, _T("%d"), sign_tool);
	WritePrivateProfileString (_T("sign_tool"), 
		_T("sign_tool"),
		szBuf,
		iniFile.c_str());

	_stprintf(szBuf, _T("%d"), version_type);
	WritePrivateProfileString (_T("pack"), 
		_T("version_type"),
		szBuf,
		iniFile.c_str());
}

void MainFrame::EnsureDirectory(const TCHAR *rootdir, const TCHAR *dir)
{
	if (rootdir!=0 && GetFileAttributes(rootdir)==0xFFFFFFFF)
		CreateDirectory(rootdir,0);

	if (*dir==0)
		return;
	const TCHAR *lastslash=dir, *c=lastslash;
	while (*c!=0)
	{
		if (*c=='/' || *c=='\\')
			lastslash=c; c++;
	}
	const TCHAR *name=lastslash;
	if (lastslash!=dir)
	{
		TCHAR tmp[MAX_PATH]; memcpy(tmp,dir,sizeof(TCHAR)*(lastslash-dir));
		tmp[lastslash-dir]=0;
		EnsureDirectory(rootdir,tmp);
		name++;
	}
	TCHAR cd[MAX_PATH]; *cd=0;
	if (rootdir!=0)
		_tcscpy(cd,rootdir);
	_tcscat(cd,dir);
	if (GetFileAttributes(cd)==0xFFFFFFFF)
		CreateDirectory(cd,NULL);
}

void MainFrame::DeleteDirectory(const TCHAR *inDirName)
{
	TCHAR dirName[MAX_PATH] = {0};
	TCHAR* dirNamePtr = dirName;
	TCHAR ch;
	WIN32_FIND_DATA fd = {0};
	HANDLE handle = INVALID_HANDLE_VALUE;

	if (inDirName == NULL || *inDirName == 0)
		return;

	while (ch = *inDirName++) {
		if (ch == _T('/')  ||  ch == _T('\\'))
			*dirNamePtr++ = _T('\\');
		else
			*dirNamePtr++ = ch;
	}
	if (dirNamePtr[-1] != _T('\\'))
		*dirNamePtr++ = _T('\\');
	*dirNamePtr = 0;

	_tcscat(dirNamePtr, _T("*.*"));

	handle = FindFirstFile(dirName, &fd);
	if (handle == INVALID_HANDLE_VALUE)
		return;

	*dirNamePtr = 0;

	do {
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			int skipDir = fd.cFileName[0] == _T('.')  &&
				(fd.cFileName[1] == 0  ||  (fd.cFileName[1] == _T('.')  &&  fd.cFileName[2] == 0));
			if (!skipDir) {
				_tcscpy(dirNamePtr, fd.cFileName);
				_tcscat(dirNamePtr, _T("\\"));
				DeleteDirectory(dirName);
			}
		} else {
			_tcscpy(dirNamePtr, fd.cFileName);
			SetFileAttributes(dirName, FILE_ATTRIBUTE_ARCHIVE);
			if (!DeleteFile(dirName)) {
				FindClose(handle);
				return;
			}
		}
	} while (FindNextFile(handle, &fd));

	FindClose(handle);

	*dirNamePtr = 0;
	if (!RemoveDirectory(dirName))
		return;
}

DWORD MainFrame::ApkSignThread(LPVOID lpVoid)
{
	MainFrame* pThis = reinterpret_cast<MainFrame*>(lpVoid);

	tString apkFile;
	CEditUI* pEdit = static_cast<CEditUI*>(pThis->paint_manager_.FindControl(_T("apk_file")));
	if (pEdit != NULL)
		apkFile = pEdit->GetText();

	pEdit = static_cast<CEditUI*>(pThis->paint_manager_.FindControl(_T("apk_save_path")));
	if (pEdit != NULL)
		pThis->outputPath_ = pEdit->GetText();

	TCHAR szBuf[MAX_PATH * 2] = {0};
	tString log;
	int code = 0;
	{
		tString apkFileFolder = apkFile.substr(0, apkFile.find_last_of(_T("\\")));
		tString apkFileName = apkFile.substr(apkFile.find_last_of(_T("\\")) + 1);
		apkFileName = apkFileName.substr(0, apkFileName.find_last_of(_T(".")));

		COptionUI* pShowDebug = static_cast<COptionUI*>(pThis->paint_manager_.FindControl(_T("debug_switch")));
		bool pause = false;
		if (pShowDebug != NULL)
			pause = pShowDebug->IsSelected();

		// 进行签名
		pThis->SignApk(apkFile, pThis->outputPath_);
	}

	CloseHandle(pThis->thread_);
	pThis->thread_ = NULL;

	pThis->UpdateUI();

	{
		CAutoEvent autoLock(pThis->lock_);
		pThis->thread_cancel_ = false;
		CControlUI* pButton = pThis->paint_manager_.FindControl(_T("signbtn"));
		if (pButton != NULL)
			pButton->SetText(_T("签名"));
	}

	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, _T("xxxx"));
	SetEvent(hEvent);
	CloseHandle(hEvent);

	ExitThread(0);
	return 1;
}

void MainFrame::ZipDirectory(HZIP hZip, const TCHAR* baseFolder, const TCHAR *inDirName)
{
	TCHAR dirName[MAX_PATH] = {0};
	TCHAR folderName[MAX_PATH] = {0};
	TCHAR* dirNamePtr = dirName;
	TCHAR ch;
	WIN32_FIND_DATA fd = {0};
	HANDLE handle = INVALID_HANDLE_VALUE;
	ZRESULT zResult;

	if (inDirName == NULL || *inDirName == 0 || hZip == NULL || baseFolder == NULL)
		return;

	_tcscpy_s(folderName, MAX_PATH - 1, inDirName);

	while (ch = *inDirName++) {
		if (ch == _T('/')  ||  ch == _T('\\'))
			*dirNamePtr++ = _T('\\');
		else
			*dirNamePtr++ = ch;
	}
	if (dirNamePtr[-1] != _T('\\'))
		*dirNamePtr++ = _T('\\');
	*dirNamePtr = 0;

	_tcscat(dirNamePtr, _T("*.*"));

	handle = FindFirstFile(dirName, &fd);
	if (handle == INVALID_HANDLE_VALUE)
		return;

	*dirNamePtr = 0;

	do {
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			int skipDir = fd.cFileName[0] == _T('.')  &&
				(fd.cFileName[1] == 0  ||  (fd.cFileName[1] == _T('.')  &&  fd.cFileName[2] == 0));
			if (!skipDir) {
				TCHAR zipSrcFile[MAX_PATH] = {0};
				TCHAR zipDestFile[MAX_PATH] = {0};
				_tcscpy(dirNamePtr, fd.cFileName);
				_tcscat(dirNamePtr, _T("\\"));

				memset(zipDestFile, 0, MAX_PATH);
				memset(zipSrcFile, 0, MAX_PATH);

				_stprintf_s(zipSrcFile, MAX_PATH - 1, _T("%s\\%s"), folderName, fd.cFileName);
				if (_tcslen(baseFolder) > 0)
				{
					_stprintf_s(zipDestFile, MAX_PATH - 1, _T("%s\\%s"), baseFolder, fd.cFileName);
				}
				else
				{
					_stprintf_s(zipDestFile, MAX_PATH - 1, _T("%s"), fd.cFileName);
				}
				zResult = ZipAddFolder(hZip, zipDestFile);
				ZipDirectory(hZip, zipDestFile, zipSrcFile);
			}
		} else {
			TCHAR zipSrcFile[MAX_PATH] = {0};
			TCHAR zipDestFile[MAX_PATH] = {0};
			_tcscpy(dirNamePtr, fd.cFileName);

			memset(zipDestFile, 0, MAX_PATH);
			memset(zipSrcFile, 0, MAX_PATH);

			_stprintf_s(zipSrcFile, MAX_PATH - 1, _T("%s\\%s"), folderName, fd.cFileName);
			if (_tcslen(baseFolder) > 0)
			{
				_stprintf_s(zipDestFile, MAX_PATH - 1, _T("%s\\%s"), baseFolder, fd.cFileName);
			}
			else
			{
				_stprintf_s(zipDestFile, MAX_PATH - 1, _T("%s"), fd.cFileName);
			}
			zResult = ZipAdd(hZip, zipDestFile, zipSrcFile);
		}
	} while (FindNextFile(handle, &fd));

	FindClose(handle);

	*dirNamePtr = 0;
}

bool MainFrame::ZipAPK(const tString& folder, const tString& apk)
{
	if (folder.empty() || apk.empty())
		return false;

	HZIP hZip = CreateZip(apk.c_str(), 0);
	if (hZip != NULL)
	{
		ZipDirectory(hZip, _T(""), folder.c_str());
		CloseZip(hZip);
	}

	return true;
}

void MainFrame::SignApk(const tString& old_apk, const tString& output_path)
{
	TCHAR szBuf[MAX_PATH * 2] = {0};

	tString log;
	_stprintf(szBuf, _T("准备签名所需文件....."));
	log = szBuf;
	AddLogMessage(log);

	PROCESS_INFORMATION processsInfo = {0};
	CEditUI* pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("jdk_path")));
	if (pEdit != NULL)
		jdkPath_ = pEdit->GetText();

	if (jdkPath_.at(jdkPath_.length() - 1) != _T('\\'))
		jdkPath_ += _T("\\");

	tString jarsignerFolder = jdkPath_ + _T("bin");
	tString jarsigner = jarsignerFolder;
	jarsigner += _T("\\jarsigner.exe");

	tString apkName = old_apk.substr(old_apk.find_last_of(_T("\\")) + 1);
	apkName = apkName.substr(0, apkName.find_last_of(_T(".")));

	tString apkPath = old_apk.substr(0, old_apk.find_last_of(_T("\\")));

	tString myApkNameSuffix = _T("_signed.apk");

	tString apkFileUnaligned = apkPath + _T("\\") + apkName + _T("-unaligned.apk");
	tString apkSigned = output_path + apkName + myApkNameSuffix;

	DeleteFile(apkSigned.c_str());

	DeleteFile(apkSigned.c_str());

	_stprintf(szBuf, _T("正在签名....."));
	log = szBuf;
	AddLogMessage(log);

	COptionUI* pShowDebug = static_cast<COptionUI*>(paint_manager_.FindControl(_T("debug_switch")));
	bool pause = false;
	if (pShowDebug != NULL)
		pause = pShowDebug->IsSelected();

#pragma region 签名时可能遇到的问题
	/*
	jarsigner: unable to sign jar: java.util.zip.ZipException: invalid entry compressed size (expected 463 but got 465 bytes)
	解决方法：
		(http://stackoverflow.com/questions/5089042/jarsigner-unable-to-sign-jar-java-util-zip-zipexception-invalid-entry-compres)
		You are trying to sign an already signed apk. You need to export an unsigned apk file and then sign it with jarsigner.

	JDK7本身问题
	解决方法：
		-digestalg SHA1 -sigalg MD5withRSA
	*/
#pragma endregion

	int sign_code = 0;
	if (!thread_cancel_)
	{
		pEdit = static_cast<CEditUI*>(paint_manager_.FindControl(_T("android_sdk")));
		if (pEdit != NULL)
			android_sdk_path_ = pEdit->GetText();
		if (android_sdk_path_.at(android_sdk_path_.length() - 1) != _T('\\'))
			android_sdk_path_ += _T("\\");
		//http://developer.android.com/guide/publishing/app-signing.html#signapp
		//zipalign -v 4 your_project_name-unaligned.apk your_project_name.apk
		tString zipalignFolder = android_sdk_path_ + _T("tools");
		tString zipalign = zipalignFolder;
		zipalign += _T("\\zipalign.exe");

		// http://blog.macrowen.com/Technology/3071.html
		//http://docs.oracle.com/javase/1.3/docs/tooldocs/win32/jarsigner.html
		//http://blog.csdn.net/achellies/article/details/7323260
		COptionUI* pOption = static_cast<COptionUI*>(paint_manager_.FindControl(_T("sign_tool_group1")));
		if (pOption->IsSelected())
		{
			_stprintf(szBuf, _T("\"%s\" -verbose -sigfile CERT -keystore %s -storepass %s -digestalg SHA1 -sigalg MD5withRSA -signedjar %s %s %s%s"),
				jarsigner.c_str(), keystore_.c_str(), password_.c_str(), apkFileUnaligned.c_str(), old_apk.c_str(), aliasname_.empty() ? _T("mykey") : aliasname_.c_str(), pause ? _T("& pause") : _T(""));
			sign_code = system(StringConvertor::TcharToAnsi(szBuf));
		}
		else
		{
			tString current_folder = paint_manager_.GetInstancePath().GetData();
			_stprintf(szBuf, _T("cd %s & cd \"%s\" & java -jar signapk.jar apkpack.x509.pem apkpack.pk8 %s %s%s"),
				current_folder.c_str(), current_folder.substr(0, current_folder.find_first_of(_T(":")) + 1).c_str(), old_apk.c_str(), apkFileUnaligned.c_str(), pause ? _T("& pause") : _T(""));
			sign_code = system(StringConvertor::TcharToAnsi(szBuf));
		}

		_stprintf(szBuf, _T("\"%s\" -v 4 %s %s%s"),
			zipalign.c_str(), apkFileUnaligned.c_str(), apkSigned.c_str(), pause ? _T("& pause") : _T(""));
		sign_code = system(StringConvertor::TcharToAnsi(szBuf));
	}
	if (sign_code == 0)
		_stprintf(szBuf, _T("签名完成....."));
	else
		_stprintf(szBuf, _T("签名失败....."));
	log = szBuf;
	AddLogMessage(log);

	DeleteFile(apkFileUnaligned.c_str());

	//AllocConsole();
	//HWND hConsoleWnd = GetConsoleWindow();	
	//FreeConsole();

	//STARTUPINFO si;
	//GetStartupInfo(&si);
	//si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	//si.wShowWindow=SW_HIDE;
	////si.hStdInput = si.hStdOutput = si.hStdError = (void*)hConsoleWnd; 

	//TCHAR cmdline[MAX_PATH];
	//GetSystemDirectory(cmdline,sizeof(cmdline));
	//_tcscat(cmdline, _T("\\cmd.exe  /c dir /? pause"));

	//PROCESS_INFORMATION ProcessInfo;
	//CreateProcess(NULL,cmdline,NULL,NULL,TRUE,CREATE_NEW_CONSOLE,NULL,NULL,&si,&ProcessInfo);
	//WaitForSingleObject(ProcessInfo.hProcess,INFINITE);
	//CloseHandle(ProcessInfo.hProcess);
}

bool MainFrame::UnzipAPK(const tString& apk, const CPAData& cpa)
{
	if (apk.empty()) return false;

	tString temp = apk;
	temp += _T("_temp");

	tString tag = StringConvertor::AnsiToTchar(cpa.channelName.c_str());

	tString log;
	TCHAR szBuf[MAX_PATH] = {0};
	_stprintf(szBuf, _T("为\"cpa合作者(ID:%s)\"解压APK文件"), tag.c_str());
	log = szBuf;

	HZIP hZip = OpenZip(apk.c_str(), NULL);
	if (hZip != NULL)
	{
		tString zipFile;
		ZIPENTRY zipEntry = {0};
		ZRESULT zipResult = GetZipItem(hZip, -1, &zipEntry);
		if (zipResult == ZR_OK)
		{
			for (int i = 0; i < zipEntry.index; ++i)
			{
				ZIPENTRY zipEntry2 = {0};
				GetZipItem(hZip, i, &zipEntry2);

				log = _T("提取文件---");
				log += zipEntry2.name;

				zipFile = temp + _T("\\") + zipEntry2.name;
				CStringXD cstrZipFile = zipFile.c_str();
				cstrZipFile.Replace(_T("/"), _T("\\"));

				CStringXD cstrZipFolder;
				if (zipFile.find_last_of(_T('/')) != tString::npos)
					cstrZipFolder = zipFile.substr(0, zipFile.find_last_of(_T('/'))).c_str();
				else
					cstrZipFolder = zipFile.substr(0, zipFile.find_last_of(_T('\\'))).c_str();

				cstrZipFolder.Replace(_T("/"), _T("\\"));
				EnsureDirectory(NULL, cstrZipFolder.GetBuffer());
				UnzipItem(hZip, i, cstrZipFile.GetBuffer());
			}
		}

		CloseZip(hZip);
	}

	return false;
}

static bool OnMouseEvent(void* event) {
	if( ((TEventUI*)event)->Type == UIEVENT_MOUSEENTER || ((TEventUI*)event)->Type == UIEVENT_MOUSELEAVE ) {
		CControlUI* pControl = ((TEventUI*)event)->pSender;
		if( pControl != NULL ) {
			CListContainerElementUIEx* pListElement = (CListContainerElementUIEx*)(pControl->GetTag());
			if( pListElement != NULL ) pListElement->DoEvent(*(TEventUI*)event);
		}
	}

	return true;
}


void MainFrame::AddLogMessage(const tString& message, bool bTextColorRed)
{
	CAutoEvent autoLock(lock_);
	CListUI* pList = static_cast<CListUI*>(paint_manager_.FindControl(_T("log_list")));
	if (pList != NULL)
	{
		CListLabelElementUI* pLabelElement = new CListLabelElementUI();
		if (bTextColorRed)
		{
			pList->SetItemShowHtml(true);
			TCHAR szBuf[MAX_PATH] = {0};
			_stprintf_s(szBuf, MAX_PATH, _T("{c #FF0000}%s{/c}"), message.c_str());
			pLabelElement->SetText(szBuf);
		}
		else
			pLabelElement->SetText(message.c_str());
		pList->Add(pLabelElement);

		pList->Scroll(0, 1000);
	}
}

/////////////////////////////////////////////////////////////////////////////////////
//
// ListContainerElementUIEx
const TCHAR* const kListContainerElementUIExClassName = _T("ListContainerElementUIEx");
const TCHAR* const kListContainerElementUIExInterfaceName = _T("ListContainerElementEx");

CListContainerElementUIEx::CListContainerElementUIEx()
{}

CListContainerElementUIEx::~CListContainerElementUIEx()
{}

LPCTSTR CListContainerElementUIEx::GetClass() const
{
	return kListContainerElementUIExClassName;
}

LPVOID CListContainerElementUIEx::GetInterface(LPCTSTR pstrName)
{
	if( _tcscmp(pstrName, kListContainerElementUIExInterfaceName) == 0 ) return static_cast<CListContainerElementUIEx*>(this);
    return CListContainerElementUI::GetInterface(pstrName);
}

void CListContainerElementUIEx::SetPos(RECT rc)
{
	CListContainerElementUI::SetPos(rc);

	if( m_pOwner == NULL ) return;
	TListInfoUI* pInfo = m_pOwner->GetListInfo();

	if ((GetCount() == 1) && (GetItemAt(0)->GetInterface(kIContainerUIInterfaceName) != NULL))
	{
		for( int i = 0; i < pInfo->nColumns; i++ )
		{
			RECT rcItem = { pInfo->rcColumn[i].left, m_rcItem.top, pInfo->rcColumn[i].right, m_rcItem.bottom };

			CContainerUI* pSubControls = static_cast<CContainerUI*>(GetItemAt(0));
			if (pSubControls->GetItemAt(i) != NULL)
			{
				pSubControls->GetItemAt(i)->SetPos(rcItem);
			}
		}
	}
}