#pragma once
#include "../afx/stdafx.h"
#include "../core/Core.h"
#include "afxcmn.h"
#include "afxwin.h"
#include "../controls/GradientBox.h"

// CRandomDlg dialog

class CRandomDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CRandomDlg)

public:
	CRandomDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRandomDlg();
	void attach(CCore* pCore);

	void UpdateControl();
	BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_GETRANDOM };

	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

protected:
	HICON m_hIcon;
	CCore* m_pCore;
	BOOL m_fDone;
	UINT32 m_dwBytesGathered;
	CProgressCtrl m_ctrlProgress;
	CGradientBox m_ctrlHeader;

};
