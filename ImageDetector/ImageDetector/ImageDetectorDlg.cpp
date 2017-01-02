
// ImageDetectorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ImageDetector.h"
#include "ImageDetectorDlg.h"
#include "afxdialogex.h"
#include "FileTreeDialog.h"
#include "LabelTreeDialog.h"
#include "MatchScriptTreeDialog.h"
#include "StreamingDialog.h"
#include "ImageProcessingDlg.h"
#include "../../../Common/Tess/dictionary.h"


#pragma comment (lib, "winmm.lib")


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace cv;
using namespace cvproc;
using namespace cvproc::imagematch;


CImageDetectorDlg *g_imageDetectDlg = NULL; // reference
CLabelTreeDialog *g_labelTreeDlg = NULL; // reference
CFileTreeDialog *g_fileTreeDlg = NULL; // reference
CStreamingDialog *g_streamingDlg = NULL; // reference
CMatchScriptTreeDialog *g_scriptTreeDlg = NULL; // reference


// CImageDetectorDlg dialog
CImageDetectorDlg::CImageDetectorDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_IMAGEDETECTOR_DIALOG, pParent)
	, m_loop(true)
	, m_Port(8000)
	, m_isConnect(false)
	, m_IsSaveImage(TRUE)
	, m_IsShowImage(TRUE)
	, m_IsImageMatch(FALSE)
	, m_excuteLabel(_T(""))
	, m_fileTreeDlg(NULL)
	, m_labelTreeDlg(NULL)
	, m_scriptTreeDlg(NULL)
	, m_streamingDlg(NULL)
	, m_IsMoveMenu(FALSE)
	, m_editKeyServerPort(8001)
	, m_isKeySvrConnect(false)
	, m_imageWnd(NULL)
	, m_isScreenCapture(false)
	, m_isRepeatScreenCapture(false)
	, m_repeatDelaySeconds(0)
	, m_matchResult(NULL)
	, m_IsFrapsCapture(FALSE)
{
	g_imageDetectDlg = this;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CImageDetectorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPADDRESS_IP, m_IP);
	DDX_Text(pDX, IDC_EDIT_PORT, m_Port);
	DDX_Control(pDX, IDC_BUTTON_CONNECT, m_ConnectButton);
	DDX_Check(pDX, IDC_CHECK_WRITE, m_IsSaveImage);
	DDX_Check(pDX, IDC_CHECK_SHOWIMAGE, m_IsShowImage);
	DDX_Control(pDX, IDC_LIST_LOG, m_logList);
	DDX_Control(pDX, IDC_PROGRESS_IMG, m_ImgDownloadProgress);
	DDX_Check(pDX, IDC_CHECK_IMG_MATCH, m_IsImageMatch);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_IMGMATCH_SCRIPT, m_scriptBrowser);
	DDX_Text(pDX, IDC_EDIT1, m_excuteLabel);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_IMGMATCH_FLOW_SCRIPT, m_browseFlowScript);
	DDX_Check(pDX, IDC_CHECK_MOVE_MENU, m_IsMoveMenu);
	DDX_Text(pDX, IDC_EDIT_KEYSERVER_PORT, m_editKeyServerPort);
	DDX_Control(pDX, IDC_BUTTON_CONNECT_KEYSERVER, m_buttonKeySvrConnect);
	DDX_Control(pDX, IDC_BUTTON_SEND_F11, m_buttonSendF11);
	DDX_Control(pDX, IDC_BUTTON_MOVE_SCENE, m_buttonMoveScene);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_IMGMATCH_FLOW_CMD, m_browseFlowCmd);
	DDX_Check(pDX, IDC_CHECK_FRAPSCAPTURE, m_IsFrapsCapture);
}

