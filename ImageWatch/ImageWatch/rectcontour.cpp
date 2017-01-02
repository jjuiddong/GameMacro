
#include "stdafx.h"
#include "rectcontour.h"


using namespace cv;


cRectContour::cRectContour() :
	m_contours(4)
	, m_xIdx(0)
{
}

cRectContour::cRectContour(const cv::Rect &rect) :
	m_contours(4)
	, m_xIdx(0)
{
	Init(rect);
}

cRectContour::~cRectContour()
{
}

bool cRectContour::Init(const cv::Rect &rect)
{
	m_contours[0] = rect.tl();
	m_contours[1] = Point(rect.x + rect.width, rect.y);
	m_contours[2] = Point(rect.x + rect.width, rect.y+rect.height);
	m_contours[3] = Point(rect.x, rect.y+rect.height);
	return true;
}


// 선을 그린다.
void DrawLines(Mat &dst, const vector<cv::Point> &lines, const cv::Scalar &color, const int thickness,
	const bool isLoop=true)
{
	if (lines.size() < 2)
		return;

	for (u_int i = 0; i < lines.size() - 1; ++i)
		line(dst, lines[i], lines[i + 1], color, thickness);

	if (isLoop)
		line(dst, lines[lines.size() - 1], lines[0], color, thickness);
}


// 박스 출력.
void cRectContour::Draw(cv::Mat &dst, const cv::Scalar &color, const int thickness) const
// color = cv::Scalar(0, 0, 0), thickness = 1
{
	DrawLines(dst, m_contours, color, thickness);
}


// 사각형의 중점을 리턴한다.
Point cRectContour::Center() const
{
	Point center;
	for each (auto &pt in m_contours)
		center += pt;
	
	center = Point(center.x / m_contours.size(), center.y / m_contours.size());
	return center;
}


// 사각형의 중점을 중심으로 스케일한다.
void cRectContour::ScaleCenter(const float scale)
{
	const Point center = Center();

	for (u_int i=0; i < m_contours.size(); ++i)
	{
		m_contours[i] = center + ((m_contours[i] - center) * scale);
	}
}


// 가로 세로 각각 스케일링 한다.
// 0 -------- 1
// |          |
// |    +     |
// |          |
// 3 -------- 2
void cRectContour::Scale(const float vscale, const float hscale)
{

	// 가로 스케일링
	vector<cv::Point> tmp1(4);
	tmp1[0] = m_contours[1] + (m_contours[0] - m_contours[1]) * hscale;
	tmp1[1] = m_contours[0] + (m_contours[1] - m_contours[0]) * hscale;
	tmp1[2] = m_contours[3] + (m_contours[2] - m_contours[3]) * hscale;
	tmp1[3] = m_contours[2] + (m_contours[3] - m_contours[2]) * hscale;

	// 세로 스케일링
	vector<cv::Point> tmp2(4);
	tmp2[0] = m_contours[3] + (m_contours[0] - m_contours[3]) * vscale;
	tmp2[3] = m_contours[0] + (m_contours[3] - m_contours[0]) * vscale;
	tmp2[1] = m_contours[2] + (m_contours[1] - m_contours[2]) * vscale;
	tmp2[2] = m_contours[1] + (m_contours[2] - m_contours[1]) * vscale;

	m_contours[0] = (tmp1[0] + tmp2[0]) * 0.5f;
	m_contours[1] = (tmp1[1] + tmp2[1]) * 0.5f;
	m_contours[2] = (tmp1[2] + tmp2[2]) * 0.5f;
	m_contours[3] = (tmp1[3] + tmp2[3]) * 0.5f;
}


// index 번째 포인터를 리턴한다.
Point cRectContour::At(const int index) const
{
	return m_contours[index];
}


int cRectContour::Width() const
{
	const int idx1 = m_xIdx;
	const int idx2 = m_xIdx + 1;
	const int idx3 = m_xIdx + 2;
	const int idx4 = (m_xIdx + 3) % 4;

	return (int)abs(((m_contours[idx2].x - m_contours[idx1].x) + 
		(m_contours[idx3].x - m_contours[idx4].x)) * 0.5f);
}


int cRectContour::Height() const
{
	const int idx1 = m_xIdx + 1;
	const int idx2 = m_xIdx + 2;
	const int idx3 = (m_xIdx + 3) % 4;
	const int idx4 = m_xIdx;

	return (int)abs(((m_contours[idx2].y - m_contours[idx1].y) + 
		(m_contours[idx3].y - m_contours[idx4].y)) * 0.5f);
}


cRectContour& cRectContour::operator= (const cRectContour &rhs)
{
	if (this != &rhs)
	{
		m_contours = rhs.m_contours;
		m_xIdx = rhs.m_xIdx;
	}

	return *this;
}
