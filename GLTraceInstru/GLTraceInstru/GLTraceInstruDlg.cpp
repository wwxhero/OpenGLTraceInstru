// GLTraceInstruDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GLTraceInstru.h"
#include "GLTraceInstruDlg.h"
#include <afxpriv.h>
#include <ShObjIdl.h>
#include <string>
#include <queue>
#include "GLTraceInjector.h"
#include "PatternMatch.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MAKE_STRING(r)\
	r[0], r[1]-r[0]

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
	const char* c_cpp_suffix[] = {".h", ".hpp", ".c", ".cpp", ".cxx"};
	for (int i = 0; i < sizeof(c_cpp_suffix) / sizeof(const char*); i ++)
	{
		CString str(c_cpp_suffix[i]);
		c_setFileFilters.insert(str);
	}
}

void CGLTraceInstruDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COLUMNTREE, m_columnTree);
	DDX_Control(pDX, IDC_PROGINJECTION, m_progCtrl);
}

BEGIN_MESSAGE_MAP(CGLTraceInstruDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
	ON_BN_CLICKED(IDC_BTNLOGGERFUNCGEN, &CGLTraceInstruDlg::OnBtnClickLoggerPathSel)
	ON_BN_CLICKED(IDC_BTNGLHEADER, &CGLTraceInstruDlg::OnBtnClickHeaderSel)
	ON_BN_CLICKED(IDC_BTNSRCDIR_INJECT, &CGLTraceInstruDlg::OnBtnClickFilePathSel)
	ON_BN_CLICKED(IDC_BTNINJECT, &CGLTraceInstruDlg::OnBtnClickStartInjection)
	ON_NOTIFY(NM_RCLICK, IDC_COLUMNTREE, &CGLTraceInstruDlg::OnRclickedColumntree)
	ON_UPDATE_COMMAND_UI_RANGE(IDC_EDTSRCDIR_LOGGER, IDC_BTNINJECT, &CGLTraceInstruDlg::OnUpdateBtns)
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CGLTraceInstruDlg)
EASYSIZE(IDC_COLUMNTREE, ES_BORDER, ES_BORDER, ES_BORDER, ES_BORDER, 0)
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
	UINT uTreeStyle = TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_CHECKBOXES;
	m_columnTree.GetTreeCtrl().ModifyStyle(0, uTreeStyle);

	InitTree();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CGLTraceInstruDlg::ClearTree()
{
	CTreeCtrl& treeCtrl = m_columnTree.GetTreeCtrl();
	HTREEITEM h_node = treeCtrl.GetChildItem(TVI_ROOT);
	std::queue<HTREEITEM> q_n;
	while (h_node != NULL)
	{
		// Try to get the next item
		q_n.push(h_node);
		h_node = treeCtrl.GetNextItem(h_node, TVGN_NEXT);
	}

	while (!q_n.empty())
	{
		HTREEITEM h_parent = q_n.front();
		q_n.pop();

		HTREEITEM h_child = treeCtrl.GetChildItem(h_parent);
		while (NULL != h_child)
		{
			// Try to get the next item
			TVITEM item;
			item.hItem = h_child;
			item.mask =  TVIF_HANDLE | TVIF_PARAM;
			treeCtrl.GetItem(&item);
			ItemFunc* func = (ItemFunc *)item.lParam;
			delete func;

			q_n.push(h_child);
			h_child = treeCtrl.GetNextItem(h_child, TVGN_NEXT);
		}
	}
	m_columnTree.GetTreeCtrl().DeleteAllItems();
}

void CGLTraceInstruDlg::UpdateTree(const ItemGroups* g)
{
	ClearTree();
	/*
	 *  Insert items
	 */

	HTREEITEM hRoot, hItem;
	CCustomTreeChildCtrl &ctrl = m_columnTree.GetTreeCtrl();
	for (ItemGroups::const_iterator it = g->begin(); it != g->end(); it ++)
	{
		ItemGroup item_g = *it;

		HTREEITEM hGroup = ctrl.InsertItem(item_g.first.c_str(), m_idGroup, m_idGroup);
		HTREEITEM hFunc = TVI_LAST;
		const std::list<ItemFunc*>& funcs = item_g.second;
		for (std::list<ItemFunc*>::const_iterator it = funcs.begin()
			; it != funcs.end()
			; it ++)
		{
			ItemFunc* func = *it;
			hFunc = ctrl.InsertItem(TVIF_TEXT|TVIF_PARAM|TVIF_IMAGE, func->name.c_str(), m_idItem, m_idItem, 0, 0, (LPARAM)func, hGroup, hFunc);
		}
	}
}