BEGIN_MESSAGE_MAP(CImageDetectorDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CImageDetectorDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CImageDetectorDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CImageDetectorDlg::OnBnClickedButtonConnect)
	ON_BN_CLICKED(IDC_CHECK_WRITE, &CImageDetectorDlg::OnBnClickedCheckWrite)
	ON_BN_CLICKED(IDC_CHECK_SHOWIMAGE, &CImageDetectorDlg::OnBnClickedCheckShowimage)
	ON_BN_CLICKED(IDC_CHECK_IMG_MATCH, &CImageDetectorDlg::OnBnClickedCheckImgMatch)
	ON_BN_CLICKED(IDC_BUTTON_VIEWSCRIPT, &CImageDetectorDlg::OnBnClickedButtonViewscript)
	ON_BN_CLICKED(IDC_BUTTON_SHOWCAPTURE_TREE, &CImageDetectorDlg::OnBnClickedButtonShowcaptureTree)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_SHOWLABEL_TREE, &CImageDetectorDlg::OnBnClickedButtonShowlabelTree)
	ON_BN_CLICKED(IDC_BUTTON_SCRIPT_UPDATE, &CImageDetectorDlg::OnBnClickedButtonScriptUpdate)
	ON_BN_CLICKED(IDC_BUTTON_SCRIPT_EXECUTE, &CImageDetectorDlg::OnBnClickedButtonScriptExecute)
	ON_BN_CLICKED(IDC_CHECK_MENU_TRAVERSE, &CImageDetectorDlg::OnBnClickedCheckMenuTraverse)
	ON_BN_CLICKED(IDC_CHECK_TREE_MATCH, &CImageDetectorDlg::OnBnClickedCheckTreeMatch)
	ON_BN_CLICKED(IDC_BUTTON_FLOWSCRIPT_UPDATE, &CImageDetectorDlg::OnBnClickedButtonMenuscriptUpdate)
	ON_BN_CLICKED(IDC_CHECK_MOVE_MENU, &CImageDetectorDlg::OnBnClickedCheckMoveMenu)
	ON_BN_CLICKED(IDC_BUTTON_VIEW_FLOW_SCRIPT, &CImageDetectorDlg::OnBnClickedButtonViewFlowScript)
	ON_BN_CLICKED(IDC_BUTTON_MOVE_SCENE, &CImageDetectorDlg::OnBnClickedButtonMoveScene)
	ON_BN_CLICKED(IDC_BUTTON_CONNECT_KEYSERVER, &CImageDetectorDlg::OnBnClickedButtonConnectKeyserver)
	ON_BN_CLICKED(IDC_BUTTON_SEND_F11, &CImageDetectorDlg::OnBnClickedButtonSendF11)
	ON_BN_CLICKED(IDC_CHECK_NO_UITHREAD, &CImageDetectorDlg::OnBnClickedCheckNoUithread)
	ON_BN_CLICKED(IDC_BUTTON_CLEARLOG, &CImageDetectorDlg::OnBnClickedButtonClearlog)
	ON_BN_CLICKED(IDC_BUTTON_SHOWSCRIPT_TREE, &CImageDetectorDlg::OnBnClickedButtonShowscriptTree)
	ON_BN_CLICKED(IDC_BUTTON_SHOW_STREAMING, &CImageDetectorDlg::OnBnClickedButtonShowStreaming)
	ON_BN_CLICKED(IDC_BUTTON_SHOW_IMGPROCDLG, &CImageDetectorDlg::OnBnClickedButtonShowImgprocdlg)
	ON_BN_CLICKED(IDC_BUTTON_VIEW_FLOWCMD, &CImageDetectorDlg::OnBnClickedButtonViewFlowcmd)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_REPEAT, &CImageDetectorDlg::OnBnClickedButtonRepeat)
	ON_BN_CLICKED(IDC_CHECK_FRAPSCAPTURE, &CImageDetectorDlg::OnBnClickedCheckFrapscapture)
END_MESSAGE_MAP()


BEGIN_ANCHOR_MAP(CImageDetectorDlg)
 	ANCHOR_MAP_ENTRY(IDC_BUTTON_SHOWCAPTURE_TREE, ANF_RIGHT | ANF_TOP )
	ANCHOR_MAP_ENTRY(IDC_BUTTON_SHOWLABEL_TREE, ANF_RIGHT | ANF_TOP)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_SHOW_STREAMING, ANF_RIGHT | ANF_TOP)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_SHOW_IMGPROCDLG, ANF_RIGHT | ANF_TOP)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_SCRIPT_UPDATE, ANF_RIGHT | ANF_TOP)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_VIEWSCRIPT, ANF_RIGHT | ANF_TOP)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_SCRIPT_EXECUTE, ANF_RIGHT | ANF_TOP)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_SHOWSCRIPT_TREE, ANF_RIGHT | ANF_TOP)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_FLOWSCRIPT_UPDATE, ANF_RIGHT | ANF_TOP)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_VIEW_FLOW_SCRIPT, ANF_RIGHT | ANF_TOP)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_VIEW_FLOWCMD, ANF_RIGHT | ANF_TOP)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_MOVE_SCENE, ANF_RIGHT | ANF_TOP)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_SEND_F11, ANF_RIGHT | ANF_TOP)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_CONNECT_KEYSERVER, ANF_RIGHT | ANF_TOP)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_REPEAT, ANF_RIGHT | ANF_TOP)
	
	ANCHOR_MAP_ENTRY(IDC_BUTTON_CLEARLOG, ANF_RIGHT | ANF_BOTTOM)

	ANCHOR_MAP_ENTRY(IDC_MFCEDITBROWSE_IMGMATCH_SCRIPT, ANF_LEFT | ANF_RIGHT | ANF_TOP)
	ANCHOR_MAP_ENTRY(IDC_MFCEDITBROWSE_IMGMATCH_FLOW_CMD, ANF_LEFT | ANF_RIGHT | ANF_TOP)
	ANCHOR_MAP_ENTRY(IDC_MFCEDITBROWSE_IMGMATCH_FLOW_SCRIPT, ANF_LEFT | ANF_RIGHT | ANF_TOP)
	ANCHOR_MAP_ENTRY(IDC_EDIT1, ANF_LEFT | ANF_RIGHT | ANF_TOP)
	ANCHOR_MAP_ENTRY(IDC_EDIT_KEYSERVER_PORT, ANF_LEFT | ANF_RIGHT | ANF_TOP)
	ANCHOR_MAP_ENTRY(IDC_PROGRESS_IMG, ANF_LEFT | ANF_RIGHT | ANF_TOP)

	ANCHOR_MAP_ENTRY(IDC_LIST_LOG, ANF_LEFT | ANF_RIGHT | ANF_TOP | ANF_BOTTOM)
