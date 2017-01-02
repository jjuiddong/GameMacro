
#include "stdafx.h"
#include "simplematchscript.h"
#include "rectcontour.h"


using namespace cv;

namespace
{
	static const char *g_numStr = "1234567890.";
	static const int g_numLen = 11;
	static const char *g_strStr = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@$_/.";
	static const int g_strLen = 57;
	static const char *g_strStr2 = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@$_/.1234567890";
	static const int g_strLen2 = 67;
	static const char *g_strOp = "+-*/=:()";
	static const int g_opLen = 7;
}


cSimpleMatchScript::cSimpleMatchScript()
	: m_state(WAIT)
	, m_curIdx(0)
{
}

cSimpleMatchScript::~cSimpleMatchScript()
{
}


// parse id : pid
// id -> alphabet + {alphabet | number}
string cSimpleMatchScript::pid(const string &str)
{
	if (str.empty())
		return "";

	string out;
	out.reserve(64);
	int i = 0;

	const char *n = strchr(g_strStr, str[i]);
	if (!n)
		return out;

	out += str[i++];

	while (1)
	{
		const char *n = strchr(g_strStr2, str[i]);
		if (!n || !*n)
			break;
		out += str[i++];
	}

	return out;
}


// aaa, bb, cc, "dd ee"  ff -> aaa, bb, cc, dd ee
int cSimpleMatchScript::attrs(const string &str, OUT string &out)
{
	int i = 0;

	out.reserve(64);

	bool isLoop = true;
	bool isComma = false;
	bool isString = false;
	bool isFirst = true;
	while (isLoop && str[i])
	{
		switch (str[i])
		{
		case '"':
			isFirst = false;
			isString = !isString;
			break;
		case ',': // comma
			if (isString)
			{
				out += ',';
			}
			else
			{
				isComma = true;
				out += ',';
			}
			break;

		case '\r':
		case '\n':
		case ' ': // space
			if (isFirst)
			{
				// nothing~
			}
			else if (isString)
			{
				out += ',';
			}
			else
			{
				if (!isComma)
					isLoop = false;
			}
			break;
		default:
			isFirst = false;
			isComma = false;
			out += str[i];
			break;
		}
		++i;
	}

	return i;
}


// attr - list ->  { id [ = value ] }
void cSimpleMatchScript::attr_list(const string &str)
{
	string parseStr = str;
	while (1)
	{
		trim(parseStr);

		string id = pid(parseStr);
		trim(id);
		if (id.empty())
			break;
		parseStr = parseStr.substr(id.size());
		trim(parseStr);

		const uint pos1 = parseStr.find("=");
		const uint pos2 = MIN(parseStr.find(" "), parseStr.find("\n"));

		if (pos1 < pos2) // id = data
		{
			parseStr = parseStr.substr(pos1 + 1);
			trim(parseStr);

			// aaa, bb, cc  dd -> aaa, bb, cc 
			string data;
			const int offset = attrs(parseStr, data);
			parseStr = parseStr.substr(offset);

			m_commands.push_back(std::pair<string, string>(id, data));
		}
		else // id or eof
		{
			m_commands.push_back(std::pair<string, string>(id, ""));

			if (parseStr.empty() && (pos2 == string::npos))
				break;
		}	
	}
}


// parse grammar
// var = parameter1, parameter2, parameter3
void cSimpleMatchScript::Parse(const string &script)
{
	m_commands.clear();
	attr_list(script);
}


