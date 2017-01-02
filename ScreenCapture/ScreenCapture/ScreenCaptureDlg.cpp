
// ScreenCaptureDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ScreenCapture.h"
#include "ScreenCaptureDlg.h"
#include "afxdialogex.h"
#include <direct.h>

#pragma comment (lib, "winmm.lib")

using namespace cv;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CScreenCaptureDlg dialog

CScreenCaptureDlg::CScreenCaptureDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SCREENCAPTURE_DIALOG, pParent)
	, m_loop(true)
	, m_isCapture(false)
	, m_IsPressAlt(TRUE)
	, m_SendPort(8000)
	, m_isServerStart(false)
	, m_IsPrintCapture(FALSE)
	, m_IsGray(TRUE)
	, m_IsCompressed(TRUE)
	, m_editSendDelayTime(0)
	, m_keySvrPort(8001)
	, m_IsStreaming(FALSE)
	, m_isStreamingDelay(false)
	, m_editFraps(_T(""))
	, m_IsFrapsCapture(TRUE)
	, m_isWaitFrapsCapture(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CScreenCaptureDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_LOG, m_logList);
	DDX_Check(pDX, IDC_CHECK_ALT, m_IsPressAlt);
	DDX_Text(pDX, IDC_EDIT_PORT, m_SendPort);
	DDX_Control(pDX, IDC_BUTTON_SERVERSTART, m_ServerStartButton);
	DDX_Check(pDX, IDC_CHECK_PRINT_CAPTURE, m_IsPrintCapture);
	DDX_Check(pDX, IDC_CHECK_GRAY, m_IsGray);
	DDX_Check(pDX, IDC_CHECK_COMPRESSED, m_IsCompressed);
	DDX_Control(pDX, IDC_COMBO_COMPRESS, m_CompressCombo);
	DDX_Text(pDX, IDC_EDIT_SND_DELAY_TIME, m_editSendDelayTime);
	DDX_Text(pDX, IDC_EDIT_KEYPORT, m_keySvrPort);
	DDX_Check(pDX, IDC_CHECK_STREAMING, m_IsStreaming);
	DDX_Text(pDX, IDC_EDIT_FRAPS, m_editFraps);
	DDX_Check(pDX, IDC_CHECK_FRAPS, m_IsFrapsCapture);
}

BEGIN_MESSAGE_MAP(CScreenCaptureDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CScreenCaptureDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CScreenCaptureDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_CHECK_ALT, &CScreenCaptureDlg::OnBnClickedCheckAlt)
	ON_BN_CLICKED(IDC_BUTTON_SERVERSTART, &CScreenCaptureDlg::OnBnClickedButtonServerstart)
	ON_BN_CLICKED(IDC_CHECK_PRINT_CAPTURE, &CScreenCaptureDlg::OnBnClickedCheckPrintCapture)
	ON_BN_CLICKED(IDC_CHECK_GRAY, &CScreenCaptureDlg::OnBnClickedCheckGray)
	ON_BN_CLICKED(IDC_CHECK_COMPRESSED, &CScreenCaptureDlg::OnBnClickedCheckCompressed)
	ON_BN_CLICKED(IDC_CHECK_STREAMING, &CScreenCaptureDlg::OnBnClickedCheckStreaming)
	ON_BN_CLICKED(IDC_CHECK_FRAPS, &CScreenCaptureDlg::OnBnClickedCheckFraps)
END_MESSAGE_MAP()


// CScreenCaptureDlg message handlers

BOOL CScreenCaptureDlg::OnInitDialog()
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

	CreateDirectoryA("capture", NULL);
	dbg::RemoveErrLog();
	dbg::RemoveLog();

	cConfig config;
	config.Read("screencapture_config.txt");
	m_captureKey = config.GetInt("capture_key", VK_F11);

	m_CompressCombo.AddString(L"40");
	m_CompressCombo.AddString(L"50");
	m_CompressCombo.AddString(L"60");
	m_CompressCombo.AddString(L"70");
	m_CompressCombo.AddString(L"80");
	m_CompressCombo.AddString(L"90");
	m_CompressCombo.AddString(L"100");
	m_CompressCombo.SetCurSel(6);
	m_CompressCombo.EnableWindow(FALSE);

	m_editFraps = L"C:\\Fraps\\Screenshots\\";
	UpdateData(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CScreenCaptureDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{

	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}


void CScreenCaptureDlg::OnPaint()
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

HCURSOR CScreenCaptureDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
void CScreenCaptureDlg::OnBnClickedOk()
{
}
void CScreenCaptureDlg::OnBnClickedCancel()
{
	m_loop = false;
}


void CScreenCaptureDlg::Run()
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
		}

		const int curT = timeGetTime();
		if (curT - oldT > 20)
		{
			const float deltaSeconds = (curT - oldT) * 0.001f;
			oldT = curT;
			MainLoop(deltaSeconds);
		}

		Sleep(5);
	}
}


