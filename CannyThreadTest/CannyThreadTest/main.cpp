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
#include <thread>
#include <windows.h>

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

Mat g_img;

// UI Update Thread
void TestThread(int n)
{
	Mat dst = g_img.clone();
	if (dst.data && (dst.channels() >= 3))
		cvtColor(dst, dst, CV_BGR2GRAY);
	threshold(dst, dst, 20, 255, CV_THRESH_BINARY_INV);
	cv::Canny(dst, dst, 100, 200, 5);
}


//@function main
//@brief Main function
int main(int argc, char** argv)
{
	g_img = imread("capture5342.jpg");

	int count = 1;
	while (1)
	{
		cout << "test " << count++ << endl;
		//Sleep(1000);

		std::thread thread1(TestThread, 0);
		std::thread thread2(TestThread, 0);
		std::thread thread3(TestThread, 0);
		std::thread thread4(TestThread, 0);
		std::thread thread5(TestThread, 0);

		thread1.join();
		thread2.join();
		thread3.join();
		thread4.join();
		thread5.join();
	}

	return 0;
}
