// WelcomeDlg.cpp : implementation file
//

#include "../afx/stdafx.h"
#include "../main/SafeNotes.h"
#include "../dialogs/WelcomeDlg.h"
#include "afxdialogex.h"


// CWelcomeDlg dialog

IMPLEMENT_DYNAMIC(CWelcomeDlg, CDialogEx)

CWelcomeDlg::CWelcomeDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWelcomeDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_ZSS);
}

CWelcomeDlg::~CWelcomeDlg()
{
	ASSERT(m_pCore != NULL);

	m_pCore->ClearString(&m_strPassword);
}

void CWelcomeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PSWD, m_ctrlPassword);
	DDX_Control(pDX, IDC_STATIC_NOTEBOOK, m_ctrlNotebookText);
	DDX_Control(pDX, IDC_STATIC_WELCOME_HEADER, m_ctrlHeader);
	DDX_Control(pDX, IDC_CHECK_SHOWPSWD_WELCOME, m_ctrlCheckShowPassword);
}


BEGIN_MESSAGE_MAP(CWelcomeDlg, CDialogEx)
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDOK, &CWelcomeDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_CHANGE, &CWelcomeDlg::OnBnClickedButtonChange)
	ON_BN_CLICKED(IDC_CHECK_SHOWPSWD_WELCOME, &CWelcomeDlg::OnBnClickedCheckShowpswdWelcome)
	ON_BN_CLICKED(IDCANCEL, &CWelcomeDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

void CWelcomeDlg::SetLastUsedNotebookURL( CString strURL )
{
	m_strLastUsedNotebookURL = strURL;
}

void CWelcomeDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	ASSERT(m_pCore != NULL);

	m_pCore->MixInMousePoint(&point);

	CDialogEx::OnMouseMove(nFlags, point);
}

// CWelcomeDlg message handlers
BOOL CWelcomeDlg::OnInitDialog()
{
	CString strText;

	//first call the standard constructor
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	// when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//set the window text in the border of the main dialog

	SetWindowText( WINDOW_TEXT );

	m_ctrlHeader.SetWindowTextW( _T("Welcome") );
	m_ctrlHeader.show(ZERSOSUM_DARK, ZERSOSUM_DARK);

	strText.Format( _T("Please supply the password for %s"), m_strLastUsedNotebookURL );

	m_ctrlNotebookText.SetWindowTextW(strText);

	return TRUE;
}

CString CWelcomeDlg::GetPassword()
{
	return m_strPassword;
}


void CWelcomeDlg::OnBnClickedOk()
{
	ASSERT(m_pCore != NULL);

	//give the PRNG a kick
	m_pCore->UpdatePRNG();

	//transfer the password as entered to the password member
	m_ctrlPassword.GetWindowTextW(m_strPassword);
	
	CDialogEx::OnOK();
}


void CWelcomeDlg::OnBnClickedButtonChange()
{
	ASSERT(m_pCore != NULL);

	//give the PRNG a kick
	m_pCore->UpdatePRNG();

	CString strURL;

	if(FALSE == m_pCore->SelectNotebookURL(strURL) )
	{
		return;
	}
	else
	{
		m_strLastUsedNotebookURL = strURL;
		UpdateControl();
	}
}

void CWelcomeDlg::UpdateControl()
{
	CString strText;
	UpdateData(true);

	//give the PRNG a kick
	m_pCore->UpdatePRNG();

	//update the text on the dialog with the correct url 
	strText.Format( _T("Please supply the password for %s"), m_strLastUsedNotebookURL );
	m_ctrlNotebookText.SetWindowTextW(strText);

	UpdateData(false);
}


void CWelcomeDlg::OnBnClickedCheckShowpswdWelcome()
{
	UpdateData(true);

	//if the show password box has been ticked, show the password
	if( m_ctrlCheckShowPassword.GetCheck() )
	{
		m_ctrlPassword.SetPasswordChar(0);	
	}
	//if the show password box has not been ticked, hide the password
	else
	{
		m_ctrlPassword.SetPasswordChar( HIDDEN_PSWD_CHAR );
	}

	m_ctrlPassword.RedrawWindow();

	UpdateData(true);
}


void CWelcomeDlg::OnBnClickedCancel()
{
	//skip loading a notebook
	CDialogEx::OnCancel();
}
