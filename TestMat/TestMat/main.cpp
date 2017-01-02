
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv/cvaux.h>
#include <opencv2/calib3d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <iostream>

#ifdef _DEBUG
#pragma comment(lib, "opencv_core310d.lib")
#pragma comment(lib, "opencv_imgcodecs310d.lib")
#pragma comment(lib, "opencv_features2d310d.lib")
#pragma comment(lib, "opencv_videoio310d.lib")
#pragma comment(lib, "opencv_highgui310d.lib")
#pragma comment(lib, "opencv_imgproc310d.lib")
#pragma comment(lib, "opencv_flann310d.lib")
#pragma comment(lib, "opencv_xfeatures2d310d.lib")
#pragma comment(lib, "opencv_calib3d310d.lib")
#else
#pragma comment(lib, "opencv_core310.lib")
#pragma comment(lib, "opencv_imgcodecs310.lib")
#pragma comment(lib, "opencv_features2d310.lib")
#pragma comment(lib, "opencv_videoio310.lib")
#pragma comment(lib, "opencv_highgui310.lib")
#pragma comment(lib, "opencv_imgproc310.lib")
#pragma comment(lib, "opencv_flann310.lib")
#pragma comment(lib, "opencv_xfeatures2d310.lib")
#pragma comment(lib, "opencv_calib3d310.lib")
#endif


using namespace std;
using namespace cv;

void main()
{
	Mat img = imread("capture7888.jpg");

	Mat dst;
	Mat dst1;

	cvtColor(img, dst1, CV_BGR2HSV);
	Mat lower_red_hue_range;
	Mat upper_red_hue_range;
	inRange(dst1, cv::Scalar(40, 0, 0), cv::Scalar(200, 255, 255), lower_red_hue_range);

 	cvtColor(img, dst, CV_BGR2HLS);
 	Mat lower_red_hue_range2;
 	Mat upper_red_hue_range2;
	inRange(dst, cv::Scalar(20,200, 100), cv::Scalar(80, 255, 180), lower_red_hue_range2);

	Mat dst2;
	cvtColor(img, dst2, CV_BGR2GRAY);
	threshold(dst2, dst2, 100, 255, CV_THRESH_BINARY);

// 	Mat dst = img & Scalar(255, 255, 0);
// 
// 	Mat gray;
// 	cvtColor(dst, gray, CV_BGR2GRAY);
// 	gray *= 1;
// 
// 	Mat gray2;
// 	cvtColor(img, gray2, CV_BGR2GRAY);
}
