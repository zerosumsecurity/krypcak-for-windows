
// SafeNotesDlg.cpp : implementation file
//

#include "../afx/stdafx.h"
#include "../main/SafeNotes.h"
#include "SafeNotesDlg.h"
#include "afxdialogex.h"
#include "../dialogs/DialogMessages.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSafeNotesDlg dialog




CSafeNotesDlg::CSafeNotesDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSafeNotesDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_ZSS);

	m_tabPasswords.SetCurrentUse(PSWD_VIEW);
}

void CSafeNotesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_HEADER, m_ctrlHeader);
	DDX_Control(pDX, IDC_TAB_PSWDS, m_ctrlTabPasswords);
}

BEGIN_MESSAGE_MAP(CSafeNotesDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_GEN_PWD, &CSafeNotesDlg::OnGeneratePassword)
	ON_COMMAND(ID_NOTEBOOK_OPEN, &CSafeNotesDlg::OnOpenNotebook)
	ON_COMMAND(ID_NOTEBOOK_NEW, &CSafeNotesDlg::OnNewNotebook)
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_NOTEBOOK_CHANGEPASSWORD, &CSafeNotesDlg::OnNotebookChangepassword)
	ON_COMMAND(ID_ABOUT_ABOUTKRYPCAK, &CSafeNotesDlg::OnAboutAboutkrypcak)
	ON_COMMAND(ID_FILE_ENCRYPT, &CSafeNotesDlg::OnFileEncrypt)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_PSWDS, &CSafeNotesDlg::OnSelchangeTabPswds)
	ON_COMMAND(ID_FILE_DECRYPT, &CSafeNotesDlg::OnFileDecrypt)
	ON_COMMAND(ID_NOTEBOOK_EXPORT, &CSafeNotesDlg::OnNotebookExport)
	ON_COMMAND(ID_MESSAGES_ENCRYPT, &CSafeNotesDlg::OnMessagesEncrypt)
	ON_COMMAND(ID_MESSAGES_DECRYPT, &CSafeNotesDlg::OnMessagesDecrypt)
END_MESSAGE_MAP()


// CSafeNotesDlg message handlers

BOOL CSafeNotesDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	SetWindowText( WINDOW_TEXT );

	//get the correct text in the header
	UpdateHeader();

	m_ctrlHeader.show(ZERSOSUM_DARK, ZERSOSUM_DARK);

	CTabCtrl* pTabCtrl = (CTabCtrl*)GetDlgItem(IDC_TAB_PSWDS);

	m_tabPasswords.Create(IDD_TAB_PWD, pTabCtrl);

	TCITEM item;
	item.mask    = TCIF_TEXT | TCIF_PARAM;
	item.lParam  = (LPARAM)& m_tabPasswords;
	item.pszText = _T("Files");
	pTabCtrl->InsertItem(0, &item);

	item.mask    = TCIF_TEXT | TCIF_PARAM;
	item.lParam  = (LPARAM)& m_tabPasswords;
	item.pszText = _T("Messages");
	pTabCtrl->InsertItem(1, &item);

	item.mask    = TCIF_TEXT | TCIF_PARAM;
	item.lParam  = (LPARAM)& m_tabPasswords;
	item.pszText = _T("Shared");
	pTabCtrl->InsertItem(2, &item);

	item.mask    = TCIF_TEXT | TCIF_PARAM;
	item.lParam  = (LPARAM)& m_tabPasswords;
	item.pszText = _T("Accounts");
	pTabCtrl->InsertItem(3, &item);


	CRect rcItem;   
	pTabCtrl->GetItemRect(0, &rcItem);
	m_tabPasswords.SetWindowPos(NULL, rcItem.left, rcItem.bottom + 1, 0, 0, SWP_NOSIZE | SWP_NOZORDER );

	m_tabPasswords.ShowWindow(SW_SHOW);


	UpdateControl();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSafeNotesDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSafeNotesDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSafeNotesDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CSafeNotesDlg::OnGeneratePassword()
{
	ASSERT(m_pCore != NULL);

	CPasswordDlg dlg;
	CString strText;
	strText.LoadStringW( IDS_COPY );
	dlg.SetTextOnOKButton( strText );
	CString strPassword;

	//give the PRNG a gentle kick
	m_pCore->UpdatePRNG();

	//attach the core to the password dialog
	dlg.attach(m_pCore);

	//show the password dialog
	if(IDOK == dlg.DoModal() )
	{
		//get the supplied password 
		dlg.GetPassword( strPassword );

		//copy the password to the clipboard
		m_pCore->CopyStringToClipBoard( &strPassword );
		//clear the password in memory
		m_pCore->ClearString( &strPassword );
	}
	
}

