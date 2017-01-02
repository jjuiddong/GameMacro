#pragma once
#include "afxwin.h"


class CFlowClientDlg : public CDialogEx
{
public:
	CFlowClientDlg(CWnd* pParent = NULL);	// standard constructor
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FLOWCLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


protected:
	string ReadCmdFile(const string &fileName);
	bool SendCmd(const string &fileName);


protected:
	struct sPacket
	{
		int protocol;
		char str[256];
		int result;
	};

	HICON m_hIcon;
	network::cTCPClient m_client;

	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButtonCmd1();
	afx_msg void OnBnClickedButtonCmd2();
	afx_msg void OnBnClickedButtonCmd3();
	afx_msg void OnBnClickedButtonCmd4();
	CListBox m_logListBox;
	afx_msg void OnClose();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedButtonCmd5();
	afx_msg void OnBnClickedButtonCmd6();
	afx_msg void OnBnClickedButtonCmd7();
	afx_msg void OnBnClickedButtonCmd8();
};
