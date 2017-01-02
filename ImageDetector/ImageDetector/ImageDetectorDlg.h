
// ImageDetectorDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "afxeditbrowsectrl.h"


// CImageDetectorDlg dialog
class CFileTreeDialog;
class CLabelTreeDialog;
class CMatchScriptTreeDialog;
class CStreamingDialog;
class CImageDetectorDlg : public CDialogEx
{
public:
	CImageDetectorDlg(CWnd* pParent = NULL);	// standard constructor
	enum { IDD = IDD_IMAGEDETECTOR_DIALOG };

	void Run();
	void ImageMatch(const string &fileName);
	void ImageMatch(const cv::Mat &img, const string &comment="");
	void Log(const char* fmt, ...);
	string GenerateCaptureFileName();
	string GetExecuteTreeName();


protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	void MainLoop(const float deltaSeconds);
	void ReadImageMatchScript(const string &fileName);
	void ReadMenuScript(const string &fileName);
	void DownloadAndMatch(const cv::Mat &img);
	void MatchComplete();
	void FlowControlProccess(const float deltaSeconds);
	void PrintVirtualKey(const int vkey, const int tab=0, const string &str="");
	void KeyboardProcess(const int vkey);


public:
	HICON m_hIcon;
	bool m_loop;
	bool m_isConnect;
	bool m_isKeySvrConnect;
	int m_matchStartTime;

	cvproc::cStreamingReceiver m_streamRcver;
	cvproc::imagematch::cMatchScript2 m_matchScript;
	cvproc::imagematch::cFlowControl m_flowControl;
	cvproc::imagematch::cMatchResult *m_matchResult; // reference
	network::cTCPClient m_keyClient;
	CImageFrameWnd *m_imageWnd;
	cv::Mat m_lastMatchImage;
	cv::Mat m_lastUpdateImage;

	cvproc::cFrapsCapture m_frapsCapture;
	int m_captureKey; // default : F11
	bool m_isScreenCapture;
	bool m_isRepeatScreenCapture;
	float m_repeatDelaySeconds; // 0.5 seconds
	vector<cUpdateListener*> m_updateListener;

	CFileTreeDialog *m_fileTreeDlg;
	CLabelTreeDialog *m_labelTreeDlg;
	CMatchScriptTreeDialog *m_scriptTreeDlg;
	CStreamingDialog *m_streamingDlg;


	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	DECLARE_ANCHOR_MAP();
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CIPAddressCtrl m_IP;
	int m_Port;
	afx_msg void OnBnClickedButtonConnect();
	CButton m_ConnectButton;
	BOOL m_IsSaveImage;
	afx_msg void OnBnClickedCheckWrite();
	BOOL m_IsShowImage;
	afx_msg void OnBnClickedCheckShowimage();
	CListBox m_logList;
	CProgressCtrl m_ImgDownloadProgress;
	BOOL m_IsImageMatch;
	afx_msg void OnBnClickedCheckImgMatch();
	CMFCEditBrowseCtrl m_scriptBrowser;
	CString m_excuteLabel;
	afx_msg void OnBnClickedButtonViewscript();
	afx_msg void OnBnClickedButtonShowcaptureTree();
	afx_msg void OnClose();
	afx_msg void OnBnClickedButtonShowlabelTree();
	afx_msg void OnBnClickedButtonScriptUpdate();
	afx_msg void OnBnClickedButtonScriptExecute();
	afx_msg void OnBnClickedCheckMenuTraverse();
	afx_msg void OnBnClickedCheckTreeMatch();
	afx_msg void OnBnClickedButtonMenuscriptUpdate();
	CMFCEditBrowseCtrl m_browseFlowScript;
	BOOL m_IsMoveMenu;
	afx_msg void OnBnClickedCheckMoveMenu();
	afx_msg void OnBnClickedButtonViewFlowScript();
	afx_msg void OnBnClickedButtonMoveScene();
	afx_msg void OnBnClickedButtonConnectKeyserver();
	int m_editKeyServerPort;
	CButton m_buttonKeySvrConnect;
	afx_msg void OnBnClickedButtonSendF11();
	CButton m_buttonSendF11;
	CButton m_buttonMoveScene;
	afx_msg void OnBnClickedCheckNoUithread();
	afx_msg void OnBnClickedButtonClearlog();
	afx_msg void OnBnClickedButtonShowscriptTree();
	afx_msg void OnBnClickedButtonShowStreaming();
	afx_msg void OnBnClickedButtonShowImgprocdlg();
	CMFCEditBrowseCtrl m_browseFlowCmd;
	afx_msg void OnBnClickedButtonViewFlowcmd();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedButtonRepeat();
	BOOL m_IsFrapsCapture;
	afx_msg void OnBnClickedCheckFrapscapture();
};
