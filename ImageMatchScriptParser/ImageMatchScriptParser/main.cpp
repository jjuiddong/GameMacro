//
// 2016-05-10, 
//
// ImageMatch Script Parser
//		command line
//		ImageMatchScriptParser.exe <script filename> {<detect directory path> <execute label>}
//
//
//
// gray = 0
// label_finland = finland/finland_label0.png
// src_result_roi1 = 10,20,30,30
//
// detect_track:
//		label_finish
//			label_finland1, src_result_roi1
//				label_finland2
//					finland_id0
//			label_kenya1
//				label_kenya2
//					kenya_id0
//
// detect_rank:
//		label_id
//			label_num1
//				0
//
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <windows.h>
#include <mmsystem.h>

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

struct sParseTree
{
	char str[64];
	int roi[4]; // x,y,w,h
	int depth; // only use parsing
	bool isRelation; // parent relation coordinate roi
	float threshold; // match max threshold
	cv::Point matchLoc; // match location, only use execution
	sParseTree *next;
	sParseTree *child;
};

struct sMatchInfo
{
	string label;
	double max;
	int depth;
	float threshold;
	int elapseTime;
};

ifstream g_ifs;
map<string, string> g_symbolTable;
map<string, sParseTree*> g_headTable;
map<string, Mat> g_imgTable;
sParseTree *g_root = NULL;
char *g_lineStr;
char g_scanLine[128];
char g_tmpBuffer[128];
bool g_isReverseScan = false; // 한라인을 다시 읽는다.
bool g_isError = false;
int g_lineNum = 0;
bool g_isGray = true;
bool g_isDebug = false;

bool IsBlank(const char *str, const int size)
{
	for (int i = 0; i < size; ++i)
	{
		if (!*str)
			break;
		if (('\n' == *str) || ('\r' == *str) || ('\t' == *str) || (' ' == *str))
			++str;
		else
			return false;
	}
	return true;
}

bool Open(const char *fileName)
{
	g_ifs.open(fileName);
	return g_ifs.is_open();
}

void Close()
{
	g_ifs.close();
}

bool ScanLine()
{
	if (g_isReverseScan)
	{
		--g_lineNum;
		g_lineStr = g_scanLine;
		g_isReverseScan = false;
		return true;
	}

	do
	{
		++g_lineNum;
		if (!g_ifs.getline(g_scanLine, sizeof(g_scanLine)))
			return false;
	} 
	while (IsBlank(g_scanLine, sizeof(g_scanLine)));

	g_lineStr = g_scanLine;
	return true;
}

void UnScanLine()
{
	g_isReverseScan = true;
}

// pass through blank character
char* passBlank(const char *str)
{
	while (1)
	{
		if (!*str)
			return NULL;
		if ((*str == ' ') || (*str == '\t'))
			++str;
		else
			break;
	}
	return (char*)str;
}

const char *g_numStr = "1234567890.";
const int g_numLen = 11;
const char *g_strStr = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@$_/.";
const int g_strLen = 57;
const char *g_strStr2 = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@$_/.1234567890";
const int g_strLen2 = 67;
const char *g_strOp = "+-*/=:()";
const int g_opLen = 7;

// id -> alphabet + {alphabet | number}
const char* id()
{
	// comma check
	g_lineStr = passBlank(g_lineStr);
	if (!g_lineStr)
		return "";

	// find first char
	const char *n = strchr(g_strStr, *g_lineStr);
	if (!n)
		return "";

	char *dst = g_tmpBuffer;
	*dst++ = *g_lineStr++;

	while(1)
	{
		const char *n = strchr(g_strStr2, *g_lineStr);
		if (!n || !*n)
			break;
		*dst++ = *g_lineStr++;
	}

	*dst = NULL;
	return g_tmpBuffer;
}

const char* number()
{
	// comma check
	g_lineStr = passBlank(g_lineStr);
	if (!g_lineStr)
		return "";

	char *dst = g_tmpBuffer;
	if ('+' == *g_lineStr)
		*dst++ = *g_lineStr++;
	else if ('-' == *g_lineStr)
		*dst++ = *g_lineStr++;

	while (1)
	{
		const char *n = strchr(g_numStr, *g_lineStr);
		if (!n || !*n)
			break;
		*dst++ = *g_lineStr++;
	}
	
	*dst = NULL;
	return g_tmpBuffer;
}

