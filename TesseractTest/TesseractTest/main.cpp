
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
#pragma comment(lib, "lept173d.lib")
#pragma comment(lib, "tesseract305d.lib")

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
//#pragma comment(lib, "lept173.lib")
#pragma comment(lib, "tesseract305.lib")

#endif


#include <allheaders.h>
#include <baseapi.h>

using namespace std;
using namespace tesseract;
using namespace cv;


void main()
{
	//TessBaseAPI *api = new TessBaseAPI();
	TessBaseAPI api;
	if (api.Init("./", "eng")) {
		cout << "error init tesseract english " << endl;
		return;
	}

// 	tesseract::TessBaseAPI tess;
// 	cv::Mat sub = image(cv::Rect(50, 200, 300, 100));
// 	tess.SetImage((uchar*)sub.data, sub.size().width, sub.size().height, sub.channels(), sub.step1());
// 	tess.Recognize(0);
// 	const char* out = tess.GetUTF8Text();

	Mat sub = imread("skewimg5.jpg");
	api.SetImage((uchar*)sub.data, sub.size().width, sub.size().height, sub.channels(), sub.step1());
	api.Recognize(0);

// 	Pix *image = pixRead("norway.bmp");
// 	api->SetImage(image);
	char *outText = api.GetUTF8Text();
	cout <<  outText << endl;

	api.End();
	delete[] outText;
	//pixDestroy(&image);
}