void CSafeNotesDlg::attach( CCore* pCore )
{
	ASSERT(pCore != NULL);

	m_pCore = pCore;

	//give the PRNG a gentle kick
	m_pCore->UpdatePRNG();

	//pass on the pointer to out tab control
	m_tabPasswords.attach(m_pCore);
}


void CSafeNotesDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	ASSERT(m_pCore != NULL);

	m_pCore->MixInMousePoint(&point);

	CDialogEx::OnMouseMove(nFlags, point);
}


void CSafeNotesDlg::OnOpenNotebook()
{
	ASSERT(m_pCore != NULL);

	CString strURL;
	int res=0;

	//give the PRNG a kick
	m_pCore->UpdatePRNG();

	if( m_pCore->IsNotebookLoaded() )
	{
		res = AfxMessageBox( IDS_LOAD_NOTEBOOK, MB_YESNO);

		//the user changed his mind, leave
		if(res != IDYES)
		{
			return;
		}	

		//store the current loaded notebook
		m_pCore->StartSaveNotebook();


		//clear the notebook in memory
		m_pCore->CleanNotebook();
	}
	
	//ask for an url to retrieve the notebook from
	if(FALSE == m_pCore->SelectNotebookURL(strURL) )
	{
		UpdateControl();
		return;
	}

	//try to get a password for the notebook and try to parse the it
	if( m_pCore->AskAndLoadNotebook( strURL ) )
	{
		//indicate we now have a notebook
		m_pCore->SetNotebookLoadedFlag(TRUE);
	}
	
	UpdateControl();
	return;
}

void CSafeNotesDlg::OnNewNotebook()
{
	ASSERT(m_pCore != NULL);

	UINT32 res;
	CString strFilter, strURL;
	CString strURLOut, strPassword;

	//give the PRNG a kick
	m_pCore->UpdatePRNG();

	if( m_pCore->IsNotebookLoaded() )
	{
		res = AfxMessageBox( IDS_NEW_NOTEBOOK, MB_YESNO);

		//the user changed his mind, leave
		if(res != IDYES)
		{
			return;
		}

		//store the current loaded notebook
		m_pCore->StartSaveNotebook();

		//clear the notebook in memory
		m_pCore->CleanNotebook();
	}

	if(FALSE == m_pCore->SelectNewNotebookURL(&strURL))
	{
		UpdateControl();
		return;
	}

	CSetPasswordDlg set;

	set.attach(m_pCore);
	set.SetConfirm(TRUE);
	set.SetEnableStorePassword(FALSE);
	set.SetEnableGenerateButton(TRUE);
	set.SetAutoCustomFlag();
	set.SetEnableLoadButton(FALSE);

	if(IDOK == set.DoModal() )
	{
		strPassword = set.GetPassword();
		m_pCore->SetNewNotebookPassword( &strPassword );
		m_pCore->SetNewNotebookURL(strURL);
		m_pCore->SetNotebookLoadedFlag(TRUE);

		if(ERROR_NONE == m_pCore->StartSaveNotebook() )
		{
			AfxMessageBox( IDS_CREATED_NB_SUCCES , MB_ICONINFORMATION);
		}
		else
		{
			m_pCore->CleanNotebook();
			AfxMessageBox( IDS_FAIL_CREATE_NB );
		}

	}

	m_pCore->ClearString(&strPassword);

	UpdateControl();
	return;
}

