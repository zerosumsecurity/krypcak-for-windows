
// SafeNotesDlg.h : header file
//

#pragma once
#include "../core/Core.h"
#include "../dialogs/ShowContentsDialog.h"
#include "../dialogs/PasswordDlg.h"
#include "../dialogs/RandomDlg.h"
#include "../dialogs/SetPasswordDlg.h"
#include "../controls/GradientBox.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "../controls/TabPasswords.h"

// CSafeNotesDlg dialog
class CSafeNotesDlg : public CDialogEx
{
// Construction
public:
	CSafeNotesDlg(CWnd* pParent = NULL);	// standard constructor
	void attach(CCore* pCore);
	
// Dialog Data
	enum { IDD = IDD_SAFENOTES_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	void UpdateControl(int nTabFocus = -1);
	void ShowNotes(int nTabIndex);

	void OnClose();
	void OnCancel();

// Implementation
protected:
	CCore* m_pCore;
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnGeneratePassword();
	afx_msg void OnOpenNotebook();
	afx_msg void OnNewNotebook();
	CGradientBox m_ctrlHeader;
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnNotebookChangepassword();
	afx_msg void OnAboutAboutkrypcak();
	CTabCtrl m_ctrlTabPasswords;
	CTabPasswords m_tabPasswords;
	afx_msg void OnFileEncrypt();
	afx_msg void OnSelchangeTabPswds(NMHDR*, LRESULT *pResult);
	afx_msg void OnFileDecrypt();
	void UpdateHeader();
	afx_msg void OnNotebookExport();
	afx_msg void OnMessagesEncrypt();
	afx_msg void OnMessagesDecrypt();

};
