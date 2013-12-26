#pragma once
#include "afxcmn.h"
#include "../controls/TabPasswords.h"
#include "../core/Core.h"
#include "afxwin.h"
#include "../controls/GradientBox.h"

// CShowContentsDialog dialog

class CShowContentsDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CShowContentsDialog)

public:
	CShowContentsDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CShowContentsDialog();

	void attach(CCore* pCore);
	//void SetTab(UINT32 intTabIndex);
	BOOL OnInitDialog();

	void ShowNotes(int nTabIndex);

	//void ShowPasswords(CPasswordNotesArray* pPasswords);
	void UpdateControl();
	void SetUse(UINT32 dwUse);
	void GetPassword( CString *pstrPassword );
	void SetPassword( CString *pstrPassword );

// Dialog Data
	enum { IDD = IDD_DLG_SHOW_NOTES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	HICON m_hIcon;

	DECLARE_MESSAGE_MAP()
public:
	CCore* m_pCore;
	CTabCtrl m_ctrlTabContents;
	CTabPasswords m_tabPswds;
	UINT32 m_dwTabIndex;
	UINT32 m_dwUse;
	CString m_strPassword;
	afx_msg void OnSelchangeTabContent(NMHDR*, LRESULT *pResult);
	CGradientBox m_ctrlHeader;
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};
