// GradientBox.cpp : implementation file
//

#include "../afx/stdafx.h"
#include "GradientBox.h"
#include <WinGDI.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ECLIPS_GRADIENTTEXT_BORDERSIZE_PIXELS	5
#define ECLIPS_GRADIENTTEXT_HEIGHT_PIXELS		20

/////////////////////////////////////////////////////////////////////////////
// CGradientBox

CGradientBox::CGradientBox()
{
	m_szTextInBox = "";
	m_nColors = 64;
	m_nFillType = fillVERTICAL_TOP;
	m_clrEnd = RGB(0,0,255);
//	m_clrStart = RGB(0xff,0xff,0xff);
	m_clrStart = ::GetSysColor( COLOR_BTNFACE );
	m_hIcon = NULL;
	m_istType = ICON_SIZE_UNKNOWN;
	m_iptType = ICON_POSITION_RIGHT;
	m_szIconSize = CSize(32,32);
}

CGradientBox::~CGradientBox()
{
	if(m_hIcon != NULL)
		::DestroyIcon( m_hIcon );
	m_hIcon = NULL;
}


BEGIN_MESSAGE_MAP(CGradientBox, CStatic)
	//{{AFX_MSG_MAP(CGradientBox)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGradientBox message handlers


void CGradientBox::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	//get rect
	CRect lpRect;
	GetClientRect(&lpRect);

	//fill rect
	fillGradientRect(&dc, &lpRect, m_clrStart, m_clrEnd);

	//////	ICON	/////////////////////
	UINT nIconSize = 0;
	
	//Create alpha channel icon
	//HCURSOR hCur = CreateAlphaICON(dc.m_hDC);

	if(m_nIConID != NULL)
	{
		nIconSize = addIcon(lpRect, &dc);
	}
	else 
	{
		if(m_strIconFile != "")
		{
			nIconSize = addIcon(lpRect, &dc);
		}
	}

	///////	END ICON ////////////////////

	

	//compress rect for text
	lpRect.DeflateRect(ECLIPS_GRADIENTTEXT_BORDERSIZE_PIXELS, ECLIPS_GRADIENTTEXT_BORDERSIZE_PIXELS);

	//calc text
	int nTextHeight = ECLIPS_GRADIENTTEXT_HEIGHT_PIXELS;
	CFont font;
	BOOL	fLoop = TRUE;
	CRect lpTextRect;
	lpTextRect.CopyRect(lpRect);

	//get text
	CString strTxt;
	strTxt = m_szTextInBox;//GetWindowText(strTxt);
	
	CFont* def_font = NULL;

	while (fLoop)
	{
		fLoop = FALSE;
		VERIFY(font.CreateFont(nTextHeight,
			0,                         // nWidth
			0,                         // nEscapement
			0,                         // nOrientation
			FW_NORMAL, //FW_BOLD,                   // nWeight
			FALSE,                     // bItalic
			FALSE,                     // bUnderline
			0,                         // cStrikeOut	   
			ANSI_CHARSET,              // nCharSet	   
			OUT_DEFAULT_PRECIS,        // nOutPrecision	   
			CLIP_DEFAULT_PRECIS,       // nClipPrecision	   
			DEFAULT_QUALITY,           // nQuality	   
			DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily	   
			_T("Arial")));                 // lpszFacename	  

		def_font = dc.SelectObject(&font);
		
		dc.DrawText(strTxt, -1,lpTextRect, DT_CALCRECT);
		
		int nRecWidth = lpRect.Width() - nIconSize;
		int nTextWidth = lpTextRect.Width();
		if (nTextWidth > nRecWidth)
		{
			fLoop = TRUE;
			nTextHeight -= 1;
			font.DeleteObject();
		}		
	}

	//placetext
	int nAlign = (m_nAlignType==alignLEFT)?DT_LEFT:(m_nAlignType==alignCENTER)?DT_CENTER:DT_RIGHT;

	dc.SetBkMode(TRANSPARENT);
	::SetTextColor(dc, RGB(0x52,0x8c,0xcc));
	

	lpRect.right = lpRect.right - nIconSize;
	lpRect.top += (lpRect.Height()-lpTextRect.Height())/2;
	if( m_fMultiLine )
	{
		dc.DrawText(strTxt , -1,lpRect, DT_WORDBREAK  | nAlign);		
	}
	else
	{
		dc.DrawText(strTxt , -1,lpRect, DT_SINGLELINE | DT_VCENTER | nAlign);		
	}

	//if replaced, set old font back
	if( def_font != NULL ) dc.SelectObject(def_font);

	font.DeleteObject();
}

