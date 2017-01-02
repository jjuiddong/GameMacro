// 2016-05-24 

#include "../../../Common/Common/common.h"
#include "../../../Common/Network/network.h"
#include "../../../Common/CamCommon/camcommon.h"

#include "opencv2/core.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/xfeatures2d.hpp"

#include <windows.h>
#include "Shlwapi.h"

using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "Shlwapi.lib")

void CallBackFunc(int event, int x, int y, int flags, void* userdata);
vector<cv::Point> g_clickPos;
Point g_leftClickPos;
Point g_mousePos;
bool g_isInitRect = false;
bool g_isMoveWindow = false;
Point g_rightClickPos;
Rect g_imgRect;
sRect g_frame;
int g_rectMode = 0;	// 0:4point contour, 1:2point rect
bool g_isLBtnClick = false;


int main(int argc, char** argv)
{
	if (argc < 3)
	{
		cout << "commandline <image> <save directory> <option>" << endl;
		cout << "option => bgr=255,255,255 scale=1 gray=0/1 hsv=10,0,0,100,255,255" << endl;
		cout << "skew=value " << endl;
		getchar();
		return 0;
	}

	Mat srcImg = imread(argv[1]);
	if (!srcImg.data)
	{
		cout << "not found image file" << endl;
		getchar();
		return 0;
	}

	string destDirectoryPath = argv[2];
	if (destDirectoryPath.empty())
	{
		cout << "destinaton directory path is empty!!" << endl;
		getchar();
		return 0;
	}

	string cmds;
	for (int i = 3; i < argc; ++i)
		cmds += string(argv[i]) + " ";

 	Mat img;
	cvproc::imagematch::cSimpleMatchScript *smatch = new cvproc::imagematch::cSimpleMatchScript(NULL);
 	smatch->Match(srcImg, img, cmds, "", "", "", false);
	delete smatch;

	
// 	// option 처리
// 	if (argc > 3)
// 	{
// 		vector<Mat> accMat;
// 		accMat.reserve(8);
// 
// 		Mat hsvImg;
// 		Mat hlsImg;
// 
// 		for (int i = 3; i < argc; ++i)
// 		{
// 			int bgr[3] = { 0,0,0 };
// 			sscanf(argv[i], "bgr=%d,%d,%d", bgr, bgr + 1, bgr + 2);
// 			if ((bgr[0] != 0) || (bgr[1] != 0) || (bgr[2] != 0))
// 			{
// 				img &= Scalar(bgr[0], bgr[1], bgr[2]);
// 				continue;
// 			}
// 
// 			float scale = 0;
// 			sscanf(argv[i], "scale=%f", &scale);
// 			if (scale != 0)
// 			{
// 				img *= scale;
// 				continue;
// 			}
// 
// 			int gray = 0;
// 			sscanf(argv[i], "gray=%d", &gray);
// 			if (gray)
// 			{
// 				cvtColor(img, img, CV_BGR2GRAY);
// 				cvtColor(img, img, CV_GRAY2BGR);
// 				continue;
// 			}
// 
// 			int hsv[6] = { 0,0,0, 0,0,0 }; // inrage
// 			sscanf(argv[i], "hsv=%d,%d,%d,%d,%d,%d", hsv, hsv + 1, hsv + 2, hsv+3, hsv + 4, hsv + 5);
// 			if ((hsv[0] != 0) || (hsv[1] != 0) || (hsv[2] != 0) || (hsv[3] != 0) || (hsv[4] != 0) || (hsv[5] != 0))
// 			{
// 				if (!hsvImg.data)
// 					cvtColor(img, hsvImg, CV_BGR2HSV);
// 				Mat tmp;
// 				inRange(hsvImg, cv::Scalar(hsv[0], hsv[1], hsv[2]), cv::Scalar(hsv[3], hsv[4], hsv[5]), tmp);
// 				accMat.push_back(tmp);
// 				continue;
// 			}
// 
// 			int hls[6] = { 0,0,0, 0,0,0 }; // inrage
// 			sscanf(argv[i], "hls=%d,%d,%d,%d,%d,%d", hls, hls + 1, hls + 2, hls + 3, hls + 4, hls + 5);
// 			if ((hls[0] != 0) || (hls[1] != 0) || (hls[2] != 0) || (hls[3] != 0) || (hls[4] != 0) || (hls[5] != 0))
// 			{
// 				if (!hlsImg.data)
// 					cvtColor(img, hlsImg, CV_BGR2HLS);
// 
// 				Mat tmp;
// 				inRange(hlsImg, cv::Scalar(hls[0], hls[1], hls[2]), cv::Scalar(hls[3], hls[4], hls[5]), tmp);
// 				accMat.push_back(tmp);
// 				continue;
// 			}
// 
// 			if (string(argv[i]) == string("acc"))
// 			{
// 				if (!accMat.empty())
// 				{
// 					Mat tmp(accMat.front().rows, accMat.front().cols, accMat.front().flags);
// 					for each (auto &m in accMat)
// 						tmp += m;
// 					img = tmp;
// 				}
// 			}
// 
// 			//----------------------------------------------------------------------
// 			int  thresh1 = 0;
// 			sscanf(argv[i], "threshold=%d", &thresh1);
// 			if (thresh1 > 0)
// 			{
// 				if (img.data && (img.channels() >= 3))
// 					cvtColor(img, img, CV_BGR2GRAY);
// 				threshold(img, img, thresh1, 255, CV_THRESH_BINARY);
// 				cvtColor(img, img, CV_GRAY2BGR);
// 				continue;
// 			}
// 
// 			//----------------------------------------------------------------------
// 			//     - dilate
// 			if (string(argv[i]) == "dilate")
// 			{
// 				dilate(img, img, Mat());
// 				continue;
// 			}
// 
// 			//----------------------------------------------------------------------
// 			//     - erode
// 			if (string(argv[i]) == "erode")
// 			{
// 				erode(img, img, Mat());
// 				continue;
// 			}
// 
// 			//----------------------------------------------------------------------
// 			// skew=arcAlpha
// 			float arcAlpha2 = 0;
// 			sscanf(argv[i], "skew=%f", &arcAlpha2);
// 			if (arcAlpha2 != 0)
// 			{
// 				Mat tmp = img.clone();
// 
// 				if (img.data && (img.channels() >= 3))
// 					cvtColor(img, img, CV_BGR2GRAY);
// 				threshold(img, img, 20, 255, CV_THRESH_BINARY_INV);
// 				cv::Canny(img, img, 0, 100, 5);
// 
// 				vector<vector<cv::Point>> contours;
// 				cv::findContours(img, contours, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
// 
// 				// 좌우로 가장긴 다각형을 찾는다.
// 				int maxLenIdx = -1;
// 				int maxLength = 0;
// 				std::vector<cv::Point> maxLine;
// 				std::vector<cv::Point> approx;
// 				for (u_int i = 0; i < contours.size(); i++)
// 				{
// 					cv::approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true)*arcAlpha2, true);
// 					if (approx.empty())
// 						continue;
// 
// 					int minX = 10000;
// 					int maxX = 0;
// 					for each (auto pos in approx)
// 					{
// 						if (minX > pos.x)
// 							minX = pos.x;
// 						if (maxX < pos.x)
// 							maxX = pos.x;
// 					}
// 
// 					const int len = abs(maxX - minX);
// 					if (maxLength < len)
// 					{
// 						maxLength = len;
// 						maxLenIdx = i;
// 					}
// 				}
// 
// 				if (maxLenIdx == -1)
// 					continue; // error occur
// 
// 							  // 좌우로 가장 긴 다각형의 기울어진 각도를 계산해서, 원본 이미지를 원래대로 복원한다.
// 				cv::approxPolyDP(cv::Mat(contours[maxLenIdx]), approx, cv::arcLength(cv::Mat(contours[maxLenIdx]), true)*arcAlpha2, true);
// 
// 				double maxV = 0;
// 				int idx = 0;
// 				for (u_int i = 0; i < approx.size() - 1; ++i)
// 				{
// 					const double n = cv::norm(approx[i] - approx[i + 1]);
// 					if (maxV < n)
// 					{
// 						maxV = n;
// 						idx = i;
// 					}
// 				}
// 
// 				Vector3 p1 = (approx[idx].x < approx[idx + 1].x) ? Vector3((float)approx[idx].x, (float)approx[idx].y, 0) : Vector3((float)approx[idx + 1].x, (float)approx[idx + 1].y, 0);
// 				Vector3 p2 = (approx[idx].x > approx[idx + 1].x) ? Vector3((float)approx[idx].x, (float)approx[idx].y, 0) : Vector3((float)approx[idx + 1].x, (float)approx[idx + 1].y, 0);
// 				Vector3 v = p2 - p1;
// 				v.Normalize();
// 				double angle = RAD2ANGLE(acos(v.DotProduct(Vector3(1, 0, 0))));
// 				if (v.y < 0)
// 					angle = -angle;
// 
// 				const Mat affine_matrix = getRotationMatrix2D(Point((int)p1.x, (int)p1.y), angle, 1);
// 				warpAffine(tmp, img, affine_matrix, img.size(), INTER_LINEAR, BORDER_CONSTANT, Scalar::all(255));
// 			}
// 
// 		}
// 	}




	g_imgRect = Rect(0, 0, img.cols, img.rows);
	g_frame = sRect(0, 0, img.cols, img.rows);
	imshow("image", img);
	moveWindow("image", 0, 0);
	setMouseCallback("image", CallBackFunc, NULL);

	cSkewDetect skew;
	Mat dst;
	int saveImgCnt = 0;
	cRectContour rect;

	while (1)
	{
		Rect roi(g_frame.left, g_frame.top, g_frame.Width(), g_frame.Height());
		Mat src = img.clone()(roi);
		putText(src, (g_rectMode == 0) ? "contour mode" : "rect mode", Point(20, 20), 1, 1, Scalar(0, 0, 255), 1);
		putText(src, "toggle rect mode numkey 1", Point(20, 40), 1, 1, Scalar(0, 0, 255), 1);
		
		// display mouse position
		stringstream ss;
		ss << "mouse pos = " << g_mousePos.x + g_frame.left << ", " << g_mousePos.y + g_frame.top;
		putText(src, ss.str().c_str(), Point(20, 60), 1, 1, Scalar(0, 0, 255), 1);

		// calc rectangle to skew transform
		if (g_clickPos.size() == 4)
		{
			rect.Init(g_clickPos);
			skew.Init(rect, 1.f, rect.Width(), rect.Height());
			cout << ">> skew.Transform(src);" << endl;
			dst = skew.Transform(src);
			cout << "<< skew.Transform(src);" << endl;
			if (dst.data)
				imshow("skew", dst);
			g_clickPos.clear();
			g_isInitRect = true;
		}

		// line drawing
		if (g_rectMode == 0) // contour mode
		{
			if ((g_clickPos.size() >= 1) && (g_clickPos.size() <= 3))
			{
				line(src, g_clickPos[g_clickPos.size() - 1], g_mousePos, Scalar(255, 0, 0), 2);
			}
		}

		// render rectangle line
		if (g_isInitRect)
		{
			rect.Draw(src, Scalar(255, 0, 0), 2);
		}
		else
		{
			for (int i = 0; i < (int)g_clickPos.size() - 1; ++i)
				line(src, g_clickPos[i], g_clickPos[i + 1], Scalar(255, 0, 0), 2);
			if (g_clickPos.size() > 3)
				line(src, g_clickPos[g_clickPos.size() - 1], g_clickPos[0], Scalar(255, 0, 0), 2);
		}

		if (g_rectMode == 1)
		{
			if (g_isLBtnClick)
			{
				Point p1 = g_leftClickPos;
				Point p2 = Point(g_mousePos.x, g_leftClickPos.y);
				Point p3 = g_mousePos;
				Point p4 = Point(g_leftClickPos.x, g_mousePos.y);

				line(src, p1,p2, Scalar(255, 0, 0), 2);
				line(src, p2, p3, Scalar(255, 0, 0), 2);
				line(src, p3, p4, Scalar(255, 0, 0), 2);
				line(src, p4, p1, Scalar(255, 0, 0), 2);

				// display mouse position
				stringstream ss2;
				ss2 << "rect width, height = " << abs(g_mousePos.x - g_leftClickPos.x) << ", " << abs(g_mousePos.y - g_leftClickPos.y);
				putText(src, ss2.str().c_str(), Point(20, 80), 1, 1, Scalar(0, 0, 255), 1);
			}
		}
		//--------

		imshow("image", src);
		const int key = waitKey(1);	
		if (key == VK_ESCAPE)
		{
			break;
		}
		else if (key == VK_RETURN)
		{
			// save file
			if (g_isInitRect)
			{
				g_isInitRect = false;
				string fileName;
				do
				{
					std::stringstream ss;
					ss << destDirectoryPath << "skewimg" << saveImgCnt++ << ".jpg";
					fileName = ss.str();
				} while (PathFileExistsA(fileName.c_str()));

				imwrite(fileName.c_str(), dst);
			}
		}
		else if (key == '1')
		{
			g_rectMode = (g_rectMode == 0) ? 1 : 0;
			g_clickPos.clear();
			g_isInitRect = true;
			g_isLBtnClick = false;
		}
	}

	return 0;
}


