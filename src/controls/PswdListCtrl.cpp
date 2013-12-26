// PswdListCtrl.cpp : implementation file
//

#include "../afx/stdafx.h"
#include "../controls/PswdListCtrl.h"
#include "../controls/TabPasswords.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPswdListCtrl


CPswdListCtrl::CPswdListCtrl()
{
	m_dwType = PSWD_FILE;

	_reset_sort_order();
}


CPswdListCtrl::~CPswdListCtrl()
{
}


/////////////////////////////////////////////////////////////////////////////
// CPswdListCtrl message handlers

/*
	display the passwords of the 
*/
void CPswdListCtrl::DisplayPasswords()
{
	int i, index;
	CPasswordNotesArray* pArray = NULL;
	note_password pswd;

	//clear the control
	DeleteAllItems();

	//get reference to the passwords
	pArray = m_pCore->GetPasswordArray( m_dwType );

	//if we have no reference to a password list, bail out
	if(NULL == pArray)
	{
		return;
	}
	ASSERT(pArray);

	//depending on the type of passwords, fill the displayed list
	if(PSWD_FILE == m_dwType)
	{
		for(i=0; i<pArray->GetSize(); i++)
		{
			pswd = pArray->GetAt(i);

			index = InsertItem(i , (LPCWSTR)" ", 0);

			//display date
			SetItemText(index, 0, pswd.strDate );
			//display domain
			SetItemText(index, 1, pswd.strDomain );
			//display password
			SetItemText(index, 2, pswd.strPassword );
			//set item data
			SetItemData(index,  (DWORD)(&pArray->GetAt(i)));
		}
	}

	if(PSWD_CONTACT == m_dwType)
	{
		for(i=0; i<pArray->GetSize(); i++)
		{
			pswd = pArray->GetAt(i);

			index = InsertItem(i , (LPCWSTR)" ", 0);

			//display date
			SetItemText(index, 0, pswd.strDate );
			//display username
			SetItemText(index, 1, pswd.strDomain );
			//display password
			SetItemText(index, 2, pswd.strPassword );
			//set item data
			SetItemData(index,  (DWORD)(&pArray->GetAt(i)));
		}
	}

	if(PSWD_ACCOUNT == m_dwType)
	{
		for(i=0; i<pArray->GetSize(); i++)
		{
			pswd = pArray->GetAt(i);

			index = InsertItem(i , (LPCWSTR)" ", 0);

			//display date
			SetItemText(index, 0, pswd.strDate );
			//display domain
			SetItemText(index, 1, pswd.strDomain );
			//display user
			SetItemText(index, 2, pswd.strInfo);
			//display password
			SetItemText(index, 3, pswd.strPassword );
			//set item data
			SetItemData(index,  (DWORD)(&pArray->GetAt(i)));
		}
	}

	if(PSWD_MESSAGE == m_dwType)
	{
		for(i=0; i<pArray->GetSize(); i++)
		{
			pswd = pArray->GetAt(i);

			index = InsertItem(i , (LPCWSTR)" ", 0);

			//display date
			SetItemText(index, 0, pswd.strDate );
			//display description
			SetItemText(index, 1, pswd.strDomain);
			//display password
			SetItemText(index, 2, pswd.strPassword );
			//set item data
			SetItemData(index,  (DWORD)(&pArray->GetAt(i)));
		}
	}

	//sort the list on the date by default
	Sort(0);
}

/*
	catch the user hitting 'Enter' or 'Delete' on a selected password
*/
BOOL CPswdListCtrl::PreTranslateMessage( MSG* pMsg )
{
	//was a key pressed?
	if(pMsg->message==WM_KEYDOWN)
	{
		//was it 'Return'?
		if(pMsg->wParam==VK_RETURN)
		{
			//if the user is viewing the list, let him edit the password
			if(m_dwUse == PSWD_VIEW)
			{
				m_tabParent->OnPopupEdit();
			}
			//if the user is in the process of selecting a password, copy it for the user
			if(m_dwUse == PSWD_SELECT)
			{
				m_tabParent->OnPopupUse();
			}
			
			//done
			return TRUE;
		}

		//if the user is viewing the list of passwords and hit 'Delete', do so
		if( (PSWD_VIEW == m_dwUse) && (pMsg->wParam == VK_DELETE) )
		{
			m_tabParent->OnPopupDelete();
		}
	}	

	return CListCtrl::PreTranslateMessage(pMsg);
}