void CGradientBox::fillGradientRect(CDC* pDc, CRect* pRect, COLORREF crStart, COLORREF crEnd)
{
	int nRed, nGreen, nBlue;
	float nStepRed, nStepGreen, nStepBlue;
	BOOL	fVertical;
	int nHeight = pRect->Height();
	int nWidth = pRect->Width();
	CRect rect;
	int nMax;
	int nLoop=0;


	//vertical?
	fVertical = (m_nFillType == fillVERTICAL_TOP) || (m_nFillType == fillVERTICAL_BOTTOM);

	
	//horizontal swap colors
	if ( !fVertical)
	{
		//swap colors
		COLORREF crD;
		crD = crEnd;
		crEnd = crStart;
		crStart = crD;
	}

	//calculate range for each colorchannel
	nStepRed = SubstractColor(GetRValue(crEnd),GetRValue(crStart));
	nStepGreen = SubstractColor(GetGValue(crEnd),GetGValue(crStart));
	nStepBlue = SubstractColor(GetBValue(crEnd),GetBValue(crStart));

	
	//precalculate the gradient range
	DWORD *pCR = new DWORD[m_nColors];
	for (nLoop=0; nLoop<m_nColors; nLoop++)
	{
		nRed = (int)(GetRValue(crStart) + (nStepRed * nLoop));
		nGreen = (int)(GetGValue(crStart) + (nStepGreen * nLoop));
		nBlue = (int)(GetBValue(crStart) + (nStepBlue * nLoop));
		pCR[nLoop] = (DWORD)RGB(nRed, nGreen, nBlue);
	}	

	//
	if (fVertical)	nMax = nWidth;
	else			nMax = nHeight;
	for (nLoop=0; nLoop < nMax; nLoop++)
	{
		//maak brush
		CBrush brush( (COLORREF)pCR[(nLoop*m_nColors)/nMax] );

		//maak rechthoek
		if ( fVertical)	rect.SetRect(nLoop, 0, nLoop+1, nHeight);
		else			rect.SetRect(0, nLoop, nWidth, nLoop+1);

		//vul rechthoek met brush
		pDc->FillRect(&rect, &brush);
	}

	//delete gradientrange
	delete pCR;
}

float CGradientBox::SubstractColor(int nStart, int nEnd)
{
	//for a gradient consist of 4 colors you need 3 steps, so minus 1
	return ((float)(nStart - nEnd))/(float)(m_nColors-1.0);
}

void CGradientBox::SetWindowText(CString szText, int nAlign, BOOL fMultiLine)
{
	m_szTextInBox = szText;
	m_fMultiLine = fMultiLine;
//	UpdateData(false);
	m_nAlignType = nAlign;
	Invalidate();
}
void CGradientBox::setColorSteps(int nSteps)
{
	m_nColors = nSteps;
}

void CGradientBox::setFillType(int nFillType)
{
	m_nFillType = nFillType;
	Invalidate();
}

void CGradientBox::show(COLORREF clrStart, COLORREF clrEnd)
{
	m_clrStart = clrStart;
	m_clrEnd = clrEnd;
	Invalidate();
}

