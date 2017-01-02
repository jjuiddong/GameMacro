
#include "stdafx.h"
#include "ImageWatch.h"
#include "ImageWatchDlg.h"
#include "afxdialogex.h"
#include "simplematchscript.h"

#pragma comment (lib, "winmm.lib")


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CImageWatchDlg::CImageWatchDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_IMAGEWATCH_DIALOG, pParent)
	, m_loop(true)
	, m_checkGrid(TRUE)
	, m_checkText(TRUE)
	, m_checkTextbg(FALSE)
	, m_checkWhite(TRUE)
	, m_editCmd(_T(""))
	, m_checkAutoColor(TRUE)
	, m_isHideCmdWindow(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CImageWatchDlg::~CImageWatchDlg()
{	
}


void CImageWatchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_GRID, m_checkGrid);
	DDX_Check(pDX, IDC_CHECK_TEXT, m_checkText);
	DDX_Check(pDX, IDC_CHECK_TEXTBG, m_checkTextbg);
	DDX_Check(pDX, IDC_CHECK_WHITE, m_checkWhite);
	DDX_Control(pDX, IDC_STATIC_INFO, m_staticInfo);
	DDX_Text(pDX, IDC_EDIT_CMD, m_editCmd);
	DDX_Check(pDX, IDC_CHECK_AUTOCOLOR, m_checkAutoColor);
}

BEGIN_MESSAGE_MAP(CImageWatchDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CImageWatchDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CImageWatchDlg::OnBnClickedCancel)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_CHECK_GRID, &CImageWatchDlg::OnBnClickedCheckGrid)
	ON_BN_CLICKED(IDC_CHECK_TEXT, &CImageWatchDlg::OnBnClickedCheckText)
	ON_BN_CLICKED(IDC_CHECK_TEXTBG, &CImageWatchDlg::OnBnClickedCheckTextbg)
	ON_BN_CLICKED(IDC_CHECK_WHITE, &CImageWatchDlg::OnBnClickedCheckWhite)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE, &CImageWatchDlg::OnBnClickedButtonUpdate)
	ON_BN_CLICKED(IDC_BUTTON_HELP, &CImageWatchDlg::OnBnClickedButtonHelp)
	ON_WM_SIZE()
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_CHECK_AUTOCOLOR, &CImageWatchDlg::OnBnClickedCheckAutocolor)
	ON_BN_CLICKED(IDC_BUTTON_HIDE_CMDWINDOW, &CImageWatchDlg::OnBnClickedButtonHideCmdwindow)
END_MESSAGE_MAP()

BEGIN_ANCHOR_MAP(CImageWatchDlg)
	ANCHOR_MAP_ENTRY(IDC_STATIC_INFO, ANF_LEFT | ANF_RIGHT | ANF_TOP)
	ANCHOR_MAP_ENTRY(IDC_STATIC1, ANF_LEFT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_EDIT_CMD, ANF_LEFT | ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_STATIC_PICTURE, ANF_LEFT | ANF_RIGHT | ANF_TOP | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_UPDATE, ANF_LEFT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_HIDE_CMDWINDOW, ANF_LEFT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_HELP, ANF_RIGHT| ANF_BOTTOM)
END_ANCHOR_MAP()


BOOL CImageWatchDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	InitAnchors();

	CGdiPlus::Init();

	m_imgWindow = new CImageWatchWindow();
	m_imgWindow->Create(NULL, L"CamWindow", WS_VISIBLE | WS_CHILDWINDOW, CRect(0, 0, 100, 100), this, 1100);
	m_imgWindow->SetScrollSizes(MM_TEXT, CSize(100, 100));

	if (CWnd *wnd = GetDlgItem(IDC_STATIC_PICTURE))
	{
		CRect cr;
		wnd->GetWindowRect(cr);
		ScreenToClient(cr);
		m_imgWindow->MoveWindow(cr);
	}

	m_imgWindow->ShowWindow(SW_SHOW);

	return TRUE;
}


void CImageWatchDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	CRect rcWnd;
	GetWindowRect(&rcWnd);
	HandleAnchors(&rcWnd);

	if (CWnd *wnd = GetDlgItem(IDC_STATIC_PICTURE))
	{
		CRect cr;
		wnd->GetWindowRect(cr);
		ScreenToClient(cr);
		m_imgWindow->MoveWindow(cr);
	}
}


void CImageWatchDlg::OnPaint()
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

HCURSOR CImageWatchDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CImageWatchDlg::OnBnClickedOk()
{
//	CDialogEx::OnOK();
}


void CImageWatchDlg::OnBnClickedCancel()
{
//	CDialogEx::OnCancel();
}


void CImageWatchDlg::Run()
{
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

		Sleep(0);
	}
}


void CImageWatchDlg::MainLoop(const float deltaSeconds)
{
	if ((GetActiveWindow() == this) &&
		(0x8000 & GetAsyncKeyState(VK_F5)))
	{
		OnBnClickedButtonUpdate();
	}

	if ((GetActiveWindow() == this) &&
		(0x8000 & GetAsyncKeyState(VK_F6)))
	{
		Sleep(100);
		OnBnClickedButtonHideCmdwindow();
	}

	// Update Mouse Position pixel information
	static string oldStr;
	if (oldStr != m_imgWindow->m_infoStr)
	{
		oldStr = m_imgWindow->m_infoStr;
		m_staticInfo.SetWindowTextW(str2wstr(oldStr).c_str());
	}
}


