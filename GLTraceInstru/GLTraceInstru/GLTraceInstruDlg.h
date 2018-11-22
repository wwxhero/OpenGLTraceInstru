// GLTraceInstruDlg.h : header file
//

#pragma once
#include <list>
#include <set>
#include <map>
#include "afxwin.h"
#include "ColumnTreeCtrl.h"
#include "EasySize.h"
#include "afxdialogex.h"
#include "afxcmn.h"
#include "GenGLRetFuncs.h"

// CGLTraceInstruDlg dialog
class CGLTraceInstruDlg : public CDialogEx
{
// Construction
public:
	DECLARE_EASYSIZE;
	CGLTraceInstruDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_DEMOMFC_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBtnClickHeaderSel();
	afx_msg void OnBtnClickFilePathSel();
	afx_msg void OnBtnClickStartInjection();
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()
public:
	CColumnTreeCtrl m_columnTree;
	CImageList m_imgList;
	CProgressCtrl m_progCtrl;
	std::set<CString> c_setFileFilters;
	int m_idGroup;
	int m_idItem;
private:
	void InitOptions();
	void InitTree();
	void CGLTraceInstruDlg::Recurse(LPCTSTR szPathDir, std::list<CString>& lstFiles) const;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRclickedColumntree(LPNMHDR pNMHDR, LRESULT* pResult);
private:
	class ItemFunc
	{
	public:
		ItemFunc(const std::string& a_name, bool a_ret)
			: name(a_name)
			, b_ret(a_ret)
		{
		}
	public:
		const std::string name;
		const bool b_ret;
	};
	typedef std::map<std::string, std::list<ItemFunc*>> ItemGroups;
	typedef std::pair<std::string, std::list<ItemFunc*>> ItemGroup;

	void UpdateTree(const ItemGroups* g);
	void ClearTree();

};
