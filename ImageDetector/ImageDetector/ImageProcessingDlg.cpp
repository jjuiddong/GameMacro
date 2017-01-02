
#include "stdafx.h"
#include "ImageDetector.h"
#include "ImageProcessingDlg.h"
#include "afxdialogex.h"
#include "StreamingDialog.h"
#include "LabelTreeDialog.h"
#include "ImageDetectorDlg.h"
#include "FileTreeDialog.h"
#include "StreamingDialog.h"
#include "MatchScriptTreeDialog.h"

using namespace cv;


void ImgProcessingTask(CImageProcessingDlg *arg);
void ImgProcessingUpdateTask(CImageProcessingDlg *arg);


// CImageProcessingDlg dialog
CImageProcessingDlg::CImageProcessingDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_IMGPROCESSING, pParent)
	, m_editCmd(_T(""))
	, m_radioMode(0)
	, m_isUpdateImage(false)
	, m_isPostUpdateImage(false)
	, m_smatch(NULL)
	, m_isRunThread(false)
	, m_isRunUIThread(true)
	, m_mode(PLAY)
	, m_popupImageWindow(NULL)
	, m_isShowFirstImageWnd(true)
	, m_checkShowGrid(TRUE)
	, m_checkShowText(TRUE)
	, m_checkShowTextBg(FALSE)
	, m_checkWhite(TRUE)
	, m_checkAutoColor(TRUE)
{
}

CImageProcessingDlg::~CImageProcessingDlg()
{
	m_isRunThread = false;
	if (m_thread.joinable())
		m_thread.join();

	m_isRunUIThread = false;
	if (m_UIThread.joinable())
		m_UIThread.join();
}

void CImageProcessingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_COMMAND, m_editCmd);
	DDX_Radio(pDX, IDC_RADIO_STREAMING, m_radioMode);
	DDX_Control(pDX, IDC_BUTTON_PAUSE, m_btnPause);
	DDX_Check(pDX, IDC_CHECK_GRID, m_checkShowGrid);
	DDX_Check(pDX, IDC_CHECK_TEXT, m_checkShowText);
	DDX_Check(pDX, IDC_CHECK_TEXTBG, m_checkShowTextBg);
	DDX_Check(pDX, IDC_CHECK_WHITE, m_checkWhite);
	DDX_Control(pDX, IDC_STATIC_INFO, m_staticPixelInfo);
	DDX_Check(pDX, IDC_CHECK_AUTOCOLOR, m_checkAutoColor);
}


BEGIN_MESSAGE_MAP(CImageProcessingDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CImageProcessingDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CImageProcessingDlg::OnBnClickedCancel)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_IMAGEMATCH, &CImageProcessingDlg::OnBnClickedButtonImagematch)
	ON_BN_CLICKED(IDC_BUTTON_TEMPLATE_MATCH, &CImageProcessingDlg::OnBnClickedButtonTemplateMatch)
	ON_BN_CLICKED(IDC_BUTTON_FEATURE_MATCH, &CImageProcessingDlg::OnBnClickedButtonFeatureMatch)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE, &CImageProcessingDlg::OnBnClickedButtonUpdate)
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, &CImageProcessingDlg::OnBnClickedButtonPause)
	ON_BN_CLICKED(IDC_RADIO_STREAMING, &CImageProcessingDlg::OnBnClickedRadioStreaming)
	ON_BN_CLICKED(IDC_RADIO_IMAGE, &CImageProcessingDlg::OnBnClickedRadioImage)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_NEWWINDOW, &CImageProcessingDlg::OnBnClickedButtonNewwindow)
	ON_BN_CLICKED(IDC_BUTTON_HELP, &CImageProcessingDlg::OnBnClickedButtonHelp)
	ON_BN_CLICKED(IDC_CHECK_GRID, &CImageProcessingDlg::OnBnClickedCheckGrid)
	ON_BN_CLICKED(IDC_CHECK_TEXT, &CImageProcessingDlg::OnBnClickedCheckText)
	ON_BN_CLICKED(IDC_CHECK_TEXTBG, &CImageProcessingDlg::OnBnClickedCheckTextbg)
	ON_BN_CLICKED(IDC_CHECK_WHITE, &CImageProcessingDlg::OnBnClickedCheckWhite)
	ON_BN_CLICKED(IDC_CHECK_AUTOCOLOR, &CImageProcessingDlg::OnBnClickedCheckAutocolor)
