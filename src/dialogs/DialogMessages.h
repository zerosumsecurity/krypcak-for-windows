#pragma once
#include "afxwin.h"
#include "../controls/GradientBox.h"
#include "../dialogs/PasswordDlg.h"
#include "../dialogs/ShowContentsDialog.h"
#include "../dialogs/SetPasswordDlg.h"

// CDialogMessages dialog

static const enum
{
	MODE_ENC,
	MODE_DEC
};

class CDialogMessages : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogMessages)

public:
	CDialogMessages(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogMessages();
	void attach(CCore *pCore);
	void SetMode(DWORD dwMode);

// Dialog Data
	enum { IDD = IDD_DIALOG_MESSAGES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void Encrypt();
	void Decrypt();


	DECLARE_MESSAGE_MAP()
public:
	HICON m_hIcon;
	CCore* m_pCore;

	CEdit m_ctrlInput;
	CEdit m_ctrlOutput;
	CButton m_ctrlButCrypt;
	afx_msg void OnBnClickedButCrypt();
	CGradientBox m_ctrlHeader;
	UINT32 m_dwMode;
	afx_msg void OnBnClickedButtonCopy();
	afx_msg void OnBnClickedButtonDone();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};
