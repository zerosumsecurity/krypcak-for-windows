// ProgressDlg.cpp : implementation file
//

#include "../afx/stdafx.h"
#include "../main/SafeNotes.h"
#include "../dialogs/ProgressDlg.h"
#include "../threads/KrypcakThreadInfo.h"

// CProgressDlg dialog

IMPLEMENT_DYNAMIC(CProgressDlg, CDialog)

CProgressDlg::CProgressDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProgressDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_ZSS);
	m_strHeader = _T("Progress");
}

CProgressDlg::~CProgressDlg()
{
}

void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS, m_ctrlProgress);
	DDX_Control(pDX, IDC_STATIC_HEADER, m_ctrlHeader);
	DDX_Control(pDX, IDC_STATIC_TEXT, m_ctrlText);
}


BEGIN_MESSAGE_MAP(CProgressDlg, CDialog)
	ON_MESSAGE( WM_KRYPCAK_PROGRESS, OnMessageUser )
END_MESSAGE_MAP()

void CProgressDlg::SetText( CString strText )
{
	m_ctrlText.SetWindowText(strText);
	UpdateData(false);
}

void CProgressDlg::SetHeader( CString strText )
{
	m_ctrlHeader.SetWindowText(strText);
	UpdateData(false);
}

void CProgressDlg::SetHeader( UINT32 task )
{
	switch (task)
	{
		case taskDecryptFile:
			m_strHeader = _T("Decrypt File");
			break;
		case taskEncryptFile:
			m_strHeader = _T("Encrypt File");
			break;
		case taskDecryptMessage:
			m_strHeader = _T("Decrypt Message");
			break;
		case taskEncryptMessage:
			m_strHeader = _T("Encrypt Message");
			break;
		case taskLoadNotebook:
			m_strHeader = _T("Load Notebook");
			break;
		case taskSaveNotebook:
			m_strHeader = _T("Save Notebook");
			break;
	}
}

void CProgressDlg::SetPosition( DWORD dwPos )
{
	m_ctrlProgress.SetPos(dwPos);
	UpdateData(false);
}

BOOL CProgressDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowText( WINDOW_TEXT );

	//m_ctrlProgress.SetRange(0, 100);
	m_ctrlProgress.SetPos(0);
	m_ctrlProgress.SetRange(0, 100);

	m_ctrlHeader.SetWindowTextW( m_strHeader );
	m_ctrlHeader.show(ZERSOSUM_DARK, ZERSOSUM_DARK);

	UpdateData(false);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CProgressDlg::SetRange( UINT32 dwMax )
{
	m_dwMaxProgress = dwMax;

	m_ctrlProgress.SetRange32(0, m_dwMaxProgress);
	UpdateData(false);
}


LONG CProgressDlg::OnMessageUser( WPARAM wParam, LPARAM lParam )
{
	//switch on command
	switch( wParam )
	{
		case KRYPCAK_PROGRESS_UPDATE: 
			SetPosition( lParam );
			break;

		case KRYPCAK_PROGRESS_MESSAGE: 
			{ 
				CString strMSG;
				switch ( lParam)
				{
					case KRYPCAK_PROGRESSMESSAGE_LOAD_PSWD: strMSG.Format( _T("Parsing password...") );	break;
					case KRYPCAK_PROGRESSMESSAGE_ENC_FILE: strMSG.Format( _T("Encrypting file...") );	break;
					case KRYPCAK_PROGRESSMESSAGE_DEC_FILE: strMSG.Format( _T("Decrypting file...") );	break;
				}
				SetText( strMSG );
				break;
			}
		case KRYPCAK_PROGRESS_RESULT:
			m_dwRes = (UINT32)lParam;
			break;
		case KRYPCAK_PROGRESS_CLOSE: 
			OnOK();
			break;
	}

	UpdateData(FALSE);

	return 0;
}

void CProgressDlg::SetResult( UINT32 dwRes )
{
	m_dwRes = dwRes;
}

void CProgressDlg::SetResult( CString strResult )
{
	m_strResult = strResult;
}

UINT32 CProgressDlg::GetResult()
{
	return m_dwRes;
}

CString CProgressDlg::GetStringResult()
{
	return m_strResult;
}