void CScreenCaptureDlg::MainLoop(const float deltaSeconds)
{
	if (m_isStreamingDelay)
	{
		--m_delayFrame;
		if (m_delayFrame > 0)
			return;
	}

	const bool isCapture = CheckKeyDownUp();
	const bool IsStreamingSend = m_streamSender.IsExistClient() && m_IsStreaming;

	if (IsStreamingSend || isCapture)
		Capture();

	// check keyboard server
	network::sSockBuffer packet;
	if (m_keyServer.m_recvQueue.Front(packet))
	{
		if (packet.readLen >= 4)
		{
			const int vkey = *(int*)packet.buffer;
			if (m_captureKey == vkey)
				Capture();

			KeyboardProcess(vkey);
			m_keyServer.m_recvQueue.Pop();
		}
	}

	if (m_isWaitFrapsCapture && m_frapsCapture.IsUpdateScreenShot())
	{
		m_isWaitFrapsCapture = false;
		CaptureProcess(m_frapsCapture.m_img);
	}

	if (IsStreamingSend)
	{
		m_isStreamingDelay = true;
		m_delayFrame = 3;
	}
}


void CScreenCaptureDlg::Capture()
{
	if (m_IsFrapsCapture)
	{
		if (!m_isWaitFrapsCapture)
		{
			m_frapsCapture.ScreenShot();
			m_isWaitFrapsCapture = true;
		}
	}
	else
	{
		cvproc::cScreenCapture capture;
		cv::Mat img = capture.ScreenCapture(m_IsPressAlt ? true : false);
		if (!img.empty())
		{
			CaptureProcess(img);
		}
		else
		{
			// 에러시 출력
			dbg::Log("error print screen \n");
			Log("error print screen ");
		}
	}
}


bool CScreenCaptureDlg::CheckKeyDownUp()
{
	bool isCapture = false;
	if (::GetAsyncKeyState(VK_F11))
	{
		m_isCapture = true;
	}
	else
	{
		if (m_isCapture)
		{
			if (!::GetAsyncKeyState(VK_F11))
			{
				m_isCapture = false;
				isCapture = true;
			}
		}
	}
	return isCapture;
}


void CScreenCaptureDlg::CaptureProcess(const cv::Mat &img)
{
	if (!img.data)
	{
		dbg::ErrLog("CaptureProcess() Error, no image pointer\n");
		Log("Error!! capture ");
		return;
	}

	if (!m_IsStreaming) // 스트리밍이 아닐 때만 저장한다.
	{
		// capture/capture% 파일로 저장.
		stringstream ss;
		static int cnt = 1;
		ss << "capture/capture" << cnt++ << ".jpg";
		imwrite(ss.str().c_str(), img);
		m_logList.AddString(str2wstr(ss.str()).c_str());
		m_logList.SetCurSel(m_logList.GetCount() - 1);
		dbg::Log("%s\n", ss.str().c_str());
	}

	// 캡쳐한 이미지 화면에 출력
	if (m_IsPrintCapture)
	{
		Mat dst(300, 300, img.flags);
		resize(img, dst, Size(300, 300));
		imshow("capture", dst);
		resizeWindow("capture", 300, 300);
	}

	// 캡쳐한 이미지 네트워크로 전송
	// send image
	if (m_isServerStart)
	{
		UpdateData(); // m_editSendDelayTime update

		dbg::Log("Begin Send >>\n");
		while (!m_streamSender.Send(img))
			Sleep(m_editSendDelayTime);
		dbg::Log("End Send \n");
	}
}


