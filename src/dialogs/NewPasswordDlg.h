#pragma once
#include "../afx/stdafx.h"
#include "../dialogs/PasswordDlg.h"
#include "afxwin.h"
#include "afxdtctl.h"
#include "../controls/GradientBox.h"

// CNewPasswordDlg dialog

class CNewPasswordDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CNewPasswordDlg)

public:
	CNewPasswordDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CNewPasswordDlg();

	void attach(CCore* pCore);
	void SetPasswordType(UINT32 dwType);
	void GetPassword(note_password &password);
	
	void SetInitPassword(CString strPassword);
	void SetInitDomain(CString strDomain);
	void SetInitInfo(CString strInfo);

	void SetHeader(CString strText);
	void SetTextOnOKButton(CString strText);

// Dialog Data
	enum { IDD = IDD_DIALOG_ADDPSWD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonGen();
	CCore* m_pCore;
	CEdit m_ctrlPassword;
	afx_msg void OnBnClickedOk();
	CEdit m_ctrlLowerEdit;

	CString m_strInfo;
	CString m_strDomain;
	CString m_strPassword;
	CString m_strDate;
	
	HICON m_hIcon;
	CDateTimeCtrl m_ctrlSetDate;
	CGradientBox m_ctrlHeader;

	UINT32 m_dwPasswordType;
	CStatic m_ctrlLowerText;
	CButton m_ctrlCheckGenCustom;
	CButton m_ctrlOKButton;

	CString m_strHeaderText;
	CString m_strTextOnOKButton;
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	BOOL PreTranslateMessage( MSG* pMsg );
	CEdit m_ctrlUpperEdit;
	CStatic m_ctrlUpperText;
	CStatic m_ctrlPasswordText;
	CButton m_ctrlButtonGenerate;
};