CSize CGradientBox::getIconSize(ICONSIZETYPES isIconSize)
{
	switch (isIconSize)
	{
	case ICON_SIZE_DEFAULT: return CSize(0, 0); 
	case ICON_SIZE_16: return CSize(16, 16); 
	case ICON_SIZE_24: return CSize(24, 24); 
	case ICON_SIZE_32: return CSize(32, 32); 
	case ICON_SIZE_48: return CSize(48, 48); 
	case ICON_SIZE_64: return CSize(64, 64); 
	case ICON_SIZE_128: return CSize(128, 128); 
	case ICON_SIZE_256: return CSize(256, 256); 

	default: 
	case ICON_SIZE_UNKNOWN: return m_szIconSize;
	}
}
void CGradientBox::setIcon( int nIconID )
{
	HINSTANCE hInst = AfxGetInstanceHandle();
	m_nIConID = nIconID;
	m_hIcon = (HICON)::LoadImage(hInst, MAKEINTRESOURCE(m_nIConID), IMAGE_ICON, getIconSize(m_istType).cx , getIconSize(m_istType).cy ,  LR_CREATEDIBSECTION ); 
}

void CGradientBox::setIcon( CString strIconFile )
{
	HINSTANCE hInst = AfxGetInstanceHandle();
	m_hIcon = (HICON)::LoadImage(hInst, strIconFile, IMAGE_ICON, getIconSize(m_istType).cx , getIconSize(m_istType).cy , LR_LOADFROMFILE | LR_CREATEDIBSECTION); 
}

UINT CGradientBox::addIcon( CRect &lpRect, CDC* pDc )
{
	UINT nPos = 0;
	CSize szIconSize = CSize(0,0);
	if (m_hIcon)
	{

		//Get icon size
		szIconSize = getIconSize( m_istType );

		//calculate position
		CPoint position = calculateIconPosition(&lpRect, szIconSize, m_iptType);

		//if ICON set not to right nPos is 0
		if((m_iptType != ICON_POSITION_RIGHT) && (m_iptType != ICON_POSITION_RIGHTBOTTOM) && (m_iptType != ICON_POSITION_RIGHTTOP))
			nPos = 0;
		else
		{
			//if ICON set to right calculate border size so picture is in the middle
			UINT nRight = lpRect.Width() - position.x;
			UINT nIconBorder = nRight - szIconSize.cx;
			nPos = szIconSize.cx + (nIconBorder * 2);
		}

		//Draw Icon
		pDc->DrawState(	position, szIconSize, m_hIcon, DSS_NORMAL, (CBrush*)NULL);
		
		//Destroy Icon
		//::DestroyIcon(m_hIcon);
	}
	return nPos;
}

CPoint CGradientBox::calculateIconPosition(CRect *lpRect, CSize szIconSize, ICONPOSITIONTYPES IconPositionType)
{
	CPoint position;
	UINT nWidth = lpRect->Width();
	UINT nHeight = lpRect->Height();

	switch (IconPositionType)
	{
	case ICON_POSITION_CENTER:
		{
			position.x = (nWidth - szIconSize.cx) / 2;
			position.y = (nHeight - szIconSize.cy) / 2;		
		}
		break;
	case ICON_POSITION_CENTERTOP:
		{
			position.x = (nWidth - szIconSize.cx) / 2;
			position.y = (lpRect->top);		
		}
		break;
	case ICON_POSITION_CENTERBOTTOM:
		{
			position.x = (nWidth - szIconSize.cx) / 2;
			position.y = (lpRect->bottom - szIconSize.cy);		
		}
		break;
	case ICON_POSITION_LEFT:
		{
			position.x = (lpRect->left + (nHeight - szIconSize.cy) /2);
			position.y = (nHeight - szIconSize.cy) /2;		
		}
		break;
	case ICON_POSITION_LEFTTOP:
		{
			position.x = (lpRect->left);
			position.y = (lpRect->top);		
		}
		break;
	case ICON_POSITION_LEFTBOTTOM:
		{
			position.x = (lpRect->left);
			position.y = (lpRect->bottom - szIconSize.cy);		
		}
		break;
	case ICON_POSITION_RIGHT:
		{
			//calc x			
			UINT nPosX = (lpRect->right - szIconSize.cx);
			UINT nPosY = 0;
			//adjust the icon to the center
			nPosX -= ((nHeight - szIconSize.cy)/2);
			nPosY = (nHeight - szIconSize.cy)/2;
			//added a little offset (2px) for the icon.
			nPosX -= 2;
			//set pos
			position.x = nPosX;
			position.y = nPosY;
		}
		break;
	case ICON_POSITION_RIGHTTOP:
		{
			position.x = (lpRect->right - szIconSize.cx);
			position.y = (lpRect->top);		
		}
		break;
	case ICON_POSITION_RIGHTBOTTOM	:
		{
			position.x = (lpRect->right - szIconSize.cx);
			position.y = (lpRect->bottom - szIconSize.cy);		
		}
		break;

		
	}	

	return position;
}

