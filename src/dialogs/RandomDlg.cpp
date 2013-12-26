// RandomDlg.cpp : implementation file
//

#include "../afx/stdafx.h"
#include "../main/SafeNotes.h"
#include "../dialogs/RandomDlg.h"
#include "afxdialogex.h"


// CRandomDlg dialog

IMPLEMENT_DYNAMIC(CRandomDlg, CDialogEx)

CRandomDlg::CRandomDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CRandomDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_ZSS);
}

CRandomDlg::~CRandomDlg()
{
}

void CRandomDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_ctrlProgress);
	DDX_Control(pDX, IDC_STATIC_HEADER, m_ctrlHeader);
}


BEGIN_MESSAGE_MAP(CRandomDlg, CDialogEx)
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


// CRandomDlg message handlers

BOOL CRandomDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_dwBytesGathered = 0;

	SetWindowText( WINDOW_TEXT );


	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_ctrlHeader.SetWindowTextW( _T("Generate random") );
	m_ctrlHeader.show(ZERSOSUM_DARK, ZERSOSUM_DARK);

	return true;
}

void CRandomDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	ASSERT(m_pCore != NULL);

	m_pCore->MixInMousePoint(&point);

	m_dwBytesGathered = __min(m_dwBytesGathered+1, NUM_EXTERNAL_BYTES);
	
	UpdateControl();
	
	CDialogEx::OnMouseMove(nFlags, point);
}

void CRandomDlg::attach( CCore* pCore )
{
	ASSERT(pCore != NULL);

	m_pCore = pCore;
}

void CRandomDlg::UpdateControl()
{
	Sleep(10);

	if(NUM_EXTERNAL_BYTES == m_dwBytesGathered)
	{
		ASSERT(m_pCore != NULL);

		OnOK();
	}

	m_ctrlProgress.SetPos( (100*m_dwBytesGathered)/NUM_EXTERNAL_BYTES );
}