END_MESSAGE_MAP()


BEGIN_ANCHOR_MAP(CImageProcessingDlg)
	ANCHOR_MAP_ENTRY(IDC_STATIC_VIEW, ANF_LEFT | ANF_RIGHT | ANF_TOP | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_EDIT_COMMAND, ANF_LEFT | ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_UPDATE, ANF_LEFT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_FEATURE_MATCH, ANF_RIGHT| ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_TEMPLATE_MATCH, ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_IMAGEMATCH, ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_PAUSE, ANF_LEFT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_NEWWINDOW, ANF_RIGHT| ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_HELP, ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_STATIC1, ANF_LEFT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_STATIC_INFO, ANF_LEFT | ANF_RIGHT | ANF_TOP)
END_ANCHOR_MAP()


// CImageProcessingDlg message handlers
void CImageProcessingDlg::OnBnClickedOk()
{
}
void CImageProcessingDlg::OnBnClickedCancel()
{
	CDialogEx::OnCancel();
}


BOOL CImageProcessingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	InitAnchors();

	m_imgWindow = new CImageWatchWindow();
	m_imgWindow->Create(NULL, L"CamWindow", WS_VISIBLE | WS_CHILDWINDOW, CRect(0, 0, 100, 100), this, 1100);
	m_imgWindow->SetScrollSizes(MM_TEXT, CSize(100, 100));

	if (CWnd *wnd = GetDlgItem(IDC_STATIC_VIEW))
	{
		CRect cr;
		wnd->GetWindowRect(cr);
		ScreenToClient(cr);
		m_imgWindow->MoveWindow(cr);
	}

	m_imgWindow->ShowWindow(SW_SHOW);

	m_editCmd = L"img=@capture_select \r\n"\
 		//L"imagematch=@tree_label \r\n"
		;

	m_isRunUIThread = true;
	m_UIThread = std::thread(ImgProcessingUpdateTask, this);

	CRect wr;
	GetWindowRect(wr);
	m_popupImageWindow = new CImageFrameWnd();
	m_popupImageWindow->Init("Image Processing", Mat(),
		CRect(wr.right - 20, wr.top, wr.right + 280, wr.top + 300), this);
	m_popupImageWindow->ShowWindow(SW_HIDE);

	UpdateData(FALSE);

	return TRUE; 
}


void CImageProcessingDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	CRect rcWnd;
	GetWindowRect(&rcWnd);
	HandleAnchors(&rcWnd);

	if (CWnd *wnd = GetDlgItem(IDC_STATIC_VIEW))
	{
		CRect cr;
		wnd->GetWindowRect(cr);
		ScreenToClient(cr);		m_imgWindow->MoveWindow(cr);	}}


void CImageProcessingDlg::Update(const float deltaSeconds)
{
	if (m_isPostUpdateImage && m_imgWindow->m_bitmap)
	{
		static CSize oldBound;

		// 이미지가 로딩된 후, 스크롤을 업데이트 한다.
		CRect cr;
		m_imgWindow->GetClientRect(cr);
		CSize size((int)m_imgWindow->m_bitmap->GetWidth(), m_imgWindow->m_bitmap->GetHeight());
		if (oldBound != size)
		{
			CSize scrollSize((int)m_imgWindow->m_bound.Width + cr.Width() * 2, (int)m_imgWindow->m_bound.Height + cr.Height() * 2);
			m_imgWindow->SetScrollSizes(MM_TEXT, scrollSize);
			m_imgWindow->SetScrollPos(SB_VERT, 0);
			m_imgWindow->SetScrollPos(SB_HORZ, 0);
		}

		oldBound = size;
		m_isPostUpdateImage = false;
		m_imgWindow->InvalidateRect(NULL);
	}


	if ((GetActiveWindow() == this) &&
		(0x8000 & GetAsyncKeyState(VK_F5)))
	{
		Sleep(100);
		OnBnClickedButtonUpdate();
	}


	// Update Mouse Position pixel information
	static string oldStr;
	if (oldStr != m_imgWindow->m_infoStr)
	{
		oldStr = m_imgWindow->m_infoStr;
		m_staticPixelInfo.SetWindowTextW(str2wstr(oldStr).c_str());
	}
}