char op()
{
	g_lineStr = passBlank(g_lineStr);
	if (!g_lineStr)
		return NULL;
	const char *n = strchr(g_strOp, *g_lineStr);
	if (!n)
		return NULL;
	return *g_lineStr++;
}

int assigned(const char *var)
{
	g_lineStr = passBlank(g_lineStr);
	if (!g_lineStr)
	{
		cout << "line {" << g_lineNum << "} error!! assigned " << endl;
		return 0;
	}

	g_symbolTable[var] = g_lineStr;
	return 1;
}

void buildAttributes(const string &str, vector<string> &attributes)
{
	// tok1, tok2, tok3, tok4, tok5 ...  분리 
	// node->str = tok1
	// tok2~5 : roi x,y,w,h
	//char *str = node->str;
	g_lineStr = (char*)str.c_str();
	while (1)
	{
		const char *pid = id();
		const char *pnum = (!*pid) ? number() : NULL;

		if (*pid)
		{
			auto it = g_symbolTable.find(pid);
			if (g_symbolTable.end() == it)
			{
				attributes.push_back(pid);
			}
			else
			{
				char *old = g_lineStr;
				buildAttributes(it->second.c_str(), attributes);
				g_lineStr = old;
			}
		}
		else if (*pnum)
		{
			attributes.push_back(pnum);
		}
		else
		{
			cout << "line {" << g_lineNum << "} error!! tree attribute fail [" << str << "]" << endl;
			g_isError = true;
			break;
		}

		// comma check
		g_lineStr = passBlank(g_lineStr);
		if (!g_lineStr)
			break;
		if (*g_lineStr == ',')
			++g_lineStr;
	}
}


void collectTree(sParseTree *current, set<sParseTree*> &out)
{
	if (!current)
		return;
	out.insert(current);
	collectTree(current->child, out);
	collectTree(current->next, out);
}

void removeTree(sParseTree *current)
{
	set<sParseTree*> rmNodes;// 중복된 노드를 제거하지 않기위해 사용됨
	collectTree(current, rmNodes);
	for each (auto &node in rmNodes)
		delete node;
}

void setTreeAttribute(sParseTree *node, vector<string> &attribs)
{
	if (attribs.empty())
		return;

	// check threshold
	for (int i = 0; i < (int)attribs.size(); ++i)
	{
		const int pos = attribs[i].find("threshold_");
		if (string::npos != pos)
		{
			// threshold 값 설정
			const int valPos = attribs[i].find("_");
			const float threshold = (float)atof(attribs[i].substr(valPos + 1).c_str());
			node->threshold = threshold;

			// remove threshold attribute
			std::rotate(attribs.begin()+i, attribs.begin()+i+1, attribs.end());
			attribs.pop_back();
			break;
		}
	}

	strcpy_s(node->str, attribs[0].c_str());
	if (attribs.size() >= 5)
	{
		for (int i = 1; i < 5; ++i)
			node->roi[i - 1] = atoi(attribs[i].c_str());
	}
	if (attribs.size() >= 6)
	{
		if (attribs[5] == "+")
			node->isRelation = true;
	}
}

sParseTree* tree(sParseTree *current, const int depth)
{
	if (!ScanLine())
		return NULL;
	
	int cntDepth = 0;
	while (1)
	{
		if (!*g_lineStr)
			break; // error

		if (*g_lineStr == '\t')
		{
			++cntDepth;
			++g_lineStr;
		}
		else
		{
			break; // end
		}
	}

	if (cntDepth <= 0)
		return NULL; // error


	//------------------------------------------------------
	// tree node attribute
	// image name, roi x,y,w,h
	vector<string> attribs;
	buildAttributes(g_lineStr, attribs);

	// check head link
	if (!attribs.empty())
	{
		auto it = g_headTable.find(attribs[0]);
		if (g_headTable.end() != it)
		{
			// link another head tree
			current->child = it->second->child;
			return tree(current, depth);
		}
	}

	sParseTree *node = NULL;
	if (!node)
	{
		node = new sParseTree;
		memset(node, 0, sizeof(sParseTree));
		node->depth = cntDepth;

		setTreeAttribute(node, attribs);
	}
	//------------------------------------------------------

	sParseTree *reval = NULL;
	if (cntDepth == depth)
	{
		current->next = node;
		reval = tree(node, cntDepth);
	}
	else if (cntDepth == (depth + 1))
	{
		current->child = node;
		reval = tree(node, cntDepth);
	}
	else if (cntDepth > depth + 1)
	{
		// error
		cout << "line {" << g_lineNum << "} tree depth error!! " << node->str << endl;
		return NULL;
	}
	else
	{
		return node;
	}

	// search parent tree node
	while (reval)
	{
		if (depth == reval->depth)
		{
			current->next = reval;
			reval = tree(reval, depth);
		}
		else
		{
			return reval;
		}
	}

	return NULL;
}

