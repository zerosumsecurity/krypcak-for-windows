#pragma once
#include "../afx/stdafx.h"
#include "afxwin.h"
#include "../core/Core.h"
#include "../controls/GradientBox.h"
#include "../dialogs/RandomDlg.h"

// CPasswordDlg dialog

class CPasswordDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPasswordDlg)

public:
	CPasswordDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPasswordDlg();

	void attach(CCore *pCore);
	void GetPassword(CString &strPassword);
	void SetTextOnOKButton(CString strText);

// Dialog Data
	enum { IDD = IDD_DIALG_GEN_PSWD };

protected:
	void UpdateControl();
	bool ValidateInputs();
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support


	DECLARE_MESSAGE_MAP()

public:
	CButton m_ctrlCheckNum;
	CButton m_ctrlCheckLow;
	CButton m_ctrlCheckUp;
	CButton m_ctrlCheckSpec;
	CEdit m_ctrlPassword;
	CComboBox m_ctrlLanguage;
	int m_checkPhrase;
	int m_checkPassword;

private:
	HICON m_hIcon;
	CCore* m_pCore;
	CString m_strPassword;
	UINT8 m_bPswdAlphabets;
	UINT8 m_nLength;
	UINT8 m_bLanguage;
	CString m_strTextOnOKButton;

public:
	afx_msg void OnBnClickedButGen();
	int m_nLengthPassword;
	int m_nLengthPhrase;
	CEdit m_ctrlLengthPassword;
	CEdit m_ctrlLengthPhrase;
	CButton m_ctrlPassWordPhrase;
	afx_msg void OnClickedRadioPswd();
	afx_msg void OnClickedRadioPhrase();
	afx_msg void OnBnClickedOk();
	int m_checkMay;
	CButton m_ctrlMay;
	CButton m_ctrlShould;
	CGradientBox m_ctrlHeader;
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	CButton m_ctrlButtonOK;
	int m_checkShould;
};
