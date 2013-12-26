
// SafeNotes.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "../SafeNotes/resource.h"		// main symbols
#include "../core/Core.h"

// CSafeNotesApp:
// See SafeNotes.cpp for the implementation of this class
//

class CSafeNotesApp : public CWinApp
{
public:
	CSafeNotesApp();

// Overrides
public:
	virtual BOOL InitInstance();

private:
	CCore m_core;

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CSafeNotesApp theApp;