// 네트워크로 들어온 키보드입력 처리
void CScreenCaptureDlg::KeyboardProcess(const int vkey)
{
	PrintVertualKey(vkey);

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


void CScreenCaptureDlg::OnBnClickedCheckAlt()
{
	UpdateData();
}


void CScreenCaptureDlg::OnBnClickedButtonServerstart()
{
	UpdateData();

	if (m_isServerStart)
	{
		goto closeserver;
	}
	else
	{
		CString compVal;
		m_CompressCombo.GetWindowTextW(compVal);
		const int jpgQuality = _wtoi((LPCTSTR)compVal);

		if (m_streamSender.Init(m_SendPort, m_IsGray? true : false, 
			m_IsCompressed? true : false, jpgQuality))
		{
			m_streamSender.m_deltaTime = 10;
			m_isServerStart = true;
			m_ServerStartButton.SetWindowTextW(L"Stop Server");

			m_logList.AddString(L"Server Start");
			m_logList.SetCurSel(m_logList.GetCount() - 1);
			dbg::Log("Server Start \n");
			SetBackgroundColor(g_blueColor);

			GetDlgItem(IDC_CHECK_GRAY)->EnableWindow(FALSE);
			GetDlgItem(IDC_CHECK_COMPRESSED)->EnableWindow(FALSE);
			GetDlgItem(IDC_COMBO_COMPRESS)->EnableWindow(FALSE);
		}
		else
		{
			dbg::Log("Server Bind Error \n");
			m_logList.AddString(L"Server Bind Error");
			m_logList.SetCurSel(m_logList.GetCount() - 1);
			::AfxMessageBox(L"Error!! Bind Server\n");
		}

		// keyboard 서버 바인딩
		if (m_keyServer.Init(m_keySvrPort))
		{
			m_logList.AddString(L"Keyboard Server Start");
			m_logList.SetCurSel(m_logList.GetCount() - 1);
			dbg::Log("Keyboard Server Start \n");
		}
		else
		{
			goto closeserver;
		}

		// FrapsCapture 
		m_frapsCapture.Init( wstr2str((LPCTSTR)m_editFraps), VK_F9);
	}

	return;

closeserver:
	// 모든 서버를 종료시킨다.
	m_streamSender.Close();
	m_keyServer.Close();
	m_ServerStartButton.SetWindowTextW(L"Server Start");
	m_isServerStart = false;
	m_isWaitFrapsCapture = false;

	m_logList.AddString(L"Close Server");
	m_logList.AddString(L"Close Keyboard Server");
	m_logList.SetCurSel(m_logList.GetCount() - 1);
	dbg::Log("Close Server \n");
	dbg::Log("Close Keyboard Server \n");
	SetBackgroundColor(g_grayColor);

	GetDlgItem(IDC_CHECK_GRAY)->EnableWindow(TRUE);
	GetDlgItem(IDC_CHECK_COMPRESSED)->EnableWindow(TRUE);
	GetDlgItem(IDC_COMBO_COMPRESS)->EnableWindow(m_IsCompressed);
}


void CScreenCaptureDlg::OnBnClickedCheckPrintCapture()
{
	UpdateData();
}


void CScreenCaptureDlg::OnBnClickedCheckGray()
{
	UpdateData();
}


void CScreenCaptureDlg::OnBnClickedCheckCompressed()
{
	UpdateData();
	m_CompressCombo.EnableWindow(m_IsCompressed);
}


void CScreenCaptureDlg::PrintVertualKey(const int vkey)
{
	std::stringstream ss;

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
		ss << "vkey = " << vkey;
		break;
	}

	Log(ss.str().c_str());
}


void CScreenCaptureDlg::Log(const char* fmt, ...)
{
	char textString[256] = { '\0' };
	va_list args;
	va_start(args, fmt);
	vsnprintf_s(textString, sizeof(textString), _TRUNCATE, fmt, args);
	va_end(args);

	m_logList.AddString(str2wstr(textString).c_str());
	m_logList.SetCurSel(m_logList.GetCount() - 1);
}


void CScreenCaptureDlg::OnBnClickedCheckStreaming()
{
	UpdateData();

	m_isWaitFrapsCapture = false;

	if (m_IsStreaming)
	{
		Log("Streaming On");
	}
	else
	{
		Log("Streaming Off");
	}
}


void CScreenCaptureDlg::OnBnClickedCheckFraps()
{
	UpdateData();

	if (m_IsFrapsCapture)
	{
		GetDlgItem(IDC_EDIT_FRAPS)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_EDIT_FRAPS)->EnableWindow(FALSE);
	}
}