void CGLTraceInstruDlg::InitTree()
{
	/*
	 * Set background image (works with owner-drawn tree only)
	 */

	LVBKIMAGE bk;
	bk.xOffsetPercent = bk.yOffsetPercent = 70;
	bk.hbm = LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BKGND));
	m_columnTree.GetTreeCtrl().SetBkImage(&bk);

	/*
	 * Create image list for tree & load icons
	 */

	m_imgList.Create (16, 16, ILC_COLOR32 | ILC_MASK, 5, 1);

	m_idGroup = m_imgList.Add(AfxGetApp()->LoadIcon(IDI_MYCOMPUTER));
	m_idItem = m_imgList.Add(AfxGetApp()->LoadIcon(IDI_FIXEDDISK));

	// // assign image list to tree control
	m_columnTree.GetTreeCtrl().SetImageList(&m_imgList, TVSIL_NORMAL);

	/*
	 *  Insert columns to tree control
	 */
	CRect rc;
	m_columnTree.GetWindowRect(&rc);
	m_columnTree.InsertColumn(0, _T("OpenGL APIs Functions"), LVCFMT_LEFT, rc.Width());

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

	if (hItem)
	{
		CString szState;

		if (htinfo.flags & TVHT_ONITEMBUTTON)
			szState += _T("Clicked on item's button.");

		if (htinfo.flags & TVHT_ONITEMICON)
			szState += _T("Clicked on item's icon.");

		if (htinfo.flags & TVHT_ONITEMSTATEICON)
			szState += _T("Clicked on item's state icon.");

		if (htinfo.flags & TVHT_ONITEMINDENT)
			szState += _T("Clicked on item's indent.");

		if (htinfo.flags & TVHT_ONITEMLABEL)
			szState += _T("Clicked on item's label.");

		CString szItemText = m_columnTree.GetItemText(hItem, htinfo.iSubItem);
		MessageBox(szState + _T(" Item text: ") + szItemText);
	}

}


void CGLTraceInstruDlg::OnBtnClickHeaderSel()
{
	// szFilters is a text string that includes two file name filters:
	// "*.my" for "MyType Files" and "*.*' for "All Files."
	TCHAR szFilters[] = _T("GL Header File |*.h;*.hpp|All Files (*.*)|*.*||");

	// Create an Open dialog; the default file name extension is ".my".
	CFileDialog fileDlg(TRUE, _T("h"), _T("gl3.h"),
	                    OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilters);

	// Display the file dialog. When user clicks OK, fileDlg.DoModal()
	// returns IDOK.
	if (fileDlg.DoModal() == IDOK)
	{
		CString pathName = fileDlg.GetPathName();
		CWnd *pWnd = GetDlgItem(IDC_EDTGLHEADER);
		pWnd->SetWindowText(pathName);
		std::list<Func*> lstFuncsGLHeader;
		MemSrc	 memGLHeader;

		StartParse4Funcs(pathName, &memGLHeader, lstFuncsGLHeader);
		FixMatch m_void(FIX_MATCH_CONSTRU("void"));
		int n_void = 0;
		int n_unvoid = 0;
		ItemGroups groups;
		for (std::list<Func*>::const_iterator it = lstFuncsGLHeader.begin()
		        ; it != lstFuncsGLHeader.end()
		        ; it ++)
		{
			Func* func = *it;
			const char* ret_l = func->retType[0];
			const char* ret_r = func->retType[1];
			bool b_void = m_void.Match(ret_l, ret_r);
			if (b_void)
				n_void ++;
			else
				n_unvoid ++;
			ItemFunc* itemFunc = new ItemFunc(std::string(MAKE_STRING(func->funcName)), !b_void);
			bool valid_version = (func->version[0] < func->version[1]);
			std::string str_version = valid_version ? std::string(MAKE_STRING(func->version)) : "Unknown";
			groups[str_version].push_back(itemFunc);
		}

		CString parseInfo;
		parseInfo.Format(_T("void functions:%d  unvoid functions:%d"), n_void, n_unvoid);
		SetWindowText(parseInfo);

		UpdateTree(&groups);
#ifdef TEST_START_PARSE
		for (ItemGroups::iterator it = groups.begin()
			; it != groups.end()
			; it ++)
		{
			ItemGroup g = *it;
			ATLTRACE(_T("%s:\n"), g.first.c_str());
			const std::list<ItemFunc *>& funcs = g.second;
			for (std::list<ItemFunc*>::const_iterator it = funcs.begin()
				; it != funcs.end()
				; it ++)
			{
				ItemFunc* func = *it;
				ATLTRACE(_T("\t%s\n"), func->name.c_str());
			}
		}
#endif

		CString funcPath;
		CWnd *edt_path_logger = GetDlgItem(IDC_EDTSRCDIR_LOGGER);
		edt_path_logger->GetWindowText(funcPath);
		CString declarePath;
		declarePath.Format("%s\\traceGlFuncs.h", funcPath);
		GenFuncsDecl(declarePath, &memGLHeader, lstFuncsGLHeader);
		CString definiPath;
		definiPath.Format("%s\\traceGlFuncs.cpp", funcPath);
		GenFuncsImpl(definiPath, &memGLHeader, lstFuncsGLHeader);

		EndParse4Funcs(&memGLHeader, lstFuncsGLHeader);

	}
}

