//
// 2016-07-11, jjuiddong
// 이미지 매칭 스크립트
// 심플 버전
//
//
#pragma once


class cSimpleMatchScript
{
public:
	cSimpleMatchScript();
	virtual ~cSimpleMatchScript();

	string Match(INOUT cv::Mat &src, OUT cv::Mat &dst, const string &script, 
		const bool showMsg=true);


protected:
	void Parse(const string &script);
	string pid(const string &str);
	int attrs(const string &str, OUT string &out);
	void attr_list(const string &str);

public:
	enum STATE { WAIT, BEGIN_MATCH};
	STATE m_state;
	vector<std::pair<string, string>> m_commands;
	cv::Mat m_src;
	cv::Mat m_tessImg;
	int m_curIdx;
	int m_beginMatchTime;
};
