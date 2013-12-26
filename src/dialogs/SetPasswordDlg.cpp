// SetPasswordDlg.cpp : implementation file
//

#include "../afx/stdafx.h"
#include "../main/SafeNotes.h"
#include "../dialogs/SetPasswordDlg.h"
#include "afxdialogex.h"
#include "../dialogs/ShowContentsDialog.h"

// CSetPasswordDlg dialog

IMPLEMENT_DYNAMIC(CSetPasswordDlg, CDialogEx)

CSetPasswordDlg::CSetPasswordDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSetPasswordDlg::IDD, pParent)
	//, m_pToolTipLoad(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_ZSS);
	m_fEnableGenerateButton = TRUE;
	m_fEnableLoadButton = TRUE;
	m_fStorePasswordFlag = FALSE;
	m_fAutoCustom = FALSE;
	m_fShowStorePasswordCheck = TRUE;
	m_strHeaderText = _T("Enter password");

	m_pToolTipLoad = NULL;
	m_pToolTipGenerate = NULL;
	m_pToolTipCustom = NULL;
}

CSetPasswordDlg::~CSetPasswordDlg()
{
	m_pCore->ClearString(&m_strPassword);
	m_pCore->ClearString(&m_strURL);

	delete m_pToolTipLoad;
	delete m_pToolTipGenerate;
	delete m_pToolTipCustom;
}

void CSetPasswordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PSWD, m_ctrlPassword);
	DDX_Control(pDX, IDC_EDIT_CONFIRM, m_ctrlConfirm);
	DDX_Control(pDX, IDC_CHECK_STORE, m_checkStore);
	DDX_Control(pDX, IDC_BUTTON_GEN, m_ctrlGenerate);
	DDX_Control(pDX, IDC_STATIC_HEADER, m_ctrlHeader);
	DDX_Control(pDX, IDC_STATIC_TEXTCONFIRM, m_ctrlConfirmText);
	DDX_Control(pDX, IDC_CHECK_CUSTOM, m_ctrlCustomGenerate);
	DDX_Control(pDX, IDC_CHECK_SHOW, m_ctrlShowPasword);
	DDX_Control(pDX, IDC_BUTTON_LOAD, m_ctrlButtonLoad);
}


BEGIN_MESSAGE_MAP(CSetPasswordDlg, CDialogEx)
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDOK, &CSetPasswordDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_GEN, &CSetPasswordDlg::OnBnClickedButtonGen)
	ON_BN_CLICKED(IDC_CHECK_STORE, &CSetPasswordDlg::OnClickedCheckStore)
	ON_BN_CLICKED(IDC_CHECK_SHOW, &CSetPasswordDlg::OnBnClickedCheckShow)
	ON_BN_CLICKED(IDC_BUTTON_LOAD, &CSetPasswordDlg::OnBnClickedButtonLoad)
END_MESSAGE_MAP()

CString CSetPasswordDlg::GetPassword()
{
	return m_strPassword;
}


// CSetPasswordDlg message handlers


void CSetPasswordDlg::OnBnClickedOk()
{
	CString strPswd1, strPswd2;

	m_fStorePasswordFlag = m_checkStore.GetCheck();
	m_ctrlPassword.GetWindowTextW(strPswd1);

	//check the password is not too large
	if( strPswd1.GetLength() > MAX_PSWD_SIZE )
	{
			m_pCore->ReportError(ERROR_PSWD_TOO_LARGE);
			return;
	}

	//if the two passwords should be equal, check this
	if(m_fConfirm)
	{
		m_ctrlConfirm.GetWindowTextW(strPswd2);

		if( strPswd2 == strPswd1)
		{
			m_strPassword = strPswd1;
			m_pCore->ClearString(&strPswd1);
			m_pCore->ClearString(&strPswd2);
			CDialogEx::OnOK();
		}
		else
		{
			m_pCore->ReportError(ERROR_NON_MATCHING_PSWDS);
		}
	}
	else
	{
		m_strPassword = strPswd1;
		m_pCore->ClearString(&strPswd1);
		CDialogEx::OnOK();
	}
}

void CSetPasswordDlg::OnBnClickedButtonGen()
{
	UpdateData(TRUE);

	CPasswordDlg dlg;
	CString strPassword = _T("");

	//if the box for custom password generation is not checked, 
	//generate a password with the default parameters
	if( !m_ctrlCustomGenerate.GetCheck() )
	{
		m_pCore->GeneratePassword(strPassword, DEFAULT_PASSWORD_LENGTH, DEFAULT_PASSWORD_ALPHABET_MASK);
	}
	//if the box for custom password generation is checked, launch
	//the window to generate a password 
	else
	{
		dlg.attach(m_pCore);
		dlg.SetTextOnOKButton( _T("Use") );

		if(IDOK == dlg.DoModal() )
		{
			//get the password from the password generating dialog
			dlg.GetPassword( strPassword );
		}
	}

	//move the generated password to the edit control
	m_ctrlPassword.SetWindowText( strPassword );

	if(m_fConfirm)
	{
		//move the generated password to the edit control for 
		//the confirmed password
		m_ctrlConfirm.SetWindowText( strPassword );
	}

	//wipe the string that contained the password
	//NOTE: we can do this, the password will be retrieved from the 
	//edit control when 'OK'is pressed 
	m_pCore->ClearString(&strPassword);
}

void CSetPasswordDlg::attach( CCore* pCore )
{
	m_pCore = pCore;
}

void CSetPasswordDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	m_pCore->MixInMousePoint(&point);
	
	CDialogEx::OnMouseMove(nFlags, point);
}

