
#include "stdafx.h"
#include "ImageDetector.h"
#include "StreamingDialog.h"
#include "afxdialogex.h"
#include "ImageDetectorDlg.h"
#include "FileTreeDialog.h"
#include "LabelTreeDialog.h"
#include "ImageProcessingDlg.h"

using namespace cv;

void StreamingTask(CStreamingDialog *arg);
void StreamingDlgUITask(CStreamingDialog *arg);


CStreamingDialog::CStreamingDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_STREAMING, pParent)
	, m_editCmd(_T(""))
	, m_isShow(false)
	, m_threadLoop(true)
	, m_sleepMillis(1)
	, m_radioMode(0)
	, m_playMode(PLAY)
	, m_updateOption(false)
	, m_updateSrcImage(false)
	//, m_staticFPS(0)
	, m_isUIUpdate(false)
	, m_streamingFPS(0)
{
}

CStreamingDialog::~CStreamingDialog()
{
	m_threadLoop = false;
	if (m_thread.joinable()) // 쓰레드 종료.
		m_thread.join();

	m_isRunUIThread = false;
	if (m_UIThread.joinable())
		m_UIThread.join();

	for each (auto dlg in m_imgProcDlgs)
	{
		dlg->DestroyWindow();
		if (g_imageDetectDlg)
			removevector(g_imageDetectDlg->m_updateListener, dlg);
		delete dlg;
	}
	m_imgProcDlgs.clear();

}

void CStreamingDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_COMMAND, m_editCmd);
	DDX_Radio(pDX, IDC_RADIO_STREAMING, m_radioMode);
	DDX_Control(pDX, IDC_BUTTON_PAUSE, m_btnPause);
	DDX_Control(pDX, IDC_STATIC_FPS, m_staticFPS);
}


BEGIN_MESSAGE_MAP(CStreamingDialog, CDialogEx)
	ON_BN_CLICKED(IDOK, &CStreamingDialog::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CStreamingDialog::OnBnClickedCancel)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_UPDATE, &CStreamingDialog::OnBnClickedButtonUpdate)
	ON_BN_CLICKED(IDC_BUTTON_HELP, &CStreamingDialog::OnBnClickedButtonHelp)
	ON_BN_CLICKED(IDC_RADIO_STREAMING, &CStreamingDialog::OnBnClickedRadioStreaming)
	ON_BN_CLICKED(IDC_RADIO_IMAGE, &CStreamingDialog::OnBnClickedRadioImage)
	ON_BN_CLICKED(IDC_BUTTON_TEMPLATE_MATCH, &CStreamingDialog::OnBnClickedButtonTemplateMatch)
	ON_BN_CLICKED(IDC_BUTTON_FEATURE_MATCH, &CStreamingDialog::OnBnClickedButtonFeatureMatch)
	ON_BN_CLICKED(IDC_BUTTON_IMAGEMATCH, &CStreamingDialog::OnBnClickedButtonImagematch)
	ON_BN_CLICKED(IDC_BUTTON_CAPTURE, &CStreamingDialog::OnBnClickedButtonCapture)
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, &CStreamingDialog::OnBnClickedButtonPause)
	ON_BN_CLICKED(IDC_BUTTON_NEWWINDOW, &CStreamingDialog::OnBnClickedButtonNewwindow)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


BEGIN_ANCHOR_MAP(CStreamingDialog)
	ANCHOR_MAP_ENTRY(IDC_STATIC_VIEW, ANF_LEFT | ANF_RIGHT | ANF_TOP | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_EDIT_COMMAND, ANF_LEFT | ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_HELP, ANF_LEFT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_UPDATE, ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_FEATURE_MATCH, ANF_LEFT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_TEMPLATE_MATCH, ANF_LEFT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_IMAGEMATCH, ANF_LEFT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_CAPTURE, ANF_LEFT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_PAUSE, ANF_LEFT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_NEWWINDOW, ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_STATIC1, ANF_LEFT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_STATIC_FPS, ANF_RIGHT | ANF_TOP)
END_ANCHOR_MAP()



// CStreamingDialog message handlers
void CStreamingDialog::OnBnClickedOk()
{
	//CDialogEx::OnOK();
}

