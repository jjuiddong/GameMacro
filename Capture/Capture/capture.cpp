// ScreenShotPlay.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <fstream>
#include <windows.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#pragma comment(lib, "winmm.lib")

#ifdef _DEBUG
	#pragma comment(lib, "opencv_core300d.lib")
	#pragma comment(lib, "opencv_highgui300d.lib")
	#pragma comment(lib, "opencv_imgcodecs300d.lib")
#else
	#pragma comment(lib, "opencv_core300.lib")
	#pragma comment(lib, "opencv_highgui300.lib")
	#pragma comment(lib, "opencv_imgcodecs300.lib")
#endif

using namespace std;
using namespace cv;

namespace std
{
	typedef unsigned int uint32_t;
	typedef int int32_t;
	typedef unsigned short uint16_t;
	typedef short int16_t;
}

typedef struct
{
	std::uint32_t biSize;
	std::int32_t  biWidth;
	std::int32_t  biHeight;
	std::uint16_t  biPlanes;
	std::uint16_t  biBitCount;
	std::uint32_t biCompression;
	std::uint32_t biSizeImage;
	std::int32_t  biXPelsPerMeter;
	std::int32_t  biYPelsPerMeter;
	std::uint32_t biClrUsed;
	std::uint32_t biClrImportant;
} DIB;

typedef struct
{
	std::uint16_t type;
	std::uint32_t bfSize;
	std::uint32_t reserved;
	std::uint32_t offset;
} HEADER;

typedef struct
{
	HEADER header;
	DIB dib;
} BMP;

HWND g_dirt3Hwnd = NULL;
void FocusWindow();
void PressScreenShot();
Mat GetClipboardImage();

int main()
{
	cout << "capture screenshot " << endl;
	cout << "capture loop " << endl;

	int oldT = timeGetTime();
	
	bool isWindowPos = false;
	while (1)
	{
// 		cout << "focus window.." << endl;
// 		FocusWindow();
// 
// 		cout << "press screenshot " << endl;
// 		PressScreenShot();

		const int curT = timeGetTime();
		if (!GetAsyncKeyState(VK_F12))
			continue;

		if (curT - oldT < 300)
			continue;
		oldT = curT;

		PressScreenShot();

		cout << "get clipboard image" << endl;
		Mat img = GetClipboardImage();
		if (!img.empty())
		{
			if (!isWindowPos)
			{
				cvNamedWindow("screenshot", 0);
				cvResizeWindow("screenshot", 400, 300);
				isWindowPos = true;
			}

			imshow("screenshot", Mat(img));

			static int cnt = 0;
			char fileName[128];
			sprintf_s(fileName, "capture %d.jpg", cnt++);
			imwrite(fileName, img);
		}
		else
		{
			cout << "empty image capture" << endl;
		}

		Sleep(10);
	}
}


void FocusWindow()
{
	HWND hwnd = NULL;

	while (!hwnd)
	{
		Sleep(1000);
		hwnd = FindWindow(L"NeonClass_41", L"DiRT 3");
		if (hwnd)
		{
			cout << "focus window..." << hwnd << endl;
			g_dirt3Hwnd = hwnd;
		}
	}

	int cnt = 0;
	while (cnt < 100)
	{
		if (SetForegroundWindow(hwnd))
			break;
		Sleep(100);
		++cnt;
	}
}


void PressScreenShot()
{
	INPUT input;
	WORD vkey = VK_SNAPSHOT; // see link below
	input.type = INPUT_KEYBOARD;
	input.ki.wScan = MapVirtualKey(vkey, MAPVK_VK_TO_VSC);
	input.ki.time = 0;
	input.ki.dwExtraInfo = 0;
	input.ki.wVk = vkey;
	input.ki.dwFlags = KEYEVENTF_SCANCODE;
	SendInput(1, &input, sizeof(INPUT));

	Sleep(300);

	input.ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE;
	SendInput(1, &input, sizeof(INPUT));
}


Mat GetClipboardImage()
{
	// 클립보드 내용 가져와서 파일로 저장
	::OpenClipboard(NULL);

	std::cout << "Format Bitmap: " << IsClipboardFormatAvailable(CF_BITMAP) << "\n";
	std::cout << "Format DIB: " << IsClipboardFormatAvailable(CF_DIB) << "\n";
	std::cout << "Format DIBv5: " << IsClipboardFormatAvailable(CF_DIBV5) << "\n";

	if (!IsClipboardFormatAvailable(CF_BITMAP) &&
		!IsClipboardFormatAvailable(CF_DIB) &&
		!IsClipboardFormatAvailable(CF_DIBV5))
		return Mat();

	HANDLE hClipboard = GetClipboardData(CF_DIB);
	if (!hClipboard)
		hClipboard = GetClipboardData(CF_DIBV5);
	if (!hClipboard)
		hClipboard = GetClipboardData(CF_BITMAP);

	Mat reval;
	if (hClipboard != NULL && hClipboard != INVALID_HANDLE_VALUE)
	{
		void* dib = GlobalLock(hClipboard);
		if (!dib)
			return Mat();

		DIB *info = reinterpret_cast<DIB*>(dib);
		BMP bmp = { 0 };
		bmp.header.type = 0x4D42;
		bmp.header.offset = 54;
		bmp.header.bfSize = info->biSizeImage + bmp.header.offset;
		bmp.dib = *info;

// 		std::cout << "Type: " << std::hex << bmp.header.type << std::dec << "\n";
// 		std::cout << "bfSize: " << bmp.header.bfSize << "\n";
// 		std::cout << "Reserved: " << bmp.header.reserved << "\n";
// 		std::cout << "Offset: " << bmp.header.offset << "\n";
// 		std::cout << "biSize: " << bmp.dib.biSize << "\n";
// 		std::cout << "Width: " << bmp.dib.biWidth << "\n";
// 		std::cout << "Height: " << bmp.dib.biHeight << "\n";
// 		std::cout << "Planes: " << bmp.dib.biPlanes << "\n";
// 		std::cout << "Bits: " << bmp.dib.biBitCount << "\n";
// 		std::cout << "Compression: " << bmp.dib.biCompression << "\n";
// 		std::cout << "Size: " << bmp.dib.biSizeImage << "\n";
// 		std::cout << "X-res: " << bmp.dib.biXPelsPerMeter << "\n";
// 		std::cout << "Y-res: " << bmp.dib.biYPelsPerMeter << "\n";
// 		std::cout << "ClrUsed: " << bmp.dib.biClrUsed << "\n";
// 		std::cout << "ClrImportant: " << bmp.dib.biClrImportant << "\n";

		reval = Mat(bmp.dib.biHeight, bmp.dib.biWidth, CV_8UC4);
		CopyMemory(reval.data, (info + 1), bmp.dib.biSizeImage);
		flip(reval, reval, 0);

		GlobalUnlock(dib);
	}

	CloseClipboard();
	return reval;
}