//Deze functie kan worden gebruikt voor het automatisch laten berekenen van de juiste icon size. 
//Dit kan alleen aan de hand van een ico.file dus NIET vanuit de resource
CSize CGradientBox::calculateIconSize(CRect *lpRect)
{
	LPICONDIRENTRY    	lpIDE = NULL;
	LPICONRESOURCE    	lpIR = NULL, lpNew = NULL;
	DWORD dwBytesRead;
	UINT i = 0;
	HANDLE hFile1 = NULL;

	if( (hFile1 = CreateFile( TEXT("C:\\test2.ico"), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL )) == INVALID_HANDLE_VALUE )
	{
		//AfxMessageBox("Error Opening File for Reading");
		return NULL;
	}

	if( (lpIR = (LPICONRESOURCE)malloc( sizeof(ICONRESOURCE) )) == NULL )
	{
		//AfxMessageBox("Error Allocating Memory");
		CloseHandle( hFile1 );
		return NULL;
	}
	// Read in the header
	if( (lpIR->nNumImages = ReadICOHeader( hFile1 )) == (UINT)-1 )
	{
		//AfxMessageBox("Error Reading File Header");
		CloseHandle( hFile1 );
		free( lpIR );
		return NULL;
	}
	// Adjust the size of the struct to account for the images
	if( (lpNew = (LPICONRESOURCE)realloc( lpIR, sizeof(ICONRESOURCE) + ((lpIR->nNumImages-1) * sizeof(ICONIMAGE2)) )) == NULL )
	{
		//AfxMessageBox("Error Allocating Memory");
		CloseHandle( hFile1 );
		free( lpIR );
		return NULL;
	}
	lpIR = lpNew;

	// Allocate enough memory for the icon directory entries
	if( (lpIDE = (LPICONDIRENTRY)malloc( lpIR->nNumImages * sizeof( ICONDIRENTRY ) ) ) == NULL )
	{
	    //AfxMessageBox("Error Allocating Memory");
		CloseHandle( hFile1 );
		free( lpIR );
		return NULL;
	}

	if( ! ReadFile( hFile1, lpIDE, lpIR->nNumImages * sizeof( ICONDIRENTRY ), &dwBytesRead, NULL ) )
	{
		//AfxMessageBox("Error Reading File");
		CloseHandle( hFile1 );
		free( lpIR );
		return NULL;
	}
	if( dwBytesRead != lpIR->nNumImages * sizeof( ICONDIRENTRY ) )
	{
		//AfxMessageBox("Error Reading File");
		CloseHandle( hFile1 );
		free( lpIR );
		return NULL;
	}

	TRACE("Aantal Images: %d\n", lpIR->nNumImages);
	// Loop through and read in each image
	for( i = 0; i < lpIR->nNumImages; i++ )
	{
		// Allocate memory for the resource
		if( (lpIR->IconImages[i].lpBits = (LPBYTE)malloc(lpIDE[i].dwBytesInRes)) == NULL )
		{
			//AfxMessageBox("Error allocating memory");
			CloseHandle( hFile1 );
			free( lpIR );
			free( lpIDE );
			return NULL;
		}
		lpIR->IconImages[i].dwNumBytes = lpIDE[i].dwBytesInRes;
		// Seek to beginning of this image
		if( SetFilePointer( hFile1, lpIDE[i].dwImageOffset, NULL, FILE_BEGIN ) == 0xFFFFFFFF )
		{
			//AfxMessageBox("Error Seeking in File");
			CloseHandle( hFile1 );
			free( lpIR );
			free( lpIDE );
			return NULL;
		}
		// Read it in
		if( ! ReadFile( hFile1, lpIR->IconImages[i].lpBits, lpIDE[i].dwBytesInRes, &dwBytesRead, NULL ) )
		{
			//AfxMessageBox("Error Reading File");
			CloseHandle( hFile1 );
			free( lpIR );
			free( lpIDE );
			return NULL;
		}
		if( dwBytesRead != lpIDE[i].dwBytesInRes )
		{
			//AfxMessageBox("Error Reading File");
			CloseHandle( hFile1 );
			free( lpIDE );
			free( lpIR );
			return NULL;
		}

		// BITMAPINFO is at beginning of bits
		lpIR->IconImages[i].lpbi = (LPBITMAPINFO)lpIR->IconImages[i].lpBits;

		lpIR->IconImages[i].Width = lpIR->IconImages[i].lpbi->bmiHeader.biWidth;

		// Icons are stored in funky format where height is doubled - account for it
		lpIR->IconImages[i].Height = (lpIR->IconImages[i].lpbi->bmiHeader.biHeight) / 2;
		
		// How many colors?
		lpIR->IconImages[i].Colors = lpIR->IconImages[i].lpbi->bmiHeader.biPlanes * lpIR->IconImages[i].lpbi->bmiHeader.biBitCount;

	}

	//get biggest icon size for rect
	CSize szReturn;
	UINT nSmallest;
	if(lpRect->Width() < lpRect->Height())
		nSmallest = lpRect->Width();
	else
		nSmallest = lpRect->Height();

	UINT nSize = 0;
	for( i = 0; i < lpIR->nNumImages; i++ )
	{
		if(lpIR->IconImages[i].Width < nSmallest)
			if(lpIR->IconImages[i].Width > nSize)
				nSize = lpIR->IconImages[i].Width;
	}
	TRACE("ICON SIZE : Width %d Height %d\n", nSize, nSize);
	szReturn = CSize(nSize, nSize);


	// Clean up	
	free( lpIDE );
	CloseHandle( hFile1 );

	for ( i = 0; i < lpIR->nNumImages; i++ )
	{
		free(lpIR->IconImages[i].lpBits);
	}
	free(lpIR);

	return szReturn;
}


