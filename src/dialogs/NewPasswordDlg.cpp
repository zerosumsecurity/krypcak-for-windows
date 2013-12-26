// NewPasswordDlg.cpp : implementation file
//

#include "../afx/stdafx.h"
#include "../main/SafeNotes.h"
#include "../dialogs/NewPasswordDlg.h"
#include "afxdialogex.h"


// CNewPasswordDlg dialog

IMPLEMENT_DYNAMIC(CNewPasswordDlg, CDialogEx)

CNewPasswordDlg::CNewPasswordDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CNewPasswordDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_ZSS);

	m_strDate = _T("");
	m_strInfo = _T("");
	m_strDomain = _T("");
	m_strPassword = _T("");

	m_strHeaderText = _T("Set password details");
	m_strTextOnOKButton = _T("Add");
}

CNewPasswordDlg::~CNewPasswordDlg()
{
	m_pCore->ClearString(&m_strPassword);
	m_pCore->ClearString(&m_strInfo);
	m_pCore->ClearString(&m_strDomain);
	m_pCore->ClearString(&m_strDate);
}

void CNewPasswordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_NEWPASSWD, m_ctrlPassword);
	DDX_Control(pDX, IDC_EDIT_LOWER, m_ctrlLowerEdit);
	DDX_Control(pDX, IDC_DATETIMEPICKER1, m_ctrlSetDate);
	DDX_Control(pDX, IDC_STATIC_HEADER, m_ctrlHeader);
	DDX_Control(pDX, IDC_STATIC_LOWER_TEXT, m_ctrlLowerText);
	DDX_Control(pDX, IDC_CHECK_CUSTOM, m_ctrlCheckGenCustom);
	DDX_Control(pDX, IDOK, m_ctrlOKButton);
	DDX_Control(pDX, IDC_EDIT_UPPER, m_ctrlUpperEdit);
	DDX_Control(pDX, IDC_STATIC_UPPER_TEXT, m_ctrlUpperText);
	DDX_Control(pDX, IDC_STATIC_PASSWORD, m_ctrlPasswordText);
	DDX_Control(pDX, IDC_BUTTON_GEN, m_ctrlButtonGenerate);
}


BEGIN_MESSAGE_MAP(CNewPasswordDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_GEN, &CNewPasswordDlg::OnBnClickedButtonGen)
	ON_BN_CLICKED(IDOK, &CNewPasswordDlg::OnBnClickedOk)
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


// CNewPasswordDlg message handlers
BOOL CNewPasswordDlg::OnInitDialog()
{
	//first call the standard constructor
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	// when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//set the window text in the border of the main dialog

	SetWindowText( WINDOW_TEXT );

	m_ctrlHeader.SetWindowTextW( m_strHeaderText );
	m_ctrlHeader.show(ZERSOSUM_DARK,ZERSOSUM_DARK);

	switch (m_dwPasswordType)
	{
		case PSWD_FILE:
			m_ctrlLowerText.SetWindowTextW( _T("File") );
			m_ctrlUpperEdit.ShowWindow(FALSE);
			m_ctrlUpperText.ShowWindow(FALSE);
			m_ctrlHeader.SetWindowTextW( _T("File password") );
			break;
		case PSWD_CONTACT:
			m_ctrlLowerText.SetWindowTextW( _T("Contact") );
			m_ctrlUpperEdit.ShowWindow(FALSE);
			m_ctrlUpperText.ShowWindow(FALSE);
			m_ctrlHeader.SetWindowTextW( _T("Shared password") );
			break;
		case PSWD_ACCOUNT:
			m_ctrlLowerText.SetWindowTextW( _T("User") );
			m_ctrlUpperText.SetWindowTextW( _T("Domain") );
			m_ctrlHeader.SetWindowTextW( _T("Account password") );
			break;
		case PSWD_MESSAGE:
			m_ctrlLowerText.SetWindowTextW( _T("Message id") );
			m_ctrlUpperEdit.ShowWindow(FALSE);
			m_ctrlUpperText.ShowWindow(FALSE);
			m_ctrlHeader.SetWindowTextW( _T("Message password") );
			break;
		default:
			break;
	}

	m_ctrlLowerEdit.SetWindowTextW(m_strDomain);
	m_ctrlPassword.SetWindowTextW(m_strPassword);
	m_ctrlUpperEdit.SetWindowTextW(m_strInfo);

	m_ctrlOKButton.SetWindowTextW( m_strTextOnOKButton );

	if( m_strPassword.GetLength() > 0)
	{
		GotoDlgCtrl(GetDlgItem(IDCANCEL));
	}
	else
	{
		GotoDlgCtrl(GetDlgItem(IDC_BUTTON_GEN));
	}
		 
	return FALSE;
	


}