END_ANCHOR_MAP()


BOOL CImageDetectorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	InitAnchors();

	CreateDirectoryA("capture", NULL);
	CreateDirectoryA("label", NULL);

	dbg::RemoveErrLog();
	dbg::RemoveLog();

	// Read Config File
	cConfig config;
	config.Read("config.txt");

	BYTE ipAddr[4];
	GetIPtoInt(config.GetString("ip", "192.168.0.100"), ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
	m_IP.SetAddress(ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
	m_Port = config.GetInt("port", m_Port);
	m_captureKey = config.GetInt("capturekey", VK_F11);

	m_fileTreeDlg = new CFileTreeDialog(this);
	m_fileTreeDlg->Create(CFileTreeDialog::IDD, this);
	g_fileTreeDlg = m_fileTreeDlg;

	m_labelTreeDlg = new CLabelTreeDialog(this);
	m_labelTreeDlg->Create(CLabelTreeDialog::IDD, this);
	g_labelTreeDlg = m_labelTreeDlg;

	m_scriptTreeDlg = new CMatchScriptTreeDialog(this);
	m_scriptTreeDlg->Create(CMatchScriptTreeDialog::IDD, this);
	g_scriptTreeDlg = m_scriptTreeDlg;

	m_streamingDlg = new CStreamingDialog(this);
	m_streamingDlg->Create(CStreamingDialog::IDD, this);
	g_streamingDlg = m_streamingDlg;

	const string scriptFileName = config.GetString("script_filename", "dirt3_trackname.txt");
	const string execLabel = config.GetString("script_exec_label", "detect_trackname_roi");
	const string flowFileName = config.GetString("flow_filename", "flow.txt");
	const string flowCommand = config.GetString("flow_command", "flowcmd.txt");

	m_scriptBrowser.SetWindowTextW(str2wstr(scriptFileName).c_str());
	m_excuteLabel = str2wstr(execLabel).c_str();
	ReadImageMatchScript(scriptFileName);

	m_browseFlowScript.SetWindowTextW(str2wstr(flowFileName).c_str());
	m_browseFlowCmd.SetWindowTextW(str2wstr(flowCommand).c_str());
	ReadMenuScript(flowFileName);
	m_flowControl.m_isLog = true; // 로그 남김

	m_frapsCapture.Init("C:/Fraps/Screenshots/", VK_F9);
	m_buttonSendF11.EnableWindow(TRUE);


	UpdateData(FALSE);




// 	tess::cDictionary dict;
// 	dict.Init("dictionary.txt");
//
// 	{
// 		vector<string> out;
// 		float maxFitness;
// 		string result1 = dict.Search("location finland", out, maxFitness);
// 		
// 		out.clear();
// 		string result2 = dict.Search("<< -- track  ?? ??? isorjarvi ???", out, maxFitness);
// 
// 		out.clear();
// 		string result3 = dict.Search("<< -- track  ?? ??? isojarvi ???", out, maxFitness);
// 
// 		out.clear();
// 		string result4 = dict.Search("ve icle class pro", out, maxFitness);
// 
// 		out.clear();
// 		string result5 = dict.Search("ve icle class moern", out, maxFitness);
// 
// 		out.clear();
// 		string result6 = dict.Search("veicle class clasic", out, maxFitness);
// 
// 		out.clear();
// 		string result7 = dict.Search("ve icle class classic2 moerna", out, maxFitness);
// 
// 		int a = 0;
// 	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CImageDetectorDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

void CImageDetectorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}
HCURSOR CImageDetectorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
void CImageDetectorDlg::OnBnClickedOk()
{
}
void CImageDetectorDlg::OnBnClickedCancel()
{
	//m_loop = false;
}


void CImageDetectorDlg::OnBnClickedButtonConnect()
{
	UpdateData();

	if (m_isConnect)
	{
		m_isConnect = false;
		m_isKeySvrConnect = false;
		m_ConnectButton.SetWindowTextW(L"Connect - ScreenCapture");
		m_buttonKeySvrConnect.SetWindowTextW(L"Connect - Key Server");
		m_buttonSendF11.EnableWindow(FALSE);
		m_streamRcver.Close();
		m_keyClient.Close();

		Log("Close Server");
		dbg::Log("Server Close \n");
		SetBackgroundColor(g_grayColor);
	}
	else
	{
		Log("Try Connect");

		const string ip = GetIP(m_IP);
		if (m_streamRcver.Init(false, ip, m_Port, 0, 1, 1))
		{
			m_isConnect = true;
			m_ConnectButton.SetWindowTextW(L"Close");
			Log("Server Start");
			dbg::Log("Server Start \n");
			SetBackgroundColor(g_blueColor);
		}
		else
		{
			goto servererror;
		}

		m_keyClient.Close();
		if (m_keyClient.Init(ip, m_editKeyServerPort))
		{
			m_isKeySvrConnect = true;
			m_buttonKeySvrConnect.SetWindowTextW(L"Close");
			m_buttonSendF11.EnableWindow(TRUE);
			Log("Success Connect KeyServer ");
		}
		else
		{
			goto servererror;
		}
	}

	return;

servererror:
	dbg::Log("Server Connect Error \n");
	Log("Connection Error \n");
	::AfxMessageBox(L"Connet Error!!");
	m_buttonSendF11.EnableWindow(FALSE);
	Log("Error Connect KeyServer ");
	m_isKeySvrConnect = false;
}


void CImageDetectorDlg::Run()
{
	//메시지 구조체
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	int oldT = timeGetTime();
	while (m_loop)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
 			TranslateMessage(&msg);
 			DispatchMessage(&msg);
			//AfxGetApp()->PumpMessage();
		}

		const int curT = timeGetTime();
		if (curT - oldT > 20)
		{
			const float deltaSeconds = (curT - oldT) * 0.001f;
			oldT = curT;
			MainLoop(deltaSeconds);

			for each (auto listener in m_updateListener)
				listener->Update(deltaSeconds);
		}
	}
}