void CStreamingDialog::OnBnClickedCancel()
{
	m_isShow = false;

	m_threadLoop = false;
	if (m_thread.joinable()) // 쓰레드 종료.
		m_thread.join();

	CDialogEx::OnCancel();
}

BOOL CStreamingDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	InitAnchors();

	CGdiPlus::Init();

	m_captureScreen1 = new CCamWindow();
	m_captureScreen1->Create(NULL, L"CamWindow", WS_VISIBLE | WS_CHILDWINDOW, CRect(0, 0, 100, 100), this, 1100);
	m_captureScreen1->SetScrollSizes(MM_TEXT, CSize(100, 100));

	m_captureScreen2 = new CCamWindow();
	m_captureScreen2->Create(NULL, L"CamWindow", WS_VISIBLE | WS_CHILDWINDOW, CRect(0, 0, 100, 100), this, 1100);
	m_captureScreen2->SetScrollSizes(MM_TEXT, CSize(100, 100));

	if (CWnd *wnd = GetDlgItem(IDC_STATIC_VIEW))
	{
		CRect cr;
		wnd->GetWindowRect(cr);
		ScreenToClient(cr);
		CRect r1 = cr;
		r1.right = cr.Width() / 2 + cr.left;
		CRect r2 = cr;
		r2.left += cr.Width() / 2;
		m_captureScreen1->MoveWindow(r1);
		m_captureScreen2->MoveWindow(r2);
	}

	m_captureScreen1->ShowWindow(SW_SHOW);
	m_captureScreen2->ShowWindow(SW_SHOW);

	m_editCmd = L"hsv=35,0,100,120,255,255 gray=1";

	m_isRunUIThread = true;
	m_UIThread = std::thread(StreamingDlgUITask, this);

	UpdateData(FALSE);

	return TRUE;
}


void CStreamingDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	
	CRect rcWnd;
	GetWindowRect(&rcWnd);
	HandleAnchors(&rcWnd);

	if (CWnd *wnd = GetDlgItem(IDC_STATIC_VIEW))
	{
		CRect cr;
		wnd->GetWindowRect(cr);
		ScreenToClient(cr);
		CRect r1 = cr;
		r1.right = cr.Width() / 2 + cr.left;
		CRect r2 = cr;
		r2.left += cr.Width() / 2;
		m_captureScreen1->MoveWindow(r1);
		m_captureScreen2->MoveWindow(r2);
	}
}


void CStreamingDialog::OnBnClickedButtonUpdate()
{
	UpdateData();
	AutoCSLock cs(m_cs);
	m_optionStr = wstr2str((LPCTSTR)m_editCmd);
	m_updateOption = true;
}


void CStreamingDialog::OnBnClickedButtonHelp()
{
	g_imageDetectDlg->Log("------------------------------------------------------------");
	g_imageDetectDlg->Log("Streaming Command >> ");
	g_imageDetectDlg->Log("    - bgr=num1,num2,num3");
	g_imageDetectDlg->Log("        - Mat &= Scalar(num1,num2,num3)");
	g_imageDetectDlg->Log("    - scale=num");
	g_imageDetectDlg->Log("        - Mat *= num");
	g_imageDetectDlg->Log("    - gray=0/1");
	g_imageDetectDlg->Log("        - gray convert");
	g_imageDetectDlg->Log("    - hsv=num1,num2,num3,num4,num5,num6");
	g_imageDetectDlg->Log("        - hsv converting, inRange( Scalar(num1,num2,num3), Scalar(num4,num5,num6) )");
}


bool CStreamingDialog::ShowStreaming(const string &ip, const int port)
{
	m_isShow = true;
	m_ip = ip;
	m_port = port;

	// Streaming Receive Thread 
	m_threadLoop = false;
	if (m_thread.joinable()) // 쓰레드 종료.
		m_thread.join();

	m_threadLoop = true;
	m_thread = std::thread(StreamingTask, this);

	ShowWindow(SW_SHOW);
	return true;
}


string CStreamingDialog::GetOptionStr()
{
	return m_optionStr;
}


