
#include "matchscript.h"

using namespace cv;
using namespace match;
using namespace std;


struct sThreadArg
{
	bool isUsed;
	const Mat *input;
	char inputName[64];
	cMatchScript *p;
	sParseTree *node;
	sParseTree *parent;
	bool isEnd;
	int result;
};

unsigned __stdcall ThreadProcess(void *ptr)
{
	sThreadArg *arg = (sThreadArg*)ptr;
	string result;
	arg->result = arg->p->executeTreeEx(*arg->input, arg->inputName, arg->parent, arg->node);
	arg->isEnd = true;
	return 0;
}


// multi thread, image match
unsigned __stdcall MatchThreadProcess(void *ptr)
{
	sMatchThreadArg *matchArg = (sMatchThreadArg*)ptr;

	//cout << "cMatchScript::ExecuteEx Start >>\n";

	const Mat &A = *matchArg->input;
	const string inputName = matchArg->inputName;
	const bool isRemoveInput = matchArg->removeInput;
	cMatchScript *matchScript = matchArg->p;
	sParseTree *root = matchScript->m_parser.m_root;
	matchScript->clearResultTree(matchScript->m_parser.m_root);

	// find node correspond to label
	sParseTree *nodeLabel = matchArg->nodeLabel;

	if (!nodeLabel->child)
	{
		matchArg->isEnd = true;
		matchArg->result = -2;
		matchArg->resultStr = "not found label";
		return -2; // label node is empty
	}

	const int MAX_CORE = 9; // std::thread::hardware_concurrency() * 2 + 1
	HANDLE handles[MAX_CORE];
	sThreadArg args[MAX_CORE];
	ZeroMemory(handles, sizeof(handles));
	ZeroMemory(args, sizeof(args));

	typedef pair<sParseTree*, sParseTree*> ItemType; //<parent node, child node>
	vector< ItemType > q; // work like queue
	q.reserve(8);
	q.push_back(ItemType(NULL, nodeLabel->child));

	int coreCnt = 0;
	int reval = 0;
	string detectNodeStr = "~ fail ~";
	bool isLoop = true; // 모든 쓰레드가 끝나야, 루프가 종료된다.
	while ( (!q.empty() || isLoop) && !matchScript->m_isThreadTerminate)
	{
		isLoop = false;
		int emptyIdx = -1;

		for (int i = 0; i < MAX_CORE; ++i)
		{
			if (args[i].isUsed)
			{
				isLoop = true; // 동작하는 쓰레드가 있음
				break;
			}
		}

		// find empty thread
		for (int i = 0; i < MAX_CORE; ++i)
		{
			if (!args[i].isUsed)
			{
				emptyIdx = i;
				break;
			}
		}

		// find finish thread
		int endIdx = -1;
		for (int i = 0; i < MAX_CORE; ++i)
		{
			if (args[i].isUsed && args[i].isEnd)
			{
				endIdx = i;
				break;
			}
		}

		if ((emptyIdx < 0) && (endIdx < 0))
		{
			Sleep(1);
			continue;
		}

		// finish thread 처리
		if (endIdx >= 0)
		{
			sParseTree *node = args[endIdx].node;
			args[endIdx].isUsed = false;
			args[endIdx].isEnd = false;
			handles[endIdx] = NULL;
			node->result = args[endIdx].result;

			if (args[endIdx].result > 0)
			{
				sParseTree *child = node->child;
				if (!child) // success!! terminal node, end fuction
				{
					detectNodeStr = node->str;
					reval = 1;
					break; // success, loop terminate
				}
				else if (child && !child->child)
				{
					detectNodeStr = child->str;
					reval = 1;
					break; // success, loop terminate
				}
				else
				{
					q.push_back( ItemType(node, child) ); // high priority
					rotateright(q); // push front
				}
			}
			--coreCnt;
		}
		
		// match node 처리
		if (emptyIdx >= 0 && !q.empty())
		{
			ItemType item = q.front();
			rotatepopvector(q, 0); // pop front
			sParseTree *parent = item.first;
			sParseTree *node = item.second;

			if (node->result >= 0)
			{
				if (node->result  > 0)
				{
					sParseTree *child = node->child;
					if (!child) // success!! terminal node, end fuction
					{
						detectNodeStr = node->str;
						reval = 1;
						break; // success, loop terminate
					}
					else
					{
						q.push_back(ItemType(node, child)); // high priority
						rotateright(q); // push front
					}
				}
			}
			else
			{
				if (string("dummy") == node->str)
				{
					sParseTree *child = node->child;
					if (child)
					{
						q.push_back(ItemType(node, child)); // high priority
						rotateright(q); // push front
					}
				}
				else
				{
					isLoop = true; // 스레드 하나 실행
					ZeroMemory(&args[emptyIdx], sizeof(args[emptyIdx]));
					args[emptyIdx].isUsed = true;
					args[emptyIdx].input = &A;
					strcpy_s(args[emptyIdx].inputName, matchArg->inputName.c_str());
					args[emptyIdx].p = matchScript;
					args[emptyIdx].node = node;
					args[emptyIdx].parent = parent;
					handles[emptyIdx] = (HANDLE)_beginthreadex(NULL, 0, ThreadProcess, &args[emptyIdx], 0, NULL);
					++coreCnt;
				}
			}

			// push sibling node (breath first loop)
			if (node->next)
			{
				q.push_back( ItemType(parent, node->next) ); // low priority
				if (parent) // if parent node, high priority
					rotateright(q); // push front, 
			}
		}
	}

	matchArg->resultStr = detectNodeStr;
	matchArg->result = reval;
	matchArg->isEnd = true;

	for (int i = 0; i < MAX_CORE; ++i)
	{
		if (handles[i])
		{
			WaitForSingleObject(handles[i], INFINITE);
			handles[i] = NULL;
		}
	}

	// 입력으로 저장되었던 정보를 제거한다.
	if (isRemoveInput)
	{
		matchScript->RemoveInputImage(inputName);
	}

	return 0;
}



