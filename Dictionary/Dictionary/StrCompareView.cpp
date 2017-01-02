#include "stdafx.h"
#include "StrCompareView.h"

CStrCompareView::CStrCompareView()
{
}

CStrCompareView::~CStrCompareView()
{
}


BEGIN_MESSAGE_MAP(CStrCompareView, CScrollView)
END_MESSAGE_MAP()


void CStrCompareView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
	CSize sizeTotal;
	sizeTotal.cx = sizeTotal.cy = 100;
	SetScrollSizes(MM_TEXT, sizeTotal);
}

void CStrCompareView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();

	int x = 0;
	int y = 0;
	stringstream ss;
	ss << "total process time = " << m_totalProcessTime << " millisceonds";
	pDC->TextOutW(0, y, str2wstr(ss.str()).c_str());
	y += 20;

	for each (auto &str in m_result)
	{
		const CString txt = str2wstr(str).c_str();
		pDC->TextOutW(0, y, txt);
		const CSize textSize = pDC->GetOutputTextExtent(txt);
		if (x < textSize.cx)
			x = textSize.cx;
		y += 20;
	}

	CSize sizeTotal;
	sizeTotal.cx = x;
	sizeTotal.cy = y;
	SetScrollSizes(MM_TEXT, sizeTotal);
}


bool CStrCompareView::Init()
{
	m_dict.Init("dictionary.txt");

	m_input.reserve(1024);
	std::ifstream ifs("testset.txt");
	if (!ifs.is_open())
		return false;
	string line;
	while (getline(ifs, line))
	{
		trim(line);
		if (line.empty())
			continue;
		m_input.push_back(line);
	}

	const int t1 = timeGetTime();
	m_result.reserve(1024);
	for each (auto &str in m_input)
	{
		float fitness;
		vector<string> out;
		string dictStr = m_dict.Search(str, out, fitness);

		stringstream ss;
		ss << str << " ----------> " << dictStr;
		m_result.push_back(ss.str());
	}

	m_totalProcessTime = timeGetTime() - t1;

	InvalidateRect(NULL);
	return true;
}