void CSafeNotesDlg::UpdateControl(int nTabFocus)
{
	ASSERT(m_pCore != NULL);

	UpdateData(true);
	BOOL flag = m_pCore->IsNotebookLoaded();

	CMenu *pM = GetMenu();	
	pM->EnableMenuItem(ID_NOTEBOOK_NEW, MF_ENABLED );
	pM->EnableMenuItem(ID_NOTEBOOK_OPEN, MF_ENABLED );
	pM->EnableMenuItem(ID_NOTEBOOK_CHANGEPASSWORD, flag?MF_ENABLED:MF_GRAYED );	
	pM->EnableMenuItem(ID_NOTEBOOK_EXPORT, flag?MF_ENABLED:MF_GRAYED );	

	//set the correct tab
	if(-1 == nTabFocus)
	{
		m_tabPasswords.SetCurrentView(m_ctrlTabPasswords.GetCurFocus());
	}
	else
	{
		m_tabPasswords.SetCurrentView( nTabFocus );
		m_ctrlTabPasswords.SetCurFocus( nTabFocus );
	}

	//load the passwords associated with the current set type
	ShowNotes(m_ctrlTabPasswords.GetCurFocus());

	UpdateHeader();

	//refresh the dialog
	m_tabPasswords.ShowWindow(SW_SHOW);

	UpdateData(false);
}

void CSafeNotesDlg::OnNotebookChangepassword()
{
	ASSERT(m_pCore != NULL);

	//give the PRNG a gentle kick
	m_pCore->UpdatePRNG();

	m_pCore->ChangePasswordNotebook();
}


void CSafeNotesDlg::OnAboutAboutkrypcak()
{
	ASSERT(m_pCore != NULL);

	CAboutDlg about;

	//give the PRNG a gentle kick
	m_pCore->UpdatePRNG();

	about.DoModal();
}


void CSafeNotesDlg::OnFileEncrypt()
{
	ASSERT(m_pCore != NULL);

	CString strFilter, strFilterOut, strFolder;
	CString strPassword, strURL, strURLOut;
	BOOL fDone  = FALSE;
	BOOL fStorePassword = FALSE;

	//give the PRNG a gentle kick
	m_pCore->UpdatePRNG();

	//build filter for file open dialog
	strFilter.Format(_T("%s (*.%s)|*.%s||"), _T("All Files"), _T("*"), _T("*"));

	CFileDialog dlgPlain(true, NULL, NULL, OFN_HIDEREADONLY, strFilter);	
	dlgPlain.m_ofn.lpstrTitle = _T("Select file for encryption");

	//if the user has not selected a plain file, we are done
	if( IDOK != dlgPlain.DoModal())
	{
		//clear the member holding the path of the file to be encrypted
		return;
	}

	//get the selected input file
	strURL = dlgPlain.GetPathName();

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
	fStorePassword = pswd.GetStorePasswordFlag();

	//build filter
	strFilterOut.Format(_T("%s (*.%s)|*.%s||"), _T("Krypcak Encrypted file"), _T("kef"), _T("kef"));

	//build output_filename
	CString strFileName = strURL + ENC_FILE_EXT;

	//create file dialog
	CFileDialog save(FALSE, _T("kef"), strFileName, OFN_OVERWRITEPROMPT, strFilterOut);		

	do{

		//did the user select a file path?
		if( IDOK != save.DoModal() )
		{			
			return;
		}

		//yes he did, so get it		
		strURLOut = save.GetPathName();
		fDone = true;

	}while(!fDone);

	
	//perform the encryption
	UINT32 nRes = m_pCore->StartEncryptFile( &strPassword, strURL, strURLOut, fStorePassword);

	//switch on the encryption results
	switch (nRes)
	{
		case ERROR_NONE:
			//encryption worked
			AfxMessageBox( IDS_FILE_ENC_SUCCESS, MB_ICONINFORMATION );
			break;
		default:
			m_pCore->ReportError(nRes);
			break;
	}

	//empty the password string
	m_pCore->ClearString( &strPassword );	
	
	UpdateControl(PSWD_FILE);
}




void CSafeNotesDlg::ShowNotes(int nTabIndex)
{
	//set the current type of password at the tab control
	m_tabPasswords.m_ctrlContent.SetType(nTabIndex);

	//(re-)initialize the tab control 
	m_tabPasswords.InitShowPasswords();

	//show the passwords on screen
	m_tabPasswords.m_ctrlContent.DisplayPasswords();
}