cMatchScript::cMatchScript()
	: m_matchType(0)
	, m_detector(cv::xfeatures2d::SURF::create(400))
	, m_isMatchThreadLoop(false)
	, m_threadHandle(NULL)
	, m_isThreadTerminate(false)
	, m_matchCount(0)
	, m_inputId(0)
{
}

cMatchScript::~cMatchScript()
{
	Clear();
}


void cMatchScript::build(sParseTree *parent, sParseTree *current)
{
	RET(!current);

	vector<string> attribs;
	buildAttributes(current, current->str, attribs);

	// check head link
	if (current->type == 0)
	{
		if (!attribs.empty())
		{
			auto it = m_headTable.find(attribs[0]);
			if (m_headTable.end() != it)
			{
				// link another head tree
				if (parent)
				{
					parent->child = it->second->child;
					delete current;
					return;
				}
			}
		}
	}

	setTreeAttribute(current, attribs);

	build(current, current->child);
	build(parent, current->next);
}


// tok1, tok2, tok3, tok4, tok5 ...  분리 
// node->str = tok1
// tok2~5 : roi x,y,w,h
//char *str = node->str;
void cMatchScript::buildAttributes(const sParseTree *node, const string &str, vector<string> &attributes)
{
 	m_parser.m_lineStr = (char*)str.c_str();
	while (1)
	{
		const char *pid = m_parser.id();
		const char *pnum = (!*pid) ? m_parser.number() : NULL;

		if (*pid)
		{
			auto it = m_parser.m_symbolTable.find(pid);
			if (m_parser.m_symbolTable.end() == it)
			{
				attributes.push_back(pid);
			}
			else
			{
				char *old = m_parser.m_lineStr;
				buildAttributes(node, it->second.c_str(), attributes);
				m_parser.m_lineStr = old;
			}
		}
		else if (*pnum)
		{
			attributes.push_back(pnum);
		}
		else
		{
			//cout << "line {" << node->lineNum << "} error!! tree attribute fail [" << str << "]\n";
			m_parser.m_isError = true;
			break;
		}

		// comma check
		m_parser.m_lineStr = m_parser.passBlank(m_parser.m_lineStr);
		if (!m_parser.m_lineStr)
			break;
		if (*m_parser.m_lineStr == ',')
			++m_parser.m_lineStr;
	}
}

