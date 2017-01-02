#pragma once

#include "dictionary2.h"

class CStrCompareView : public CScrollView
{
public:
	CStrCompareView();
	virtual ~CStrCompareView();

	bool Init();

public:
	tess::cDictionary2 m_dict;
	vector<string> m_input;
	vector<string> m_result;
	int m_totalProcessTime;

public:
	virtual void OnDraw(CDC* pDC);
	virtual void OnInitialUpdate();
	DECLARE_MESSAGE_MAP()
};
