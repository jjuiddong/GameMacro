//
// 2016-06-28, jjuiddong
// ParseTree ��� Ŭ����
//
// scrollview double buffering
//		- http://adnoctum.tistory.com/149 ����
//
#pragma once


class CParseTreeView : public CScrollView
{
public:
	CParseTreeView();
	virtual ~CParseTreeView();

	void ShowTree(const cvproc::imagematch::cMatchScript2 &script, const string &treeName);
	void HideTree();
	bool IsShow();
	void Update(const float deltaSeconds);
	void ShowAllImage(const bool isShow);
	void SetSearchString(const string &searchStr);


protected:
	void MakeSrcTreeImage();
	bool DrawTreeImage();
	cv::Point DrawTree(CDC *pDC, cvproc::imagematch::sParseTree *node, const cv::Point prevPos, const cv::Point pos);
	CBrush& SelectTreeNodeColor(CDC* pDC, const int itemId, cvproc::imagematch::sParseTree *node);
	int GetTreeItem(const CPoint cursorPos);
	CImage* LoadImage(const string &fileName);
	Gdiplus::Bitmap* LoadBitmap(const string &name);
	void DrawEdge(CDC *pDC, const int x, const int y, const int width, const int height);
	void ClearImagePool();
	void ClearBitmapPool();


public:
	string m_treeName; // display tree name
	const cvproc::imagematch::cMatchScript2 *m_matchScript;
	cv::Point  m_maxTreeSize; // physical(pixel) tree node display size
	CRect m_clientRect; // client window size, using display tree node optimze
	bool m_isShow; // ȭ�鿡 ��µǰ� �ִ� ��Ȳ�̶�� true
	bool m_isShowProcCnt; // show proc_cnt > 0 node
	bool m_isShowSuccess; // show result > 0 node
	bool m_isShowRunning; // show running thread, run node

	float m_incTime; // �����ֱ� 30fps
	enum {BLUE, WHITE,YELLOW,RED,PINK};
	CBrush m_brush[5]; // blue, white, yellow, red, pink
	CImage m_treeImgSrc; // Ʈ�� ���� �̹���, rect = tree image rect
	CImage m_displayImg; // ȭ�鿡 ��µ� Ʈ�� �̹���, rect = client rect

	set<cvproc::imagematch::sParseTree*> m_treeNodes;
	bool m_nodesRun[1024]; // ������� �����ϱ� ������, ����ȭ�� ����, �ӽ� ���۷� ������ ���¸� �Ǵ��Ѵ�. key = node->id
	int m_itemId2NodeId[2048]; // Ʈ�� ������ ���̵� ��� ���̵�� �ٲ��ִ� ���̺�
	CRect m_nodesPosition[2048]; // Ʈ����� ��ġ, key = itemId
	bool m_nodesShowImage[2048]; // Ʈ�� ��� �̹��� ���, key = itemId
	int m_incItemId; // ������ ���̵� ����ϱ����� ����, 0���� �����ȴ�.
	int m_itemCount; // ��µǴ� Ʈ����� ����, OnDraw()�� ����
	int m_selectItem; // tree item id, not node id
	int m_oldSelectItem;
	bool m_leftBtnDown; // mouse scroll, left mouse button down
	CPoint m_clickPos;
	string m_searchStr;

	map<string, CImage*> m_imagePool; // key=filename, data=image
	map<string, Gdiplus::Bitmap*> m_bitmapPool; // key=filename, data=bitmap


protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};
