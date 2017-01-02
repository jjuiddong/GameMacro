#pragma once

#include "matchparser.h"

namespace match 
{

	class cMatchScript;
	struct sMatchThreadArg
	{
		const cv::Mat *input;
		string inputName;
		bool removeInput;
		cMatchScript *p;
		sParseTree *nodeLabel;
		string resultStr;
		bool isEnd;
		int result;
	};


	class cMatchScript
	{
	public:
		cMatchScript();
		virtual ~cMatchScript();

		bool Read(const string &fileName);
		void Exec();
		void ExecuteEx(const string &label, const string &inputName, const cv::Mat &input, const bool isBlockMode=false, const bool registerInput=true);
		int IsFinishLoop(OUT string &result);
		void FinishMatchThread();
		void SetInputImage(const cv::Mat &img, const string &name="@input");
		void RemoveInputImage(const string &name);
		bool FindLabel(const string &label);
		void Clear();


	public:
		cv::Mat& loadImage(const string &fileName);
		void loadDescriptor(const string &fileName, OUT vector<cv::KeyPoint> **keyPoints, OUT cv::Mat **descriptor);
		void loadDescriptor(const string &keyName, const cv::Mat &img, OUT vector<cv::KeyPoint> **keyPoints, OUT cv::Mat **descriptor, const bool isSearch = true);
		void build(sParseTree *parent, sParseTree *current);
		void buildAttributes(const sParseTree *node, const string &str, vector<string> &attributes);
		void setTreeAttribute(sParseTree *node, vector<string> &attribs);
		int executeTreeEx(const cv::Mat &input, const string &inputName, sParseTree *parent, sParseTree *node);
		void clearResultTree(sParseTree *node);


	public:
		cParser m_parser;
		map<string, sParseTree*> m_headTable;
		map<string, cv::Mat* > m_imgTable;
		map<string, vector<cv::KeyPoint>* > m_keyPointsTable; // use feature detection
		map<string, cv::Mat* > m_descriptorTable; // use feature detection
		cv::Ptr<cv::xfeatures2d::SURF> m_detector;
		bool m_isGray = true;
		bool m_isDebug = false;
		int m_matchType; // 0=template match, 1=feature match, default = 0

		CriticalSection m_loadImgCS;
		CriticalSection m_loadDescriptCS;
		int m_inputId; // input image counting
		bool m_isMatchThreadLoop;
		bool m_isThreadTerminate; // false 일 때만, 스레드가 동작한다. 스레드를 중간에 멈추는 용도로 씀.
		HANDLE m_threadHandle;
		sMatchThreadArg m_threadArg;
		int m_matchCount; // debug
	};

} 
