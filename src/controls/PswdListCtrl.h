#if !defined(AFX_PswdListCtrl_H__01DA7421_D70E_4369_9A43_2312221D8CF0__INCLUDED_)
#define AFX_PswdListCtrl_H__01DA7421_D70E_4369_9A43_2312221D8CF0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PswdListCtrl.h : header file
//

#include "../core/Core.h"
#include "../dialogs/PasswordDlg.h"

#define MAX_NUM_COLUMNS			4

/////////////////////////////////////////////////////////////////////////////
// CPswdListCtrl window

class CTabPasswords;

int CALLBACK _sort_func_ascending(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int CALLBACK _sort_func_descending(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

class CPswdListCtrl : public CListCtrl
{
// Construction
public:


	CPswdListCtrl();
	virtual ~CPswdListCtrl();

	void attach(CCore* pCore){m_pCore = pCore;}
	void attachParent(CTabPasswords *pTab);

	void DisplayPasswords();


	int _get_column_content(int nColumn);

	void Sort(int nColumn);
	void _reset_sort_order();

	void SetType(UINT32 dwType){m_dwType = dwType;}
	BOOL PreTranslateMessage(MSG* pMsg);
	BOOL EditSelectedPassword();
	BOOL DeleteSelectedPasswords();
	BOOL CopySelectedPassword();
	BOOL AddPassword();
	BOOL GetSelectedPassword(CString &strPassword);
	void SetUse(UINT32 dwUse);

private:
	CCore*	m_pCore;
	CTabPasswords* m_tabParent;
	UINT32  m_dwType;
	UINT32	m_dwUse;
	UINT8   m_bSortAscending[MAX_NUM_COLUMNS];

	// Generated message map functions
protected:

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PswdListCtrl_H__01DA7421_D70E_4369_9A43_2312221D8CF0__INCLUDED_)
