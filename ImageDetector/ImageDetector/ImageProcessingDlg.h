#pragma once
#include "afxwin.h"


class CImageProcessingDlg : public CDialogEx, public cUpdateListener
{
public:
	CImageProcessingDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CImageProcessingDlg();
	enum { IDD = IDD_DIALOG_IMGPROCESSING };

	void SetSourceImage(const cv::Mat &img);
	void ImageProcessing();
	virtual void Update(const float deltaSeconds) override;


public:
	enum PLAYMODE { PLAY, PAUSE };
	PLAYMODE m_mode;
	bool m_isUpdateImage;
	bool m_isPostUpdateImage;
	cv::Mat m_srcImage; // 원본 이미지
	cv::Mat m_cvtImage; // 변환 된 후, 이미지
	CImageWatchWindow *m_imgWindow;
	string m_option;

	cvproc::imagematch::cSimpleMatchScript m_smatch;

	CImageFrameWnd *m_popupImageWindow; // show tesseract roi image
	bool m_isShowFirstImageWnd;

	std::thread m_thread;
	bool m_isRunThread;
	bool m_isRunUIThread;
	std::thread m_UIThread;
	cv::Mat m_test;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	DECLARE_ANCHOR_MAP();
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CString m_editCmd;
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedButtonImagematch();
	afx_msg void OnBnClickedButtonTemplateMatch();
	afx_msg void OnBnClickedButtonFeatureMatch();
	afx_msg void OnBnClickedButtonUpdate();
	afx_msg void OnBnClickedButtonPause();
	afx_msg void OnBnClickedRadioStreaming();
	afx_msg void OnBnClickedRadioImage();
	int m_radioMode;
	CButton m_btnPause;
	afx_msg void OnClose();
	afx_msg void OnBnClickedButtonNewwindow();
	afx_msg void OnBnClickedButtonHelp();
	BOOL m_checkShowGrid;
	BOOL m_checkShowText;
	BOOL m_checkShowTextBg;
	afx_msg void OnBnClickedCheckGrid();
	afx_msg void OnBnClickedCheckText();
	afx_msg void OnBnClickedCheckTextbg();
	BOOL m_checkWhite;
	afx_msg void OnBnClickedCheckWhite();
	CStatic m_staticPixelInfo;
	BOOL m_checkAutoColor;
	afx_msg void OnBnClickedCheckAutocolor();
};
