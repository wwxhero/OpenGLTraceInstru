// DemoMFCDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "ColumnTreeCtrl.h"
#include "EasySize.h"
#include "afxdialogex.h"
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
	DECLARE_MESSAGE_MAP()
public:
	CColumnTreeCtrl m_columnTree;
	CImageList m_imgList;
private:
	void InitOptions();
	void InitTree();
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRclickedColumntree(LPNMHDR pNMHDR, LRESULT* pResult);
};