void CImageDetectorDlg::MainLoop(const float deltaSeconds)
{
	if (m_matchResult && m_matchResult->m_isRun && m_matchResult->m_isEnd)
	{
		MatchComplete();
	}

	if (m_scriptTreeDlg)
		m_scriptTreeDlg->Update(deltaSeconds);

	if (!m_isConnect && !m_IsFrapsCapture)
		return;

	FlowControlProccess(deltaSeconds);

	if (m_isRepeatScreenCapture)
	{
		m_repeatDelaySeconds -= deltaSeconds;
		if (m_repeatDelaySeconds < 0)
		{
			m_isScreenCapture = true;
			m_frapsCapture.ScreenShot();
			m_repeatDelaySeconds = 10000; // 캡쳐가 끝날 때까지 대기
		}
	}


	bool isUpdateImg = false;
	if (m_isScreenCapture && m_frapsCapture.IsUpdateScreenShot())
	{
		m_isScreenCapture = false;

		if (m_frapsCapture.m_img.data)
		{
			m_lastUpdateImage = m_frapsCapture.m_img;
			m_frapsCapture.m_img = Mat();
			isUpdateImg = true;
		}
		else
		{
			m_frapsCapture.ScreenShot();
			m_isScreenCapture = true;// 캡쳐 재시도.
		}

		if (m_isRepeatScreenCapture && !m_isScreenCapture)
		{
			// 열려있는 ImageProcessingDlg 를 Update 한다.
			for each(auto dlg in g_streamingDlg->m_imgProcDlgs)
				dlg->SetSourceImage(m_lastUpdateImage);

			m_repeatDelaySeconds = 0.5f;
		}
	}

	if (!m_IsFrapsCapture)
	{
		isUpdateImg = m_streamRcver.Update() ? true : false;
		if (isUpdateImg)
			m_lastUpdateImage = m_streamRcver.m_cloneImage.clone();
	}

	if (isUpdateImg && m_IsShowImage)
	{
		if (!m_imageWnd)
		{
			CRect wr;
			GetWindowRect(wr);
			wr.OffsetRect(-300, 0);
			wr.left = MAX(0, wr.left);
			wr.right = MAX(wr.left + 300, 300);
			wr.top = MAX(0, wr.top);
			wr.bottom = MAX(wr.top + 300, 300);
			m_imageWnd = new CImageFrameWnd();
			m_imageWnd->Init("download", m_lastUpdateImage, wr, this);
		}
		else
		{
			m_imageWnd->ShowImage(m_lastUpdateImage);
		}

		m_imageWnd->ShowWindow(SW_SHOW);
	}

	static bool isBegindownload = false;
	if (isBegindownload != m_streamRcver.m_isBeginDownload)
	{
		if (m_streamRcver.m_isBeginDownload)
		{
			SetBackgroundColor(g_greenColor);
		}
		else
		{
			SetBackgroundColor(g_blueColor);
		}
		isBegindownload = m_streamRcver.m_isBeginDownload;
	}


	int maxLen, readLen;
	if (m_streamRcver.DownLoadState(maxLen, readLen))
	{
		if (readLen == 0)
			m_ImgDownloadProgress.SetRange(readLen, maxLen);
		m_ImgDownloadProgress.SetPos(readLen);
	}

	if (isUpdateImg)
	{
		m_logList.AddString(str2wstr(m_streamRcver.CheckError()).c_str());
		m_logList.AddString(formatw("recv bytes = %d", m_streamRcver.m_tcpClient.m_recvBytes).c_str());
		m_logList.SetCurSel(m_logList.GetCount() - 1);
		m_streamRcver.m_tcpClient.m_recvBytes = 0;

		m_ImgDownloadProgress.SetPos(maxLen);
	}

	if (isUpdateImg && m_IsSaveImage)
	{
		string fileName = GenerateCaptureFileName();
		imwrite(fileName.c_str(), m_lastUpdateImage);

		Log("%s Write Image File", fileName.c_str());
	}

	// 이미지 매치
	if (isUpdateImg && m_IsImageMatch)
	{
		DownloadAndMatch(m_lastUpdateImage);
	}

	// 이미지 다운로드 후, 자동 리프레쉬
	if (isUpdateImg && g_fileTreeDlg->m_IsAutoRefresh)
	{
		g_fileTreeDlg->Refresh();
	}
}


