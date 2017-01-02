#pragma once
#include "afxcmn.h"


// CFileTreeDialog dialog
class CFileTreeDialog : public CDialogEx
{
public:
	CFileTreeDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFileTreeDialog();

	enum { IDD = IDD_DIALOG_FILETREE };

	void Refresh();


public:
	string m_selectFileName;
	CImageFrameWnd *m_imageWnd;


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	DECLARE_ANCHOR_MAP();
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CFileTreeCtrl m_FileTree;
	afx_msg void OnBnClickedButtonImagematch();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelchangedTreeFile(NMHDR *pNMHDR, LRESULT *pResult);
	BOOL m_IsShow;
	afx_msg void OnBnClickedCheckShow();
	afx_msg void OnBnClickedButtonRefresh();
	afx_msg void OnBnClickedButtonLabelcreator();
	BOOL m_IsAutoRefresh;
	afx_msg void OnBnClickedCheckAutoRefresh();
	afx_msg void OnBnClickedButtonExplorer();
	CString m_strLabelCreatorArg;
	afx_msg void OnBnClickedButtonHelp();
	afx_msg void OnBnClickedButtonTostreamingdlg();
	afx_msg void OnBnClickedCheckAutoUpdate();
	BOOL m_IsAutoUpdate;
};