// threshold_0.9 
// templatematch, featurematch
void cMatchScript::setTreeAttribute(sParseTree *node, vector<string> &attribs)
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
			std::rotate(attribs.begin() + i, attribs.begin() + i + 1, attribs.end());
			attribs.pop_back();
			break;
		}
	}

	// check match type
	node->matchType = -1; // defautl value is -1
	for (int i = 0; i < (int)attribs.size(); ++i)
	{
		const int pos1 = attribs[i].find("featurematch");
		const int pos2 = attribs[i].find("templatematch");
		if ((string::npos != pos1) || (string::npos != pos2))
		{
			if (string::npos != pos1)
				node->matchType = 1;
			if (string::npos != pos2)
				node->matchType = 0;

			// remove threshold attribute
			std::rotate(attribs.begin() + i, attribs.begin() + i + 1, attribs.end());
			attribs.pop_back();
			break;
		}
	}

	strcpy_s(node->str, attribs[0].c_str());

	if (node->type == 0) // tree
	{
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
	else if (node->type == 1) // exec
	{
		if (attribs.size() >= 2)
		{
			strcat_s(node->str, " ");
			strcat_s(node->str, attribs[1].c_str());
		}
	}

}


Mat& cMatchScript::loadImage(const string &fileName)
{
	AutoCSLock cs(m_loadImgCS);

	auto it = m_imgTable.find(fileName);
	if (m_imgTable.end() != it)
		return *it->second;

	Mat *img = new Mat();
	*img = imread(fileName);
	if (m_isGray && img->data)
		cvtColor(*img, *img, CV_BGR2GRAY);
	m_imgTable[fileName] = img;
	return *img;
}


void cMatchScript::loadDescriptor(const string &fileName, OUT vector<KeyPoint> **keyPoints, OUT Mat **descriptor)
{
	{
		AutoCSLock cs(m_loadDescriptCS); // 소멸자 호출을 위한 지역변수
		auto it = m_keyPointsTable.find(fileName);
		if (m_keyPointsTable.end() != it)
		{
			*keyPoints = it->second;
			*descriptor = m_descriptorTable[fileName];
			return;
		}
	}

	const Mat &src = loadImage(fileName);
	loadDescriptor(fileName, src, keyPoints, descriptor);
}


void cMatchScript::loadDescriptor(const string &keyName, const cv::Mat &img,
	OUT vector<KeyPoint> **keyPoints, OUT Mat **descriptor, const bool isSearch) //isSearch=true
{
	AutoCSLock cs(m_loadDescriptCS);

	if (isSearch)
	{
		auto it = m_keyPointsTable.find(keyName);
		if (m_keyPointsTable.end() != it)
		{
			*keyPoints = it->second;
			*descriptor = m_descriptorTable[keyName];
			return; // find, and return
		}
	}

	// not found, Compute keyPoints, Descriptor
	vector<KeyPoint> *keyPts = new vector<KeyPoint>();
	Mat *descr = new Mat();
	m_detector->detectAndCompute(img, Mat(), *keyPts, *descr);

	delete m_keyPointsTable[keyName];
	m_keyPointsTable[keyName] = keyPts;
	delete m_descriptorTable[keyName];
	m_descriptorTable[keyName] = descr;

	*keyPoints = keyPts;
	*descriptor = descr;
}