string CImageDetectorDlg::GenerateCaptureFileName()
{
	// 중복되지 않는 파일명으로 생성한다.
	static int imgCnt = 1;
	string fileName;
	do
	{
		std::stringstream ss;
		ss << "capture/capture" << imgCnt++ << ".jpg";
		fileName = ss.str();
	} while (PathFileExistsA(fileName.c_str()));

	return fileName;
}


void CImageDetectorDlg::MatchComplete()
{
	RET(!m_matchResult);

	Log("result2 = %s", m_matchResult->m_resultStr.c_str());
	Log("    - elapseTime = %d milliseconds", timeGetTime() - m_matchStartTime);
	Log("    - match count = %d ", m_matchResult->m_matchCount);

	m_matchResult->MatchComplete();

	if (m_streamRcver.IsConnect())
		SetBackgroundColor(g_blueColor);
	else
		SetBackgroundColor(g_grayColor);
}


// 자동 씬 전환을 처리한다.
void CImageDetectorDlg::FlowControlProccess(const float deltaSeconds)
{
	using namespace std;
	using namespace cvproc::imagematch;
	RET(!m_IsMoveMenu);

	if (m_lastUpdateImage.data)
		SetBackgroundColor(g_yellowColor);

	int vkey=0;
	static cFlowControl::STATE oldState = cFlowControl::WAIT;
	const cFlowControl::STATE state = m_flowControl.Update(deltaSeconds, m_lastUpdateImage, vkey);

	if ((state != oldState) && (cFlowControl::REACH == state))
		Log("Reach");

	oldState = state;

	if (m_lastUpdateImage.data)
	{
		m_lastUpdateImage = Mat(); // 이미지 초기화
		SetBackgroundColor(g_blueColor);

		// flow control 상태 출력
		stringstream ss;
		ss << "current = " << ((m_flowControl.m_detectNode) ? m_flowControl.m_detectNode->attrs["id"] : "none");
		ss << ", next = " << ((m_flowControl.m_nextNode) ? m_flowControl.m_nextNode->attrs["id"] : "none");
		Log(ss.str().c_str());
	}

	// 키값이 정해지면~
	if (vkey != 0)
	{
		if (m_IsFrapsCapture)
		{
			if (vkey == m_captureKey)
			{
				m_isScreenCapture = true;
				m_frapsCapture.ScreenShot();
			}
			else
			{
				KeyboardProcess(vkey);
			}

			PrintVirtualKey(vkey, 0, "Send Keyboard  -> ");
		}
		else if (m_keyClient.IsConnect())
		{
			m_keyClient.Send((BYTE*)&vkey, sizeof(vkey));
			PrintVirtualKey(vkey, 0, "Send Keyboard  -> ");
		}
	}
}