/*
	recover the selected password and let the user edit it
*/
BOOL CPswdListCtrl::EditSelectedPassword()
{
	CNewPasswordDlg dlg;
	int selected_count, n, i;
	t_note_password password, tmp;
	CString strPassword, strDomain, strInfo;
	note_password* pPswd = NULL;

	//get the number of selected items
	selected_count = GetSelectedCount();

	//we want exactly one to be selected
	if(1 != selected_count)
	{
		return FALSE;
	}

	//get the number of items in the list
	n = GetItemCount();	

	//loop over all items in the control 
	for(i=n-1; i>=0; i--)
	{
		//is the item selected
		if( GetItemState(i, LVIS_SELECTED) )
		{
			//get the associated item data
			pPswd = (note_password*)GetItemData(i);
		}
	}

	tmp.strDate = pPswd->strDate;
	tmp.strPassword = pPswd->strPassword;
	tmp.strInfo = pPswd->strInfo;
	tmp.strDomain = pPswd->strDomain;

	//fill the password dialog with the selected password
	dlg.attach(m_pCore);
	dlg.SetInitInfo(pPswd->strInfo);
	dlg.SetInitPassword(pPswd->strPassword);
	dlg.SetInitDomain(pPswd->strDomain);
	dlg.SetPasswordType(m_dwType);
	dlg.SetHeader(_T("Edit password details"));
	dlg.SetTextOnOKButton(_T("Apply changes"));

	//show the user the password in the dialog
	if( IDOK == dlg.DoModal() )
	{
		//get the password from the dialog
		dlg.GetPassword(password);

		//remove the old password
		m_pCore->RemovePassword(&tmp);

		//pass on the password to the core
		m_pCore->AddPassword(&password);

		//refresh the display 
		DisplayPasswords();
	}

	m_pCore->CleanPassword(&tmp);

	//done
	return TRUE;
}

/*
	delete all selected passwords
*/
BOOL CPswdListCtrl::DeleteSelectedPasswords()
{
	int i=0;
	int selected_count = 0;
	note_password* pPswd = NULL;
	int res, n;

	//get how many were selected
	selected_count = GetSelectedCount();

	//if none selected, there is nothing to do
	if(0 == selected_count)
	{
		return FALSE;
	}

	//warn the user this action can not be undone
	res = AfxMessageBox( IDS_WARN_DELETE, MB_YESNO);

	//if the user changed his mind, bail out
	if(res != IDYES)
	{
		return FALSE;
	}

	//the user is sure, so get ready to delete all selected items
	n = GetItemCount();	

	//loop over all items in the control 
	for(i=n-1; i>=0; i--)
	{
		//is the item selected
		if( GetItemState(i, LVIS_SELECTED) )
		{
			//get the associated item data
			pPswd = (note_password*)GetItemData(i);
			//remove the item from the control and the underlying list
			m_pCore->RemovePassword(pPswd);
		}
	}

	//refresh the list
	DisplayPasswords();

	//done
	return TRUE;
}

/*
	copy the selected password to the clipboard
*/
BOOL CPswdListCtrl::CopySelectedPassword()
{
	int selected_count, n, i;
	note_password* pPswd = NULL;
	selected_count = GetSelectedCount();

	//we can only copy a single item
	if(1 != selected_count)
	{
		AfxMessageBox( IDS_SINGLE_ITEM );
		return FALSE;
	}

	//get ready to retrieve the selected item
	n = GetItemCount();	

	//loop over all items in the control 
	for(i=n-1; i>=0; i--)
	{
		//is the item selected
		if( GetItemState(i, LVIS_SELECTED) )
		{
			//get the associated item data
			pPswd = (note_password*)GetItemData(i);
		}
	}

	//copy the password to the clipboard (and wipe after a number of seconds)
	m_pCore->CopyStringToClipBoard( &pPswd->strPassword );
	
	//done
	return TRUE;
}

/*
	add a new password to the list by asking the user to fill in the details of the password
*/
BOOL CPswdListCtrl::AddPassword()
{
	note_password password;
	CNewPasswordDlg dlg;

	//set the current type of password 
	dlg.SetPasswordType(m_dwType);

	//attach the core 
	dlg.attach(m_pCore);

	//launch the window 
	if( IDOK == dlg.DoModal() )
	{
		//get the password from the dialog
		dlg.GetPassword(password);

		//pass on the password to the core
		m_pCore->AddPassword(&password);

		//add the new password to the display 
		DisplayPasswords();
	}

	return TRUE;
}