string cSimpleMatchScript::Match(INOUT cv::Mat &src, OUT cv::Mat &dst, const string &script
	,const bool showMsg)
	//showMsg=true
{
	m_src = src.clone(); // 결과 정보를 출력할 때, 쓰임
	dst = src.clone();
	m_tessImg = Mat();

	vector<Mat> accMat;
	accMat.reserve(8);

	Parse(script);

	const int t1 = timeGetTime();

	string errMsg;
	char value[128];
	for each (auto &item in m_commands)
	{
		const string cmd = item.first;
		const string param = item.second;

		//----------------------------------------------------------------------
		// roi=x,y,width,height
		if (cmd == "roi")
		{
			cv::Rect roi = { 0,0,0,0 };
			sscanf(param.c_str(), "%d,%d,%d,%d", &roi.x, &roi.y, &roi.width, &roi.height);
			if (roi.area() > 0)
			{
				if (dst.data)
					dst = dst(roi);
				continue;
			}
		}

		//----------------------------------------------------------------------
		// rect=x,y,width,height
		if (cmd == "rect")
		{
			cv::Rect rect = { 0,0,0,0 };
			sscanf(param.c_str(), "%d,%d,%d,%d", &rect.x, &rect.y, &rect.width, &rect.height);
			if (rect.area() > 0)
			{
				cRectContour r(rect);
				r.Draw(dst, Scalar(0, 0, 255), 2);
				continue;
			}
		}

		//----------------------------------------------------------------------
		//     - bgr=num1,num2,num3
		//         - Mat &= Scalar(num1,num2,num3)
		if (cmd == "bgr")
		{
			int bgr[3] = { 0,0,0 };
			sscanf(param.c_str(), "%d,%d,%d", bgr, bgr + 1, bgr + 2);
			if ((bgr[0] != 0) || (bgr[1] != 0) || (bgr[2] != 0))
			{
				if (dst.data)
					dst &= Scalar(bgr[0], bgr[1], bgr[2]);
				continue;
			}
		}


		//----------------------------------------------------------------------
		//     - scale=num
		//         - Mat *= num
		if (cmd == "scale")
		{
			float scale = 0;
			sscanf(param.c_str(), "%f", &scale);
			if (scale != 0)
			{
				if (dst.data)
					dst *= scale;
				continue;
			}
		}

		//----------------------------------------------------------------------
		//	- gray convert
		if (cmd == "gray")
		{
			if (dst.data && (dst.channels() >= 3))
				cvtColor(dst, dst, CV_BGR2GRAY);
			continue;
		}

		//----------------------------------------------------------------------
		if (cmd == "threshold")
		{
			int  thresh1 = 0;
			sscanf(param.c_str(), "%d", &thresh1);
			if (thresh1 > 0)
			{
				if (dst.data && (dst.channels() >= 3))
					cvtColor(dst, dst, CV_BGR2GRAY);
				threshold(dst, dst, thresh1, 255, CV_THRESH_BINARY);
				continue;
			}
		}

		//----------------------------------------------------------------------
		if (cmd == "adapthreshold")
		{
			double thresh_c = 0;
			int block_size = 0;
			sscanf(param.c_str(), "%d,%lf", &block_size, &thresh_c);
			if (thresh_c > 0)
			{
				if (dst.data && (dst.channels() >= 3))
					cvtColor(dst, dst, CV_BGR2GRAY);
				adaptiveThreshold(dst, dst, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, block_size, thresh_c);
				continue;
			}
		}

		//----------------------------------------------------------------------
		if (cmd == "invert")
		{
			int  thresh = 0;
			sscanf(param.c_str(), "%d", &thresh);
			if (thresh > 0)
			{
				if (dst.data && (dst.channels() >= 3))
					cvtColor(dst, dst, CV_BGR2GRAY);
				threshold(dst, dst, thresh, 255, CV_THRESH_BINARY_INV);
				continue;
			}
		}

		
		//----------------------------------------------------------------------
		// cvt = hsv/hls/bgr
		if (cmd == "cvt")
		{
			if (param == "bgr-hsv")
			{
				cvtColor(dst, dst, CV_BGR2HSV);
			}
			else if (param == "bgr-hls")
			{
				cvtColor(dst, dst, CV_BGR2HLS);
			}
			else if (param == "bgr-gray")
			{
				cvtColor(dst, dst, CV_BGR2GRAY);
			}
			else if (param == "hsv-bgr")
			{
				cvtColor(dst, dst, CV_HSV2BGR);
			}
			else if (param == "hls-bgr")
			{
				cvtColor(dst, dst, CV_HLS2BGR);
			}
			else if (param == "gray-bgr")
			{
				cvtColor(dst, dst, CV_GRAY2BGR);
			}
		}


		//----------------------------------------------------------------------
		//     - hsv=num1,num2,num3,num4,num5,num6
		//         - hsv converting, inRange( Scalar(num1,num2,num3), Scalar(num4,num5,num6) )
		if (cmd == "hsv")
		{
			int hsv[6] = { 0,0,0, 0,0,0 }; // inrage
			sscanf(param.c_str(), "%d,%d,%d,%d,%d,%d", hsv, hsv + 1, hsv + 2, hsv + 3, hsv + 4, hsv + 5);
			if ((hsv[0] != 0) || (hsv[1] != 0) || (hsv[2] != 0) || (hsv[3] != 0) || (hsv[4] != 0) || (hsv[5] != 0))
			{
				if (dst.data)
				{
					cvtColor(dst, dst, CV_BGR2HSV);
					inRange(dst, cv::Scalar(hsv[0], hsv[1], hsv[2]), cv::Scalar(hsv[3], hsv[4], hsv[5]), dst);
					cvtColor(dst, dst, CV_GRAY2BGR);
					accMat.push_back(dst.clone());
				}
				continue;
			}
		}

		//----------------------------------------------------------------------
		//     - hls=num1,num2,num3,num4,num5,num6
		//         - hsv converting, inRange( Scalar(num1,num2,num3), Scalar(num4,num5,num6) )
		if (cmd == "hls")
		{
			int hsl[6] = { 0,0,0, 0,0,0 }; // inrage
			sscanf(param.c_str(), "%d,%d,%d,%d,%d,%d", hsl, hsl + 1, hsl + 2, hsl + 3, hsl + 4, hsl + 5);
			if ((hsl[0] != 0) || (hsl[1] != 0) || (hsl[2] != 0) || (hsl[3] != 0) || (hsl[4] != 0) || (hsl[5] != 0))
			{
				if (dst.data)
				{
					cvtColor(dst, dst, CV_BGR2HLS);
					inRange(dst, cv::Scalar(hsl[0], hsl[1], hsl[2]), cv::Scalar(hsl[3], hsl[4], hsl[5]), dst);
					cvtColor(dst, dst, CV_GRAY2BGR);
					accMat.push_back(dst.clone());
				}
				continue;
			}
		}

		if (cmd == "acc")
		{
			if (!accMat.empty())
			{
				Mat tmp(accMat.front().rows, accMat.front().cols, accMat.front().flags);
				for each (auto &m in accMat)
					tmp += m;
				dst = tmp;
			}
			continue;
		}


		//----------------------------------------------------------------------
		// Re Load
		if (cmd == "reload")
		{
			dst = src.clone();
		}


		//----------------------------------------------------------------------
		// hough
		if (cmd == "hough")
		{
			ZeroMemory(value, sizeof(value));
			sscanf(param.c_str(), "%s", value);
			if (value[0] != NULL)
			{
				std::vector<cv::Vec2f> lines;
				cv::HoughLines(dst, lines, 1, 0.1f, 80);

				// ~~~~ programming
			}
		}


		//----------------------------------------------------------------------
		// canny=threshold
		if (cmd == "canny")
		{
			ZeroMemory(value, sizeof(value));
			sscanf(param.c_str(), "%s", value);
			if (value[0] != NULL)
			{
				const int threshold1 = atoi(value);
				cv::Canny(dst, dst, 0, threshold1, 5);
			}
		}


		//----------------------------------------------------------------------
		// findcontours=arcAlpha
		if (cmd == "findcontours")
		{
			float arcAlpha = 0;
			sscanf(param.c_str(), "%f", &arcAlpha);
			if (arcAlpha != 0)
			{
				cRectContour rect;
				vector<vector<cv::Point>> contours;
				cv::findContours(dst, contours, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

				// ~~~~ programming
			}
		}


		//----------------------------------------------------------------------
		//     - dilate
		if (cmd == "dilate")
		{
			dilate(dst, dst, Mat());
			continue;
		}

		//----------------------------------------------------------------------
		//     - erode
		if (cmd == "erode")
		{
			erode(dst, dst, Mat());
			continue;
		}

	}

	const int t2 = timeGetTime();
	if (dst.data && showMsg)
		putText(dst, ::format("time=%d", t2-t1).c_str(), Point(0, 60), 1, 2.f, Scalar(255, 255, 255), 2);

	return errMsg;
}

