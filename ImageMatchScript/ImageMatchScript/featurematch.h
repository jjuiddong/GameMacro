//	sample code
// https://fossies.org/dox/opencv-3.1.0/tutorial_feature_homography.html
//
#pragma once

#include "global.h"

namespace match
{

	class cFeatureMatch
	{
	public:
		cFeatureMatch(const bool gray = true, const bool debug = false);
		virtual ~cFeatureMatch();
		bool Match(const cv::Mat &img, const cv::Mat &templ, const string &comment = "");
		bool Match(const string &srcFileName, const string &templFileName, const string &comment = "");
		bool SIFTMatch(const cv::Mat &img, const cv::Mat &templ, const string &comment = "");

		bool Match(const cv::Mat &img, const vector<cv::KeyPoint> &imgKeyPoints,  const cv::Mat &imgDescriptors,
			const cv::Mat &templ, const vector<cv::KeyPoint> &templKeyPoints, const cv::Mat &templDescriptors,
			const string &comment = "");


	public:
		double m_min;
		double m_max;
		bool m_gray;
		bool m_debug; // true이면 매칭 결과를 화면에 출력한다.
		cv::Mat m_matResult;
		cRectContour m_rect;
		bool m_IsDetection;
	};

}