// ParseTreeView.cpp : implementation file
//

#include "stdafx.h"
#include "ImageDetector.h"
#include "ParseTreeView.h"
#include "ImageDetectorDlg.h"


using namespace cv;
using namespace cvproc;
using namespace cvproc::imagematch;


CParseTreeView::CParseTreeView() 
	: m_isShow(false)
	, m_incTime(0)
	, m_isShowProcCnt(false)
	, m_isShowSuccess(false)
	, m_isShowRunning(false)
	, m_incItemId(0)
	, m_itemCount(0)
	, m_selectItem(-1)
	, m_oldSelectItem(-1)
{
	CSize sizeTotal;
	sizeTotal.cx = sizeTotal.cy = 100;
	SetScrollSizes(MM_TEXT, sizeTotal);

	m_brush[BLUE].CreateSolidBrush(RGB(0, 0, 255));
	m_brush[WHITE].CreateSolidBrush(RGB(255, 255, 255));
	m_brush[YELLOW].CreateSolidBrush(RGB(255, 255, 0));
	m_brush[RED].CreateSolidBrush(RGB(255, 0, 0));
	m_brush[PINK].CreateSolidBrush(RGB(255, 0, 255));
}

CParseTreeView::~CParseTreeView()
{
	ClearImagePool();
	ClearBitmapPool();
}


BEGIN_MESSAGE_MAP(CParseTreeView, CScrollView)
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()


void CParseTreeView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

// 	CSize sizeTotal;
// 	sizeTotal.cx = sizeTotal.cy = 100;
// 	SetScrollSizes(MM_TEXT, sizeTotal);
}


void CParseTreeView::ShowTree(const cvproc::imagematch::cMatchScript2 &script, const string &treeName)
{
	m_isShow = true;
	m_matchScript = &script;
	m_treeName = treeName;

	// 트리 노드 리스트를 만든다. (최적화를 위해)
	m_treeNodes.clear();
	if (const sParseTree *labelNode = m_matchScript->FindTreeLabel(treeName))
		script.m_parser.collectTree(labelNode, m_treeNodes);

	// run table 생성, 쓰레드 동기화 때문에 필요.
	ZeroMemory(m_nodesRun, sizeof(m_nodesRun));
	for each (auto it in m_treeNodes)
		m_nodesRun[it->id] = it->isRun;

	// tree node position clear
	ZeroMemory(m_nodesPosition, sizeof(m_nodesPosition));
	ZeroMemory(m_itemId2NodeId, sizeof(m_itemId2NodeId));
	ZeroMemory(m_nodesShowImage, sizeof(m_nodesShowImage));

	ClearImagePool();
	ClearBitmapPool();
}


void CParseTreeView::OnDraw(CDC* pDC)
{
	RET(!m_isShow);
	RET(!m_matchScript);

	MakeSrcTreeImage();

	if (!m_displayImg.IsNull())
	{
		CPoint cur_pos = GetScrollPosition();
		m_displayImg.Draw(pDC->GetSafeHdc(), cur_pos.x, cur_pos.y);
	}
}


void CParseTreeView::MakeSrcTreeImage()
{
	RET(!m_matchScript);

	const sParseTree *root = m_matchScript->FindTreeLabel(m_treeName);
	RET(!root);

	CRect cr;
	GetClientRect(cr);
	m_clientRect = cr;
	const CPoint scrollPos0 = GetScrollPosition();
	const Point scrollPos(scrollPos0.x, scrollPos0.y);

	// window size change -> resize double buffer memory
	if (m_displayImg.IsNull() || (m_displayImg.GetWidth() != cr.Width()) || (m_displayImg.GetHeight() != cr.Height()))
	{
		m_displayImg.Destroy();
		m_treeImgSrc.Destroy();
		m_displayImg.Create(cr.Width(), cr.Height(), 24);
		m_treeImgSrc.Create(cr.Width(), cr.Height(), 24);
	}

	// double buffering
	CDC *pDC = CDC::FromHandle(m_treeImgSrc.GetDC());
	pDC->FillRect(cr, &m_brush[WHITE]);
	m_maxTreeSize = Point(0, 0);
	m_incItemId = 0;
	DrawTree(pDC, (sParseTree*)root, Point(0, 0) - scrollPos, Point(20, 10) - scrollPos);
	m_maxTreeSize += scrollPos;
	m_itemCount = m_incItemId;

	// display mem dc
	CDC *pDestDC = CDC::FromHandle(m_displayImg.GetDC());
	m_treeImgSrc.BitBlt(pDestDC->GetSafeHdc(), 0, 0, cr.Width(), cr.Height(), 0, 0, SRCCOPY);
	m_treeImgSrc.ReleaseDC();
	m_displayImg.ReleaseDC();

	CSize sizeTotal;
	sizeTotal.cx = m_maxTreeSize.x;
	sizeTotal.cy = m_maxTreeSize.y + 30;
	SetScrollSizes(MM_TEXT, sizeTotal);
}


