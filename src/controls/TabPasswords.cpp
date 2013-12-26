// TabPasswords.cpp : implementation file
//

#include "../afx/stdafx.h"
#include "../main/SafeNotes.h"
#include "../controls/TabPasswords.h"
#include "../dialogs/ShowContentsDialog.h"
#include "afxdialogex.h"


// CTabPasswords dialog

IMPLEMENT_DYNAMIC(CTabPasswords, CDialogEx)

CTabPasswords::CTabPasswords(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTabPasswords::IDD, pParent)
{
	//SetCopyFlag(TRUE);
	SetCurrentUse(PSWD_VIEW);
	m_ctrlContent.SetUse(m_dwUse);
	m_ctrlContent.attachParent(this);
}

CTabPasswords::~CTabPasswords()
{
	m_pCore->ClearString( &m_strPassword );
}

void CTabPasswords::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PASSWORDS, m_ctrlContent);
}


BEGIN_MESSAGE_MAP(CTabPasswords, CDialogEx)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_PASSWORDS, &CTabPasswords::OnItemchangedListPasswords)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_PASSWORDS, &CTabPasswords::OnDblclkListPasswords)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_POPUP_EDIT, &CTabPasswords::OnPopupEdit)
	ON_COMMAND(ID_POPUP_DELETE, &CTabPasswords::OnPopupDelete)
	ON_COMMAND(ID_POPUP_COPY, &CTabPasswords::OnPopupCopy)
	ON_COMMAND(ID_POPUP_ADD, &CTabPasswords::OnPopupAdd)
	ON_COMMAND(ID_POPUP_USE, &CTabPasswords::OnPopupUse)
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_POPUP_SHARED, &CTabPasswords::OnPopupShared)
	ON_NOTIFY(HDN_ITEMCLICK, 0, &CTabPasswords::OnItemclickListPasswords)
END_MESSAGE_MAP()


// CTabPasswords message handlers
BOOL CTabPasswords::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	DWORD dwStyle;
	int	nScrollwidth = 21;
	int nColWidth;
	int nColRest;
	int nWidth;
	int nCols;
	CRect	rect;


	m_ctrlContent.GetWindowRect(&rect);
	nWidth = rect.Width();
	nCols = 36;
	nColWidth = (nWidth - nScrollwidth)/ nCols;
	nColRest = (nWidth - nScrollwidth)% nCols;

	m_ctrlContent.InsertColumn(0, _T("Date"), LVCFMT_LEFT, nColWidth * 6);
	m_ctrlContent.InsertColumn(1, _T("File"), LVCFMT_LEFT, nColWidth * 16);
	m_ctrlContent.InsertColumn(2, _T("Password"), LVCFMT_LEFT, (nColWidth * 14) + nColRest);	

	dwStyle = m_ctrlContent.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;
	dwStyle |= LVS_EX_GRIDLINES;
	m_ctrlContent.SetExtendedStyle( dwStyle);	

	m_dwCurrentView = PSWD_FILE;

	m_ctrlContent.SetType(m_dwCurrentView);
	m_ctrlContent.attach(m_pCore);

	m_finished = FALSE;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

BOOL CTabPasswords::InitShowPasswords()
{
	CDialogEx::OnInitDialog();

	DeleteCurrentList();
	
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	DWORD dwStyle;
	int	nScrollwidth = 21;
	int nColWidth;
	int nColRest;
	int nWidth;
	int nCols;
	CRect	rect;

	m_ctrlContent.GetWindowRect(&rect);
	nWidth = rect.Width();
	nCols = 36;
	nColWidth = (nWidth - nScrollwidth)/ nCols;
	nColRest = (nWidth - nScrollwidth)% nCols;

	m_ctrlContent.InsertColumn(0, _T("Date"), LVCFMT_LEFT, nColWidth * 6);
	switch (m_dwCurrentView)
	{
		case PSWD_FILE:
			m_ctrlContent.InsertColumn(1, _T("File"), LVCFMT_LEFT, nColWidth * 15);
			m_ctrlContent.InsertColumn(2, _T("Password"), LVCFMT_LEFT, (nColWidth * 15) + nColRest);	
			break;
		case PSWD_CONTACT:
			m_ctrlContent.InsertColumn(1, _T("Contact"), LVCFMT_LEFT, nColWidth * 15);
			m_ctrlContent.InsertColumn(2, _T("Shared password"), LVCFMT_LEFT, (nColWidth * 15) + nColRest);	
			break;
		case PSWD_ACCOUNT:
			m_ctrlContent.InsertColumn(1, _T("Domain"), LVCFMT_LEFT, nColWidth * 10);
			m_ctrlContent.InsertColumn(2, _T("Username"), LVCFMT_LEFT, nColWidth * 10);
			m_ctrlContent.InsertColumn(3, _T("Password"), LVCFMT_LEFT, (nColWidth * 10) + nColRest);	
			break;
		case PSWD_MESSAGE:
			m_ctrlContent.InsertColumn(1, _T("Message id"), LVCFMT_LEFT, nColWidth * 10);
			m_ctrlContent.InsertColumn(2, _T("Password"), LVCFMT_LEFT, (nColWidth * 20) + nColRest);	
			break;
		default:
			m_ctrlContent.InsertColumn(1, _T("Information"), LVCFMT_LEFT, nColWidth * 30);
			break;
	}
	
	
	dwStyle = m_ctrlContent.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;
	dwStyle |= LVS_EX_GRIDLINES;
	m_ctrlContent.SetExtendedStyle( dwStyle);	

	return TRUE;  // return TRUE  unless you set the focus to a control
}

