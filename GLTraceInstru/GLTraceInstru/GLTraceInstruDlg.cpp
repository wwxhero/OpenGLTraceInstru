// GLTraceInstruDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GLTraceInstru.h"
#include "GLTraceInstruDlg.h"
#include "OpenGLApisDesc.h"
#include <ShObjIdl.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
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

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CGLTraceInstruDlg dialog




CGLTraceInstruDlg::CGLTraceInstruDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGLTraceInstruDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGLTraceInstruDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COLUMNTREE, m_columnTree);
}

BEGIN_MESSAGE_MAP(CGLTraceInstruDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTNDIR, &CGLTraceInstruDlg::OnBtnClickFilePathSel)
	ON_NOTIFY(NM_RCLICK, IDC_COLUMNTREE, &CGLTraceInstruDlg::OnRclickedColumntree)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CGLTraceInstruDlg)
    EASYSIZE(IDC_COLUMNTREE,ES_BORDER,ES_BORDER, ES_BORDER,ES_BORDER,0)
END_EASYSIZE_MAP


// CGLTraceInstruDlg message handlers

BOOL CGLTraceInstruDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
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

	// TODO: Add extra initialization here

	INIT_EASYSIZE;

	// set style for tree view
	UINT uTreeStyle = TVS_HASBUTTONS|TVS_HASLINES|TVS_LINESATROOT|TVS_CHECKBOXES;
	m_columnTree.GetTreeCtrl().ModifyStyle(0,uTreeStyle);

	InitTree();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CGLTraceInstruDlg::InitTree()
{
	/*
	 * Set background image (works with owner-drawn tree only)
	 */

	LVBKIMAGE bk;
	bk.xOffsetPercent = bk.yOffsetPercent = 70;
	bk.hbm = LoadBitmap(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDB_BKGND));
	m_columnTree.GetTreeCtrl().SetBkImage(&bk);

	/*
	 * Create image list for tree & load icons
	 */

    m_imgList.Create (16, 16, ILC_COLOR32|ILC_MASK,5,1);

    int id_Group = m_imgList.Add(AfxGetApp()->LoadIcon(IDI_MYCOMPUTER));
    int id_Item = m_imgList.Add(AfxGetApp()->LoadIcon(IDI_FIXEDDISK));

	// // assign image list to tree control
	m_columnTree.GetTreeCtrl().SetImageList(&m_imgList,TVSIL_NORMAL);

	/*
	 *  Insert columns to tree control
	 */
	CRect rc;
	m_columnTree.GetWindowRect(&rc);
	m_columnTree.InsertColumn(0, _T("OpenGL APIs Functions"), LVCFMT_LEFT, rc.Width());


	/*
	 *  Insert items
	 */

	HTREEITEM hRoot,hItem;
	CCustomTreeChildCtrl &ctrl = m_columnTree.GetTreeCtrl();
	for (int i_g = 0; i_g < sizeof(g_apiGroup)/sizeof(ApiGroup); i_g ++)
	{
		ApiGroup g = g_apiGroup[i_g];
		HTREEITEM hGroup = ctrl.InsertItem(g.szName, id_Group, id_Group);
		HTREEITEM hFunc = TVI_LAST;
		for (int i_func = g.i_left; i_func < g.i_right; i_func ++)
		{
			hFunc = ctrl.InsertItem(g_glApiNames[i_func], id_Item, id_Item, hGroup, hFunc);
		}
	}
}

void CGLTraceInstruDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CGLTraceInstruDlg::OnPaint()
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
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CGLTraceInstruDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CGLTraceInstruDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	UPDATE_EASYSIZE;

}

void CGLTraceInstruDlg::OnRclickedColumntree(LPNMHDR pNMHDR, LRESULT* pResult)
{
	CPoint pt;
	::GetCursorPos(&pt);

	m_columnTree.ScreenToClient(&pt);


	CTVHITTESTINFO htinfo;
	htinfo.pt = pt;
	HTREEITEM hItem = m_columnTree.HitTest(&htinfo);

	if(hItem)
	{
		CString szState;

		if(htinfo.flags&TVHT_ONITEMBUTTON)
			szState += _T("Clicked on item's button.");

		if(htinfo.flags&TVHT_ONITEMICON)
			szState += _T("Clicked on item's icon.");

		if(htinfo.flags&TVHT_ONITEMSTATEICON)
			szState += _T("Clicked on item's state icon.");

		if(htinfo.flags&TVHT_ONITEMINDENT)
			szState += _T("Clicked on item's indent.");

		if(htinfo.flags&TVHT_ONITEMLABEL)
			szState += _T("Clicked on item's label.");

		CString szItemText = m_columnTree.GetItemText(hItem, htinfo.iSubItem);
		MessageBox(szState + _T(" Item text: ") + szItemText);
	}

}

void CGLTraceInstruDlg::OnBtnClickFilePathSel()
{
	CFolderPickerDialog folderPickerDialog(NULL, OFN_FILEMUSTEXIST  | OFN_ENABLESIZING, this, sizeof(OPENFILENAME)); //fixme: | OFN_ALLOWMULTISELECT
	CWnd *pEdit = GetDlgItem(IDC_EDTSRCDIR);
	CString folders;
	if (folderPickerDialog.DoModal() == IDOK)
	{
		POSITION pos = folderPickerDialog.GetStartPosition();
		if (pos)
			folders = folderPickerDialog.GetNextPathName(pos);
		while (pos)
		{
			CString path = folderPickerDialog.GetNextPathName(pos);
			folders += ";";
			folders += path;
		}
	}
	pEdit->SetWindowText(folders);
}
