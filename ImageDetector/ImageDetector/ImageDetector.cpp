
#include "stdafx.h"
#include "ImageDetector.h"
#include "ImageDetectorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CImageDetectorApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

CImageDetectorApp::CImageDetectorApp()
{
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
}
CImageDetectorApp theApp;

BOOL CImageDetectorApp::InitInstance()
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);
	CWinApp::InitInstance();
	AfxEnableControlContainer();
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	CImageDetectorDlg *dlg = new CImageDetectorDlg();
	dlg->Create(CImageDetectorDlg::IDD);
	dlg->EnableWindow(TRUE);
	m_pMainWnd = dlg;
	dlg->ShowWindow(SW_SHOW);
	dlg->Run();
	delete dlg;
	return FALSE;
}