void CSafeNotesDlg::OnSelchangeTabPswds(NMHDR*, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;

	UpdateControl();
}


void CSafeNotesDlg::OnFileDecrypt()
{
	ASSERT(m_pCore != NULL);

	CString strFilter, strFilterOut, strFolder;
	CString strPassword, strURL;
	CString strURLOut;
	BOOL fStorePassword = FALSE;
	int nRes;

	//give the PRNG a kick
	m_pCore->UpdatePRNG();

	//build filter for file open dialog
	strFilter.Format(_T("%s (*.%s)|*.%s||"), _T("Krypcak Encrypted file"), _T("kef"), _T("kef"));

	CFileDialog dlgEnc(true, NULL, NULL, OFN_HIDEREADONLY, strFilter);	
	dlgEnc.m_ofn.lpstrTitle = _T("Select file for decryption");

	//if the user has not selected a plain file, we are done
	if( IDOK != dlgEnc.DoModal())
	{
		//clear the member holding the path of the file to be encrypted
		return;
	}

	strURL = dlgEnc.GetPathName();

	//try to find the password for the file in the notebook
	//if it is not there, ask the user to enter one
	if( ERROR_NONE != m_pCore->FindPasswordForFile(&strPassword, strURL) )
	{
		CSetPasswordDlg set;
		set.attach(m_pCore);
		set.SetConfirm(FALSE);
		set.SetURL(strURL);
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


	//build filter
	strFilterOut.Format(_T("%s (*.%s)|*.%s||"), _T("All Files"), _T("*"), _T("*"));

	//build output_filename
	CString strFileName = strURL;
	m_pCore->StripDecryptFileName(strFileName);

	//create file dialog
	CFileDialog save(FALSE, NULL, strFileName, OFN_OVERWRITEPROMPT, strFilterOut);		

	BOOL fDone  = FALSE;

	do{

		//did the user select a file path?
		if( IDOK != save.DoModal() )
		{			
			return;
		}

		//yes he did, so get it		
		strURLOut = save.GetPathName();
		fDone = true;

	}while(!fDone);

	//decrypt the file
	nRes = m_pCore->StartDecryptFile( &strPassword , strURL, strURLOut, fStorePassword );

	switch (nRes)
	{
		case ERROR_NONE:
			//encryption worked
			AfxMessageBox(IDS_DEC_SUCCESS, MB_ICONINFORMATION );
			break;
		default:
			m_pCore->ReportError(nRes);
			break;
	}

	m_pCore->ClearString( &strPassword );

	UpdateControl(PSWD_FILE);
}

void CSafeNotesDlg::UpdateHeader()
{
	ASSERT(m_pCore != NULL);

	if( m_pCore->IsNotebookLoaded() )
	{
		//strip the url of the notebook to just the filename
		CString strHeader = ::PathFindFileNameW( m_pCore->GetLoadedNotebookURL() );

		//if the filename is too long, just use "Notebook" as caption
		if(strHeader.GetLength() > MAX_LENGTH_DLG_HEADER)
		{
			m_ctrlHeader.SetWindowTextW( MAKEINTRESOURCE(IDS_NOTEBOOK) );
		}
		else
		{
			m_ctrlHeader.SetWindowTextW( strHeader );
		}
	}
	else
	{
		m_ctrlHeader.SetWindowTextW( MAKEINTRESOURCE(IDS_NO_NOTEBOOK) );
	}
}


void CSafeNotesDlg::OnNotebookExport()
{
	ASSERT(m_pCore != NULL);

	m_pCore->ExportNotebook();
}


void CSafeNotesDlg::OnMessagesEncrypt()
{
	CDialogMessages dlg;
	
	dlg.attach(m_pCore);
	dlg.SetMode(MODE_ENC);
	dlg.DoModal();

	UpdateControl(PSWD_MESSAGE);
}


void CSafeNotesDlg::OnMessagesDecrypt()
{
	CDialogMessages dlg;

	dlg.attach(m_pCore);
	dlg.SetMode(MODE_DEC);
	dlg.DoModal();

	UpdateControl(PSWD_MESSAGE);
}

void CSafeNotesDlg::OnCancel()
{
	m_pCore->CloseAndCleanUp();

	CDialogEx::OnCancel();
}


