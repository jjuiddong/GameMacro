// FileTreeDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ImageDetector.h"
#include "FileTreeDialog.h"
#include "afxdialogex.h"
#include "ImageDetectorDlg.h"
#include "StreamingDialog.h"
#include "ImageProcessingDlg.h"

using namespace cv;

// CFileTreeDialog dialog
CFileTreeDialog::CFileTreeDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_FILETREE, pParent)
	, m_IsShow(TRUE)
	, m_IsAutoRefresh(TRUE)
	, m_strLabelCreatorArg(_T("gray=1"))
	, m_imageWnd(NULL)
	, m_IsAutoUpdate(FALSE)
{
}

CFileTreeDialog::~CFileTreeDialog()
{
}

void CFileTreeDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE_FILE, m_FileTree);
	DDX_Check(pDX, IDC_CHECK_SHOW, m_IsShow);
	DDX_Check(pDX, IDC_CHECK_AUTO_REFRESH, m_IsAutoRefresh);
	DDX_Text(pDX, IDC_EDIT_LABELCREATOR_ARG, m_strLabelCreatorArg);
	DDX_Check(pDX, IDC_CHECK_AUTO_UPDATE, m_IsAutoUpdate);
}


BEGIN_MESSAGE_MAP(CFileTreeDialog, CDialogEx)
	ON_BN_CLICKED(IDOK, &CFileTreeDialog::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CFileTreeDialog::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_IMAGEMATCH, &CFileTreeDialog::OnBnClickedButtonImagematch)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_CHECK_SHOW, &CFileTreeDialog::OnBnClickedCheckShow)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_FILE, &CFileTreeDialog::OnSelchangedTreeFile)
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, &CFileTreeDialog::OnBnClickedButtonRefresh)
	ON_BN_CLICKED(IDC_BUTTON_LABELCREATOR, &CFileTreeDialog::OnBnClickedButtonLabelcreator)
	ON_BN_CLICKED(IDC_CHECK_AUTO_REFRESH, &CFileTreeDialog::OnBnClickedCheckAutoRefresh)
	ON_BN_CLICKED(IDC_BUTTON_EXPLORER, &CFileTreeDialog::OnBnClickedButtonExplorer)
	ON_BN_CLICKED(IDC_BUTTON_HELP, &CFileTreeDialog::OnBnClickedButtonHelp)
	ON_BN_CLICKED(IDC_BUTTON_TOSTREAMINGDLG, &CFileTreeDialog::OnBnClickedButtonTostreamingdlg)
	ON_BN_CLICKED(IDC_CHECK_AUTO_UPDATE, &CFileTreeDialog::OnBnClickedCheckAutoUpdate)
END_MESSAGE_MAP()


BEGIN_ANCHOR_MAP(CFileTreeDialog)
 	ANCHOR_MAP_ENTRY(IDC_TREE_FILE, ANF_LEFT | ANF_RIGHT | ANF_TOP | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_CHECK_SHOW, ANF_LEFT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_IMAGEMATCH, ANF_LEFT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_REFRESH, ANF_LEFT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_EXPLORER, ANF_LEFT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_LABELCREATOR, ANF_LEFT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_CHECK_AUTO_REFRESH, ANF_LEFT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_CHECK_AUTO_UPDATE, ANF_LEFT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_EDIT_LABELCREATOR_ARG, ANF_LEFT | ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_TOSTREAMINGDLG, ANF_LEFT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_HELP, ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_STATIC1, ANF_LEFT | ANF_BOTTOM)
END_ANCHOR_MAP()


// CFileTreeDialog message handlers
BOOL CFileTreeDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	InitAnchors();

	Refresh();

	return TRUE;
}


void CFileTreeDialog::OnBnClickedOk()
{
}
void CFileTreeDialog::OnBnClickedCancel()
{
	ShowWindow(SW_HIDE);
}


void CFileTreeDialog::OnBnClickedCheckShow()
{
	UpdateData();
}


void CFileTreeDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	CRect rcWnd;
	GetWindowRect(&rcWnd);
	HandleAnchors(&rcWnd);
}


void CFileTreeDialog::OnSelchangedTreeFile(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	*pResult = 0;

	const string fileName = m_FileTree.GetSelectFilePath(pNMTreeView->itemNew.hItem);
	if (common::GetFileExt(fileName).empty() || (fileName == "capture"))
		return;

	m_selectFileName = fileName;

	if (m_IsShow)
	{
		if (!m_imageWnd)
		{
			CRect wr;
			GetWindowRect(wr);
			m_imageWnd = new CImageFrameWnd();
			m_imageWnd->Init("Capture", fileName, CRect(wr.right-20, wr.top, wr.right + 280, wr.top + 300), this);
		}
		else
		{
			m_imageWnd->ShowImage(fileName);
		}
		m_imageWnd->ShowWindow(SW_SHOW);
	}

	// 열려있는 ImageProcessingDlg 를 Update 한다.
	if (m_IsAutoUpdate)
	{
		Mat img = imread(fileName);
		if (img.data)
		{
			for each(auto dlg in g_streamingDlg->m_imgProcDlgs)
				dlg->SetSourceImage(img);
		}
	}
}


void CFileTreeDialog::OnBnClickedButtonImagematch()
{
	if (g_imageDetectDlg)
	{
		g_imageDetectDlg->ImageMatch(m_selectFileName);
	}
}


void CFileTreeDialog::OnBnClickedButtonRefresh()
{
	Refresh();
}


void CFileTreeDialog::OnBnClickedButtonLabelcreator()
{
	UpdateData();

	CONST string args = common::wstr2str((LPCTSTR)m_strLabelCreatorArg);

	stringstream ss;
	ss << m_selectFileName << " label/" << " " << args;
	ShellExecuteA(m_hWnd, "open", "LabelCreator.exe", ss.str().c_str(), NULL, SW_SHOW);
}


void CFileTreeDialog::OnBnClickedCheckAutoRefresh()
{
	UpdateData();
}

void CFileTreeDialog::OnBnClickedCheckAutoUpdate()
{
	UpdateData();
}


void CFileTreeDialog::Refresh()
{
	list<string> extList;
	extList.push_back("png");
	extList.push_back("jpg");
	m_FileTree.Update("capture/", extList, 1);
}


void CFileTreeDialog::OnBnClickedButtonExplorer()
{
	ShellExecuteA(m_hWnd, "explore", "capture", NULL, NULL, SW_SHOW);
}


void CFileTreeDialog::OnBnClickedButtonTostreamingdlg()
{
	if (g_streamingDlg)
	{
		g_streamingDlg->SetImageMode(imread(m_selectFileName));
		g_streamingDlg->ShowStreaming("", 0);
	}	
}


void CFileTreeDialog::OnBnClickedButtonHelp()
{
	g_imageDetectDlg->Log("------------------------------------------------------------");
	g_imageDetectDlg->Log("Label Creator Argument >> " );
	g_imageDetectDlg->Log("    - bgr=num1,num2,num3");
	g_imageDetectDlg->Log("        - Mat &= Scalar(num1,num2,num3)");
	g_imageDetectDlg->Log("    - scale=num");
	g_imageDetectDlg->Log("        - Mat *= num");
	g_imageDetectDlg->Log("    - gray=0/1");
	g_imageDetectDlg->Log("        - gray convert");
	g_imageDetectDlg->Log("    - hsv=num1,num2,num3,num4,num5,num6");	
	g_imageDetectDlg->Log("        - hsv converting, inRange( Scalar(num1,num2,num3), Scalar(num4,num5,num6) )");
}