void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
	if (event == EVENT_LBUTTONDOWN)
	{
		g_isLBtnClick = true;
		g_leftClickPos = Point(x, y);

		if (g_rectMode ==0)
			g_clickPos.push_back(Point(x, y));
	}
	else if (event == EVENT_LBUTTONUP)
	{
		if (g_isLBtnClick && (g_rectMode==1))
		{
			g_isLBtnClick = false;

			const Point lt = Point(min(g_leftClickPos.x, g_mousePos.x), min(g_leftClickPos.y, g_mousePos.y)); // left top
			const Point rb = Point(max(g_leftClickPos.x, g_mousePos.x), max(g_leftClickPos.y, g_mousePos.y)); // right bottom
			const Point df = lt - rb;
			if ((df.x == 0) || (df.y == 0))
				return;

			const Point p1 = lt;
			const Point p2 = Point(rb.x, lt.y);
			const Point p3 = rb;
			const Point p4 = Point(lt.x, rb.y);

			g_clickPos.clear();
			g_clickPos.push_back(p1);
			g_clickPos.push_back(p2);
			g_clickPos.push_back(p3);
			g_clickPos.push_back(p4);
		}
		//g_clickPos.push_back(Point(x, y));
	}
	else if (event == EVENT_RBUTTONDOWN)
	{
		g_isInitRect = false;
		g_clickPos.clear();
		g_isMoveWindow = true;
		g_rightClickPos = Point(x, y);
	}
	else if (event == EVENT_RBUTTONUP)
	{
		g_isMoveWindow = false;
	}
	else if (event == EVENT_MBUTTONDOWN)
	{		
	}
	else if (event == EVENT_MOUSEMOVE)
	{
		g_mousePos = Point(x, y);

		if (g_isMoveWindow)
		{
 			Point diff = g_rightClickPos - Point(x, y);
			OffsetRect(&g_frame, diff.x, diff.y);
			g_frame.left = MAX(0, g_frame.left);
			g_frame.top = MAX(0, g_frame.top);
			g_frame.left = MIN(g_imgRect.width, g_frame.left);
			g_frame.top = MIN(g_imgRect.height, g_frame.top);
			g_frame.right = g_imgRect.width;
			g_frame.bottom = g_imgRect.height;

			g_rightClickPos = Point(x,y);
		}
	}
}

