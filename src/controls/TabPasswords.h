#pragma once
#include "afxcmn.h"
#include "../dialogs/NewPasswordDlg.h"
#include "../controls/PswdListCtrl.h"
#include "afxwin.h"

class CShowContentsDialog;

class CTabPasswords : public CDialogEx
{
	DECLARE_DYNAMIC(CTabPasswords)

public:
	CTabPasswords(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTabPasswords();
	BOOL OnInitDialog();
	void attach(CCore* pCore);
	void attachParent(CShowContentsDialog* pDlg);

	void SetCurrentView(UINT32 dwSelection);
	void SetCurrentUse(UINT32 dwUse);

	//void SetCopyFlag(BOOL flag);

	CString GetPassword();

	BOOL InitShowPasswords();
	BOOL InitShowGeneralNotes();

	void DeleteCurrentList();
	void OnOK();

// Dialog Data
	enum { IDD = IDD_TAB_PWD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	HICON m_hIcon;
	CCore* m_pCore;
	CShowContentsDialog* m_parentDlg;

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_finished;
	CString m_strPassword;
	CString m_strInfo;
	//BOOL m_fFlagCopyPassword;
	CPswdListCtrl m_ctrlContent;
	UINT32 m_dwCurrentView;
	UINT32 m_dwUse;
	afx_msg void OnItemchangedListPasswords(NMHDR*, LRESULT *pResult);

	void ShowAddedPassword(note_password password);
	afx_msg void OnDblclkListPasswords(NMHDR*, LRESULT *pResult);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint point);
	afx_msg void OnPopupEdit();
	afx_msg void OnPopupDelete();
	afx_msg void OnPopupCopy();
	afx_msg void OnPopupAdd();
	afx_msg void OnPopupUse();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnPopupShared();
	afx_msg void OnItemclickListPasswords(NMHDR *pNMHDR, LRESULT *pResult);
};

