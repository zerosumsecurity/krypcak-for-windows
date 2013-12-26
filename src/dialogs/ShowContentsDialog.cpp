// ShowContentsDialog.cpp : implementation file
//

#include "../afx/stdafx.h"
#include "../main/SafeNotes.h"
#include "../dialogs/ShowContentsDialog.h"
#include "afxdialogex.h"


// CShowContentsDialog dialog

IMPLEMENT_DYNAMIC(CShowContentsDialog, CDialogEx)

CShowContentsDialog::CShowContentsDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(CShowContentsDialog::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_ZSS);
	m_tabPswds.attachParent(this);
	m_tabPswds.SetCurrentUse(m_dwUse);
}

CShowContentsDialog::~CShowContentsDialog()
{
}

void CShowContentsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB_CONTENT, m_ctrlTabContents);
	DDX_Control(pDX, IDC_STATIC_HEADER, m_ctrlHeader);
}


BEGIN_MESSAGE_MAP(CShowContentsDialog, CDialogEx)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_CONTENT, &CShowContentsDialog::OnSelchangeTabContent)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CShowContentsDialog::OnBnClickedButtonClose)
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


// CShowContentsDialog message handlers

// CSafeNotesDlg message handlers

BOOL CShowContentsDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	SetWindowTextW( WINDOW_TEXT );

	//strip the url of the notebook to just the filename
	CString strHeader = ::PathFindFileNameW( m_pCore->GetLoadedNotebookURL() );

	//if the filename is too long, just use "Notebook" as caption
	if(strHeader.GetLength() > MAX_LENGTH_DLG_HEADER)
	{
		m_ctrlHeader.SetWindowTextW( _T("Notebook") );
	}
	else
	{
		m_ctrlHeader.SetWindowTextW( strHeader );
	}
	m_ctrlHeader.show(ZERSOSUM_DARK, ZERSOSUM_DARK);


	CTabCtrl* pTabCtrl = (CTabCtrl*)GetDlgItem(IDC_TAB_CONTENT);
 
	m_tabPswds.Create(IDD_TAB_PWD, pTabCtrl);
 

	TCITEM item;
	item.mask    = TCIF_TEXT | TCIF_PARAM;
	item.lParam  = (LPARAM)& m_tabPswds;
	item.pszText = _T("Files");
	pTabCtrl->InsertItem(0, &item);

	item.mask    = TCIF_TEXT | TCIF_PARAM;
	item.lParam  = (LPARAM)& m_tabPswds;
	item.pszText = _T("Messages");
	pTabCtrl->InsertItem(1, &item);

	item.mask    = TCIF_TEXT | TCIF_PARAM;
	item.lParam  = (LPARAM)& m_tabPswds;
	item.pszText = _T("Shared");
	pTabCtrl->InsertItem(2, &item);

	item.mask    = TCIF_TEXT | TCIF_PARAM;
	item.lParam  = (LPARAM)& m_tabPswds;
	item.pszText = _T("Accounts");
	pTabCtrl->InsertItem(3, &item);

	CRect rcItem;   
	pTabCtrl->GetItemRect(0, &rcItem);

	m_tabPswds.SetWindowPos(NULL, rcItem.left, rcItem.bottom + 1, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
	m_tabPswds.ShowWindow(SW_SHOW);

	pTabCtrl->SetCurSel(PSWD_FILE);

	ShowNotes(PSWD_FILE);

	return TRUE; 
}

void CShowContentsDialog::attach( CCore* pCore )
{
	ASSERT(pCore != NULL);

	m_pCore = pCore;

	//pass on the pointer to out tab control
	m_tabPswds.attach(m_pCore);
}

void CShowContentsDialog::UpdateControl()
{
	UpdateData(true);

	//if our tab control indicated we are done, leave 
	if(m_tabPswds.m_finished)
	{
		OnOK();
	}

	//set the correct tab
	m_tabPswds.SetCurrentView(m_ctrlTabContents.GetCurFocus());

	//load the passwords associated with the current set type
	ShowNotes(m_ctrlTabContents.GetCurFocus());

	//refresh the dialog
	m_tabPswds.ShowWindow(SW_SHOW);

	UpdateData(false);
}


void CShowContentsDialog::ShowNotes(int nTabIndex)
{
	//set the current type of password at the tab control
	m_tabPswds.m_ctrlContent.SetType(nTabIndex);
	
	//(re-)initialize the tab control 
	m_tabPswds.InitShowPasswords();

	//show the passwords on screen
	m_tabPswds.m_ctrlContent.DisplayPasswords();
}

void CShowContentsDialog::OnSelchangeTabContent(NMHDR*, LRESULT *pResult)
{
	//update the dialog to display the current type of passwords
	UpdateControl();

	*pResult = 0;
}

void CShowContentsDialog::OnBnClickedButtonClose()
{
	OnOK();
}

void CShowContentsDialog::SetUse( UINT32 dwUse )
{
	m_dwUse = dwUse;
	m_tabPswds.SetCurrentUse( m_dwUse );
}

void CShowContentsDialog::GetPassword(CString *pstrPassword)
{
	*pstrPassword = m_strPassword;
}

void CShowContentsDialog::SetPassword( CString *pstrPassword )
{
	m_strPassword = *pstrPassword;
}


void CShowContentsDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	ASSERT(m_pCore != NULL);

	m_pCore->MixInMousePoint(&point);

	CDialogEx::OnMouseMove(nFlags, point);
}