bool CParseTreeView::DrawTreeImage()
{
	RETV(m_displayImg.IsNull(), false);

	CClientDC dc(this);
	CScrollView::OnPrepareDC(&dc);
	m_displayImg.Draw(dc.GetSafeHdc(), 0, 0);
	return true;
}


Point CParseTreeView::DrawTree(CDC *pDC, cvproc::imagematch::sParseTree *node, 
	const Point prevPos, const Point pos)
{
	RETV(!node, pos);

	if (m_isShowProcCnt && (node->processCnt == 0))
	{
		const Point posN = DrawTree(pDC, node->next, prevPos, pos);
		return posN;
	}
	if (m_isShowSuccess && (node->result <= 0))
	{
		const Point posN = DrawTree(pDC, node->next, prevPos, pos);
		return posN;
	}
	if (m_isShowSuccess && (node->result > 0) && ('@'==node->attrs["id"][0]))
	{
		const Point posC = DrawTree(pDC, node->child, prevPos, pos);
		const Point posN = DrawTree(pDC, node->next, prevPos, Point(pos.x, posC.y));
		return posN;
	}
	if (m_isShowRunning && !m_nodesRun[node->id])
	{
		const Point posC = DrawTree(pDC, node->child, prevPos, pos);
		const Point posN = DrawTree(pDC, node->next, prevPos, Point(pos.x, posC.y));
		return posN;
	}

	const bool isTerminal = node->child == NULL;
	stringstream ss;
	if (isTerminal)
	{
		ss << node->attrs["id"];
	}
	else
	{
		ss << node->attrs["id"];
		for (int i = 0; i < 3; ++i)
		{
			if (!node->IsEmptyHsv(i))
			{
				const string key = common::format("hsv%d", i);
				ss << ", hsv" << i << "=" << node->attrs[key];
			}
		}
		for (int i = 0; i < 3; ++i)
		{
			if (!node->IsEmptyHls(i))
			{
				const string key = common::format("hls%d", i);
				ss << ", hls" << i << "=" << node->attrs[key];
			}
		}

		if (!node->IsEmptyBgr())
		{
			ss << ", bgr=" << node->attrs["scalar"];
		}

		if (!node->IsEmptyRoi())
		{
			ss << ", roi=" << node->attrs["roi"];
		}

		if ((node->processCnt > 0))// && (node->matchType == 0)) // templateMatch
		{
			ss << ", max=" << node->max;
		}
		
		ss << ", proc_cnt=" << node->processCnt;
	}

	const bool isRender = (m_searchStr.empty()) ? true : (string(node->attrs["id"]).find(m_searchStr) != string::npos);

	CSize textSize(0,0);
	int alphaH = 0;
	int imgMaxX = 0;

	if (isRender)
	{
		const CString str = str2wstr(ss.str()).c_str();
		textSize = pDC->GetOutputTextExtent(str);
		const CRect itemPosition(pos.x - 5, pos.y - 5, pos.x + textSize.cx + 5, pos.y + textSize.cy + 5);

		pDC->SelectObject(SelectTreeNodeColor(pDC, m_incItemId, node));
		pDC->RoundRect(itemPosition, { 5, 5 });
		pDC->TextOut(pos.x, pos.y, str);

		if (m_nodesShowImage[m_incItemId])
		{
			//원본 이미지 출력
			if (CImage *img = LoadImage(node->attrs["id"]))
			{
				DrawEdge(pDC, pos.x, pos.y + 20, img->GetWidth(), img->GetHeight());
				img->Draw(pDC->GetSafeHdc(), pos.x, pos.y+20);
				alphaH = img->GetHeight();
				imgMaxX = pos.x + img->GetWidth() + 20;
			}

			// 매칭된 이미지를 보여준다.
			if ((node->result > 0) && ('@' != node->attrs["id"][0]) && (strchr(node->attrs["id"].c_str(), '.')) && (node->attrs["type"] != "featurematch"))
			{
				Gdiplus::Bitmap*img = LoadBitmap(node->attrs["id"]);
				if (!img)
				{
					Mat out;
					cMatchProcessor::Get()->RemoveHsvImage(0);
					cMatchProcessor::Get()->RemoveScalarImage(0);

					cMatchResult *matchResult = cMatchProcessor::Get()->m_lastMatchResult;
					if (matchResult->m_srcImage.data)
					{
						cMatchProcessor::Get()->SetInputImage(matchResult->m_srcImage, "@parseview");
						g_imageDetectDlg->m_matchScript.GetCloneMatchingArea(
							matchResult->m_srcImage,
							"@parseview", 0, (sParseTree*)node, &out);

						if (out.data)
						{
							img = CGdiPlus::CopyMatToBmp(out);
							m_bitmapPool[node->attrs["id"]] = img;
						}
					}
				}

				if (img)
				{
					DrawEdge(pDC, imgMaxX-1, pos.y + 20, img->GetWidth()+1, img->GetHeight());
					Gdiplus::Graphics g(*pDC);
					g.DrawImage(img, imgMaxX, pos.y + 20);
					imgMaxX += img->GetWidth() + 20 + 10; // 10 = 여분
				}
			}
		}

		pDC->MoveTo(prevPos.x, prevPos.y+20);
		pDC->LineTo(prevPos.x, pos.y+5);
		pDC->LineTo(pos.x-5, pos.y+5);

		// Save Tree Item Position
		m_itemId2NodeId[m_incItemId] = node->id;
		m_nodesPosition[m_incItemId] = itemPosition;
		++m_incItemId;
	} // isRender


	Point posC, posN;
	if (node->child)
	{
		posC = DrawTree(pDC, node->child, pos, (isRender) ? pos + Point(30, alphaH + 30) : pos);
	}
	else
	{
		posC = (isRender) ? pos + Point(0, alphaH+30) : pos;
	}

	if (node->next)
	{
		posN = DrawTree(pDC, node->next, prevPos, Point(pos.x, posC.y));
	}
	else
	{
		posN = (isRender) ? Point(pos.x, posC.y) : Point(pos.x, posC.y);
	}

	m_maxTreeSize.x = MAX(MAX(pos.x+ textSize.cx + 5, imgMaxX), m_maxTreeSize.x);
	m_maxTreeSize.y = MAX(pos.y+ alphaH, m_maxTreeSize.y);

	return posN;
}


