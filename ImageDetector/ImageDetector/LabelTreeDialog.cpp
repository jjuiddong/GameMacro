// LabelTree.cpp : implementation file
//

#include "stdafx.h"
#include "ImageDetector.h"
#include "LabelTreeDialog.h"
#include "afxdialogex.h"
#include "FileTreeDialog.h"
#include "ImageDetectorDlg.h"

using namespace cv;


// CLabelTree dialog
CLabelTreeDialog::CLabelTreeDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_LABELTREE, pParent)
	, m_IsShow(TRUE)
	, m_strMatchOption(_T("gray=1"))
	, m_imageWnd(NULL)
{

}

CLabelTreeDialog::~CLabelTreeDialog()
{
}

void CLabelTreeDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_SHOW, m_IsShow);
	DDX_Control(pDX, IDC_TREE_LABEL, m_labelTree);
	DDX_Text(pDX, IDC_EDIT_MATCHOPTION, m_strMatchOption);
}


BEGIN_MESSAGE_MAP(CLabelTreeDialog, CDialogEx)
	ON_BN_CLICKED(IDOK, &CLabelTreeDialog::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CLabelTreeDialog::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_CHECK_SHOW, &CLabelTreeDialog::OnBnClickedCheckShow)
	ON_WM_SIZE()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_LABEL, &CLabelTreeDialog::OnSelchangedTreeFile)
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, &CLabelTreeDialog::OnBnClickedButtonRefresh)
	ON_BN_CLICKED(IDC_BUTTON_MATCH, &CLabelTreeDialog::OnBnClickedButtonMatch)
	ON_BN_CLICKED(IDC_BUTTON_FEATURE_MATCH, &CLabelTreeDialog::OnBnClickedButtonFeatureMatch)
	ON_BN_CLICKED(IDC_BUTTON_EXPLORER, &CLabelTreeDialog::OnBnClickedButtonExplorer)
	ON_BN_CLICKED(IDC_BUTTON_HELP, &CLabelTreeDialog::OnBnClickedButtonHelp)
END_MESSAGE_MAP()

BEGIN_ANCHOR_MAP(CLabelTreeDialog)
 	ANCHOR_MAP_ENTRY(IDC_TREE_LABEL, ANF_LEFT | ANF_RIGHT | ANF_TOP | ANF_BOTTOM)
 	ANCHOR_MAP_ENTRY(IDC_CHECK_SHOW, ANF_LEFT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_MATCH, ANF_LEFT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_FEATURE_MATCH, ANF_LEFT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_REFRESH, ANF_LEFT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_EXPLORER, ANF_LEFT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_EDIT_MATCHOPTION, ANF_LEFT | ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_STATIC1, ANF_LEFT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_HELP, ANF_RIGHT | ANF_BOTTOM)
END_ANCHOR_MAP()


// CLabelTree message handlers
void CLabelTreeDialog::OnBnClickedOk()
{
}
void CLabelTreeDialog::OnBnClickedCancel()
{
	ShowWindow(SW_HIDE);
}


void CLabelTreeDialog::OnBnClickedCheckShow()
{
	UpdateData();
}


BOOL CLabelTreeDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	InitAnchors();

	list<string> extList;
	extList.push_back("png");
	extList.push_back("jpg");
	m_labelTree.Update("label/", extList, 1);

	return TRUE;
}


void CLabelTreeDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	CRect rcWnd;
	GetWindowRect(&rcWnd);
	HandleAnchors(&rcWnd);
}


void CLabelTreeDialog::OnSelchangedTreeFile(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	*pResult = 0;

	const string fileName = m_labelTree.GetSelectFilePath(pNMTreeView->itemNew.hItem);
	if (common::GetFileExt(fileName).empty() || (fileName == "label"))
		return;

	m_selectFileName = fileName;

	if (m_IsShow)
	{
		if (!m_imageWnd)
		{
			CRect wr;
			g_imageDetectDlg->GetWindowRect(wr);

			m_imageWnd = new CImageFrameWnd();
			m_imageWnd->Init("LabelTree", fileName, CRect(wr.left, 0, wr.left+300, 300), this, true);
		}
		else
		{
			m_imageWnd->ShowImage(fileName);
		}
		m_imageWnd->ShowWindow(SW_SHOW);
	}
}


void CLabelTreeDialog::OnBnClickedButtonRefresh()
{
	list<string> extList;
	extList.push_back("png");
	extList.push_back("jpg");
	m_labelTree.Update("label/", extList, 1);
}


// Match Option
//		- gray = 0/1 (Áö¿ø ¾ÈµÊ)
//		- roi = x,y,w,h
cv::Rect CLabelTreeDialog::ParseMatchOption(const string &option)
{
	cv::Rect reVal = { 0,0,0,0 };
	sscanf(option.c_str(), "roi=%d,%d,%d,%d", &reVal.x, &reVal.y, &reVal.width, &reVal.height);
	return reVal;
}


// ÅÛÇÃ¸´ ¸ÅÄ¡
void CLabelTreeDialog::OnBnClickedButtonMatch()
{
	RET(!g_fileTreeDlg);

	UpdateData();

	const int t1 = timeGetTime();
	g_imageDetectDlg->Log("Template Match Start << %s", g_fileTreeDlg->m_selectFileName.c_str());
	
	cvproc::cTemplateMatch match(0.7f, true, true);
	match.Match(g_fileTreeDlg->m_selectFileName, m_selectFileName, wstr2str((LPCTSTR)m_strMatchOption) + " gray=1", g_fileTreeDlg->m_selectFileName);

	const int t2 = timeGetTime();
	g_imageDetectDlg->Log("Template Match End, elpaseTime = %d millisecond", t2 - t1);
}


void CLabelTreeDialog::OnBnClickedButtonFeatureMatch()
{
	RET(!g_fileTreeDlg);

	UpdateData();

	const int t1 = timeGetTime();
	g_imageDetectDlg->Log("Feature Match Start << %s", g_fileTreeDlg->m_selectFileName.c_str());

	cvproc::cFeatureMatch match(true, true);
	match.Match(g_fileTreeDlg->m_selectFileName, m_selectFileName, wstr2str((LPCTSTR)m_strMatchOption) + " gray=1", g_fileTreeDlg->m_selectFileName);

	const int t2 = timeGetTime();
	g_imageDetectDlg->Log("Feature Match End, elpaseTime = %d millisecond", t2 - t1);
}


void CLabelTreeDialog::OnBnClickedButtonExplorer()
{
	ShellExecuteA(m_hWnd, "explore", "label", NULL, NULL, SW_SHOW);
}


void CLabelTreeDialog::OnBnClickedButtonHelp()
{
	g_imageDetectDlg->Log("------------------------------------------------------------");
	g_imageDetectDlg->Log("Match Option >> ");
	g_imageDetectDlg->Log("    - roi=x,y,width,height");
	g_imageDetectDlg->Log("    - bgr=num1,num2,num3");
	g_imageDetectDlg->Log("        - Mat &= Scalar(num1,num2,num3)");
	g_imageDetectDlg->Log("    - scale=num");
	g_imageDetectDlg->Log("        - Mat *= num");
	g_imageDetectDlg->Log("    - gray=0/1");
	g_imageDetectDlg->Log("        - gray convert");
	g_imageDetectDlg->Log("    - hsv=num1,num2,num3,num4,num5,num6");
	g_imageDetectDlg->Log("        - hsv converting, inRange( Scalar(num1,num2,num3), Scalar(num4,num5,num6) )");
}
