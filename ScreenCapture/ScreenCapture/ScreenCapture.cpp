
// ScreenCapture.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "ScreenCapture.h"
#include "ScreenCaptureDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CScreenCaptureApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

CScreenCaptureApp::CScreenCaptureApp()
{
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
}

CScreenCaptureApp theApp;

BOOL CScreenCaptureApp::InitInstance()
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);
	CWinApp::InitInstance();
	AfxEnableControlContainer();

	//CShellManager *pShellManager = new CShellManager;
	//CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
	//SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CScreenCaptureDlg *dlg = new CScreenCaptureDlg();
	dlg->Create(CScreenCaptureDlg::IDD);
	m_pMainWnd = dlg;
	dlg->ShowWindow(SW_SHOW);
	dlg->Run();

// 	if (pShellManager != NULL)
// 	{
// 		delete pShellManager;
// 	}
	delete dlg;
	return FALSE;
}
