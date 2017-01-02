#pragma once
#include "afxcmn.h"


// CLabelTree dialog

class CLabelTreeDialog : public CDialogEx
{
public:
	CLabelTreeDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLabelTreeDialog();
	enum { IDD = IDD_DIALOG_LABELTREE };


	string m_selectFileName;
	CImageFrameWnd *m_imageWnd;


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	cv::Rect ParseMatchOption(const string &option);

	DECLARE_MESSAGE_MAP()
	DECLARE_ANCHOR_MAP();
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	BOOL m_IsShow;
	afx_msg void OnBnClickedCheckShow();
	CFileTreeCtrl m_labelTree;
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelchangedTreeFile(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonRefresh();
	afx_msg void OnBnClickedButtonMatch();
	afx_msg void OnBnClickedButtonFeatureMatch();
	afx_msg void OnBnClickedButtonExplorer();
	CString m_strMatchOption;
	afx_msg void OnBnClickedButtonHelp();
};
