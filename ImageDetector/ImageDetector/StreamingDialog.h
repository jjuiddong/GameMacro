//
// 2016-07-03, jjuiddong
// 스트리밍 영상을 받아, 이미지 프로세싱 후에 화면에 출력한다.
//
#pragma once
#include "afxwin.h"

class CImageProcessingDlg;
class CStreamingDialog : public CDialogEx
{
public:
	CStreamingDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CStreamingDialog();
	enum { IDD = IDD_DIALOG_STREAMING };

	bool ShowStreaming(const string &ip, const int port);
	void SetImageMode(const cv::Mat &img);
	string GetOptionStr();
	CImageProcessingDlg* AddImageProcessingDlg();
	void RemoveImageProcessingDlg(CImageProcessingDlg *dlg);


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support


public:
	enum MODE {STREAMING, IMAGE};
	enum PLAYMODE { PLAY, PAUSE };
	PLAYMODE m_playMode;
	bool m_isShow;
	string m_ip;
	int m_port;
	cvproc::cStreamingReceiver m_streamRcver;
	cv::Mat m_srcImage; // 원본 이미지, 이미지 모드에서 사용.
	cv::Mat m_cvtImage; // 변환 된 후, 이미지
	CCamWindow *m_captureScreen1;
	CCamWindow *m_captureScreen2;
	string m_optionStr;
	bool m_updateOption; // m_optionStr이 수정되면 true가 된다.
	bool m_updateSrcImage; // m_srcImage가 수정되면 true가 된다.
	int m_streamingFPS;

	vector<CImageProcessingDlg*> m_imgProcDlgs;


	// thread variable
	std::thread m_thread;
	bool m_threadLoop;
	int m_sleepMillis;
	common::CriticalSection m_cs;

	bool m_isUIUpdate;
	bool m_isRunUIThread;
	std::thread m_UIThread;


	DECLARE_MESSAGE_MAP()
	DECLARE_ANCHOR_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CString m_editCmd;
	afx_msg void OnBnClickedButtonUpdate();
	afx_msg void OnBnClickedButtonHelp();
	afx_msg void OnBnClickedRadioStreaming();
	afx_msg void OnBnClickedRadioImage();
	int m_radioMode;
	afx_msg void OnBnClickedButtonTemplateMatch();
	afx_msg void OnBnClickedButtonFeatureMatch();
	afx_msg void OnBnClickedButtonImagematch();
	afx_msg void OnBnClickedButtonCapture();
	afx_msg void OnBnClickedButtonPause();
	CButton m_btnPause;
	afx_msg void OnBnClickedButtonNewwindow();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	CStatic m_staticFPS;
};
