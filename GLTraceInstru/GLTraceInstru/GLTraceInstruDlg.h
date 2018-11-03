// GLTraceInstruDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "ColumnTreeCtrl.h"
#include "EasySize.h"
#include "afxdialogex.h"
#include <list>
#include <set>
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
	afx_msg void OnBtnClickFilePathSel();
	afx_msg void OnBtnClickStartInjection();
	DECLARE_MESSAGE_MAP()
public:
	CColumnTreeCtrl m_columnTree;
	CImageList m_imgList;
	std::set<CString> c_setFileFilters;
private:
	void InitOptions();
	void InitTree();
	void CGLTraceInstruDlg::Recurse(LPCTSTR szPathDir, std::list<CString>& lstFiles) const;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRclickedColumntree(LPNMHDR pNMHDR, LRESULT* pResult);
};
