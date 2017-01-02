
#include "stdafx.h"
#include "resource.h"		// main symbols
#include <mmsystem.h>
#include "StrCompareView.h"
#pragma comment(lib, "winmm.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class CDictionaryApp : public CWinApp
{
public:
	CDictionaryApp();
	virtual BOOL InitInstance();
	DECLARE_MESSAGE_MAP()
};

class CDictionaryDlg : public CDialogEx
{
public:
	CDictionaryDlg(CWnd* pParent = NULL);
	enum { IDD = IDD_DICTIONARY_DIALOG };
	virtual void DoDataExchange(CDataExchange* pDX);
	void Run();

	HICON m_hIcon;
	bool m_loop;
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	DECLARE_ANCHOR_MAP();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

extern CDictionaryApp theApp;

BEGIN_MESSAGE_MAP(CDictionaryApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

CDictionaryApp::CDictionaryApp()
{
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
}

CDictionaryApp theApp;

BOOL CDictionaryApp::InitInstance()
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);
	CWinApp::InitInstance();
	AfxEnableControlContainer();
	CDictionaryDlg *dlg = new CDictionaryDlg();
	dlg->Create(CDictionaryDlg::IDD);
	m_pMainWnd = dlg;
	dlg->ShowWindow(SW_SHOW);
	dlg->Run();
	dlg->DestroyWindow();
	delete dlg;
	ControlBarCleanUp();
	return FALSE;
}


CDictionaryDlg::CDictionaryDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DICTIONARY_DIALOG, pParent)
	, m_loop(true)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDictionaryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDictionaryDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CDictionaryDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDictionaryDlg::OnBnClickedCancel)
	ON_WM_SIZE()
END_MESSAGE_MAP()

enum {
	ID_VIEW = 100,
};

BEGIN_ANCHOR_MAP(CDictionaryDlg)
	ANCHOR_MAP_ENTRY(ID_VIEW, ANF_LEFT | ANF_RIGHT | ANF_TOP | ANF_BOTTOM)
END_ANCHOR_MAP()

BOOL CDictionaryDlg::OnInitDialog()
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

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	CRect cr;
	GetClientRect(cr);
	CStrCompareView *view = new CStrCompareView();
	view->Create(NULL, NULL, WS_VISIBLE | WS_CHILDWINDOW | WS_EX_WINDOWEDGE, cr, this, ID_VIEW);
	view->OnInitialUpdate();
	view->ShowWindow(SW_SHOW);
	view->Init();

	InitAnchors();

	return TRUE;
}

void CDictionaryDlg::OnBnClickedOk()
{
	CDialogEx::OnOK();
}
void CDictionaryDlg::OnBnClickedCancel()
{
	m_loop = false;
}

void CDictionaryDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	CRect rcWnd;
	GetWindowRect(&rcWnd);
	HandleAnchors(&rcWnd);
}


void CDictionaryDlg::Run()
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
			//MainLoop(deltaSeconds);
		}
	}
}
