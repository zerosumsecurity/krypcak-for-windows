#if !defined(AFX_GRADIENTBOX_H__05945C55_EE08_11D4_A3CF_00E07D76F21B__INCLUDED_)
#define AFX_GRADIENTBOX_H__05945C55_EE08_11D4_A3CF_00E07D76F21B__INCLUDED_

#include "../afx/stdafx.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GradientBox.h : header file
//

#define FILL_VERTICAL	0
#define FILL_HORIZONTAL	1

/////////////////////////////////////////////////////////////////////////////
// CGradientBox window

class CGradientBox : public CStatic
{
// Construction
public:
	CGradientBox();
	virtual ~CGradientBox();



// Attributes
public:
	enum FILLTYPES{
		fillVERTICAL_TOP=0, 
		fillVERTICAL_BOTTOM,
		fillHORIZONTAL_LEFT,
		fillHORIZONTAL_RIGHT
	};

	enum ALIGNTYPES{
		alignLEFT = 0,
		alignCENTER,
		alignRIGHT
	};
	
	enum ICONPOSITIONTYPES{
		ICON_POSITION_CENTER=0,
		ICON_POSITION_CENTERTOP,
		ICON_POSITION_CENTERBOTTOM,
		ICON_POSITION_LEFT,
		ICON_POSITION_LEFTTOP,
		ICON_POSITION_LEFTBOTTOM,
		ICON_POSITION_RIGHT,
		ICON_POSITION_RIGHTTOP,
		ICON_POSITION_RIGHTBOTTOM
	};

	enum ICONSIZETYPES{
		ICON_SIZE_UNKNOWN, /*use csize*/
		ICON_SIZE_DEFAULT, /* Get his own ICON SIZE */
		ICON_SIZE_MAX, /* Make as big as rectangle */
		ICON_SIZE_16, /* 16x16 */
		ICON_SIZE_24, /* 246x24 */
		ICON_SIZE_32, /* 326x32 */
		ICON_SIZE_48, /* 48x48 */
		ICON_SIZE_64, /* 64x64 */
		ICON_SIZE_128, /* 128x128 */
		ICON_SIZE_256 /* 256x256 */
	};


// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGradientBox)
	//}}AFX_VIRTUAL

// Implementation
public:
	void	SetWindowText(CString szText, int nAlignment = alignRIGHT, BOOL fMultiLine = false);
	
	void	setColorSteps(int nSteps = 128);
	
	void	setFillType(int nFillType = fillVERTICAL_TOP);
	
	void	show(COLORREF clrStart, COLORREF clrEnd);

	void    setIconSize(ICONSIZETYPES istType = ICON_SIZE_32){ m_istType = istType; }
	void	setIconSize(CSize szIconSize){m_szIconSize = szIconSize;}
	void	setIconPosition(ICONPOSITIONTYPES iptType = ICON_POSITION_CENTER){m_iptType = iptType;}
	void	setIcon( int nIconID );
	void	setIcon( CString strIconFile );

	HICON	m_hIcon;
	int		m_nIConID;
	CString m_strIconFile;
	BOOL	m_fMultiLine;


	// Generated message map functions
protected:
	//{{AFX_MSG(CGradientBox)
	afx_msg void OnPaint();


	//}}AFX_MSG
	ICONSIZETYPES m_istType;
	CSize m_szIconSize;
	ICONPOSITIONTYPES m_iptType;

	HCURSOR CreateAlphaICON(HDC pDC);
	CSize	getIconSize(ICONSIZETYPES isIconSize);
	UINT	addIcon( CRect &lpRect, CDC* pDc);
	CSize	calculateIconSize(CRect *lpRect );
	UINT	ReadICOHeader( HANDLE hFile );	
	CPoint	calculateIconPosition(CRect *lpRect, CSize szIconSize, ICONPOSITIONTYPES IconPositionType);

	void	fillGradientRect(CDC* dc, CRect* lpRect,COLORREF m_clrStart, COLORREF m_clrEnd);
	float	SubstractColor(int, int);


private:
	CString		m_szTextInBox;
	COLORREF	m_clrStart;
	COLORREF	m_clrEnd;
	int			m_nColors;
	int			m_nFillType;
	int			m_nAlignType;

	DECLARE_MESSAGE_MAP()

};

typedef struct
{
	BYTE        bWidth;          // Width, in pixels, of the image
	BYTE        bHeight;         // Height, in pixels, of the image
	BYTE        bColorCount;     // Number of colors in image (0 if >=8bpp)
	BYTE        bReserved;       // Reserved ( must be 0)
	WORD        wPlanes;         // Color Planes
	WORD        wBitCount;       // Bits per pixel
	DWORD       dwBytesInRes;    // How many bytes in this resource?
	DWORD       dwImageOffset;   // Where in the file is this image?
} ICONDIRENTRY, *LPICONDIRENTRY;

typedef struct
{
	WORD           idReserved;   // Reserved (must be 0)
	WORD           idType;       // Resource Type (1 for icons)
	WORD           idCount;      // How many images?
	ICONDIRENTRY   idEntries[1]; // An entry for each image (idCount of 'em)
} ICONDIR, *LPICONDIR;

typedef struct
{
	BITMAPINFOHEADER   icHeader;      // DIB header
	RGBQUAD         icColors[1];   // Color table
	BYTE            icXOR[1];      // DIB bits for XOR mask
	BYTE            icAND[1];      // DIB bits for AND mask
} ICONIMAGE, *LPICONIMAGE;

typedef struct
{
	UINT			Width;
	UINT			Height;
	UINT			Colors; // Width, Height and bpp
	LPBYTE			lpBits;                // ptr to DIB bits
	DWORD			dwNumBytes;            // how many bytes?
	LPBITMAPINFO	lpbi;                  // ptr to header
	LPBYTE			lpXOR;                 // ptr to XOR image bits
	LPBYTE			lpAND;                 // ptr to AND image bits
} ICONIMAGE2, *LPICONIMAGE2;

typedef struct
{
	BOOL		bHasChanged;                     // Has image changed?
	TCHAR		szOriginalICOFileName[MAX_PATH]; // Original name
	TCHAR		szOriginalDLLFileName[MAX_PATH]; // Original name
	UINT		nNumImages;                      // How many images?
	ICONIMAGE2	IconImages[1];                   // Image entries
} ICONRESOURCE, *LPICONRESOURCE;




/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRADIENTBOX_H__05945C55_EE08_11D4_A3CF_00E07D76F21B__INCLUDED_)