void CStreamingDialog::OnBnClickedRadioStreaming()
{
	UpdateData();
}
void CStreamingDialog::OnBnClickedRadioImage()
{
	UpdateData();
}

void CStreamingDialog::SetImageMode(const cv::Mat &img)
{
	AutoCSLock cs(m_cs);
	m_srcImage = img.clone();
	m_radioMode = IMAGE;
	m_updateSrcImage = true;
	UpdateData(FALSE);
}


void CStreamingDialog::OnBnClickedButtonTemplateMatch()
{
	AutoCSLock cs(m_cs);

	if (!m_cvtImage.data)
	{
		AfxMessageBox(L"Nothing Scene Image \n");
		return;
	}

	g_labelTreeDlg->UpdateData();
	Mat label = imread(g_labelTreeDlg->m_selectFileName);
	if (!label.data)
	{
		AfxMessageBox(L"Select Label Image From Label Tree Dialog \n");
		return;
	}

	cvproc::cConvertImage cvtImg;
	cvtImg.Convert(label, label, wstr2str((LPCTSTR)g_labelTreeDlg->m_strMatchOption));

	if (m_cvtImage.channels() != label.channels())
	{
		AfxMessageBox(L"Scene, Object Image Channel is Different \n");
		return;
	}

	const int t1 = timeGetTime();
	g_imageDetectDlg->Log("Template Match Start << %s", g_fileTreeDlg->m_selectFileName.c_str());
	cvproc::cTemplateMatch match(0.7f, true, true);
	match.Match(m_cvtImage, label, "Template Match");
	const int t2 = timeGetTime();
	g_imageDetectDlg->Log("Template Match End, elpaseTime = %d millisecond", t2 - t1);
}


void CStreamingDialog::OnBnClickedButtonFeatureMatch()
{
	AutoCSLock cs(m_cs);

	if (!m_cvtImage.data)
	{
		AfxMessageBox(L"Nothing Scene Image \n");
		return;
	}

	g_labelTreeDlg->UpdateData();
	Mat label = imread(g_labelTreeDlg->m_selectFileName);
	if (!label.data)
	{
		AfxMessageBox(L"Select Label Image From Label Tree Dialog \n");
		return;
	}

	cvproc::cConvertImage cvtImg;
	cvtImg.Convert(label, label, wstr2str((LPCTSTR)g_labelTreeDlg->m_strMatchOption));

	if (m_cvtImage.channels() != label.channels())
	{
		AfxMessageBox(L"Scene, Object Image Channel is Different \n");
		return;
	}

	const int t1 = timeGetTime();
	g_imageDetectDlg->Log("Feature Match Start << %s", g_fileTreeDlg->m_selectFileName.c_str());
	cvproc::cFeatureMatch match(true, true);
	match.Match(m_cvtImage, label, "Feature Match");
	const int t2 = timeGetTime();
	g_imageDetectDlg->Log("Feature Match End, elpaseTime = %d millisecond", t2 - t1);
}


void CStreamingDialog::OnBnClickedButtonImagematch()
{
	if (g_imageDetectDlg)
	{
		g_imageDetectDlg->ImageMatch(m_cvtImage, "StreamingDlg ImageMatch");
	}
}


void CStreamingDialog::OnBnClickedButtonCapture()
{
	AutoCSLock cs(m_cs);
	if (m_captureScreen1 && m_captureScreen1->m_bitmap)
	{
		if (m_streamRcver.m_cloneImage.data)
		{
			const string fileName = g_imageDetectDlg->GenerateCaptureFileName();
			imwrite(fileName, m_streamRcver.m_cloneImage);
		}
		else
		{
			::AfxMessageBox(L"Error!! not have receive image ");
		}
	}
}


void CStreamingDialog::OnBnClickedButtonPause()
{
	if (m_playMode == PLAY)
	{
		m_playMode = PAUSE;
		m_btnPause.SetWindowTextW(L"Play");
	}
	else // PAUSE
	{
		m_playMode = PLAY;
		m_btnPause.SetWindowTextW(L"Pause");
	}	
}


void CStreamingDialog::OnBnClickedButtonNewwindow()
{
	AddImageProcessingDlg();
}


