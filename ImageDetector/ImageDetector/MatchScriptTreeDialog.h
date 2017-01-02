//
// 2016-06-28, jjuiddong
// match script tree display
//
#pragma once
#include "afxwin.h"


class CParseTreeView;
class CMatchScriptTreeDialog : public CDialogEx
{
public:
	CMatchScriptTreeDialog(CWnd* pParent = NULL);
	virtual ~CMatchScriptTreeDialog();
	enum { IDD = IDD_DIALOG_SCRIPTTREE };

	void Update(const float deltaSeconds);
	void ShowTree(const cvproc::imagematch::cMatchScript2 &script, const string &treeName);


protected:
	virtual void DoDataExchange(CDataExchange* pDX); 
	void ChangeModeRadioButton(const int radioBtn);


protected:
	CParseTreeView *m_parseTreeView;


	DECLARE_ANCHOR_MAP();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CComboBox m_comboCommand;
	int m_radioNodeType;
	afx_msg void OnBnClickedRadioRunning();
	afx_msg void OnBnClickedRadioGrproccnt();
	afx_msg void OnBnClickedRadioSuccess();
	afx_msg void OnBnClickedRadioAll();
	BOOL m_IsShowImage;
	afx_msg void OnBnClickedCheckShowImage();
	afx_msg void OnEditchangeComboCmd();
};