void CNewPasswordDlg::OnBnClickedButtonGen()
{
	CPasswordDlg dlg;
	CString strPassword;

	if(m_ctrlCheckGenCustom.GetCheck())
	{

		dlg.attach(m_pCore);
		dlg.SetTextOnOKButton( _T("Use") );

		if(IDOK == dlg.DoModal())
		{
			dlg.GetPassword(strPassword);
		}
	}
	else
	{
		m_pCore->GeneratePassword(strPassword, DEFAULT_PASSWORD_LENGTH, DEFAULT_PASSWORD_ALPHABET_MASK);
	}

	m_ctrlPassword.SetWindowTextW(strPassword);

	UpdateData(false);
}

void CNewPasswordDlg::attach( CCore* pCore )
{
	m_pCore = pCore;
}


void CNewPasswordDlg::OnBnClickedOk()
{
	CTime today;
	CString strTmp;
	note_password password;

	if(GDT_VALID == m_ctrlSetDate.GetTime(today))
	{
		m_strDate = today.Format( _T("%Y-%m-%d") );
	}
	else
	{
		m_strDate = _T("-unknown- ");
	}

	
	switch (m_dwPasswordType)
	{
		case PSWD_FILE:
			m_ctrlLowerEdit.GetWindowTextW(m_strDomain);
			m_ctrlPassword.GetWindowTextW(m_strPassword);
			break;
		case PSWD_CONTACT:
			m_ctrlLowerEdit.GetWindowTextW(m_strDomain);
			m_ctrlPassword.GetWindowTextW(m_strPassword);
			break;
		case PSWD_ACCOUNT:
			m_ctrlUpperEdit.GetWindowTextW(m_strDomain);
			m_ctrlPassword.GetWindowTextW(m_strPassword);
			m_ctrlLowerEdit.GetWindowTextW(m_strInfo);
			break;
		case PSWD_MESSAGE:
			m_ctrlLowerEdit.GetWindowTextW(m_strDomain);
			m_ctrlPassword.GetWindowTextW(m_strPassword);
			break;
	}

	if( m_strPassword.GetLength() > MAX_PSWD_SIZE )
	{
		AfxMessageBox( IDS_MAX_PSWD_LENGTH );
		return;
	}

	if( m_strDomain.GetLength() > MAX_PSWD_SIZE )
	{
		AfxMessageBox( _T("These strings can have at most 255 characters.") );
		return;
	}

	if( m_strInfo.GetLength() > MAX_PSWD_SIZE )
	{
		AfxMessageBox( _T("These strings can have at most 255 characters.") );
		return;
	}

	GetPassword(password);

	m_pCore->AddPassword(&password);

	CDialogEx::OnOK();
}

void CNewPasswordDlg::GetPassword( note_password &password )
{
	password.strInfo = m_strInfo;
	password.dwStrInfoByteLength = sizeof(wchar_t)*m_strInfo.GetLength();
	password.strDomain = m_strDomain;
	password.dwStrDomainByteLength = sizeof(wchar_t)*m_strDomain.GetLength();
	password.strPassword = m_strPassword;
	password.dwStrPasswordByteLength = sizeof(wchar_t)*m_strPassword.GetLength();
	password.strDate = m_strDate;
	password.dwStrDateByteLength = sizeof(wchar_t)*m_strDate.GetLength();
	password.dwType = m_dwPasswordType;

	memset(password.iv, 0, SIZE_IV_IN_BYTES);
}

void CNewPasswordDlg::SetPasswordType( UINT32 dwType )
{
	m_dwPasswordType = dwType;
}

void CNewPasswordDlg::SetInitPassword( CString strPassword )
{
	m_strPassword = strPassword;
}

void CNewPasswordDlg::SetInitDomain( CString strDomain )
{
	m_strDomain = strDomain;
}

void CNewPasswordDlg::SetTextOnOKButton( CString strText )
{
	m_strTextOnOKButton = strText;
}

void CNewPasswordDlg::SetHeader( CString strText )
{
	m_strHeaderText = strText;
}



void CNewPasswordDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	ASSERT(m_pCore != NULL);

	m_pCore->MixInMousePoint(&point);

	CDialogEx::OnMouseMove(nFlags, point);
}

void CNewPasswordDlg::SetInitInfo( CString strInfo )
{
	m_strInfo = strInfo;
}

BOOL CNewPasswordDlg::PreTranslateMessage( MSG* pMsg )
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN)
		{
			CWnd* pControl;
			pControl = this->GetFocus();

			if( pControl->IsKindOf(RUNTIME_CLASS(CEdit)) )
			{
				if( 0 == (GetKeyState(VK_CONTROL)&0x8000) )
				{
					return TRUE;
				}
			}
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}