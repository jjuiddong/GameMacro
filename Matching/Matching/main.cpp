
#include <iostream>
#include <string.h>
// for OpenCV2
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
//#include "opencv2/gpu/gpu.hpp"
#include "opencv2/highgui/highgui.hpp"

#pragma comment(lib, "opencv_core300d.lib")
#pragma comment(lib, "opencv_imgcodecs300d.lib")
#pragma comment(lib, "opencv_features2d300d.lib")
#pragma comment(lib, "opencv_videoio300d.lib")
#pragma comment(lib, "opencv_highgui300d.lib")
 #pragma comment(lib, "opencv_imgproc300d.lib")
// #pragma comment(lib, "opencv_flann300d.lib")
// #pragma comment(lib, "opencv_objdetect300d.lib")
// #pragma comment(lib, "opencv_ts300d.lib")


using namespace cv;
using namespace std;

void main()
{
	string mapStr[] = {
		"finland_tupasentie_pro.png",
		"finland_alhojarvi.png",
		"finland_hanisjarventie.png",
		"finland_hanisjarventie2.png",
		"finland_tupasentie_pro5.png",
		"finland_tupasentie_pro6.png",
		"finland_isojarvi.png",
		"finland_kaatselk.png",
		"finland_kakaristo.png",
		"finland_lankamaa.png",
		"finland_lankamaa2.png",
		"finland_mutanen.png",
		"finland_mutanen2.png",
		"finland_tupasentie_pro2.png",
		"finland_tupasentie_pro3.png",
		"finland_tupasentie_pro4.png",
		"kenya_mwatate_pro.png",
	};

	string labelStr[] = {
		"finland_tupasentie_label.png",
		"finland_alhojarvi_label.png",
		"finland_hanisjarventie_label.png",
		"finland_isojarvi_label.png",
		"finland_kaatselk_label.png",
		"finland_kakaristo_label.png",
		"finland_lankamaa_label.png",
		"finland_mutanen_label.png",
		"kenya_mwatate_pro_label.png",
	};
	
	const int mapSize = sizeof(mapStr) / sizeof(string);
	const int labelSize = sizeof(labelStr) / sizeof(string);

	vector<Mat> labels;
	labels.reserve(labelSize);
	for (int k = 0; k < labelSize; ++k)
	{
		Mat B = imread("label/" + labelStr[k]);
		cvtColor(B, B, CV_BGR2GRAY);
		labels.push_back(B);
	}

	for (int i = 0; i < mapSize; ++i)
	{
		Mat A = imread("track result/" + mapStr[i]);
		if (A.empty())
			continue;

		cvtColor(A, A, CV_BGR2GRAY);

		double bestMax = 0;
		int bestIdx = 0;
		Point bestLeft_top;
		for (int k = 0; k < (int)labels.size(); ++k)
		{
			double min, max;
			Point left_top;

			Mat C(cvSize(A.cols - labels[k].cols + 1, A.rows - labels[k].rows + 1), IPL_DEPTH_32F);
			cv::matchTemplate(A, labels[k], C, CV_TM_CCOEFF_NORMED);
			cv::minMaxLoc(C, &min, &max, NULL, &left_top);

			if (bestMax < max)
			{
				bestMax = max;
				bestLeft_top = left_top;
				bestIdx = k;
			}

			//cout << "k = " << k << ", min = " << min << ", max = " << max << endl;
		}

		Mat B = imread(labelStr[bestIdx]);
		cv::rectangle(A, bestLeft_top, cvPoint(bestLeft_top.x + B.cols, bestLeft_top.y + B.rows), CV_RGB(255, 0, 0)); // 찾은 물체에 사격형을 그린다.
		cout << "find label = " << labelStr[bestIdx].c_str() << endl;

		imshow("result", A); // 결과 보기
							 //imshow("T9-sample", B);
							 //imshow("C", C); // 상관계수 이미지 보기
		cvWaitKey(0);
	}

}
