//
// thread test
//

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <windows.h>
#include <mmsystem.h>
#include <process.h>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"

#ifdef _DEBUG
#pragma comment(lib, "opencv_core310d.lib")
#pragma comment(lib, "opencv_imgcodecs310d.lib")
#pragma comment(lib, "opencv_features2d310d.lib")
#pragma comment(lib, "opencv_videoio310d.lib")
#pragma comment(lib, "opencv_highgui310d.lib")
#pragma comment(lib, "opencv_imgproc310d.lib")
#else
#pragma comment(lib, "opencv_core310.lib")
#pragma comment(lib, "opencv_imgcodecs310.lib")
#pragma comment(lib, "opencv_features2d310.lib")
#pragma comment(lib, "opencv_videoio310.lib")
#pragma comment(lib, "opencv_highgui310.lib")
#pragma comment(lib, "opencv_imgproc310.lib")
#endif

#pragma comment(lib, "winmm.lib")

using namespace std;
using namespace cv;

Mat g_A, g_B;


//void templateMatch(const string &sceneFileName, const string &objectFileName)
void templateMatch(Mat &A, Mat &B)
{
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
}

bool startTest = false;
unsigned __stdcall ThreadProcess(void *pThreadPtr)
{
	bool *isEnd = (bool*)pThreadPtr;
	while (!startTest);// Sleep(1);

	templateMatch(g_A, g_B);

	*isEnd = true;
	return 0;
}


void main()
{
 	g_A = imread("single_advance2_result.png", IMREAD_GRAYSCALE);
 	g_B = imread("num7.png", IMREAD_GRAYSCALE);
// 	g_A = imread("capture1541.jpg", IMREAD_GRAYSCALE);
// 	g_B = imread("dirt3_single_temp3.jpg", IMREAD_GRAYSCALE);

	// i5 - dual core system
	// single thread = 8 templateMatch, 420 milliseconds
	//							 16 templateMatch, 800 milliseconds
	const int t1 = timeGetTime();
	templateMatch(g_A, g_B);
	templateMatch(g_A, g_B);
	templateMatch(g_A, g_B);
	templateMatch(g_A, g_B);
	templateMatch(g_A, g_B);
	templateMatch(g_A, g_B);
	templateMatch(g_A, g_B);
	templateMatch(g_A, g_B);
	templateMatch(g_A, g_B);
	templateMatch(g_A, g_B);
	templateMatch(g_A, g_B);
	templateMatch(g_A, g_B);
	templateMatch(g_A, g_B);
	templateMatch(g_A, g_B);
	templateMatch(g_A, g_B);
	templateMatch(g_A, g_B);
	const int t2 = timeGetTime();
	cout << "elapsed time1 = " << t2 - t1 << endl;

	// i5 - dual core system
	// multi thread = 8 templateMatch, 200 milliseconds
	//							16 templateMatch, 390 milliseconds, while
	//							16 templateMatch, 330 milliseconds, WaitForSingleObject
	//							16 templateMatch, 350 milliseconds, sleep(1)
	const int t3 = timeGetTime();
	bool isEnd[16];
	ZeroMemory(isEnd, sizeof(isEnd));
	HANDLE hThread[16];
	for (int i = 0; i < 16; ++i)
	{
		hThread[ i] = (HANDLE)_beginthreadex(NULL, 0, ThreadProcess, &isEnd[i], 0, NULL);
	}
	startTest = true;
//  	WaitForSingleObject(hThread[0], 10000);
// 	WaitForSingleObject(hThread[1], 10000);
// 	WaitForSingleObject(hThread[2], 10000);
// 	WaitForSingleObject(hThread[3], 10000);
// 	WaitForSingleObject(hThread[4], 10000);
// 	WaitForSingleObject(hThread[5], 10000);
// 	WaitForSingleObject(hThread[6], 10000);
// 	WaitForSingleObject(hThread[7], 10000);
// 	WaitForSingleObject(hThread[8], 10000);
// 	WaitForSingleObject(hThread[9], 10000);
// 	WaitForSingleObject(hThread[10], 10000);
// 	WaitForSingleObject(hThread[11], 10000);
// 	WaitForSingleObject(hThread[12], 10000);
// 	WaitForSingleObject(hThread[13], 10000);
// 	WaitForSingleObject(hThread[14], 10000);
// 	WaitForSingleObject(hThread[15], 10000);
	while (1)
	{
		if ((isEnd[0] && isEnd[1] && isEnd[2] && isEnd[3] &&
			isEnd[4] && isEnd[5] && isEnd[6] && isEnd[7]) &&

			(isEnd[8] && isEnd[9] && isEnd[10] && isEnd[11] &&
				isEnd[12] && isEnd[13] && isEnd[14] && isEnd[15]))
			break;
		Sleep(1);
	}
	const int t4 = timeGetTime();
	cout << "elapsed time2 = " << t4 - t3 << endl;
	waitKey(0);
}
