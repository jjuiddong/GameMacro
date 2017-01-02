//
// 2016-06-10, jjuiddong
// scene 이미지와 결과 라벨을 비교해서, 인식 알고리즘이 잘 작동하는지
// 확인 하는 프로그램.
//
// config 파일 내용
// match_script_name = filename.txt
// execute_label = labelname
// test_filename = test.txt
//
// test 스크립트 형식
// filename label
//

#include "../../../Common/Common/common.h"
#include "../../../Common/CamCommon/camcommon.h"
#include "../../../Common/Network/network.h"

#pragma comment (lib, "winmm.lib")

using namespace std;
using namespace  cv;
using namespace cvproc;
using namespace cvproc::imagematch;

void main(int argc, char *argv[])
{
	if (argc != 2)
	{
		cout << "commandline <config filename>" << endl;
		getchar();
		return;
	}

	dbg::RemoveErrLog();
	dbg::RemoveLog();
	dbg::RemoveLog2("log2.txt");

	const string fileName = argv[1];
	
	cout << "Read Config File << " << fileName << endl << endl;
	cConfig config;
	if (!config.Read(fileName))
	{
		cout << "Error!! read config file = " << fileName << endl;
		getchar();
		return;
	}

	const string matchScriptName = config.GetString("match_script_name");
	const string executeLabelName  = config.GetString("execute_label");
	const string testFileName = config.GetString("test_filename");
	if (matchScriptName.empty() || executeLabelName.empty() || testFileName.empty())
	{
		cout << "Error!! script option " << endl;
		cout << "match_script_name = " << matchScriptName << endl;
		cout << "execute_label = " << executeLabelName << endl;
		cout << "test_filename = " << testFileName << endl;
		return;
	}

	cout << "match_script_name = " << matchScriptName << endl;
	cout << "execute_label = " << executeLabelName << endl;
	cout << "test_filename = " << testFileName << endl;
	cout << endl;

	cMatchScript2 matchScript;
	if (matchScript.Read(matchScriptName))
	{
		cout << "Success Read matchscript << " << matchScriptName << endl;
	}
	else
	{
		cout << "Error!! Read matchscript << " << matchScriptName << endl;
		getchar();
		return;
	}


	ifstream ifs(testFileName.c_str());
	if (!ifs.is_open())
	{
		cout << "Error!! read test script = " << testFileName << endl;
		getchar();
		return;
	}

	vector< pair<string, string> > testSet;
	testSet.reserve(512);
	while (!ifs.eof())
	{
		string line;
		getline(ifs, line);

		stringstream ss(line);
		string filePath, label;
		ss >> filePath >> label;

		if (filePath.empty() || label.empty())
			break;

		testSet.push_back(pair<string, string>(filePath, label));
	}
	ifs.close();

	cout << "Start Test ---------------------------------------" << endl;
	dbg::Log("Start Test ---------------------------------------\n");
	dbg::Log2("log2.txt", "Start Test ---------------------------------------\n");
	int totalCount = 0;
	int errCount = 0;
	int detectCount = 0;
	int totalTime = 0;
	for each (auto item in testSet)
	{
		++totalCount;

		Mat img = imread(item.first.c_str());
		if (!img.data)
		{
			++errCount;
			cout << "Error!! Read Image " << item.first << endl;
			dbg::ErrLog("Error!! Read Image %s \n", item.first.c_str());
			dbg::Log2("log2.txt", "Error!! Read Image %s \n", item.first.c_str());
			continue;
		}

		const int t1 = timeGetTime();
		matchScript.ExecuteEx(executeLabelName, item.first, img, true);
		const int t2 = timeGetTime();
		const string result = matchScript.m_threadArg.resultStr;

		if (result != item.second)
		{
			cout << "Error!! Match " << item.first << ", " << item.second << ", " << result << endl;
			dbg::ErrLog("Error!! Match %s, %s, %s\n", item.first.c_str(), item.second.c_str(), result.c_str());
			dbg::Log2("log2.txt", "Error!! Match %s, %s, %s \n", item.first.c_str(), item.second.c_str(), result.c_str());
		}
		else
		{
			++detectCount;
			cout << "Match!! " << item.first << ", " << item.second << ", " << t2 - t1 << endl;
			dbg::Log("Match %s, %s %d \n", item.first.c_str(), item.second.c_str(), t2-t1);
			dbg::Log2("log2.txt", "Match %s, %s %d\n", item.first.c_str(), item.second.c_str(), t2-t1);
		}

		totalTime += (t2 - t1);
	}

	cout << "End Test ---------------------------------------" << endl;

	cout << "total = " << totalCount << ", detect = " << detectCount << ", error = " << errCount 
		<< ", time = " << totalTime/1000 << ", avr time = " << (totalTime / totalCount)<< endl;
	dbg::Log("total = %d, detect = %d, error = %d, time = %d, avr time = %d \n", totalCount, detectCount, errCount, totalTime/1000, totalTime/totalCount);
	dbg::Log2("log2.txt", "total = %d, detect = %d, error = %d, time = %d, avr time = %d \n", totalCount, detectCount, errCount, totalTime / 1000, totalTime / totalCount);
}
