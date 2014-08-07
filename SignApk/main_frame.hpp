//
// main_frame.hpp
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

#ifndef MAINFRAME_HPP
#define MAINFRAME_HPP
#include "lock.hpp"

#define WM_INCREASE_PROGRESS (WM_USER + 0x1001)

class CListContainerElementUIEx;
class WindowImplBase;

struct CPAData
{
	// 渠道名称
	string channelName;

	// CPA开关
	string doSwitchCPA;

	// cpa合作者ID
	string partnerID;

	string subPartnerID;

	// cpa公钥
	string sourcePublicKey;

	// 是否做活动的开关
	string doSwitchPomotionVersion;

	// CPS开关
	string doSwitchCPS;

	// cps联盟号
	string unionId;

	// 联盟站点ID
	string unionSiteId;

	// 存放路径
	string path;

	// 是否为推广包
	bool bPromotionPackage;

	CPAData()
		: bPromotionPackage(true)
	{}
};

class MainFrame : public WindowImplBase
{
public:

	MainFrame();
	~MainFrame();

public:

	LPCTSTR GetWindowClassName() const;	

	virtual void OnFinalMessage(HWND hWnd);

	virtual void Init();

	virtual LRESULT ResponseDefaultKeyEvent(WPARAM wParam);

	virtual tString GetSkinFile();

	virtual tString GetSkinFolder();

	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

protected:	

	void Notify(TNotifyUI& msg);
	void OnPrepare(TNotifyUI& msg);
	void OnExit(TNotifyUI& msg);
	void OnTimer(TNotifyUI& msg);

private:

	void LoadProperty();
	void SaveProperty();

	void IncreaseProgess();

	void UpdateUI();

	bool UnzipAPK(const tString& apk, const CPAData& cpa);

	bool ZipAPK(const tString& folder, const tString& apk);
	void ZipDirectory(HZIP hZip, const TCHAR* baseFolder, const TCHAR *inDirName);

	void SignApk(const tString& old_apk, const tString& output_path);

	void AddLogMessage(const tString& message, bool bTextColorRed = false);

	void EnsureDirectory(const TCHAR *rootdir, const TCHAR *dir);
	void DeleteDirectory(const TCHAR *inDirName);

	static DWORD WINAPI ApkSignThread(LPVOID lpVoid);
private:

	tString jdkPath_;
	tString android_sdk_path_;
	tString keystore_;
	tString password_;
	tString aliasname_;
	tString outputPath_;

	tString host_;

	bool thread_cancel_;

	CEvent lock_;
	HANDLE thread_;
};

/////////////////////////////////////////////////////////////////////////////////////
//
class MyBuilderCallback : public IDialogBuilderCallback
{
public:
	CControlUI* CreateControl(LPCTSTR pstrClass, CPaintManagerUI* pManager);
};

/////////////////////////////////////////////////////////////////////////////////////
//
// ListContainerElementUIEx
extern const TCHAR* const kListContainerElementUIExClassName;// = _T("ListContainerElementUIEx");
extern const TCHAR* const kListContainerElementUIExInterfaceName;// = _T("ListContainerElementEx");
class CListContainerElementUIEx : public CListContainerElementUI
{
public:
	CListContainerElementUIEx();

	~CListContainerElementUIEx();

    LPCTSTR GetClass() const;
    LPVOID GetInterface(LPCTSTR pstrName);

	void SetPos(RECT rc);

protected:
};


#endif // MAINFRAME_HPP