void CImageDetectorDlg::DownloadAndMatch(const cv::Mat &img)
{
	UpdateData();

	Log("ImageMatch Start >>");
	SetBackgroundColor(g_yellowColor);

	const int t1 = timeGetTime();
	cMatchResult matchResult;
	const string labelTreeName = wstr2str((LPCTSTR)m_excuteLabel).c_str();
	matchResult.Init(&m_matchScript, img, "", 0, 
		(sParseTree*)m_matchScript.FindTreeLabel(labelTreeName),
		true, true);

	cMatchProcessor::Get()->Match(matchResult);
	const int t2 = timeGetTime();
	Log("result = %s", matchResult.m_resultStr.c_str());
	Log("    - elapseTime = %d milliseconds", t2 - t1);

	SetBackgroundColor(g_blueColor);
}


void CImageDetectorDlg::ImageMatch(const string &fileName)
{
	UpdateData();
	if (fileName.empty())
	{
		Log("Error!! not found image << [%s] ", fileName.c_str());
		return;
	}

	Log("image match << %s ", fileName.c_str());
	ImageMatch(imread(fileName), fileName);
}


void CImageDetectorDlg::ImageMatch(const cv::Mat &img, const string &comment)// comment=""
{
	UpdateData();
	SetBackgroundColor(g_yellowColor);

	m_lastMatchImage = img.clone();
	string result;

	m_matchStartTime = timeGetTime();
	const string labelTreeName = wstr2str((LPCTSTR)m_excuteLabel).c_str();
	
	if (m_matchResult)
		cMatchProcessor::Get()->FreeMatchResult(m_matchResult);

	m_matchResult = cMatchProcessor::Get()->AllocMatchResult();
	m_matchResult->Init(&m_matchScript, m_lastMatchImage, "", 0,
		(sParseTree*)m_matchScript.FindTreeLabel(labelTreeName),
		true, false);
	
	cMatchProcessor::Get()->Match(*m_matchResult);
}


void CImageDetectorDlg::OnBnClickedCheckWrite()
{
	UpdateData();
}
void CImageDetectorDlg::OnBnClickedCheckShowimage()
{
	UpdateData();
}
void CImageDetectorDlg::OnBnClickedCheckImgMatch()
{
	UpdateData();
}

void CImageDetectorDlg::OnBnClickedButtonViewscript()
{
	CString str;
	m_scriptBrowser.GetWindowTextW(str);
	ShellExecuteW(m_hWnd, L"open", L"notepad", (LPCTSTR)str, NULL, SW_SHOW);
}


void CImageDetectorDlg::OnBnClickedButtonViewFlowScript()
{
// 	CString str;
// 	m_browseFlowScript.GetWindowTextW(str);
// 	ShellExecuteW(m_hWnd, L"open", L"notepad", (LPCTSTR)str, NULL, SW_SHOW);

	m_flowControl.m_flowScript.PrintGraph("menutree");
	ShellExecuteW(m_hWnd, L"open", L"notepad", L"print_graph.txt", NULL, SW_SHOW);
}

void CImageDetectorDlg::OnBnClickedButtonViewFlowcmd()
{
	CString str;
	m_browseFlowCmd.GetWindowTextW(str);
	ShellExecuteW(m_hWnd, L"open", L"notepad", (LPCTSTR)str, NULL, SW_SHOW);
}



void CImageDetectorDlg::OnBnClickedButtonShowcaptureTree()
{
	if (m_fileTreeDlg)
	{
		// 처음 다이얼로그 위치 설정
		static bool mov = true;
		if (mov)
		{
			mov = false;
			CRect wr;
			GetWindowRect(wr);
			CRect dr;
			m_fileTreeDlg->GetWindowRect(dr);
			dr.OffsetRect(-CPoint(dr.left, dr.top));
			dr.OffsetRect(CPoint(wr.right, wr.top));
			dr.OffsetRect(-CPoint(0, dr.Height() - (wr.Height() / 2)));
			m_fileTreeDlg->MoveWindow(dr);
		}

		m_fileTreeDlg->ShowWindow(SW_SHOW);
	}
}


void CImageDetectorDlg::OnClose()
{
	m_loop = false;
	m_updateListener.clear();
	m_streamRcver.Close();
	m_keyClient.Close();
	g_imageDetectDlg = NULL;
	g_labelTreeDlg = NULL;
	g_fileTreeDlg = NULL;
	g_scriptTreeDlg = NULL;
	m_matchResult = NULL;
	SAFE_DELETE(m_fileTreeDlg);
	SAFE_DELETE(m_labelTreeDlg);
	SAFE_DELETE(m_scriptTreeDlg);
	SAFE_DELETE(m_streamingDlg);

	cMatchProcessor::Release();
	cvproc::imagematch::ReleaseImageMatch();

	CDialogEx::OnClose();
}


