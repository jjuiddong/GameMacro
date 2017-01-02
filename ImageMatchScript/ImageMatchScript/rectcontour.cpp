
#include "global.h"
#include "rectcontour.h"


using namespace cv;
using namespace match;



// hhttp://www.gisdeveloper.co.kr/15
// AP1 - AP2 를 지나는 직선
// AP3 - AP4 를 지나는 직선
// 두 직선의 교점을 찾아 IP에 저장한다.
// 교점이 없다면 false를 리턴한다.
bool GetIntersectPoint(const Point2f& AP1, const Point2f& AP2,
	const Point2f& BP1, const Point2f& BP2, Point2f* IP)
{
	float t;
	float s;
	float under = (BP2.y - BP1.y)*(AP2.x - AP1.x) - (BP2.x - BP1.x)*(AP2.y - AP1.y);
	if (under == 0) return false;

	float _t = (BP2.x - BP1.x)*(AP1.y - BP1.y) - (BP2.y - BP1.y)*(AP1.x - BP1.x);
	float _s = (AP2.x - AP1.x)*(AP1.y - BP1.y) - (AP2.y - AP1.y)*(AP1.x - BP1.x);

	t = _t / under;
	s = _s / under;

	if (t<0.0 || t>1.0 || s<0.0 || s>1.0) return false;
	if (_t == 0 && _s == 0) return false;

	IP->x = AP1.x + t * (float)(AP2.x - AP1.x);
	IP->y = AP1.y + t * (float)(AP2.y - AP1.y);

	return true;
}


bool GetIntersectPoint(const cv::Point& AP1, const cv::Point& AP2,
	const cv::Point& BP1, const cv::Point& BP2, cv::Point2f* IP)
{
	return GetIntersectPoint(cv::Point2f((float)AP1.x, (float)AP1.y),
		cv::Point2f((float)AP2.x, (float)AP2.y),
		cv::Point2f((float)BP1.x, (float)BP1.y),
		cv::Point2f((float)BP2.x, (float)BP2.y),
		IP);
}



// contours 를 순서대로 정렬한다.
// 0 ---------- 1
// |				    |
// |					|
// |					|
// 3 ---------- 2
//
// chIndices 가 NULL이 아니면, src 인덱스가 바뀐 정보를 리턴한다.
bool OrderedContours(const cv::Point src[4], OUT cv::Point dst[4], OUT int *chIndices=NULL)
{
	//--------------------------------------------------------------------
	// 4 point cross check
	int crossIndices[3][4] =
	{
		{ 0, 1, 2, 3 },
		{ 0, 2, 1, 3 },
		{ 0, 3, 1, 2 },
	};

	int crossIdx = -1;
	for (int i = 0; i < 3; ++i)
	{
		// line1 = p1-p2 
		// line2 = p3-p4
		const cv::Point p1 = src[crossIndices[i][0]];
		const cv::Point p2 = src[crossIndices[i][1]];
		const cv::Point p3 = src[crossIndices[i][2]];
		const cv::Point p4 = src[crossIndices[i][3]];

		cv::Point2f tmp;
		if (GetIntersectPoint(p1, p2, p3, p4, &tmp))
		{
			crossIdx = i;
			break;
		}
	}

	if (crossIdx < 0)
		return false;

	// p1 ------ p2
	// |              |
	// |              |
	// |              |
	// p4 ------ p3
	const cv::Point p1 = src[crossIndices[crossIdx][0]];
	const cv::Point p2 = src[crossIndices[crossIdx][2]];
	const cv::Point p3 = src[crossIndices[crossIdx][1]];
	const cv::Point p4 = src[crossIndices[crossIdx][3]];


	//--------------------------------------------------------------------
	// 중점 계산
	// 가장 큰 박스를 찾는다.
	Point center = p1;
	center += p2;
	center += p3;
	center += p4;

	// 중심점 계산
	center.x /= 4;
	center.y /= 4;

	Vector3 v1 = Vector3((float)p1.x, (float)p1.y, 0) - Vector3((float)center.x, (float)center.y, 0);
	v1.Normalize();
	Vector3 v2 = Vector3((float)p2.x, (float)p2.y, 0) - Vector3((float)center.x, (float)center.y, 0);
	v2.Normalize();

	// 0 ---------- 1
	// |				    |
	// |					|
	// |					|
	// 3 ---------- 2
	const Vector3 crossV = v1.CrossProduct(v2);
	if (crossV.z > 0)
	{
		dst[0] = p1;
		dst[1] = p2;
		dst[2] = p3;
		dst[3] = p4;

		if (chIndices)
		{
			chIndices[0] = crossIndices[crossIdx][0];
			chIndices[1] = crossIndices[crossIdx][2];
			chIndices[2] = crossIndices[crossIdx][1];
			chIndices[3] = crossIndices[crossIdx][3];
		}
	}
	else
	{
		dst[0] = p2;
		dst[1] = p1;
		dst[2] = p4;
		dst[3] = p3;

		if (chIndices)
		{
			chIndices[0] = crossIndices[crossIdx][2];
			chIndices[1] = crossIndices[crossIdx][0];
			chIndices[2] = crossIndices[crossIdx][3];
			chIndices[3] = crossIndices[crossIdx][1];
		}
	}

	return true;
}

