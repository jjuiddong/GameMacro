//
// �簢 �ڽ��� ǥ���ϴ� Ŭ����.
//
#pragma once


class cRectContour
{
public:
	cRectContour();
	cRectContour(const cv::Rect &rect);
	virtual ~cRectContour();

	bool Init(const cv::Rect &rect);
	void Draw(cv::Mat &dst, const cv::Scalar &color = cv::Scalar(0, 0, 0), const int thickness = 1) const;

	int Width() const;
	int Height() const;
	cv::Point At(const int index) const;
	cv::Point Center() const;
	void ScaleCenter(const float scale);
	void Scale(const float vscale, const float hscale);

	cRectContour& operator = (const cRectContour &rhs);


public:
	vector<cv::Point> m_contours;
	int m_xIdx; // x axis index
};