void CImageProcessingDlg::OnBnClickedButtonImagematch()
{
}


void CImageProcessingDlg::OnBnClickedButtonTemplateMatch()
{
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


void CImageProcessingDlg::OnBnClickedButtonFeatureMatch()
{
}


void CImageProcessingDlg::OnBnClickedButtonUpdate()
{
	UpdateData();

 	m_isUpdateImage = true;
	m_isPostUpdateImage = false;
 	m_option = wstr2str((LPCTSTR)m_editCmd);
}


void CImageProcessingDlg::OnBnClickedButtonPause()
{
	if (m_mode == PLAY)
	{
		m_btnPause.SetWindowTextW(L"Play");
		m_mode = PAUSE;
	}
	else
	{
		m_btnPause.SetWindowTextW(L"Pause");
		m_mode = PLAY;		
	}
}


void CImageProcessingDlg::OnBnClickedButtonNewwindow()
{
	CImageProcessingDlg *dlg = g_streamingDlg->AddImageProcessingDlg();
	dlg->m_option = m_option;
	dlg->m_editCmd = m_editCmd;
	dlg->UpdateData(FALSE);

	CRect wr;
	GetWindowRect(wr);
	wr.OffsetRect(30, 30);
	dlg->MoveWindow(wr);
}


void CImageProcessingDlg::OnBnClickedRadioStreaming()
{
}


void CImageProcessingDlg::OnBnClickedRadioImage()
{
}


void CImageProcessingDlg::OnClose()
{
	g_streamingDlg->RemoveImageProcessingDlg(this);
	m_popupImageWindow->ShowWindow(SW_HIDE);

	CDialogEx::OnClose();
	delete this;
}


void CImageProcessingDlg::SetSourceImage(const cv::Mat &img)
{
	RET(m_mode == PAUSE);

	if (-1 != m_smatch.IsMatchComplete())
		return; // 매칭 연산 중이라면, 이미지를 업데이트 하지 않는다.

	if (m_isUpdateImage) // 이미지 프로세싱을 하고 있다면, 끝날때까지 대기한다.
		return;
	if (m_isRunThread) // 이미지 프로세싱이 끝나고, 화면 갱신될때까지 대기한다.
		return;

	m_srcImage = img.clone();
	m_isUpdateImage = true;
	m_isPostUpdateImage = false;
}


// command 값을 읽어드려, 이미지 프로세싱을 처리한다.
void CImageProcessingDlg::ImageProcessing()
{
	m_smatch.m_matchScript = &g_imageDetectDlg->m_matchScript;
	const string errMsg = m_smatch.Match(m_srcImage, m_cvtImage, m_option,
		g_labelTreeDlg->m_selectFileName,
		g_fileTreeDlg->m_selectFileName,
		wstr2str((LPCTSTR)g_imageDetectDlg->m_excuteLabel));

	if (m_smatch.IsMatchComplete() == -1)
	{ // feature match, template match
		SetWindowText(L"Image Processing Dialog");
		m_imgWindow->Render(m_cvtImage);

		if (m_smatch.m_tessImg.data)
		{
			if (m_isShowFirstImageWnd)
			{ // 이미지 윈도우를 부모 다이얼로그 위에 출력한다.
				m_isShowFirstImageWnd = false;
 				CRect wr;
				GetWindowRect(wr);
				m_popupImageWindow->MoveWindow(CRect(wr.left, (wr.top-100 < 0)? 0 : wr.top - 100, 
					wr.right + 200, (wr.top < 100)? 100 : wr.top));
			}

			m_popupImageWindow->ShowImage(m_smatch.m_tessImg);
 			m_popupImageWindow->ShowWindow(SW_SHOW);
		}
	}
	else
	{
		SetWindowText(L"Image Processing Dialog - Processing");

		m_isRunThread = false;
		if (m_thread.joinable())
			m_thread.join();

		m_isRunThread = true;
		m_thread = std::thread(ImgProcessingTask, this);
	}

	if (!errMsg.empty())
		::AfxMessageBox(str2wstr(errMsg).c_str());
}


void CImageProcessingDlg::OnBnClickedButtonHelp()
{
	string str =
		"\r\n"\
		"Script Command List \r\n"\
		"-------------------------------------------------------------------------------\r\n"\
		"\r\n"\
		"img = @capture_select \r\n"\
		"templatematch = @label_select \r\n"\
		"featurematch = @label_select \r\n"\
		"imagematch = @tree_label \r\n"\
		"imagematch2 = @tree_label \r\n"\
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
		"deskew = 0.005, debug_enable \r\n"\
		"\r\n"\
		"tess = dictionary.txt \r\n"\
		"	-> tesseract \r\n"\
		;	std::ofstream ofs("imgprocdlg_help.txt");	ofs << str;	ofs.close();	
	ShellExecuteW(m_hWnd, L"open", L"notepad", L"imgprocdlg_help.txt", NULL, SW_SHOW);

	UpdateData(FALSE);
}




void CImageProcessingDlg::OnBnClickedCheckGrid()
{
	UpdateData();
	m_imgWindow->m_isShowGrid = m_checkShowGrid;
	m_imgWindow->InvalidateRect(NULL);
}


void CImageProcessingDlg::OnBnClickedCheckText()
{
	UpdateData();
	m_imgWindow->m_isShowText = m_checkShowText;
	m_imgWindow->InvalidateRect(NULL);
}


void CImageProcessingDlg::OnBnClickedCheckTextbg()
{
	UpdateData();
	m_imgWindow->m_isShowTextBg = m_checkShowTextBg;
	m_imgWindow->InvalidateRect(NULL);
}

void CImageProcessingDlg::OnBnClickedCheckWhite()
{
	UpdateData();
	m_imgWindow->m_isShowWhite = m_checkWhite;
	m_imgWindow->InvalidateRect(NULL);
}

void CImageProcessingDlg::OnBnClickedCheckAutocolor()
{
	UpdateData();
	m_imgWindow->m_isAutoColor = m_checkAutoColor;
	m_imgWindow->InvalidateRect(NULL);
}



//---------------------------------------------------------------------------------------------------
// Image Matching Thread
void ImgProcessingTask(CImageProcessingDlg *arg)
{
	if (arg->m_smatch.IsMatchComplete() == -1)
		return;

	// 매칭이 끝날 때까지 대기
	while ((arg->m_smatch.IsMatchComplete() == 0) && arg->m_isRunThread)
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

	if (arg->m_isRunThread) // 루프 강제 종료가 아닐 때만, 화면을 갱신한다.
	{
 		Mat dst;
 		arg->m_smatch.UpdateMatchResult(dst);
 		arg->m_imgWindow->Render(dst);
 		arg->SetWindowText(L"Image Processing Dialog");
 
		// ParseTree View 가 화면에  보여지고 있는 상태라면, 갱신한다.
		if (g_scriptTreeDlg->IsWindowVisible())
			g_scriptTreeDlg->ShowTree(*arg->m_smatch.m_matchScript,
				wstr2str((LPCTSTR)g_imageDetectDlg->m_excuteLabel));
	}
	else
	{
		arg->m_smatch.TerminiateMatch();
	}

	arg->m_isRunThread = false; // 쓰레드 종료.
	arg->m_isPostUpdateImage = true;
}


// UI Update Thread
void ImgProcessingUpdateTask(CImageProcessingDlg *arg)
{
	while (arg->m_isRunUIThread)
	{
		if (arg->m_isUpdateImage)
		{
			arg->ImageProcessing();
			arg->m_isUpdateImage = false;
			arg->m_isPostUpdateImage = true;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}
