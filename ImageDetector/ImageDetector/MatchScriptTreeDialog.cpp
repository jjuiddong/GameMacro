// MatchScriptTree.cpp : implementation file
//

#include "stdafx.h"
#include "ImageDetector.h"
#include "MatchScriptTreeDialog.h"
#include "afxdialogex.h"
#include "ParseTreeView.h"



// CMatchScriptTree dialog

CMatchScriptTreeDialog::CMatchScriptTreeDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_SCRIPTTREE, pParent)
	, m_parseTreeView(NULL)
	, m_radioNodeType(0)
	, m_IsShowImage(FALSE)
{

}

CMatchScriptTreeDialog::~CMatchScriptTreeDialog()
{
}

void CMatchScriptTreeDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_CMD, m_comboCommand);
	DDX_Radio(pDX, IDC_RADIO_ALL, m_radioNodeType);
	DDX_Check(pDX, IDC_CHECK_SHOW_IMAGE, m_IsShowImage);
}


BEGIN_MESSAGE_MAP(CMatchScriptTreeDialog, CDialogEx)
	ON_BN_CLICKED(IDOK, &CMatchScriptTreeDialog::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CMatchScriptTreeDialog::OnBnClickedCancel)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_RADIO_RUNNING, &CMatchScriptTreeDialog::OnBnClickedRadioRunning)
	ON_BN_CLICKED(IDC_RADIO_GRPROCCNT, &CMatchScriptTreeDialog::OnBnClickedRadioGrproccnt)
	ON_BN_CLICKED(IDC_RADIO_SUCCESS, &CMatchScriptTreeDialog::OnBnClickedRadioSuccess)
	ON_BN_CLICKED(IDC_RADIO_ALL, &CMatchScriptTreeDialog::OnBnClickedRadioAll)
	ON_BN_CLICKED(IDC_CHECK_SHOW_IMAGE, &CMatchScriptTreeDialog::OnBnClickedCheckShowImage)
	ON_CBN_EDITCHANGE(IDC_COMBO_CMD, &CMatchScriptTreeDialog::OnEditchangeComboCmd)
END_MESSAGE_MAP()


BEGIN_ANCHOR_MAP(CMatchScriptTreeDialog)
 	ANCHOR_MAP_ENTRY(IDC_STATIC_GRAPH, ANF_LEFT | ANF_RIGHT | ANF_TOP | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_COMBO_CMD, ANF_LEFT | ANF_RIGHT | ANF_TOP )
END_ANCHOR_MAP()


// CMatchScriptTree message handlers
void CMatchScriptTreeDialog::OnBnClickedOk()
{
}
void CMatchScriptTreeDialog::OnBnClickedCancel()
{
	CDialogEx::OnCancel();
}


BOOL CMatchScriptTreeDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	InitAnchors();

	if (CWnd *wnd = GetDlgItem(IDC_STATIC_GRAPH))
	{
		CRect cr;
		wnd->GetWindowRect(cr);
		ScreenToClient(cr);
		m_parseTreeView = new CParseTreeView();
		m_parseTreeView->Create(NULL, NULL, WS_VISIBLE | WS_CHILDWINDOW | WS_EX_WINDOWEDGE, cr, this, 100);
		m_parseTreeView->ShowWindow(SW_SHOW);
	}

	return TRUE;
}


void CMatchScriptTreeDialog::ShowTree(const cvproc::imagematch::cMatchScript2 &script, const string &treeName)
{
	m_parseTreeView->ShowTree(script, treeName);
	m_parseTreeView->ShowAllImage(m_IsShowImage ? true : false);
	m_parseTreeView->InvalidateRect(NULL);
}


void CMatchScriptTreeDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	CRect rcWnd;
	GetWindowRect(&rcWnd);
	HandleAnchors(&rcWnd);

	if (CWnd *wnd = GetDlgItem(IDC_STATIC_GRAPH))
	{
		CRect cr;
		wnd->GetWindowRect(cr);
		ScreenToClient(cr);
		m_parseTreeView->MoveWindow(cr);
	}
}


//  화면 갱신
void CMatchScriptTreeDialog::Update(const float deltaSeconds)
{
	m_parseTreeView->Update(deltaSeconds);
}


void CMatchScriptTreeDialog::ChangeModeRadioButton(const int radioBtn)
{
	m_parseTreeView->m_isShowProcCnt = false;
	m_parseTreeView->m_isShowSuccess = false;
	m_parseTreeView->m_isShowRunning = false;

	switch (radioBtn)
	{
	case 0: break; // all
	case 1: m_parseTreeView->m_isShowRunning = true; break;
	case 2: m_parseTreeView->m_isShowProcCnt = true; break;
	case 3: m_parseTreeView->m_isShowSuccess = true; break;
	default: assert(0); break;
	}

	m_parseTreeView->InvalidateRect(NULL);
}


void CMatchScriptTreeDialog::OnBnClickedRadioAll()
{
	UpdateData();
	ChangeModeRadioButton(0);
}

void CMatchScriptTreeDialog::OnBnClickedRadioRunning()
{
	UpdateData();
	ChangeModeRadioButton(1);
}


void CMatchScriptTreeDialog::OnBnClickedRadioGrproccnt()
{
	UpdateData();
	ChangeModeRadioButton(2);
}


void CMatchScriptTreeDialog::OnBnClickedRadioSuccess()
{
	UpdateData();
	ChangeModeRadioButton(3);
}


void CMatchScriptTreeDialog::OnBnClickedCheckShowImage()
{
	UpdateData();
	m_parseTreeView->ShowAllImage(m_IsShowImage? true : false);
}


void CMatchScriptTreeDialog::OnEditchangeComboCmd()
{
	CString str;
	m_comboCommand.GetWindowTextW(str);
	m_parseTreeView->SetSearchString(wstr2str((LPCTSTR)str));
}
