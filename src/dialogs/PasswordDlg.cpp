// PasswordDlg.cpp : implementation file
//

#include "../afx/stdafx.h"
#include "../main/SafeNotes.h"
#include "../dialogs/PasswordDlg.h"
#include "afxdialogex.h"


// CPasswordDlg dialog

IMPLEMENT_DYNAMIC(CPasswordDlg, CDialogEx)

CPasswordDlg::CPasswordDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPasswordDlg::IDD, pParent)
	, m_checkPhrase(0)
	, m_checkPassword(1)
	, m_nLengthPassword(DEFAULT_PASSWORD_LENGTH)
	, m_nLengthPhrase(4)
	, m_checkMay(1)
	, m_checkShould(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_ZSS);
}

CPasswordDlg::~CPasswordDlg()
{
	m_pCore->ClearString(&m_strPassword);
}

void CPasswordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_NUM, m_ctrlCheckNum);
	DDX_Control(pDX, IDC_CHECK_LOW, m_ctrlCheckLow);
	DDX_Control(pDX, IDC_CHECK_UP, m_ctrlCheckUp);
	DDX_Control(pDX, IDC_CHECK_SPEC, m_ctrlCheckSpec);
	DDX_Control(pDX, IDC_EDIT_PSWD, m_ctrlPassword);
	DDX_Control(pDX, IDC_COMBO_LANG, m_ctrlLanguage);
	DDX_Text(pDX, IDC_EDIT_LENGTH_PSWD, m_nLengthPassword);
	DDX_Text(pDX, IDC_EDIT_LENGTH_PHRASE, m_nLengthPhrase);
	DDX_Control(pDX, IDC_EDIT_LENGTH_PSWD, m_ctrlLengthPassword);
	DDX_Control(pDX, IDC_EDIT_LENGTH_PHRASE, m_ctrlLengthPhrase);
	DDX_Radio(pDX, IDC_RADIO_PSWD, m_checkPassword);
	DDX_Radio(pDX, IDC_RADIO_MAY, m_checkMay);
	DDX_Control(pDX, IDC_RADIO_PSWD, m_ctrlPassWordPhrase);
	DDX_Control(pDX, IDC_RADIO_MAY, m_ctrlMay);
	DDX_Control(pDX, IDCANCEL, m_ctrlShould);
	DDX_Control(pDX, IDC_GRADIENT, m_ctrlHeader);
	DDX_Control(pDX, IDOK, m_ctrlButtonOK);
}


BEGIN_MESSAGE_MAP(CPasswordDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUT_GEN, &CPasswordDlg::OnBnClickedButGen)
	ON_BN_CLICKED(IDC_RADIO_PSWD, &CPasswordDlg::OnClickedRadioPswd)
	ON_BN_CLICKED(IDC_RADIO_PHRASE, &CPasswordDlg::OnClickedRadioPhrase)
	ON_BN_CLICKED(IDOK, &CPasswordDlg::OnBnClickedOk)
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


// CPasswordDlg message handlers


void CPasswordDlg::OnBnClickedButGen()
{
	UpdateData(true);

	//update window control
	UpdateControl();

	//check if no funny business
	if(!ValidateInputs())
	{
		return;
	}

	//check if we should make a password or passphrase
	if(m_checkPassword == 0)
	{
		//make the password
		m_pCore->GeneratePassword(m_strPassword, (UINT32)m_nLengthPassword, m_bPswdAlphabets);
	}
	else
	{
		//make the passphrase
		CString strLang;
		m_ctrlLanguage.GetWindowTextW(strLang);

		//in dutch
		if( strLang == _T("Dutch") )
		{
			m_pCore->GeneratePassphrase(m_strPassword, (UINT32)m_nLengthPhrase, DUTCH);
		}
		//in english
		if( strLang == _T("English (common)") )
		{
			m_pCore->GeneratePassphrase(m_strPassword, (UINT32)m_nLengthPhrase, ENGLISH);
		}

		//RFC 2289 words
		if( strLang == _T("English (RFC 2289)") )
		{
			m_pCore->GeneratePassphrase(m_strPassword, (UINT32)m_nLengthPhrase, RFC2289);
		}
	}
	
	//update the control
	m_ctrlPassword.SetWindowTextW(m_strPassword);

	//pass focus to the copy button
	GotoDlgCtrl(GetDlgItem(IDOK));

	//update the dialog
	UpdateData(false);
}