/*
	delete all columns 
*/
void CTabPasswords::DeleteCurrentList()
{
	int i=0;

	//get the header
	CHeaderCtrl* pHeader = (CHeaderCtrl*)m_ctrlContent.GetDlgItem(0);

	//get the number of columns
	int n = pHeader->GetItemCount();
	
	//delete all columns
	for(i=n-1; i>=0; i--)
	{
		m_ctrlContent.DeleteColumn(0);
	}

}

/*
	set the current view
*/
void CTabPasswords::SetCurrentView( UINT32 dwSelection )
{
	m_dwCurrentView = dwSelection;
}

/*
	attach reference to the main CCore object
*/
void CTabPasswords::attach( CCore* pCore )
{
	ASSERT(pCore != NULL);

	m_pCore = pCore;
}

/*
	update when something changed
*/
void CTabPasswords::OnItemchangedListPasswords(NMHDR*, LRESULT *pResult)
{
	UpdateData(true);

	*pResult = 0;
}

/*
*/
//void CTabPasswords::SetCopyFlag( BOOL flag )
//{
//	m_fFlagCopyPassword = flag;
//}


CString CTabPasswords::GetPassword()
{
	return m_strPassword;
}


void CTabPasswords::OnDblclkListPasswords(NMHDR*, LRESULT *pResult)
{
	//LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	if(PSWD_VIEW == m_dwUse)
	{
		if(m_ctrlContent.GetSelectedCount() > 0)
		{
			OnPopupEdit();
		}
		else
		{
			OnPopupAdd();
		}
		
	}
	if(PSWD_SELECT == m_dwUse)
	{
		OnPopupUse();
	}
}


void CTabPasswords::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CMenu mnuPopup;

	if(PSWD_VIEW == m_dwUse)
	{
		mnuPopup.LoadMenu(IDR_MENU_PSWD);

		CMenu *mnuPopupMenu = mnuPopup.GetSubMenu(0);
		ASSERT(mnuPopupMenu);


		if( 0 == m_ctrlContent.GetSelectedCount() )
		{
			mnuPopupMenu->EnableMenuItem(ID_POPUP_DELETE, MF_GRAYED);
		}
		if( 1 != m_ctrlContent.GetSelectedCount() )
		{
			mnuPopupMenu->EnableMenuItem(ID_POPUP_COPY, MF_GRAYED);
			mnuPopupMenu->EnableMenuItem(ID_POPUP_EDIT, MF_GRAYED);
			mnuPopupMenu->EnableMenuItem(ID_POPUP_SHARED, MF_GRAYED);
		}
		if( !m_pCore->IsNotebookLoaded() )
		{
			mnuPopupMenu->EnableMenuItem(ID_POPUP_ADD, MF_GRAYED);
		}

		mnuPopupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	}

	if(PSWD_SELECT == m_dwUse)
	{
		mnuPopup.LoadMenu(IDR_MENU_USE);

		CMenu *mnuPopupMenu = mnuPopup.GetSubMenu(0);
		ASSERT(mnuPopupMenu);

		if( 1 != m_ctrlContent.GetSelectedCount() )
		{
			mnuPopupMenu->EnableMenuItem(ID_POPUP_USE, MF_GRAYED);
		}

		mnuPopupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	}

}

void CTabPasswords::OnPopupEdit()
{
	m_ctrlContent.EditSelectedPassword();
}

void CTabPasswords::OnPopupDelete()
{
	m_ctrlContent.DeleteSelectedPasswords();
}


void CTabPasswords::OnPopupCopy()
{
	m_ctrlContent.CopySelectedPassword();
}


void CTabPasswords::OnPopupAdd()
{
	m_ctrlContent.AddPassword();
}

void CTabPasswords::OnPopupUse()
{
	m_ctrlContent.GetSelectedPassword( m_strPassword );
	m_parentDlg->SetPassword( &m_strPassword );
	m_parentDlg->OnBnClickedButtonClose();
}

void CTabPasswords::SetCurrentUse( UINT32 dwUse )
{
	m_dwUse = dwUse;
}

void CTabPasswords::OnOK()
{
	return;
}

void CTabPasswords::attachParent( CShowContentsDialog* pDlg )
{
	ASSERT(pDlg != NULL);

	m_parentDlg = pDlg;
}


void CTabPasswords::OnMouseMove(UINT nFlags, CPoint point)
{
	ASSERT(m_pCore != NULL);

	m_pCore->MixInMousePoint(&point);

	CDialogEx::OnMouseMove(nFlags, point);
}


void CTabPasswords::OnPopupShared()
{
	CNewPasswordDlg dlg;
	m_ctrlContent.GetSelectedPassword( m_strPassword );

	dlg.attach(m_pCore);
	dlg.SetInitPassword(m_strPassword);	
	dlg.SetPasswordType(PSWD_CONTACT);
	dlg.SetHeader(_T("Set password sharing details"));
	dlg.SetTextOnOKButton(_T("Store as shared pasword"));

	dlg.DoModal();

	m_ctrlContent.DisplayPasswords();
}


void CTabPasswords::OnItemclickListPasswords(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	// TODO: Add your control notification handler code here

	m_ctrlContent.Sort(phdr->iItem);

	*pResult = 0;
}
