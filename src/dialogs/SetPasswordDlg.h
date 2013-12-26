#pragma once
#include "afxwin.h"
#include "../dialogs/PasswordDlg.h"
#include "../core/Core.h"
#include "../controls/GradientBox.h"
//#include "ShowContentsDialog.h"

// CSetPasswordDlg dialog

class CSetPasswordDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSetPasswordDlg)

public:
	CSetPasswordDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSetPasswordDlg();
	CString GetPassword();
	void attach(CCore* pCore);
	void SetConfirm(BOOL flag);
	void SetURL(CString strURL);
	void SetEnableStorePassword(BOOL flag);
	void SetEnableGenerateButton(BOOL flag);
	void SetEnableLoadButton(BOOL flag);
	void SetShowStorePasswordCheck(BOOL flag);
	void SetHeaderText( CString strText);
	BOOL GetStorePasswordFlag();
	void SetAutoCustomFlag();

// Dialog Data
	enum { IDD = IDD_NEW_PASSWORD };

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonGen();
	afx_msg void OnBnClickedButtonLoad();
	afx_msg void OnBnClickedCheckShow();
	afx_msg void OnClickedCheckStore();

	BOOL PreTranslateMessage( MSG* pMsg );

	DECLARE_MESSAGE_MAP()

public:


private:
	//controls
	CEdit m_ctrlPassword;
	CEdit m_ctrlConfirm;
	CButton m_checkStore;
	CButton m_ctrlGenerate;
	CGradientBox m_ctrlHeader;
	CStatic m_ctrlConfirmText;
	CButton m_ctrlCustomGenerate;
	CButton m_ctrlShowPasword;
	CButton m_ctrlButtonLoad;
	CToolTipCtrl* m_pToolTipLoad;
	CToolTipCtrl* m_pToolTipGenerate;
	CToolTipCtrl* m_pToolTipCustom;

	//members
	HICON m_hIcon;
	CCore* m_pCore;

	BOOL m_fStorePasswordFlag;
	BOOL m_fConfirm;
	BOOL m_fModeEncrypt;
	BOOL m_fAutoCustom;
	BOOL m_fEnableGenerateButton;
	BOOL m_fEnableLoadButton;
	BOOL m_fShowStorePasswordCheck;

	CString m_strPassword;
	CString m_strTextOnButton;
	CString m_strURL;
	CString m_strHeaderText;
};