BOOL CPasswordDlg::OnInitDialog()
{
	//first call the standard constructor
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	// when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//set the window text in the border of the main dialog
	CString strMsg;
	strMsg.Format( WINDOW_TEXT );

	SetWindowText(strMsg);

	CheckRadioButton(IDC_RADIO_PSWD, IDC_RADIO_PHRASE, IDC_RADIO_PSWD);
	CheckRadioButton(IDC_RADIO_MAY, IDC_RADIO_SHOULD, IDC_RADIO_SHOULD);

	m_ctrlCheckNum.SetCheck( DEFAULT_PASSWORD_ALPHABET_MASK&1);
	m_ctrlCheckLow.SetCheck( (DEFAULT_PASSWORD_ALPHABET_MASK>>1)&1 );
	m_ctrlCheckUp.SetCheck( (DEFAULT_PASSWORD_ALPHABET_MASK>>2)&1 );

	
	m_ctrlLanguage.AddString( _T("Dutch") );
	m_ctrlLanguage.AddString( _T("English (common)") );
	m_ctrlLanguage.AddString( _T("English (RFC 2289)") );

	m_ctrlLanguage.SetCurSel(1);
	
	m_ctrlHeader.SetWindowTextW( _T("Generate password") );
	m_ctrlHeader.show(ZERSOSUM_DARK, ZERSOSUM_DARK);

	m_ctrlButtonOK.SetWindowTextW( m_strTextOnOKButton );

	

	UpdateControl();
	
	return TRUE;
}

void CPasswordDlg::UpdateControl()
{
	UpdateData(true);

	m_bPswdAlphabets = 0;

	if( m_ctrlCheckNum.GetCheck()  ) m_bPswdAlphabets |= 0x01;
	if( m_ctrlCheckLow.GetCheck()  ) m_bPswdAlphabets |= 0x02;
	if( m_ctrlCheckUp.GetCheck()   ) m_bPswdAlphabets |= 0x04;
	if( m_ctrlCheckSpec.GetCheck() ) m_bPswdAlphabets |= 0x08;
	if( m_checkMay == 1)			 m_bPswdAlphabets |= 0x80;



	///give the PRNG a kick
	m_pCore->UpdatePRNG();

	UpdateData(false);
}

void CPasswordDlg::attach( CCore *pCore )
{
	m_pCore = pCore;
}

void CPasswordDlg::OnClickedRadioPswd()
{
	UpdateControl();
}


void CPasswordDlg::OnClickedRadioPhrase()
{
	UpdateControl();
}

void CPasswordDlg::OnBnClickedOk()
{
	m_ctrlPassword.GetWindowTextW(m_strPassword);

	if( m_strPassword.GetLength() > MAX_PSWD_SIZE)
	{
		AfxMessageBox( IDS_MAX_PSWD_LENGTH );
		return;
	}

	CDialogEx::OnOK();
}

void CPasswordDlg::GetPassword( CString &strPassword )
{
	strPassword = m_strPassword;
}

bool CPasswordDlg::ValidateInputs()
{
	CString strMsg;

	if(m_checkPassword == 0)
	{	

		if(1 > m_nLengthPassword)
		{
			AfxMessageBox( IDS_MIN_PSWD_LENGTH );
			return false;
		}

		int num_alphabets = 0;
		if( m_ctrlCheckNum.GetCheck()  ) num_alphabets++;
		if( m_ctrlCheckLow.GetCheck()  ) num_alphabets++;
		if( m_ctrlCheckUp.GetCheck()   ) num_alphabets++;
		if( m_ctrlCheckSpec.GetCheck() ) num_alphabets++;

		if(0 == num_alphabets)
		{
			AfxMessageBox( IDS_MIN_CHAR_SET );
			return false;
		}
		if(num_alphabets > m_nLengthPassword)
		{
			AfxMessageBox( IDS_MIN_CHAR_SET_PSWD_LENTH );
			return false;
		}
		if(m_nLengthPassword > MAX_PSWD_SIZE)
		{
			AfxMessageBox( IDS_MAX_PSWD_LENGTH );
			return false;
		}
	}

	if(m_checkPassword == 1)
	{
		if(1 > m_nLengthPassword)
		{
			AfxMessageBox( IDS_MIN_PSWD_LENGTH );
			return false;
		}
		if(m_nLengthPhrase > MAX_PHRASE_SIZE)
		{
			AfxMessageBox( IDS_MAX_PHRASE_SIZE );
			return false;
		}

		CString strLang;
		m_ctrlLanguage.GetWindowTextW(strLang);

		if( strLang != _T("Dutch") && strLang != _T("English (common)") && strLang != _T("English (RFC 2289)") )
		{
			AfxMessageBox( IDS_UNKNOWN_LANG );
			return false;
		}
	}

	return true;
}


void CPasswordDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	ASSERT(m_pCore != NULL);

	m_pCore->MixInMousePoint(&point);

	CDialogEx::OnMouseMove(nFlags, point);
}

void CPasswordDlg::SetTextOnOKButton( CString strText )
{
	m_strTextOnOKButton = strText;
}