BOOL CSetPasswordDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	SetWindowText( WINDOW_TEXT );

	// TODO: Add extra initialization here


	m_ctrlConfirmText.ShowWindow(m_fConfirm);
	m_ctrlConfirm.ShowWindow(m_fConfirm);
	
	m_ctrlGenerate.SetWindowTextW( _T("Generate") );
	m_ctrlButtonLoad.SetWindowTextW( _T("Load") );

	if(FALSE == m_fEnableGenerateButton)
	{
		m_ctrlGenerate.EnableWindow(FALSE); 
		m_ctrlCustomGenerate.EnableWindow(FALSE);
	}
	
	if(FALSE == m_fEnableLoadButton)
	{
		m_ctrlButtonLoad.EnableWindow(FALSE); 
	}

	m_ctrlHeader.SetWindowTextW( m_strHeaderText );
	m_ctrlHeader.show(ZERSOSUM_DARK, ZERSOSUM_DARK);

	if( m_pCore->IsNotebookLoaded() && m_fShowStorePasswordCheck )
	{
		m_checkStore.EnableWindow(TRUE);
		m_checkStore.SetCheck(TRUE);
	}
	else 
	{
		m_checkStore.EnableWindow(FALSE);

	}

	if(m_fAutoCustom)
	{
		m_ctrlCustomGenerate.SetCheck(1);
	}
	

	m_ctrlGenerate.EnableWindow(m_fEnableGenerateButton);

	//tooltips
	if( m_fEnableLoadButton )
	{
		m_pToolTipLoad = new CToolTipCtrl;
		m_pToolTipLoad->Create(this);
		m_pToolTipLoad->AddTool( &m_ctrlButtonLoad, IDS_TOOLTIP_LOAD );
		m_pToolTipLoad->Activate(TRUE);
	}

	if( m_fEnableGenerateButton )
	{
		m_pToolTipGenerate = new CToolTipCtrl;
		m_pToolTipGenerate->Create(this);
		m_pToolTipGenerate->AddTool( &m_ctrlGenerate, IDS_TOOLTIP_GENERATE );
		m_pToolTipGenerate->Activate(TRUE);

		m_pToolTipCustom = new CToolTipCtrl;
		m_pToolTipCustom->Create(this);
		m_pToolTipCustom->AddTool( &m_ctrlCustomGenerate, IDS_TOOLTIP_CUSTOM );
		m_pToolTipCustom->Activate(TRUE);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSetPasswordDlg::SetConfirm( BOOL flag )
{
	m_fConfirm = flag;
}

BOOL CSetPasswordDlg::GetStorePasswordFlag()
{
	return m_fStorePasswordFlag;
}


void CSetPasswordDlg::OnClickedCheckStore()
{
	m_fStorePasswordFlag = m_checkStore.GetCheck();
}

void CSetPasswordDlg::SetURL( CString strURL )
{
	m_strURL = strURL;
}

void CSetPasswordDlg::OnBnClickedCheckShow()
{
	UpdateData(true);

	//if the check box has been ticked
	if( m_ctrlShowPasword.GetCheck() )
	{
		//set the edit control for the password to display the contents
		m_ctrlPassword.SetPasswordChar(0);	
		if(m_fConfirm)
		{
			//set the edit control for the confirmed password to display the contents
			m_ctrlConfirm.SetPasswordChar(0);
		}
	}
	//if the check box has not been ticked
	else
	{
		//set the edit control for the password to hide the contents
		m_ctrlPassword.SetPasswordChar( HIDDEN_PSWD_CHAR );
		if(m_fConfirm)
		{
			//set the edit control for the confirmed password to display the contents
			m_ctrlConfirm.SetPasswordChar( HIDDEN_PSWD_CHAR );
		}
	}

	//redraw the window
	m_ctrlConfirm.RedrawWindow();
	m_ctrlPassword.RedrawWindow();

	UpdateData(true);
}

void CSetPasswordDlg::SetEnableStorePassword( BOOL flag )
{
	m_fStorePasswordFlag = flag;
}

void CSetPasswordDlg::SetEnableGenerateButton( BOOL flag )
{
	m_fEnableGenerateButton = flag;
}

void CSetPasswordDlg::SetShowStorePasswordCheck( BOOL flag )
{
	m_fShowStorePasswordCheck = flag;
}



void CSetPasswordDlg::OnBnClickedButtonLoad()
{
	CShowContentsDialog dlg;
	CString strPassword = _T("");
	dlg.attach(m_pCore);
	dlg.SetUse(PSWD_SELECT);

	if(IDOK == dlg.DoModal())
	{
		dlg.GetPassword( &strPassword );
	}

	//move the generated password to the edit control
	m_ctrlPassword.SetWindowText( strPassword );

	if(m_fConfirm)
	{
		//move the generated password to the edit control for 
		//the confirmed password
		m_ctrlConfirm.SetWindowText( strPassword );
	}

	m_pCore->ClearString( &strPassword );
}

BOOL CSetPasswordDlg::PreTranslateMessage( MSG* pMsg )
{
	if(m_pToolTipLoad != NULL)
	{
		m_pToolTipLoad->RelayEvent(pMsg);
	}

	if(m_pToolTipGenerate!= NULL)
	{
		m_pToolTipGenerate->RelayEvent(pMsg);
	}

	if(m_pToolTipCustom!= NULL)
	{
		m_pToolTipCustom->RelayEvent(pMsg);
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CSetPasswordDlg::SetHeaderText( CString strText )
{
	m_strHeaderText = strText;
}

void CSetPasswordDlg::SetEnableLoadButton( BOOL flag )
{
	m_fEnableLoadButton = flag;
}

void CSetPasswordDlg::SetAutoCustomFlag()
{
	m_fAutoCustom = TRUE;
}