void CGLTraceInstruDlg::OnDestroy()
{
	ClearTree();
	CDialog::OnDestroy();
}

void CGLTraceInstruDlg::OnUpdateBtns(CCmdUI* cmdUI)
{
	struct Entry
	{
		unsigned int cmdId;
		unsigned int depId;
	} entries [] = {
		{IDC_BTNGLHEADER, IDC_EDTSRCDIR_LOGGER}
		, {IDC_BTNSRCDIR_INJECT, IDC_EDTGLHEADER}
		, {IDC_BTNINJECT, IDC_EDTSRCDIR_INJECT}
	};

	for (int i_entry = 0; i_entry < sizeof(entries)/sizeof(Entry); i_entry ++)
	{
		if (entries[i_entry].cmdId == cmdUI->m_nID)
		{
			CWnd *pEdt = GetDlgItem(entries[i_entry].depId);
			CString strPath;
			pEdt->GetWindowText(strPath);
			cmdUI->Enable(!strPath.IsEmpty());
			break;
		}
	}
}

LRESULT CGLTraceInstruDlg::OnKickIdle(WPARAM, LPARAM)
{
    UpdateDialogControls(this, FALSE);
    return 0;
}

void CGLTraceInstruDlg::OnBtnClickLoggerPathSel()
{
	CFolderPickerDialog folderPickerDialog(NULL, OFN_FILEMUSTEXIST  | OFN_ENABLESIZING, this, sizeof(OPENFILENAME)); //fixme: | OFN_ALLOWMULTISELECT
	CWnd *pEdit = GetDlgItem(IDC_EDTSRCDIR_LOGGER);
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

void CGLTraceInstruDlg::OnBtnClickFilePathSel()
{
	CFolderPickerDialog folderPickerDialog(NULL, OFN_FILEMUSTEXIST  | OFN_ENABLESIZING, this, sizeof(OPENFILENAME)); //fixme: | OFN_ALLOWMULTISELECT
	CWnd *pEdit = GetDlgItem(IDC_EDTSRCDIR_INJECT);
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


void CGLTraceInstruDlg::OnBtnClickStartInjection()
{
	std::set<std::string> void_tokens;
	std::set<std::string> unvoid_tokens;
	std::set<std::string> sync_tokens;
	CTreeCtrl& treeCtrl = m_columnTree.GetTreeCtrl();
	HTREEITEM h_node = treeCtrl.GetChildItem(TVI_ROOT);
	std::queue<HTREEITEM> q_n;
	while (h_node != NULL)
	{
		// Get the text for the item. Notice we use TVIF_TEXT because
		// we want to retrieve only the text, but also specify TVIF_HANDLE
		// because we're getting the item by its handle.
		// Try to get the next item
		q_n.push(h_node);
		h_node = treeCtrl.GetNextItem(h_node, TVGN_NEXT);
	}

	while (!q_n.empty())
	{
		HTREEITEM h_parent = q_n.front();
		q_n.pop();
		HTREEITEM h_child = treeCtrl.GetChildItem(h_parent);
		while (NULL != h_child)
		{
			if (treeCtrl.GetCheck(h_child))
			{
				TVITEM item;
				item.hItem = h_child;
				item.mask =  TVIF_HANDLE | TVIF_PARAM;
				treeCtrl.GetItem(&item);
				ItemFunc* func = (ItemFunc *)item.lParam;
				if (func->b_ret)
					unvoid_tokens.insert(func->name);
				else
					void_tokens.insert(func->name);
			}
			// Try to get the next item
			q_n.push(h_child);
			h_child = treeCtrl.GetNextItem(h_child, TVGN_NEXT);
		}
	}

	CString strSyncFuncs;
	CWnd *pEdtSync = GetDlgItem(IDC_EDTSYNCFUNCS);
	pEdtSync->GetWindowText(strSyncFuncs);
	const char* p = strSyncFuncs.LockBuffer();
	const char* p_end = p + strSyncFuncs.GetLength();
	NameMatch name;
	if (name.Match(p, p_end))
	{
		const char* token[2] = {0};
		name.Range(token);
		sync_tokens.insert(std::string(MAKE_STRING(token)));
		FixMatch semicolon(FIX_MATCH_CONSTRU(";"));
		BlankMatch blank;
		BlankMatchStar blank_star(&blank);
		PatternMatch* g[] = {&blank_star, &semicolon, &blank_star, &name};
		And m(g, 4);
		while (m.Match(p, p_end))
		{
			name.Range(token);
			sync_tokens.insert(std::string(MAKE_STRING(token)));
		}

	}


	strSyncFuncs.UnlockBuffer();



#ifdef TEST_START_PARSE
	ATLTRACE("\n\nvoid functions:");
	for (std::set<std::string>::iterator it = void_tokens.begin()
		; it != void_tokens.end()
		; it ++)
	{
		const std::string& str = *it;
		ATLTRACE("\n\t%s", str.c_str());
	}

	ATLTRACE("\n\nunvoid functions:");
	for (std::set<std::string>::iterator it = unvoid_tokens.begin()
		; it != unvoid_tokens.end()
		; it ++)
	{
		const std::string& str = *it;
		ATLTRACE("\n\t%s", str.c_str());
	}

	ATLTRACE("\n\nsync functions:");
	for (std::set<std::string>::iterator it = sync_tokens.begin()
		; it != sync_tokens.end()
		; it ++)
	{
		const std::string& str = *it;
		ATLTRACE("\n\t%s", str.c_str());
	}

#endif

	CWnd *pEdit = GetDlgItem(IDC_EDTSRCDIR_INJECT);
	CString strDirPath;
	pEdit->GetWindowTextA(strDirPath);
	if (strDirPath.IsEmpty())
	{
		AfxMessageBox("Please select a source file directory");
		return;
	}
	std::list<CString> files;
	Recurse(strDirPath, files);
	m_progCtrl.SetRange(0, files.size());
	int pos = 0;
	for (std::list<CString>::iterator it = files.begin(); it != files.end(); it ++)
	{
		pos ++;
		m_progCtrl.SetPos(pos);
		Inject(*it, void_tokens);
	}
	AfxMessageBox("Done!!!");
	m_progCtrl.SetPos(0);
}

void CGLTraceInstruDlg::Recurse(LPCTSTR szPathDir, std::list<CString>& lstFiles) const
{
	CFileFind finder;

	// build a string with wildcards
	CString strWildcard(szPathDir);
	strWildcard += _T("\\*.*");

	// start working for files
	BOOL bWorking = finder.FindFile(strWildcard);

	while (bWorking)
	{
		bWorking = finder.FindNextFile();

		// skip . and .. files; otherwise, we'd
		// recur infinitely!

		if (finder.IsDots())
			continue;

		// if it's a directory, recursively search it

		if (finder.IsDirectory())
		{
			CString szPathDir_prime = finder.GetFilePath();
			Recurse(szPathDir_prime, lstFiles);
		}
		else
		{
			CString fileName = finder.GetFileName();
			int i_dot = fileName.Find('.', 0);
			int i_dotn = -1;
			while (-1 != (i_dotn = fileName.Find('.', i_dot + 1)))
			{
				i_dot = i_dotn;
			}
			if (i_dot > 0)
			{
				CString lastName = fileName.Right(fileName.GetLength() - i_dot);
				if (c_setFileFilters.find(lastName) != c_setFileFilters.end())
				{
					CString fileName_full = finder.GetFilePath();
					lstFiles.push_back(fileName_full);
				}
			}
		}
	}

	finder.Close();
}

#undef MAKE_STRING