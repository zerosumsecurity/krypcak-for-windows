// DialogMessages.cpp : implementation file
//

#include "../afx/stdafx.h"
#include "../main/SafeNotes.h"
#include "../dialogs/DialogMessages.h"
#include "afxdialogex.h"


// CDialogMessages dialog

IMPLEMENT_DYNAMIC(CDialogMessages, CDialogEx)

CDialogMessages::CDialogMessages(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDialogMessages::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_ZSS);
	m_dwMode = MODE_ENC;
}

CDialogMessages::~CDialogMessages()
{
}

void CDialogMessages::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_INPUT, m_ctrlInput);
	DDX_Control(pDX, IDC_EDIT_OUTPUT, m_ctrlOutput);
	DDX_Control(pDX, IDC_BUT_CRYPT, m_ctrlButCrypt);
	DDX_Control(pDX, IDC_STATIC_HEADER, m_ctrlHeader);
}


BEGIN_MESSAGE_MAP(CDialogMessages, CDialogEx)
	ON_BN_CLICKED(IDC_BUT_CRYPT, &CDialogMessages::OnBnClickedButCrypt)
	ON_BN_CLICKED(IDC_BUTTON_COPY, &CDialogMessages::OnBnClickedButtonCopy)
	ON_BN_CLICKED(IDC_BUTTON_DONE, &CDialogMessages::OnBnClickedButtonDone)
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

void CDialogMessages::attach( CCore *pCore )
{
	m_pCore = pCore;
}


// CDialogMessages message handlers


void CDialogMessages::Encrypt()
{
	ASSERT(m_pCore != NULL);
	UINT32 dwRes;

	CString strInput, strOutput, strPassword;
	BOOL fStore;
	m_ctrlInput.GetWindowTextW(strInput);
	int n = strInput.GetLength();

	if(0==n)
	{
		m_pCore->ReportError(ERROR_NO_PLAIN_MESSAGE);
		m_ctrlOutput.SetWindowTextW( _T("") );
		GotoDlgCtrl(GetDlgItem(IDC_BUTTON_DONE)); 
		return;
	}
	if( n > MAX_NUM_CHARS_PLAIN_MESSAGE )
	{
		m_pCore->ReportError(ERROR_PLAIN_MESSAGE_TOO_LARGE);
		m_ctrlOutput.SetWindowTextW( _T("") );
		GotoDlgCtrl(GetDlgItem(IDC_BUTTON_DONE)); 
		return;
	}
	
	//attach the core to the password dialog
	CSetPasswordDlg pswd;
	pswd.attach(m_pCore);
	pswd.SetConfirm(TRUE);
	pswd.SetEnableStorePassword( m_pCore->IsNotebookLoaded() );
	pswd.SetEnableGenerateButton(TRUE);
	pswd.SetEnableLoadButton( m_pCore->IsNotebookLoaded() );

	if( IDOK != pswd.DoModal() )
	{
		return;
	}

	//get the user supplied password
	strPassword = pswd.GetPassword();
	fStore = pswd.GetStorePasswordFlag();

	dwRes= m_pCore->EncryptAndEncode(&strInput, &strPassword, strOutput, TRUE);

	
	if(ERROR_NONE == dwRes)
	{
		m_ctrlOutput.SetWindowTextW(strOutput);
		GotoDlgCtrl(GetDlgItem(IDC_BUTTON_COPY)); 
	}
	else
	{
		m_ctrlOutput.Clear();
		GotoDlgCtrl(GetDlgItem(IDC_BUTTON_DONE)); 
	}
	

}