// return value 0 : fail
//					    1 : success
int cMatchScript::executeTreeEx(const Mat &input, const string &inputName, sParseTree *parent, sParseTree *node)
{
	if (!node)
		return 0;
	if (!node->child)
		return 1; // terminal node, success finish
	if (string("dummy") == node->str) // dummy node, excute child node
		return 1;

	const Mat &matObj = loadImage(node->str);
	if (matObj.empty())
		return 0;

	const cv::Size csize(input.cols - matObj.cols + 1, input.rows - matObj.rows + 1);
	if ((csize.height < 0) || csize.width < 0)
		return 0;

	// roi x,y,w,h
	Rect roi(0, 0, input.cols, input.rows);
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
	if (input.cols < roi.x + roi.width)
		roi.width = input.cols - roi.x;
	if (input.rows < roi.y + roi.height)
		roi.height = input.rows - roi.y;

	double min, max;
	Point left_top;
	const float maxThreshold = (node->threshold == 0) ? 0.9f : node->threshold;
	bool isDetect = false;

	const bool isFeatureMatch = ((m_matchType == 1) && (node->matchType == -1)) || (node->matchType == 1);
	//cout << "executeTree " << node->str << ", matchType=" << isFeatureMatch << "\n";

	m_matchCount++;
	if (!isFeatureMatch) // --> templatematch
	{
		Mat matResult(csize, IPL_DEPTH_32F);
		cv::matchTemplate(input(roi), matObj, matResult, CV_TM_CCOEFF_NORMED);
		cv::minMaxLoc(matResult, &min, &max, NULL, &left_top);
		isDetect = max > maxThreshold;

		//cout << "    - templatematch max=" << max << endl;
	}
	else // feature match
	{
		vector<KeyPoint> *objectKeyPoints, *sceneKeyPoints;
		Mat *objectDescriotor, *sceneDescriptor;
 		loadDescriptor(node->str, &objectKeyPoints, &objectDescriotor);
		loadDescriptor(inputName, &sceneKeyPoints, &sceneDescriptor);

		cFeatureMatch match(m_isGray, m_isDebug);
		isDetect = match.Match(input(roi), *sceneKeyPoints, *sceneDescriptor, matObj, *objectKeyPoints, *objectDescriotor);
		max = isDetect ? 1 : 0;
	}

	if (isDetect)
	{
		node->matchLoc = left_top + Point(roi.x, roi.y);
		return 1;
	}

	return 0;
}


// 스크립트의 exec 명령어를 실행한다.
void cMatchScript::Exec()
{
	sParseTree *node = m_parser.m_execRoot;
	while (node)
	{
		stringstream ss(node->str);
		string label, file;
		ss >> label >> file;
		if (label.empty() || file.empty())
			break;

		Mat img = imread(file.c_str());
		if (!img.data)
			break;

		if (m_isGray)
			cvtColor(img, img, CV_BGR2GRAY);

		const int t1 = timeGetTime();
		ExecuteEx(label, file, img, true);
		const int t2 = timeGetTime();
		cout << "exec(" << label << ") << " << file << " = " << m_threadArg.resultStr << ", " << t2-t1 << endl;

		node = node->next;
	}
}


// multithread match
// 최종 결과 정보는 m_threadArg.resultStr 에 저장된다.
void cMatchScript::ExecuteEx(const string &label, const string &inputName, const Mat &input, 
	const bool isBlockMode, const bool registerInput) //isBlockMode=false, registerInput=true
{
	if (m_isMatchThreadLoop)
	{
		// 이미 실행 중인 스레드를 종료한다.
		FinishMatchThread();
	}

	// find node correspond to label
	sParseTree *nodeLabel = NULL;
	sParseTree *node = m_parser.m_root;
	while (node)
	{
		if (label == node->str)
		{
			nodeLabel = node;
			break;
		}
		node = node->next;
	}
	if (!nodeLabel)
	{
		m_threadArg.result = 0;
		m_threadArg.resultStr = "not found label";
		return;
	}

	m_matchCount = 0;

	// 중복된 input name을 쓸 경우, 비동기처리시 문제가 발생한다.
	// 쓰레드가 끝나지 않은 상황에서, 같은 input name을 바꿀 수 있다.
	// 그래서 중복되지 않는 input name을 설정해야 한다.
	string inputId;
	if (registerInput)
	{
		stringstream ss;
		ss << "@input" << m_inputId++;
		m_inputId = min(1000000, m_inputId); // limit check
		inputId = ss.str();
		SetInputImage(input, inputId);
	}
	else
	{
		inputId = inputName;
		if (inputId.empty())
			inputId = "@input";
	}
	//

	m_isMatchThreadLoop = true;
	m_isThreadTerminate = false;

	m_threadArg.input = &loadImage(inputId);//&m_input;
	m_threadArg.inputName = inputId;
	m_threadArg.removeInput = registerInput; // 입력이 자동으로 등록되면, 제거도 자동으로 된다.
	m_threadArg.isEnd = false;
	m_threadArg.p = this;
	m_threadArg.nodeLabel = nodeLabel;
	m_threadArg.result = 0;

	m_threadHandle = (HANDLE)_beginthreadex(NULL, 0, MatchThreadProcess, &m_threadArg, 0, NULL);

	if (isBlockMode)
	{
		// 쓰레드를 종료 확인하고, 리턴
		while (!m_threadArg.isEnd)
			Sleep(1);
		//WaitForSingleObject(m_threadHandle, INFINITE);
		m_threadHandle = NULL;
		m_isMatchThreadLoop = false;
	}
}