void printSymbol()
{
	cout << "printSymbol--------------------" << endl;
	for each (auto &kv in g_symbolTable)
	{
		cout << "{ " << kv.first << ", " << kv.second << " }" << endl;
	}
}

void printTree(sParseTree *p, const int depth)
{
	if (!p)
		return;

	for (int i = 0; i < depth; ++i)
		cout << "\t";
	cout << p->str;
	if (p->roi[0] || p->roi[1] || p->roi[2] || p->roi[3])
		cout << ", " << p->roi[0] << ", " << p->roi[1] << ", " << p->roi[2] << ", " << p->roi[3];
	if (p->isRelation)
		cout << ", +";
	cout << endl;

	printTree(p->child, depth + 1);
	printTree(p->next, depth);
}

Mat& loadImage(const string &fileName)
{
	auto it = g_imgTable.find(fileName);
	if (g_imgTable.end() != it)
		return it->second;

	Mat img = imread(fileName);
	if (g_isGray)
		cvtColor(img, img, CV_BGR2GRAY);
	g_imgTable[fileName] = img;
	return g_imgTable[fileName];
}

string executeTree(Mat &A, sParseTree *parent, sParseTree *node, const int depth, vector<sMatchInfo> &out)
{
	if (!node)
		return "~ fail ~";
	if (!node->child)
		return node->str; // terminal node, success finish

	Mat B = loadImage(node->str);
	if (B.empty())
		return string("not found image [") + node->str + "]";

	Mat C(cvSize(A.cols - B.cols + 1, A.rows - B.rows + 1), IPL_DEPTH_32F);

	// roi x,y,w,h
	Rect roi(0, 0, A.cols, A.rows);
	if ((node->roi[0] != 0) || (node->roi[1] != 0) || (node->roi[2] != 0) || (node->roi[3] != 0))
		roi = { node->roi[0], node->roi[1], node->roi[2], node->roi[3] };
	if (node->isRelation && parent)
	{
		roi.x += parent->matchLoc.x;
		roi.y += parent->matchLoc.y;
	}

	// roi limit check
	roi.x = max(0, roi.x);
	roi.y = max(0, roi.y);
	if (A.cols < roi.x + roi.width)
		roi.width = A.cols - roi.x;
	if (A.rows < roi.y + roi.height)
		roi.height = A.rows - roi.y;

	double min, max;
	Point left_top;
	cv::matchTemplate(A(roi), B, C, CV_TM_CCOEFF_NORMED);
	cv::minMaxLoc(C, &min, &max, NULL, &left_top);

	// save match information
	if ((int)out.size() < depth + 1)
		out.push_back({});

	const float maxThreshold = (node->threshold == 0) ? 0.9f : node->threshold;
	if (out[depth].max < max)
	{
		out[depth].max = max;
		out[depth].depth = depth;
		out[depth].threshold = maxThreshold;
		out[depth].label = node->str;
	}
	//

	if (max > maxThreshold)
	{
		if (g_isDebug)
		{
			Mat img = A.clone();
			cvtColor(img, img, CV_GRAY2BGR);
			putText(img, node->str, Point(20, 20), 1, 1.3f, Scalar(0, 0, 255), 2);
			Point pos = left_top + Point(roi.x, roi.y);
			cv::rectangle(img, pos, cvPoint(pos.x + B.cols, pos.y + B.rows), CV_RGB(255, 0, 0));
			imshow("result", img);
			waitKey(0);
		}

		node->matchLoc = left_top + Point(roi.x, roi.y);
		return executeTree(A, node, node->child, depth+1, out);
	}

	return executeTree(A, parent, node->next, depth, out);
}

