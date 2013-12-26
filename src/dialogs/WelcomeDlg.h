#pragma once

#include "../SafeNotes/resource.h"
#include "afxwin.h"
#include "../controls/GradientBox.h"

// CWelcomeDlg dialog

class CCore;
class CWelcomeDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CWelcomeDlg)

	CWelcomeDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWelcomeDlg();

	virtual BOOL OnInitDialog();
	void SetLastUsedNotebookURL(CString strURL);
	CString GetPassword();
	void attach(CCore* pCore){m_pCore = pCore;};
	void UpdateControl();

// Dialog Data
	enum { IDD = IDD_DIALOG_WELCOME };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()

private:
	afx_msg void OnBnClickedCheckShowpswdWelcome();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonChange();

	void OnMouseMove(UINT nFlags, CPoint point);

private:
	CCore* m_pCore;
	HICON m_hIcon;
	CString m_strLastUsedNotebookURL;
	CString m_strPassword;
	CEdit m_ctrlPassword;
	CStatic m_ctrlNotebookText;
	CGradientBox m_ctrlHeader;
	CButton m_ctrlCheckShowPassword;
};