void CDialogMessages::Decrypt()
{
	ASSERT(m_pCore != NULL);

	CString strInput, strOutput, strPassword, strID;
	UINT8 iv[SIZE_IV_IN_BYTES];
	UINT32 dwRes;
	BOOL fStorePassword = FALSE;
	int n = 0;

	m_ctrlInput.GetWindowTextW(strInput);
	n = strInput.GetLength();

	if(0==n)
	{
		m_pCore->ReportError(ERROR_NO_ENC_MESSAGE);
		m_ctrlOutput.SetWindowTextW( _T("") );
		GotoDlgCtrl(GetDlgItem(IDC_BUTTON_DONE)); 
		return;
	}
	if(n > MAX_NUM_CHARS_ENCRYPTED_MESSAGE)
	{
		m_pCore->ReportError(ERROR_ENC_MESSAGE_TOO_LARGE);
		m_ctrlOutput.SetWindowTextW( _T("") );
		GotoDlgCtrl(GetDlgItem(IDC_BUTTON_DONE)); 
		return;
	}

	//try to find the password for the file in the notebook
	//if it is not there, ask the user to enter one

	dwRes = m_pCore->PreParseEncryptedMessage(&strInput, strID, iv);

	if(ERROR_NONE != dwRes  )
	{
		m_pCore->ReportError(dwRes);
		m_ctrlOutput.SetWindowTextW( _T("") );
		GotoDlgCtrl(GetDlgItem(IDC_BUTTON_DONE)); 
		return;
	}

	if( FALSE == m_pCore->FindPasswordInNotebook(strPassword, iv)  )
	{
		CSetPasswordDlg set;
		set.attach(m_pCore);
		set.SetConfirm(FALSE);
		set.SetEnableGenerateButton(FALSE);
		set.SetEnableStorePassword( m_pCore->IsNotebookLoaded() ); 
		set.SetEnableLoadButton( m_pCore->IsNotebookLoaded() );

		if( IDOK != set.DoModal() )
		{
			return;
		}

		//get the entered password
		strPassword = set.GetPassword();
		fStorePassword = set.GetStorePasswordFlag();
	}

	dwRes = m_pCore->DecodeAndDecrypt(&strInput, &strPassword, strID, strOutput, fStorePassword);

	if(ERROR_NONE == dwRes)
	{
		m_ctrlOutput.SetWindowTextW(strOutput);
		GotoDlgCtrl(GetDlgItem(IDC_BUTTON_COPY)); 
	}
	else
	{
		m_ctrlOutput.SetWindowTextW( _T("") );
		GotoDlgCtrl(GetDlgItem(IDC_BUTTON_DONE)); 
	}
}

BOOL CDialogMessages::OnInitDialog()
{
	
	//first call the standard constructor
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	// when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	SetWindowTextW( WINDOW_TEXT );

	if(MODE_DEC == m_dwMode)
	{
		m_ctrlHeader.SetWindowTextW( _T("Decrypt message") );
		m_ctrlButCrypt.SetWindowTextW( _T("Decrypt") );
	}
	else
	{
		m_dwMode = MODE_ENC;
		m_ctrlHeader.SetWindowTextW( _T("Encrypt message") );
		m_ctrlButCrypt.SetWindowTextW( _T("Encrypt") );
	}
	
	m_ctrlHeader.show(ZERSOSUM_DARK, ZERSOSUM_DARK);

	GotoDlgCtrl(GetDlgItem(IDC_EDIT_INPUT)); 

	return FALSE;
	
}

void CDialogMessages::SetMode( DWORD dwMode )
{
	m_dwMode = dwMode;
}

void CDialogMessages::OnBnClickedButCrypt()
{
	ASSERT(m_pCore != NULL);

	//give the prng a gentle kick
	m_pCore->UpdatePRNG();

	//switch on encrypt or decrypt
	if(MODE_DEC == m_dwMode)
	{
		Decrypt();
	}
	else
	{
		Encrypt();
	}

}

void CDialogMessages::OnBnClickedButtonCopy()
{
	ASSERT(m_pCore != NULL);

	//get the string from the control
	CString strOutput;
	m_ctrlOutput.GetWindowTextW(strOutput);

	//switch on encrypt and decrypt to decide whether to wipe the 
	//clipboard
	if(MODE_ENC == m_dwMode)
	{
		m_pCore->CopyStringToClipBoard(&strOutput, FALSE);
	}
	else
	{
		m_pCore->CopyStringToClipBoard(&strOutput, TRUE);
	}

	//transfer focus to the "Done" button
	GotoDlgCtrl(GetDlgItem(IDC_BUTTON_DONE)); 
}

void CDialogMessages::OnBnClickedButtonDone()
{
	OnOK();
}

BOOL CDialogMessages::PreTranslateMessage( MSG* pMsg )
{
	//catch messages from key pressed 
	if (pMsg->message == WM_KEYDOWN)
	{
		//catch the "Return" button
		if (pMsg->wParam == VK_RETURN)
		{
				CWnd* pControl;
				pControl = this->GetFocus();

				//check if the focus is on an edit box
				if( pControl->IsKindOf(RUNTIME_CLASS(CEdit)) )
				{
					//catch whether the "Ctrl" button is also pressed 
					if( 0 == (GetKeyState(VK_CONTROL)&0x8000) )
					{
						return TRUE;
					}
				}
		}
	}

	//proceed to normal message handling
	return CDialogEx::PreTranslateMessage(pMsg);
}


void CDialogMessages::OnMouseMove(UINT nFlags, CPoint point)
{
	ASSERT(m_pCore != NULL);

	m_pCore->MixInMousePoint(&point);

	CDialogEx::OnMouseMove(nFlags, point);
}
