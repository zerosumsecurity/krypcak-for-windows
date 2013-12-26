#pragma once
#include "../afx/stdafx.h"
#include "afxcmn.h"
#include "afxwin.h"
#include <process.h>
#include "../SafeNotes/resource.h"
#include "../controls/GradientBox.h"



// CProgressDlg dialog
class CProgressDlg : public CDialog
{
	DECLARE_DYNAMIC(CProgressDlg)

public:
	CProgressDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CProgressDlg();

	
	void SetHeader(CString strText);
	void SetHeader(UINT32 task);
	void SetText(CString strText);
	void SetPosition(DWORD dwPos);
	void SetRange(UINT32 dwMax);
	void SetResult(UINT32 dwRes);
	void SetResult(CString strResult);
	UINT32 GetResult();
	CString GetStringResult();

// Dialog Data
	enum { IDD = IDD_DIALOG_PROGRESS };

public:
	CProgressCtrl m_ctrlProgress;
	CGradientBox m_ctrlHeader;
	CStatic m_ctrlText;
	UINT32 m_dwMaxProgress;
	UINT32 m_dwRes;
	CString m_strResult;
	CString m_strHeader;

protected:
	HICON m_hIcon;
	
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL OnInitDialog();
	afx_msg LONG OnMessageUser( WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

};