void CImageDetectorDlg::OnBnClickedButtonShowlabelTree()
{
	if (m_labelTreeDlg)
	{
		// 처음 다이얼로그 위치 설정
		static bool mov = true;
		if (mov)
		{
			mov = false;
			CRect wr;
			GetWindowRect(wr);
			CRect dr;
			m_labelTreeDlg->GetWindowRect(dr);
			dr.OffsetRect(-CPoint(dr.left, dr.top));
			dr.OffsetRect(CPoint(wr.right, wr.top));
			dr.OffsetRect(CPoint(0, wr.Height() / 2));
			m_labelTreeDlg->MoveWindow(dr);
		}

		m_labelTreeDlg->ShowWindow(SW_SHOW);
	}
}


void CImageDetectorDlg::OnBnClickedButtonShowStreaming()
{
	Log("Try Connect Capture Program");

	const string ip = GetIP(m_IP);
	if (m_streamingDlg->ShowStreaming(ip, m_Port))
	{
		Log("Success Streaming Connect");
	}
	else
	{
		Log("Error Streaming Connect");
	}
}


void CImageDetectorDlg::OnBnClickedButtonShowImgprocdlg()
{
	m_streamingDlg->AddImageProcessingDlg();
}


void CImageDetectorDlg::OnBnClickedButtonShowscriptTree()
{
	UpdateData();

	m_scriptTreeDlg->ShowWindow(SW_SHOW);
	m_scriptTreeDlg->ShowTree(m_matchScript, wstr2str((LPCTSTR)m_excuteLabel).c_str());
}


void CImageDetectorDlg::OnBnClickedButtonScriptUpdate()
{
	CString fileName;
	m_scriptBrowser.GetWindowTextW(fileName);
	ReadImageMatchScript(wstr2str((LPCTSTR)fileName));

	// menu script loading
	CString str;
	m_browseFlowScript.GetWindowTextW(str);
	const string fileName2 = wstr2str((LPCTSTR)str);
	ReadMenuScript(fileName2);
}


void CImageDetectorDlg::OnBnClickedButtonScriptExecute()
{
	if (m_fileTreeDlg)
		ImageMatch(m_fileTreeDlg->m_selectFileName);
}


// ImageMatch Script 파일을 읽는다.
void CImageDetectorDlg::ReadImageMatchScript(const string &fileName)
{
	if (m_matchScript.Read(fileName))
	{
		cMatchProcessor::Get()->ClearMemPool(); // 메모리 풀 해제.

		const string str = common::format("imagematch script read [%s]\n", fileName.c_str());
		dbg::Log(str.c_str());
		Log(str.c_str());
	}
	else
	{
		dbg::ErrLog("error imagematch script read error!!\n");
		::AfxMessageBox(L"Error Imagematch Script Read Error!!\n");
	}

	m_scriptTreeDlg->ShowTree(m_matchScript, wstr2str((LPCTSTR)m_excuteLabel).c_str());
}


void CImageDetectorDlg::Log(const char* fmt, ...)
{
	char textString[256] = { '\0' };
	va_list args;
	va_start(args, fmt);
	vsnprintf_s(textString, sizeof(textString), _TRUNCATE, fmt, args);
	va_end(args);

	m_logList.AddString(str2wstr(textString).c_str());
	m_logList.SetCurSel(m_logList.GetCount() - 1);
}


void CImageDetectorDlg::OnBnClickedCheckMenuTraverse()
{
	UpdateData();
}


void CImageDetectorDlg::OnBnClickedCheckTreeMatch()
{
	UpdateData();
}


void CImageDetectorDlg::OnBnClickedButtonMenuscriptUpdate()
{
	CString str;
	m_browseFlowScript.GetWindowTextW(str);
	const string fileName = wstr2str((LPCTSTR)str);
	ReadMenuScript(fileName);
}


void CImageDetectorDlg::ReadMenuScript(const string &fileName)
{
	cConfig config;
	config.Read("config.txt");
	const string scriptFileName = config.GetString("script_filename", "dirt3_trackname.txt");
	if (m_flowControl.Init(scriptFileName, fileName, m_captureKey))
	{
		Log("init flow control");
	}
	else
	{
		::AfxMessageBox(L"flow control init error");
	}
}


void CImageDetectorDlg::OnBnClickedCheckMoveMenu()
{
	UpdateData();
}