void CImageWatchDlg::OnClose()
{
	m_loop = false;
	CDialogEx::OnClose();
}


void CImageWatchDlg::OnBnClickedCheckGrid()
{
	UpdateData();
	m_imgWindow->ShowGrid(m_checkGrid);
	m_imgWindow->InvalidateRect(NULL);
}


void CImageWatchDlg::OnBnClickedCheckText()
{
	UpdateData();
	m_imgWindow->ShowText(m_checkText);
	m_imgWindow->InvalidateRect(NULL);
}


void CImageWatchDlg::OnBnClickedCheckTextbg()
{
	UpdateData();
	m_imgWindow->ShowTextBackground(m_checkTextbg);
	m_imgWindow->InvalidateRect(NULL);
}


void CImageWatchDlg::OnBnClickedCheckWhite()
{
	UpdateData();
	m_imgWindow->ShowBlackandWhite(m_checkWhite);
	m_imgWindow->InvalidateRect(NULL);
}


void CImageWatchDlg::OnBnClickedCheckAutocolor()
{
	UpdateData();
	m_imgWindow->m_isAutoColor = m_checkAutoColor;
	m_imgWindow->InvalidateRect(NULL);
}



void CImageWatchDlg::OnBnClickedButtonUpdate()
{
	UpdateData();
	string cmd = wstr2str((LPCTSTR)m_editCmd);

	cSimpleMatchScript script;
	const string errMsg = script.Match(m_srcImg, m_dstImg, cmd, false);
	m_imgWindow->Render(m_dstImg);
}


void CImageWatchDlg::OnBnClickedButtonHideCmdwindow()
{
	static int offsetH = 0;

	m_isHideCmdWindow = !m_isHideCmdWindow;

	if (m_isHideCmdWindow)
	{	
		GetDlgItem(IDC_EDIT_CMD)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC1)->ShowWindow(SW_HIDE);

		CRect wr;
		GetDlgItem(IDC_STATIC_PICTURE)->GetWindowRect(wr);
		ScreenToClient(wr);

		CRect bwr;
		GetDlgItem(IDC_BUTTON_UPDATE)->GetWindowRect(bwr);
		ScreenToClient(bwr);

		if (offsetH == 0)
			offsetH = bwr.top - wr.bottom - 2;
		wr.bottom = bwr.top - 2;

		GetDlgItem(IDC_STATIC_PICTURE)->MoveWindow(wr);
		m_imgWindow->MoveWindow(wr);
		InitAnchors();
	}
	else
	{
		GetDlgItem(IDC_EDIT_CMD)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STATIC1)->ShowWindow(SW_SHOW);

		CRect wr;
		GetDlgItem(IDC_STATIC_PICTURE)->GetWindowRect(wr);
		ScreenToClient(wr);
		wr.bottom -= offsetH;
		GetDlgItem(IDC_STATIC_PICTURE)->MoveWindow(wr);
		m_imgWindow->MoveWindow(wr);
		InitAnchors();
	}
}


void CImageWatchDlg::OnBnClickedButtonHelp()
{
	string str =
		"\r\n"\
		"Script Command List \r\n"\
		"-------------------------------------------------------------------------------\r\n"\
		"\r\n"\
		"reload \r\n"\
		"gray \r\n"\
		"erode \r\n"\
		"dilate  \r\n"\
		"roi = 0, 0, 100, 100 \r\n"\
		"rect = 0, 0, 100, 100 \r\n"\
		"scale = 1.5 \r\n"\
		"	-> dst *= num \r\n"\
		"threshold = 100 \r\n"\
		"adapthreshold = blocksize, threshold \r\n"\
		"invert = threshold \r\n"\
		"\r\n"\
		"cvt = bgr-hsv/bgr-hls/bgr-gray/hsv-bgr/hls-bgr/gray-bgr \r\n"\
		"hsv = 80, 100, 100, 90, 255, 255 \r\n"\
		"	-> bgr-hsv; inRange(80, 100, 100, 90, 255, 255) \r\n"\
		"hls = 80, 100, 100, 90, 255, 255 \r\n"\
		"	-> bgr-hls; inRange(80, 100, 100, 90, 255, 255) \r\n"\
		"bgr = 10, 20, 30 \r\n"\
		"	-> dst &= scalar(10,20,30) \r\n"\
		"\r\n"\
		"acc \r\n"\
		"hough \r\n"\
		"canny = threshold \r\n"\
		"findcontours = approxPolyDP_alpha \r\n"\
		"\r\n"\
		;	std::ofstream ofs("imgprocdlg_help.txt");	ofs << str;	ofs.close();
	ShellExecuteW(m_hWnd, L"open", L"notepad", L"imgprocdlg_help.txt", NULL, SW_SHOW);
}


void CImageWatchDlg::OnDropFiles(HDROP hDropInfo)
{
	if (hDropInfo != NULL)
	{
		DWORD dwFileSize = 0;
		char * pFileName = NULL;
		dwFileSize = DragQueryFileA(hDropInfo, 0, NULL, 0);
		pFileName = new char[dwFileSize + 1];
		dwFileSize = DragQueryFileA(hDropInfo, 0, pFileName, dwFileSize + 1);
		m_srcImg = cv::imread(pFileName);
		delete[] pFileName;

		m_imgWindow->Render(m_srcImg);
	}

	CDialogEx::OnDropFiles(hDropInfo);
}