UINT CGradientBox::ReadICOHeader( HANDLE hFile )
{
	WORD    Input;
	DWORD	dwBytesRead;

	// Read the 'reserved' WORD
	if( ! ReadFile( hFile, &Input, sizeof( WORD ), &dwBytesRead, NULL ) )
		return (UINT)-1;
	// Did we get a WORD?
	if( dwBytesRead != sizeof( WORD ) )
		return (UINT)-1;
	// Was it 'reserved' ?   (ie 0)
	if( Input != 0 )
		return (UINT)-1;
	// Read the type WORD
	if( ! ReadFile( hFile, &Input, sizeof( WORD ), &dwBytesRead, NULL ) )
		return (UINT)-1;
	// Did we get a WORD?
	if( dwBytesRead != sizeof( WORD ) )
		return (UINT)-1;
	// Was it type 1?
	if( Input != 1 )
		return (UINT)-1;
	// Get the count of images
	if( ! ReadFile( hFile, &Input, sizeof( WORD ), &dwBytesRead, NULL ) )
		return (UINT)-1;
	// Did we get a WORD?
	if( dwBytesRead != sizeof( WORD ) )
		return (UINT)-1;
	// Return the count
	return Input;
}


HCURSOR CGradientBox::CreateAlphaICON(HDC pDC)
{
	HDC hMemDC;
	DWORD dwWidth, dwHeight;
	BITMAPV5HEADER bi;
	HBITMAP hBitmap, hOldBitmap;
	void *lpBits;
	DWORD x,y;
	HCURSOR hAlphaCursor = NULL;

	dwWidth  = 60;  // width of cursor
	dwHeight = 60;  // height of cursor

	ZeroMemory(&bi,sizeof(BITMAPV5HEADER));
	bi.bV5Size           = sizeof(BITMAPV5HEADER);
	bi.bV5Width           = dwWidth;
	bi.bV5Height          = dwHeight;
	bi.bV5Planes = 1;
	bi.bV5BitCount = 32;
	bi.bV5Compression = BI_BITFIELDS;
	// The following mask specification specifies a supported 32 BPP
	// alpha format for Windows XP.
	bi.bV5RedMask   =  0x00FF0000;
	bi.bV5GreenMask =  0x0000FF00;
	bi.bV5BlueMask  =  0x000000FF;
	bi.bV5AlphaMask =  0xFF000000; 

	//HDC hdc;
	//hdc = GetDC()->m_hDC;

	// Create the DIB section with an alpha channel.
	hBitmap = CreateDIBSection(pDC, (BITMAPINFO *)&bi, DIB_RGB_COLORS, (void **)&lpBits, NULL, (DWORD)0);

	hMemDC = CreateCompatibleDC(pDC);
//	ReleaseDC(&hdc);

	// Draw something on the DIB section.
	hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
	PatBlt(hMemDC,0,0,dwWidth,dwHeight,WHITENESS);
	SetTextColor(hMemDC,RGB(0,0,0));
	SetBkMode(hMemDC,TRANSPARENT);
	TextOut(hMemDC,0,9,_T("rgba"),4);
	SelectObject(hMemDC, hOldBitmap);
	DeleteDC(hMemDC);

	// Create an empty mask bitmap.
	HBITMAP hMonoBitmap = CreateBitmap(dwWidth,dwHeight,1,1,NULL);

	// Set the alpha values for each pixel in the cursor so that
	// the complete cursor is semi-transparent.
	DWORD *lpdwPixel;
	lpdwPixel = (DWORD *)lpBits;
	for (x=0;x<dwWidth;x++)
		for (y=0;y<dwHeight;y++)
		{
			// Clear the alpha bits
			*lpdwPixel &= 0x00FFFFFF;
			// Set the alpha bits to 0x9F (semi-transparent)
			*lpdwPixel |= 0x9F000000;
			lpdwPixel++;
		}

		ICONINFO ii;
		ii.fIcon = TRUE;  // Change fIcon to TRUE to create an alpha icon
		ii.xHotspot = 0;
		ii.yHotspot = 0;
		ii.hbmMask = hMonoBitmap;
		ii.hbmColor = hBitmap;

		// Create the alpha cursor with the alpha DIB section.
		//hAlphaCursor = CreateIconIndirect(&ii);
		HINSTANCE hInst;
		hInst = AfxGetResourceHandle();
		hAlphaCursor = (HICON)::LoadImage(hInst, MAKEINTRESOURCE(m_nIConID), IMAGE_ICON, NULL , NULL,  LR_CREATEDIBSECTION | LR_DEFAULTSIZE );
		DeleteObject(hBitmap);          
		DeleteObject(hMonoBitmap); 

		return hAlphaCursor;
}