void CImageDetectorDlg::OnBnClickedButtonMoveScene()
{
	UpdateData();

// 	vector<cGraphScript::sNode*> out;
// 	m_flowControl.m_flowScript.FindRoute("main", "timetrial_ingame_menu", out);

	if (cFlowControl::WAIT == m_flowControl.m_state)
	{
		CString fileName;
		m_browseFlowCmd.GetWindowText(fileName);
		const string cmdFileName = wstr2str((LPCTSTR)fileName);
		m_flowControl.Command(cmdFileName);

		m_buttonMoveScene.SetWindowTextW(L"Stop");
	}
	else
	{
		m_flowControl.Cancel();
		m_buttonMoveScene.SetWindowTextW(L"Move Scene");
	}
}


// 키보드입력 처리
void CImageDetectorDlg::KeyboardProcess(const int vkey)
{
	INPUT input;
	input.type = INPUT_KEYBOARD;
	input.ki.wScan = MapVirtualKey((WORD)vkey, MAPVK_VK_TO_VSC);
	input.ki.time = 0;
	input.ki.dwExtraInfo = 0;
	input.ki.wVk = (WORD)vkey;
	input.ki.dwFlags = KEYEVENTF_SCANCODE;
	SendInput(1, &input, sizeof(INPUT));

	Sleep(100);

	input.ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE;
	SendInput(1, &input, sizeof(INPUT));
}


void CImageDetectorDlg::PrintVirtualKey(const int vkey, const int tab, const string &str) //tab=0, str=""
{
	std::stringstream ss;
	ss << str;

	for (int i = 0; i < tab; ++i)
		ss << "    ";

	switch (vkey)
	{
	case VK_UP: ss << "vkey = VK_UP"; break;
	case VK_DOWN: ss << "vkey = VK_DOWN"; break;
	case VK_LEFT: ss << "vkey = VK_LEFT"; break;
	case VK_RIGHT: ss << "vkey = VK_RIGHT"; break;
	case VK_F9: ss << "vkey = VK_F9"; break;
	case VK_F10: ss << "vkey = VK_F10"; break;
	case VK_F11: ss << "vkey = VK_F11"; break;
	case VK_RETURN: ss << "vkey = VK_RETURN"; break;
	case VK_ESCAPE: ss << "vkey = VK_ESCAPE"; break;
	default:
		ss << vkey;
		break;
	}

	Log(ss.str().c_str());
}


void CImageDetectorDlg::OnBnClickedButtonConnectKeyserver()
{
	UpdateData();

	if (m_isKeySvrConnect)
	{
		m_isKeySvrConnect = false;
		m_keyClient.Close();
		m_buttonKeySvrConnect.SetWindowTextW(L"Connect - Key Server");
		m_buttonSendF11.EnableWindow(FALSE);
	}
	else
	{
		const string ip = GetIP(m_IP);	
		if (m_keyClient.Init(ip, m_editKeyServerPort))
		{
			m_isKeySvrConnect = true;
			m_buttonKeySvrConnect.SetWindowTextW(L"Close");
			m_buttonSendF11.EnableWindow(TRUE);
			Log("Success Connect KeyServer ");
		}
		else
		{
			m_buttonSendF11.EnableWindow(FALSE);
			Log("Error Connect KeyServer ");
		}
	}	
}


void CImageDetectorDlg::OnBnClickedButtonSendF11()
{
	if (m_IsFrapsCapture)
	{
		m_isScreenCapture = true;
		m_frapsCapture.ScreenShot();
	}
	else
	{
		m_keyClient.Send((BYTE*)&m_captureKey, sizeof(m_captureKey));
	}

	PrintVirtualKey(m_captureKey, 0, (m_IsFrapsCapture)? "    FrapsCapture " : "    Send Keyboard  -> ");
}


void CImageDetectorDlg::OnBnClickedButtonRepeat()
{
	const int vkey = m_captureKey;
	if (m_IsFrapsCapture)
	{
		if (m_isRepeatScreenCapture)
		{
			m_isRepeatScreenCapture = false;
		}
		else
		{
			m_isRepeatScreenCapture = true;
			m_repeatDelaySeconds = 0.5f;
		}
	}
}


void CImageDetectorDlg::OnBnClickedCheckNoUithread()
{
	UpdateData();
}


// 모든 로그를 제거한다.
void CImageDetectorDlg::OnBnClickedButtonClearlog()
{
	while (m_logList.GetCount() > 0)
		m_logList.DeleteString(0);
}


string CImageDetectorDlg::GetExecuteTreeName()
{
	//UpdateData();
	return wstr2str((LPCTSTR)m_excuteLabel);
}


void CImageDetectorDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	CRect rcWnd;
	GetWindowRect(&rcWnd);
	HandleAnchors(&rcWnd);
}


void CImageDetectorDlg::OnBnClickedCheckFrapscapture()
{
	UpdateData();
}