// contours 를 순서대로 정렬한다.
// 0 ---------- 1
// |				    |
// |					|
// |					|
// 2 ---------- 3
bool OrderedContours(const vector<cv::Point> &src, OUT vector<cv::Point> &dst)
{
	Point arr[4];
	Point out[4];

	arr[0] = src[0];
	arr[1] = src[1];
	arr[2] = src[2];
	arr[3] = src[3];

	const bool reval = OrderedContours(arr, out);
	dst[0] = out[0];
	dst[1] = out[1];
	dst[2] = out[2];
	dst[3] = out[3];

	return reval;
}




cRectContour::cRectContour() :
	m_contours(4)
	, m_xIdx(0)
{
}

cRectContour::cRectContour(const vector<cv::Point> &contours) :
	m_contours(4)
	, m_xIdx(0)
{
	Init(contours);
}

cRectContour::cRectContour(const vector<cv::KeyPoint> &contours) :
	m_contours(4)
	, m_xIdx(0)
{
	Init(contours);
}


cRectContour::cRectContour(const cv::Point &center, const float size) :
	m_contours(4)
	, m_xIdx(0)
{
	Init(center, size);
}

cRectContour::~cRectContour()
{
}


// 초기화.
bool cRectContour::Init(const cv::Point contours[4])
{
	Point dst[4];
	OrderedContours(contours, dst);
	m_contours[0] = dst[0];
	m_contours[1] = dst[1];
	m_contours[2] = dst[2];
	m_contours[3] = dst[3];
	//CalcWidthHeightVector();
	return true;
}

bool cRectContour::Init(const vector<cv::Point> &contours)
{
	OrderedContours(contours, m_contours);
	//CalcWidthHeightVector();
	return true;
}
bool cRectContour::Init(const vector<cv::Point2f> &contours)
{
	vector<cv::Point> ct(4);
	ct[0] = contours[0];
	ct[1] = contours[1];
	ct[2] = contours[2];
	ct[3] = contours[3];
	return Init(ct);
}


// 초기화.
bool cRectContour::Init(const vector<cv::KeyPoint> &keypoints)
{
	if (keypoints.size() < 4)
		return false;

	vector<Point> keys(4);
	for (u_int i = 0; i < keypoints.size(); ++i)
		keys[i] = Point(keypoints[i].pt);

	OrderedContours(keys, m_contours);
	//CalcWidthHeightVector();

	return true;
}


// 중점 center를 중심으로 size만큼 키워진 사각형을 만든다.
//
// 0 -------- 1
// |          |
// |    +     |
// |          |
// 3 -------- 2
//
bool cRectContour::Init(const cv::Point &center, const float size)
{
	m_contours[0] = center + Point(-(int)(size / 2.f), -(int)(size / 2.f));
	m_contours[1] = center + Point((int)(size / 2.f), -(int)(size / 2.f));
	m_contours[2] = center + Point((int)(size / 2.f), (int)(size / 2.f));
	m_contours[3] = center + Point(-(int)(size / 2.f), (int)(size / 2.f));
	//CalcWidthHeightVector();
	return true;
}

void cRectContour::Normalize()
{
	vector<Point> keys(4);
	for (u_int i = 0; i < m_contours.size(); ++i)
		keys[i] = m_contours[i];
	OrderedContours(keys, m_contours);
	//CalcWidthHeightVector();
}


// 박스 출력.
void cRectContour::Draw(cv::Mat &dst, const cv::Scalar &color, const int thickness)
// color = cv::Scalar(0, 0, 0), thickness = 1
{
	//DrawLines(dst, m_contours, color, thickness);
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

	return (int)(((m_contours[idx2].x - m_contours[idx1].x) + 
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


// Normalize된 상태에서 이 함수를 호출해야 한다.
void cRectContour::CalcWidthHeightVector()
{
	Vector3 p1((float)m_contours[0].x, (float)m_contours[0].y, 0);
	Vector3 p2((float)m_contours[1].x, (float)m_contours[1].y, 0);
	Vector3 v = p2 - p1;
	v.Normalize();
	Vector3 xAxis(1, 0, 0);
	if (abs(v.DotProduct(xAxis)) > 0.8f)
	{
		m_xIdx = 0;
	}
	else
	{
		m_xIdx = 1;
	}
}