string execute(const string &label, const string &srcName, Mat &A, vector<sMatchInfo> &out)
{
	sParseTree *node = g_root;
	while (node)
	{
		if (label == node->str)
		{
			// 실행시간 저장
			const int oldT = timeGetTime();
			const string result = executeTree(A, NULL, node->child, 0, out);
			if (!out.empty())
				out[0].elapseTime = timeGetTime() - oldT;

			if (g_isDebug && (result == "~ fail ~"))
			{
				Mat img = A.clone();
				cvtColor(img, img, CV_GRAY2BGR);
				putText(img, "~ Fail ~", Point(20, 20), 1, 1.3f, Scalar(0, 0, 255), 2);
				putText(img, srcName.c_str(), Point(150, 20), 1, 1.3f, Scalar(0, 0, 255), 2);
				imshow("result", img);
				waitKey(0);
			}

			return result;
		}
		node = node->next;
	}
	return "not found label";
}

void main(int argc, char *argv[])
{
	// ImageMatchScriptParser.exe <script filename> {<detect directory path> <execute label>}
	if (argc < 4)
	{
		cout << "CommandLine >> " << endl;
		cout << "\t<script filename> {<detect directory path> <execute label>}" << endl << endl;
		return;
	}

	const string fileName = argv[1];
	vector < pair<string, string> > execParams; // directory path, execute label
	for (int i = 2; i < argc-1; i += 2)
	{
		execParams.push_back(pair<string, string>(argv[i], argv[i + 1]));
	}

	if (!Open(fileName.c_str()))
	{
		cout << "not found file" << endl;
		return;
	}

	sParseTree *currentTree = NULL;
	while (ScanLine())
	{
		const char *str = id();
		if (!*str)
			continue;

		const char c = op();

		if (c == ':') // tree head
		{
			sParseTree *head  = new sParseTree;
			memset(head, 0, sizeof(sParseTree));
			strcpy_s(head->str, str);

			if (!g_root)
			{
				g_root = head;
				currentTree = head;
			}
			else
			{
				currentTree->next = head;
				currentTree = head;
			}

			tree(head, 0); // build tree
			g_headTable[head->str] = head;
			UnScanLine();
		}
		else if (c == '=')
		{
			if (!assigned(str))
				break; 
		}
		else
		{
			cout << "line {" << g_lineNum << "} error!! operator >> ";
			if (g_lineStr)
				cout << g_lineStr;
			cout << " [" << c << "]" << endl;
			g_isError = true;
			break;
		}
	}

	if (g_isError)
		return;

	printSymbol();
	cout << "\nprint tree------------" << endl;
	printTree(g_root, 0);
	Close();

	// Execute--------------------------------------------------------------
	g_isDebug = atoi(g_symbolTable["debug"].c_str()) ? true : false;

	cout << endl << endl;
	cout << "---Execute--------------------------------------------------------------" << endl;
	string srcFileNames[] = 
	{
// 		"img/img1_1.png",
// 		"img/img1_2.png",
// 		"img/img1_3.png",
// 		"img/img1_4.png",
// 		"img/img1_5.png",
// 		"img/img1_6.png",
// 		"img/img1_7.png",

		// finland
		"sourceimage/finland_alhojarvi.png",
		"sourceimage/finland_hanisjarventie.png",
		"sourceimage/finland_hanisjarventie2.png",
		"sourceimage/finland_isojarvi.png",
		"sourceimage/finland_kaatselk.png",
		"sourceimage/finland_kakaristo.png",
		"sourceimage/finland_lankamaa.png",
		"sourceimage/finland_lankamaa2.png",
		"sourceimage/finland_mutanen.png",
		"sourceimage/finland_mutanen2.png",
		"sourceimage/finland_tupasentie_pro.png",
		"sourceimage/finland_tupasentie_pro2.png",
		"sourceimage/finland_tupasentie_pro3.png",
		"sourceimage/finland_tupasentie_pro4.png",
		"sourceimage/finland_tupasentie_pro5.png",
		"sourceimage/finland_tupasentie_pro6.png",

		// kenya
		"sourceimage/kenya_mwatate_pro.png",
		"sourceimage/kenya_chumvinirise_1.png",
		"sourceimage/kenya_chumvinirise_2.png",
		"sourceimage/kenya_chumvinirise_3.png",
		"sourceimage/kenya_crocodilepoint_1.png",
		"sourceimage/kenya_crocodilepoint_2.png",
		"sourceimage/kenya_mwanda_1.png",
		"sourceimage/kenya_mwanda_2.png",
		"sourceimage/kenya_mwanda_3.png",
		"sourceimage/kenya_taitahills_1.png",
		"sourceimage/kenya_taitahills_2.png",
		"sourceimage/kenya_taitahills_3.png",
		"sourceimage/kenya_taitahills_4.png",
		"sourceimage/kenya_taitahills_5.png",
		"sourceimage/kenya_taitahills_6.png",
		"sourceimage/kenya_taventadescent_1.png",
		"sourceimage/kenya_taventadescent_2.png",
		"sourceimage/kenya_tsavo_1.png",
		"sourceimage/kenya_tsavo_2.png",
		"sourceimage/kenya_tsavo_3.png",
		"sourceimage/kenya_wundanyiclimb_1.png",
		"sourceimage/kenya_wundanyiclimb_2.png",
	};
	const int srcSize = sizeof(srcFileNames) / sizeof(string);

	ofstream ofs("log.txt");

	for each (auto &item in execParams)
	{
		for (int i = 0; i < srcSize; ++i)
		{
			vector<sMatchInfo> infos;

			Mat src = loadImage(srcFileNames[i]);
			const string result = execute(item.second, srcFileNames[i], src, infos);
			cout << "source = " << srcFileNames[i].c_str() << ",   ";
			cout << "execute(" << item.second.c_str() << ") = " << result.c_str() << endl;

			for (int i = 0; i < (int)infos.size(); ++i)
			{
				if (i == 0)
				{
					for (int k = 0; k < infos[i].depth; ++k) cout << "    ";
					cout << "    elpase time = " << infos[i].elapseTime << endl;
				}
				for (int k = 0; k < infos[i].depth; ++k) cout << "    ";
				cout << "    depth = " << infos[i].depth << endl;
				for (int k = 0; k < infos[i].depth; ++k) cout << "    ";
				cout << "    max = " << infos[i].max << endl;
				for (int k = 0; k < infos[i].depth; ++k) cout << "    ";
				cout << "    label = " << infos[i].label.c_str() << endl;
			}

			ofs << "source = " << srcFileNames[i].c_str() << ",    ";
			ofs << "execute(" << item.second.c_str() << ") = " << result.c_str() << endl;
			for (int i = 0; i < (int)infos.size(); ++i)
			{
				if (i == 0)
				{
					for (int k = 0; k < infos[i].depth; ++k) ofs << "    ";
					ofs << "    elpase time = " << infos[i].elapseTime << endl;
				}
				for (int k = 0; k < infos[i].depth; ++k) ofs << "    ";
				ofs << "    depth = " << infos[i].depth << endl;
				for (int k = 0; k < infos[i].depth; ++k) ofs << "    ";
				ofs << "    max = " << infos[i].max << endl;
				for (int k = 0; k < infos[i].depth; ++k) ofs << "    ";
				ofs << "    label = " << infos[i].label.c_str() << endl;
			}
		}
	}


/*
	for (int i = 0; i < srcSize; ++i)
	{
		Mat src = loadImage(srcFileNames[i]);	
		cout << "execute(detect_shape) = " << execute("detect_shape", src) << endl;
	}

	for (int i = 0; i < srcSize; ++i)
	{
		Mat src = loadImage(srcFileNames[i]);
		cout << "execute(detect_shape_roi) = " << execute("detect_shape_roi", src) << endl;
	}

	for (int i = 0; i < srcSize; ++i)
	{
		Mat src = loadImage(srcFileNames[i]);
		cout << "execute(detect_number) = " << execute("detect_number", src) << endl;
	}

	for (int i = 0; i < srcSize; ++i)
	{
		Mat src = loadImage(srcFileNames[i]);
		cout << "execute(detect_number2) = " << execute("detect_number2", src) << endl;
	}

	for (int i = 0; i < srcSize; ++i)
	{
		Mat src = loadImage(srcFileNames[i]);
		cout << "execute(detect_number_relation_roi) = " << execute("detect_number_relation_roi", src) << endl;
	}
*/

	removeTree(g_root);
}
