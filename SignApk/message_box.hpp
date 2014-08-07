#ifndef MESSAGE_BOX_HPP
#define MESSAGE_BOX_HPP

//MB_ABORTRETRYIGNORE The message box contains three push buttons: Abort, Retry, and Ignore. 
//MB_OK The message box contains one push button: OK. This is the default. 
//MB_OKCANCEL The message box contains two push buttons: OK and Cancel. 
//MB_RETRYCANCEL The message box contains two push buttons: Retry and Cancel. 
//MB_YESNO The message box contains two push buttons: Yes and No. 
//MB_YESNOCANCEL The message box contains three push buttons: Yes, No, and Cancel. 
int DUIMessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType = MB_OK);

class WindowImplBase;
class CMessageBox : public WindowImplBase
{
	friend int DUIMessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType);
protected:

	CMessageBox(HWND hParent, const tString& caption, const tString& message, UINT uType);
	~CMessageBox();

protected:

	LPCTSTR GetWindowClassName() const;	

	virtual void OnFinalMessage(HWND hWnd);

	virtual void Init();

	virtual LRESULT ResponseDefaultKeyEvent(WPARAM wParam);

	virtual tString GetSkinFile();

	virtual tString GetSkinFolder();

	virtual LONG GetStyle();

	virtual CControlUI* CreateControl(LPCTSTR pstrClass);

	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

protected:
	void Notify(TNotifyUI& msg);

	void OnPrepare(TNotifyUI& msg);

private:
	HWND parent_;
	tString caption_;
	tString message_;
	UINT type_;
};

#endif // MESSAGE_BOX_HPP