// return value -1 : not work thread
//						  0 : busy thread
//						  1 : thread work finished
int cMatchScript::IsFinishLoop(OUT string &result)
{
	RETV(!m_isMatchThreadLoop, -1);

	if (m_threadArg.isEnd)
	{
		m_isMatchThreadLoop = false;
		m_threadHandle = NULL;
		result = m_threadArg.resultStr;
		return 1;
	}

	return 0;
}


// 스레드가 실행 중이라면, 강제 종료시킨다.
void cMatchScript::FinishMatchThread()
{
	if (m_threadHandle)
	{
		m_isThreadTerminate = true;
		WaitForSingleObject(m_threadHandle, INFINITE); // 5 seconds wait
		m_threadHandle = NULL;
		m_isMatchThreadLoop = false;
	}
}


void cMatchScript::SetInputImage(const cv::Mat &img, const string &name) // name=@input
{
	{
		AutoCSLock cs(m_loadImgCS); // 소멸자 호출을 위한 지역변수

		delete m_imgTable[name];
		m_imgTable[name] = new Mat();
		*m_imgTable[name] = img.clone(); // register input image
	}

	{
		AutoCSLock cs(m_loadDescriptCS); // 소멸자 호출을 위한 지역변수

		delete m_keyPointsTable[name];
		m_keyPointsTable.erase(name);
		delete m_descriptorTable[name];
		m_descriptorTable.erase(name);
	}

}


void cMatchScript::RemoveInputImage(const string &name)
{
	{
		AutoCSLock cs(m_loadImgCS); // 소멸자 호출을 위한 지역변수

		auto it = m_imgTable.find(name);
		if (it != m_imgTable.end())
		{
			delete it->second;
			m_imgTable.erase(name);
		}
	}

	{
		AutoCSLock cs(m_loadDescriptCS); // 소멸자 호출을 위한 지역 변수

		delete m_keyPointsTable[name];
		m_keyPointsTable.erase(name);

		delete m_descriptorTable[name];
		m_descriptorTable.erase(name);
	}
}


bool cMatchScript::FindLabel(const string &label)
{
	sParseTree *node = m_parser.m_root;
	while (node)
	{
		if (label == node->str)
			return true;
		node = node->next;
	}
	return false;
}


// 스크립트를 읽고, 파스트리를 생성한다.
bool cMatchScript::Read(const string &fileName)
{
	Clear();

	if (!m_parser.Read(fileName))
		return false;

	sParseTree *node = m_parser.m_root;
	while (node)
	{
		m_headTable[node->str] = node;
		node = node->next;
	}

	build(NULL, m_parser.m_root);
	build(NULL, m_parser.m_execRoot);

 	m_isDebug = atoi(m_parser.m_symbolTable["debug"].c_str()) ? true : false;
 	m_matchType = atoi(m_parser.m_symbolTable["matchtype"].c_str());

	return true;
}


void cMatchScript::Clear()
{
	m_parser.Clear();
	m_headTable.clear();

	{
		AutoCSLock cs(m_loadImgCS); // 소멸자 호출을 위한 지역변수
		
		for each (auto kv in m_imgTable)
			delete kv.second;
		m_imgTable.clear();
	}

	{
		AutoCSLock cs(m_loadDescriptCS); // 소멸자 호출을 위한 지역변수

		for each (auto it in m_keyPointsTable)
			delete it.second;
		m_keyPointsTable.clear();

		for each (auto it in m_descriptorTable)
			delete it.second;
		m_descriptorTable.clear();
	}
}


// sParsetree 의 result 값을 초기화 한다.
// 함수가 재귀적으로 호출될 때, 중복 연산을 막기위해 쓰인다.
void cMatchScript::clearResultTree(sParseTree *node) 
{
	RET(!node);
	node->result = -1;
	clearResultTree(node->child);
	clearResultTree(node->next);
}
