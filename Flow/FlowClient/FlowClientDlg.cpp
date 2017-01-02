
#include "stdafx.h"
#include "FlowClient.h"
#include "FlowClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CFlowClientDlg::CFlowClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_FLOWCLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFlowClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_LOG, m_logListBox);
}

BEGIN_MESSAGE_MAP(CFlowClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CFlowClientDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CFlowClientDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_CMD1, &CFlowClientDlg::OnBnClickedButtonCmd1)
	ON_BN_CLICKED(IDC_BUTTON_CMD2, &CFlowClientDlg::OnBnClickedButtonCmd2)
	ON_BN_CLICKED(IDC_BUTTON_CMD3, &CFlowClientDlg::OnBnClickedButtonCmd3)
	ON_BN_CLICKED(IDC_BUTTON_CMD4, &CFlowClientDlg::OnBnClickedButtonCmd4)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CFlowClientDlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_CMD5, &CFlowClientDlg::OnBnClickedButtonCmd5)
	ON_BN_CLICKED(IDC_BUTTON_CMD6, &CFlowClientDlg::OnBnClickedButtonCmd6)
	ON_BN_CLICKED(IDC_BUTTON_CMD7, &CFlowClientDlg::OnBnClickedButtonCmd7)
	ON_BN_CLICKED(IDC_BUTTON_CMD8, &CFlowClientDlg::OnBnClickedButtonCmd8)
END_MESSAGE_MAP()

BOOL CFlowClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

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

	string ip = (__argc >= 2) ? common::wstr2str(__targv[1]) : "127.0.0.1";
	int port = (__argc >= 3) ? _ttoi(__targv[2]) : 10000;

	if (m_client.Init(ip, port))
	{
		SetBackgroundColor(g_blueColor);
	}
	else
	{
	}

	return TRUE; 
}

void CFlowClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

void CFlowClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CFlowClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CFlowClientDlg::OnBnClickedOk()
{
	//CDialogEx::OnOK();
}
void CFlowClientDlg::OnBnClickedCancel()
{
	//CDialogEx::OnCancel();
}


void CFlowClientDlg::OnBnClickedButtonCmd1()
{
	RET(!m_client.IsConnect());
	SendCmd("cmd1.txt");
}


void CFlowClientDlg::OnBnClickedButtonCmd2()
{
	RET(!m_client.IsConnect());
	SendCmd("cmd2.txt");
}


void CFlowClientDlg::OnBnClickedButtonCmd3()
{
	RET(!m_client.IsConnect());
	SendCmd("cmd3.txt");
}


void CFlowClientDlg::OnBnClickedButtonCmd4()
{
	RET(!m_client.IsConnect());
	SendCmd("cmd4.txt");
}


void CFlowClientDlg::OnBnClickedButtonCmd5()
{
	RET(!m_client.IsConnect());
	SendCmd("cmd5.txt");
}


void CFlowClientDlg::OnBnClickedButtonCmd6()
{
	RET(!m_client.IsConnect());
	SendCmd("cmd6.txt");
}


void CFlowClientDlg::OnBnClickedButtonCmd7()
{
}


void CFlowClientDlg::OnBnClickedButtonCmd8()
{
}


bool CFlowClientDlg::SendCmd(const string &fileName)
{
	const string cmds = ReadCmdFile(fileName);
	if (!cmds.empty())
	{
		sPacket packet;
		packet.protocol = 1;
		strcpy(packet.str, cmds.c_str());
		m_client.Send((BYTE*)&packet, sizeof(packet));

		while (m_logListBox.GetCount() != 0)
			m_logListBox.DeleteString(0);
		vector<string> out;
		common::tokenizer(cmds, ";", "", out);
		for each (auto item in out)
			m_logListBox.AddString(common::str2wstr(item).c_str());

		return true;
	}

	return false;
}


string CFlowClientDlg::ReadCmdFile(const string &fileName)
{
	using namespace std;

	string cmds;
	ifstream ifs(fileName);
	if (ifs.is_open())
	{
		//stringstream ss;
		string line;
		while (getline(ifs, line))
		{
			common::trim(line);
			if (line.empty())
				continue;
			if (line[0] == '#')
				continue;

			if (!cmds.empty())
				cmds += ";";
			cmds += line;
		}
	}

	return cmds;
}


void CFlowClientDlg::OnClose()
{
	CDialogEx::OnClose();
	CDialogEx::OnCancel();
}


void CFlowClientDlg::OnBnClickedButtonStop()
{
	RET(!m_client.IsConnect());

	sPacket packet;
	packet.protocol = 3;
	m_client.Send((BYTE*)&packet, sizeof(packet));
}