/*
	get the user selected password
*/
BOOL CPswdListCtrl::GetSelectedPassword( CString &strPassword )
{
	int selected_count, n, i;
	note_password* pPswd = NULL;

	//get the number of selected passwords
	selected_count = GetSelectedCount();

	//we only want one
	if(1 != selected_count)
	{
		AfxMessageBox( IDS_SINGLE_ITEM );
		return FALSE;
	}

	//get ready to find the selected password 
	n = GetItemCount();	

	//loop over all items in the control 
	for(i=n-1; i>=0; i--)
	{
		//is the item selected
		if( GetItemState(i, LVIS_SELECTED) )
		{
			//get the associated item data
			pPswd = (note_password*)GetItemData(i);
			strPassword = pPswd->strPassword;

			return TRUE;
		}
	}

	return FALSE;
}

/*
	set the type of use of this password list
*/
void CPswdListCtrl::SetUse( UINT32 dwUse )
{
	m_dwUse = dwUse;
}

/*
	add a reference to the parent
*/
void CPswdListCtrl::attachParent( CTabPasswords *pTab )
{
	m_tabParent = pTab;
}

/*
	get the id of the type-specific content displayed in the column
*/
int CPswdListCtrl::_get_column_content(int nColumn)
{
	switch(m_dwType)
	{
		case PSWD_FILE:
			return nColumn;

		case PSWD_MESSAGE:	
			return nColumn;

		case PSWD_CONTACT:
			return nColumn;

		case PSWD_ACCOUNT: 
			switch( nColumn )
			{
				case 0: return 0;
				case 1: return 1;
			    case 2: return 3;
				case 3: return 2;
			}

		default:
			return -1;
	}

}

/*
	callback function used to sort the columns in descending order
*/
int CALLBACK _sort_func_descending(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	int nRetVal=0;

	note_password* pPswd1 = (note_password*)lParam1;
	note_password* pPswd2 = (note_password*)lParam2;

	switch(lParamSort)
	{
	case 0:	// Date
		nRetVal = pPswd1->strDate.Compare( pPswd2->strDate );
		break;

	case 1: // Domain
		nRetVal = pPswd1->strDomain.Compare( pPswd2->strDomain );
		break;

	case 2: // Password
		nRetVal = pPswd1->strPassword.Compare( pPswd2->strPassword );
		break;

	case 3:	// Info
		nRetVal = pPswd1->strInfo.Compare( pPswd2->strInfo );
		break;

	default:
		break;
	}

	return (-nRetVal);
}

/*
	callback function used to sort the columns in ascending order
*/
int CALLBACK _sort_func_ascending(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	int nRetVal=0;

	note_password* pPswd1 = (note_password*)lParam1;
	note_password* pPswd2 = (note_password*)lParam2;

	switch(lParamSort)
	{
		case 0:	// Date
			nRetVal = pPswd1->strDate.Compare( pPswd2->strDate );
			break;

		case 1: // Domain
			nRetVal = pPswd1->strDomain.Compare( pPswd2->strDomain );
			break;

		case 2: // Password
			nRetVal = pPswd1->strPassword.Compare( pPswd2->strPassword );
			break;

		case 3:	// Info
			nRetVal = pPswd1->strInfo.Compare( pPswd2->strInfo );
			break;

		default:
			break;
	}

	return nRetVal;
}

/*
	sort the rows, depending which column header was clicked 
*/
void CPswdListCtrl::Sort( int nColumn )
{
	//get the index of which field of the password note
	//was indicated by the column header
	int content = _get_column_content(nColumn);

	//sort depending of the order they are (potentially) sorted in now
	if( m_bSortAscending[nColumn])
	{
		SortItems( _sort_func_ascending, content);
	}
	else
	{
		SortItems( _sort_func_descending, content);
	}

	//adjust the administration of the soring order
	m_bSortAscending[nColumn] ^= 1;
}

/*
	reset the flags keeping track of the order the columns are sorted in 
*/
void CPswdListCtrl::_reset_sort_order()
{
	memset(m_bSortAscending, 0x01, MAX_NUM_COLUMNS);
}