void CParseTreeView::DrawEdge(CDC *pDC, const int x, const int y, const int width, const int height)
{
	pDC->MoveTo(x, y - 1);
	pDC->LineTo(x + width, y - 1);
	pDC->LineTo(x + width, y + height);
	pDC->LineTo(x, y + height);
	pDC->LineTo(x, y);
}


CBrush& CParseTreeView::SelectTreeNodeColor(CDC* pDC, const int itemId, cvproc::imagematch::sParseTree *node)
{
	RETV(!node, m_brush[WHITE]);
	RETV(itemId == m_selectItem, m_brush[PINK]);
	RETV('@' == node->attrs["id"][0], m_brush[WHITE]); // dummy 노드는 색깔을 입히지 않음.

	int brushIdx = WHITE;

	if (m_nodesRun[node->id])
	{
		switch (node->result)
		{
		case -1:
		case 0:
			brushIdx = BLUE;			
			break;
		case 1: brushIdx = YELLOW; break;
		case 2: brushIdx = RED; break;
		default: assert(0); break;
		}
	}
	else
	{
		switch (node->result)
		{
		case -1:
		case 0:
			brushIdx = WHITE;
			break;
		case 1: brushIdx = YELLOW; break;
		case 2: brushIdx = RED; break;
		default: assert(0); break;
		}
	}

	return m_brush[brushIdx];
}


void CParseTreeView::HideTree()
{
	m_isShow = false;
}


bool CParseTreeView::IsShow()
{
	return m_isShow;
}


//  화면 갱신
void CParseTreeView::Update(const float deltaSeconds)
{
	RET(!m_isShow);

	m_incTime += deltaSeconds;
	if (m_incTime < 0.033f) // 30 fps
		return;
	m_incTime = 0;

	// 노드 상태가 바뀌면, 화면을 갱신한다.
	bool isChangeNodeState = false;
	for each (auto it in m_treeNodes)
	{
		if (m_nodesRun[it->id] != it->isRun)
		{
			isChangeNodeState = true;
			m_nodesRun[it->id] = it->isRun;
		}
	}

	if (isChangeNodeState)
		InvalidateRect(NULL);
}


