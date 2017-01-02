// 2016-05-24 

#include <stdio.h>
#include <iostream>
#include <stdio.h>
#include <iostream>
#include "opencv2/core.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/xfeatures2d.hpp"
using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;

#ifdef _DEBUG
	#pragma comment(lib, "opencv_core310d.lib")
	#pragma comment(lib, "opencv_imgcodecs310d.lib")
	#pragma comment(lib, "opencv_features2d310d.lib")
	#pragma comment(lib, "opencv_highgui310d.lib")
	#pragma comment(lib, "opencv_xfeatures2d310d.lib")
	#pragma comment(lib, "opencv_flann310d.lib")
	#pragma comment(lib, "opencv_imgproc310d.lib")
#else
	#pragma comment(lib, "opencv_core310.lib")
	#pragma comment(lib, "opencv_imgcodecs310.lib")
	#pragma comment(lib, "opencv_features2d310.lib")
	#pragma comment(lib, "opencv_highgui310.lib")
	#pragma comment(lib, "opencv_xfeatures2d310.lib")
	#pragma comment(lib, "opencv_flann310.lib")
	#pragma comment(lib, "opencv_imgproc310.lib")
#endif

#pragma comment(lib, "winmm.lib")

//@function main
//@brief Main function
int main(int argc, char** argv)
{
	if (argc != 3)
	{
		cout << "commandline <image1> <image2> \n" << endl;
		return 0;
	}

	Mat A = imread(argv[1], IMREAD_GRAYSCALE);
	Mat B = imread(argv[2], IMREAD_GRAYSCALE);
	if (!A.data || !B.data)
	{
		std::cout << " --(!) Error reading images " << std::endl;
		return -1;
	}

	Mat C(cvSize(A.cols - B.cols + 1, A.rows - B.rows + 1), IPL_DEPTH_32F);
	double min, max;
	Point left_top;
	cv::matchTemplate(A, B, C, CV_TM_CCOEFF_NORMED);
	cv::minMaxLoc(C, &min, &max, NULL, &left_top);

	cout << "max = " << max << endl;

	Mat img = A.clone();
	cvtColor(img, img, CV_GRAY2BGR);
	Point pos = left_top;
	cv::rectangle(img, pos, cvPoint(pos.x + B.cols, pos.y + B.rows), CV_RGB(255, 0, 0));
	imshow("result", img);
	waitKey(0);

	return 0;
}