CImageProcessingDlg* CStreamingDialog::AddImageProcessingDlg()
{
	CImageProcessingDlg *dlg = new CImageProcessingDlg();
	dlg->Create(CImageProcessingDlg::IDD);

	{
		AutoCSLock cs(m_cs);
		if (m_radioMode == STREAMING)
			dlg->SetSourceImage(m_streamRcver.m_cloneImage);
		else
			dlg->SetSourceImage(m_srcImage);
	}

	dlg->ShowWindow(SW_SHOW);

	m_imgProcDlgs.push_back(dlg);
	g_imageDetectDlg->m_updateListener.push_back(dlg);
	return dlg;
}


void CStreamingDialog::RemoveImageProcessingDlg(CImageProcessingDlg *dlg)
{
	AutoCSLock cs(m_cs);
	removevector(m_imgProcDlgs, dlg);
	removevector(g_imageDetectDlg->m_updateListener, dlg);
}


void CStreamingDialog::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	// 쓰레드에서 영상을 업데이트 시키게 한다.
	// 쓰레드가 윈도우가 뜨기전에 화면에 그릴 수 있기 때문에,
	// 윈도우가 나타날 때, 플래그를 다시 세팅한다.
	m_updateSrcImage = true; 
}


//---------------------------------------------------------------------------------------------
// Update Streaming Image
void StreamingTask(CStreamingDialog *arg)
{
	int sleepMillis = 100;

	if (!arg->m_ip.empty())
	{
		if (arg->m_streamRcver.Init(false, arg->m_ip, arg->m_port, 0, 1, 1))
			arg->SetWindowText(formatw("Streaming Dialog - ip=%s, port=%d Connect", arg->m_ip.c_str(), arg->m_port).c_str());
		else
			arg->SetWindowText(L"Streaming Dialog - Close");
	}

	int oldT = timeGetTime();
	int streamingFPS = 0;

	Mat img;
	while (arg->m_threadLoop)
	{
		{// 크리티컬 섹션 소멸자 호출을 위해..
			AutoCSLock cs(arg->m_cs);
			bool refreshImage = arg->m_updateOption;
			arg->m_updateOption = false; // 초기화

			if (CStreamingDialog::STREAMING == arg->m_radioMode)
			{
				sleepMillis = arg->m_sleepMillis;
				const bool isUpdateImg = arg->m_streamRcver.Update() ? true : false;
				if (isUpdateImg && (arg->m_playMode == CStreamingDialog::PLAY))
				{
					refreshImage = true;
					++streamingFPS;
					img = arg->m_streamRcver.m_cloneImage.clone();
				}

				const int curT = timeGetTime();
				if (curT - oldT > 1000)
				{
					oldT = curT;
					arg->m_streamingFPS = streamingFPS;
					arg->m_isUIUpdate = true;
					streamingFPS = 0;
				}
			}
			else if (CStreamingDialog::IMAGE == arg->m_radioMode)
			{
				sleepMillis = 200;
				if (arg->m_updateSrcImage || refreshImage)
				{
					refreshImage = true;
					img = arg->m_srcImage;
					arg->m_updateSrcImage = false;
				}
			}

			// display image
			if (img.data && refreshImage)
			{
				arg->m_captureScreen1->Render(img);
				cvproc::cConvertImage cvt;
				cvt.Convert(img, arg->m_cvtImage, arg->GetOptionStr());

				// display FPS
				arg->m_captureScreen2->Render(arg->m_cvtImage);

				// 스트리밍일 때만, 다른 윈도우에 전달한다.
				if (CStreamingDialog::STREAMING == arg->m_radioMode)
				{
					for each(auto dlg in arg->m_imgProcDlgs)
						dlg->SetSourceImage(img);
				}
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(sleepMillis));
	}
}


void StreamingDlgUITask(CStreamingDialog *arg)
{
	while (arg->m_isRunUIThread)
	{
		if (arg->m_isUIUpdate)
		{
			arg->m_staticFPS.SetWindowTextW(formatw("FPS = %d", arg->m_streamingFPS).c_str());
			arg->m_isUIUpdate = false;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}
