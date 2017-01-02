#pragma once
#include "afxwin.h"
#include "afxcmn.h"


class CScreenCaptureDlg : public CDialogEx
{
public:
	CScreenCaptureDlg(CWnd* pParent = NULL);	// standard constructor

	enum { IDD = IDD_SCREENCAPTURE_DIALOG };

	void Run();


protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	void MainLoop(const float deltaSeconds);
	void Capture();
	void CaptureProcess(const cv::Mat &img);
	bool CheckKeyDownUp();
	void KeyboardProcess(const int vkey);
	void PrintVertualKey(const int vkey);
	void Log(const char* fmt, ...);


protected:
	HICON m_hIcon;
	bool m_loop;
	bool m_isCapture;
	bool m_isWaitFrapsCapture; // Fraps 가 캡쳐가 끝날때까지 대기중 일때, true
	bool m_isServerStart;
	bool m_isStreamingDelay; // UI 입력을 위해, 중간에 딜레이를 준다.
	int m_delayFrame;
	int m_captureKey; // default : F11
	cvproc::cStreamingSender m_streamSender;
	network::cTCPServer m_keyServer;
	cvproc::cFrapsCapture m_frapsCapture;


	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CListBox m_logList;
	BOOL m_IsPressAlt;
	afx_msg void OnBnClickedCheckAlt();
	int m_SendPort;
	afx_msg void OnBnClickedButtonServerstart();
	CButton m_ServerStartButton;
	BOOL m_IsPrintCapture;
	afx_msg void OnBnClickedCheckPrintCapture();
	BOOL m_IsGray;
	BOOL m_IsCompressed;
	CComboBox m_CompressCombo;
	int m_editSendDelayTime;
	afx_msg void OnBnClickedCheckGray();
	afx_msg void OnBnClickedCheckCompressed();
	int m_keySvrPort;
	BOOL m_IsStreaming;
	afx_msg void OnBnClickedCheckStreaming();
	CString m_editFraps;
	afx_msg void OnBnClickedCheckFraps();
	BOOL m_IsFrapsCapture;
};
