#if !defined(AFX_KRYCAKTHREADINFO_H__7BC1CAAA_6EAD_4222_ABB1_2B4F9033F09C__INCLUDED_)
#define AFX_KRYCAKTHREADINFO_H__7BC1CAAA_6EAD_4222_ABB1_2B4F9033F09C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../afx/stdafx.h"
#include <afxwin.h>
#include <process.h>
#include "../core/Core.h"

//
#define WM_KRYPCAK_PROGRESS			(WM_USER+0)

#define KRYPCAK_PROGRESS_UPDATE      		0
#define KRYPCAK_PROGRESS_MESSAGE      		1
#define KRYPCAK_PROGRESS_RESULT				2
#define KRYPCAK_PROGRESS_CLOSE				3


#define KRYPCAK_PROGRESSMESSAGE_LOAD_PSWD	1
#define KRYPCAK_PROGRESSMESSAGE_DEC_FILE	2
#define KRYPCAK_PROGRESSMESSAGE_ENC_FILE	3

UINT threadWORKERMAIN( LPVOID lParam);

class CWorkerThreadKrypcak  
{
public:
	CWorkerThreadKrypcak();
	virtual ~CWorkerThreadKrypcak();

	//attach methods
	void attach( CCore *pC){m_pCore = pC;}
	void attach( CProgressDlg *pDlg){m_pDlg = pDlg;}
	//set methods
	void setTask( UINT32 task ){m_task = task;}
	void setContext(crypt_context* pContext){m_pContext = pContext;}
	
	

	//get methods
	HANDLE getHandle(){return m_hThread;}
	
	//public methods
	BOOL start();
	BOOL stop( BOOL fWait);

//protected:
	BOOL initialize();
	BOOL execute();
	BOOL terminate();
	
protected:
	crypt_context*  m_pContext;
	CProgressDlg*	m_pDlg;
	UINT32			m_task;
	CCore*			m_pCore;
	HANDLE			m_done;

private:
	HANDLE	m_hThread;
};

#endif // !defined(AFX_KRYCAKTHREADINFO_H__7BC1CAAA_6EAD_4222_ABB1_2B4F9033F09C__INCLUDED_)
