#include "../afx/stdafx.h"
#include "../main/SafeNotes.h"
#include "../threads/KrypcakThreadInfo.h"
#include <process.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


UINT threadWORKERMAIN( LPVOID lParam)
{
	CWorkerThreadKrypcak* pInfo = NULL;
	pInfo= (CWorkerThreadKrypcak*)lParam;

	//delay for gui to init
	Sleep(200);

	//run init exec terminate
 	pInfo->initialize();
 	pInfo->execute();
 	pInfo->terminate();

	return 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWorkerThreadKrypcak::CWorkerThreadKrypcak()
{
}

CWorkerThreadKrypcak::~CWorkerThreadKrypcak() {}

BOOL CWorkerThreadKrypcak::initialize()
{
	return true;
}
BOOL CWorkerThreadKrypcak::terminate()
{
	//send msg to dlg to close

	if( m_pDlg != NULL) 
	{
			if( m_pDlg->m_hWnd != NULL)
			{
				PostMessage( m_pDlg->m_hWnd, WM_KRYPCAK_PROGRESS, KRYPCAK_PROGRESS_CLOSE, 0);
			}
	}

	SetEvent(m_hThread);

	//AfxEndThread(0);

	return true;
}

// start the specified task threaded
BOOL CWorkerThreadKrypcak::start()
{
	//DWORD dwID=0;
	CWinThread *pT=NULL;

	pT = AfxBeginThread( threadWORKERMAIN, this);
	m_hThread = pT->m_hThread;
	
	return true;
}

// Main function for the thread
BOOL CWorkerThreadKrypcak::execute()
{
	BOOL fRet = false;
	switch( m_task )
	{
		//file routines
		case taskEncryptMessage:	fRet = m_pCore->EncryptMessage(m_pContext, m_pDlg); break;
		case taskDecryptMessage:	fRet = m_pCore->DecryptMessage(m_pContext, m_pDlg); break;
		case taskEncryptFile:		fRet = m_pCore->EncryptFile(m_pContext, m_pDlg); break;
		case taskDecryptFile:		fRet = m_pCore->DecryptFile(m_pContext, m_pDlg); break;
		case taskLoadNotebook:	    fRet = m_pCore->LoadNotebook(m_pContext, m_pDlg); break;
		case taskSaveNotebook:		fRet = m_pCore->SaveNotebook( m_pDlg ); break;
	}
	return fRet;
}