BOOL CParseTreeView::OnEraseBkgnd(CDC* pDC)
{
	//return CScrollView::OnEraseBkgnd(pDC);
	return TRUE;
}


// 커서 위치에 있는 트리 노드를 리턴한다.
int CParseTreeView::GetTreeItem(const CPoint cursorPos)
{
	for (int i = 0; i < m_itemCount; ++i)
	{
		if (m_nodesPosition[i].PtInRect(cursorPos)) // m_nodesPosition[] 은 이미 scrollpos 가 적용된 상태다.
			return i;
	}
	return -1;
}


void CParseTreeView::OnMouseMove(UINT nFlags, CPoint point)
{
	bool isDraw = false;
	m_selectItem = GetTreeItem(point);
	if (m_oldSelectItem != m_selectItem)
		isDraw = true;

	m_oldSelectItem = m_selectItem;

	if (m_leftBtnDown)
	{
		const int vert = GetScrollPos(SB_VERT);
		const int horz = GetScrollPos(SB_HORZ);
		const CPoint diff = m_clickPos - point;
		m_clickPos = point;
		SetScrollPos(SB_VERT, vert + diff.y * 4);
		SetScrollPos(SB_HORZ, horz + diff.x * 4);
		isDraw = true;
	}

	if (isDraw)
		InvalidateRect(NULL);

	CScrollView::OnMouseMove(nFlags, point);
}


void CParseTreeView::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_leftBtnDown = true;
	m_clickPos = point;
	SetCapture();
	SetFocus();

	bool isShow = true;
	m_selectItem = GetTreeItem(point);
	RET(m_selectItem < 0);

	m_nodesShowImage[m_selectItem] = !m_nodesShowImage[m_selectItem]; // toggle
	InvalidateRect(NULL);
	CScrollView::OnLButtonDown(nFlags, point);
}


void CParseTreeView::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_leftBtnDown = false;
	ReleaseCapture();

	CScrollView::OnLButtonUp(nFlags, point);
}


void CParseTreeView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	bool isShow = true;
	m_selectItem = GetTreeItem(point);
	RET(m_selectItem < 0);

	const int nodeId = m_itemId2NodeId[m_selectItem];
	sParseTree *node = m_matchScript->m_nodeTable[nodeId];
	RET(!node);

	if (isShow)
	{
		CImageFrameWnd *imageWnd = new CImageFrameWnd();
		imageWnd->Init("img", node->attrs["id"], CRect(0, 300, 300, 600), this, true);
		imageWnd->ShowWindow(SW_SHOW);
	}

	CScrollView::OnMButtonDblClk(nFlags, point);
}

void CParseTreeView::ShowAllImage(const bool isShow)
{
	memset(m_nodesShowImage, isShow, sizeof(m_nodesShowImage));
	InvalidateRect(NULL);
}


// 이미 읽은 파일이라면, 포인터를 리턴한다.
CImage* CParseTreeView::LoadImage(const string &fileName)
{
	auto it = m_imagePool.find(fileName);
	if (it != m_imagePool.end())
		return it->second;

	if (fileName.find('.') == string::npos)
	{
		// 확장자가 없으면, 파일이 아님.
		return NULL;
	}

	CImage *img = new CImage();
	if (S_OK != img->Load(str2wstr(fileName).c_str()))
	{
		delete img;
		return NULL;
	}

	m_imagePool[fileName] = img;
	return img;
}


// 이미 읽은 파일이라면, 포인터를 리턴한다.
Gdiplus::Bitmap* CParseTreeView::LoadBitmap(const string &key)
{
	auto it = m_bitmapPool.find(key);
	if (it != m_bitmapPool.end())
		return it->second;
	return NULL;
}


void CParseTreeView::ClearImagePool()
{
	// clear image memroy pool
	for each (auto it in m_imagePool)
		delete it.second;
	m_imagePool.clear();
}


void CParseTreeView::ClearBitmapPool()
{
	// clear bitmap memroy pool
	for each (auto it in m_bitmapPool)
		delete it.second;
	m_bitmapPool.clear();
}


void CParseTreeView::SetSearchString(const string &searchStr)
{
	m_searchStr = searchStr;
	InvalidateRect(NULL);
}
