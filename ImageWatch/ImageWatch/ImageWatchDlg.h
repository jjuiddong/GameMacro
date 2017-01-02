#pragma once

#include "ImageWatchWindow.h"
#include "afxwin.h"


class CImageWatchDlg : public CDialogEx
{
public:
	CImageWatchDlg(CWnd* pParent = NULL);
	virtual ~CImageWatchDlg();
	enum { IDD = IDD_IMAGEWATCH_DIALOG };

	void Run();


protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	void MainLoop(const float deltaSeconds);


protected:
	HICON m_hIcon;
	bool m_loop;
	bool m_isHideCmdWindow;
	cv::Mat m_srcImg;
	cv::Mat m_dstImg;
	CImageWatchWindow *m_imgWindow;


	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	DECLARE_ANCHOR_MAP();
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnClose();
	afx_msg void OnBnClickedCheckGrid();
	afx_msg void OnBnClickedCheckText();
	afx_msg void OnBnClickedCheckTextbg();
	afx_msg void OnBnClickedCheckWhite();
	afx_msg void OnBnClickedButtonUpdate();
	afx_msg void OnBnClickedButtonHelp();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	BOOL m_checkGrid;
	BOOL m_checkText;
	BOOL m_checkTextbg;
	BOOL m_checkWhite;
	CStatic m_staticInfo;
	CString m_editCmd;
	BOOL m_checkAutoColor;
	afx_msg void OnBnClickedCheckAutocolor();
	afx_msg void OnBnClickedButtonHideCmdwindow();
};
