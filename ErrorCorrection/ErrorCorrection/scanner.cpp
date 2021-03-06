
#include "global.h"
#include "scanner.h"
#include "dictionary.h"

using namespace tess;


tess::cScanner::cScanner()
{
}

cScanner::cScanner(const cDictionary &dict, const string &str)
{
	Init(dict, str);
}

tess::cScanner::~cScanner()
{
}


// 소문자 변환, 공백문자는 최대 1개 까지, 
// 사용하지 않는 특수 문자 제거
// 짧은 단어는 제외시킨다.
bool cScanner::Init(const cDictionary &dict, const string &str)
{
	string src = str;
	trim(src);
	lowerCase(src);

	//m_str.clear();
	string dst;

	// 쓰이는 문자, 모호한 기호를 제외한 문자 제거
	for (uint i = 0; i < src.length(); ++i)
	{
		const char c = src[i];
		if ((dict.m_useChar[c]) || (dict.m_ambiguousTable[c] != 0))
			dst += c;
	}
	
	// 공백 최대 1개
	bool isBlank = false;
	for (uint i = 0; i < dst.length(); )
	{
		if (dst[i] == ' ')
		{
			if (isBlank)
			{ // 공백문자를 가장뒤로 옮긴 후, 제거
				rotatepopvector(dst, i);
			}
			else
			{
				++i;
				isBlank = true;
			}
		}
		else
		{
			++i;
			isBlank = false;
		}
	}

	// 짧은 단어들은 제외시킨다.
	stringstream ss;
	bool isFirst = true;
	vector<string> strs;
	tokenizer(dst, " ", "", strs);
	for each (auto str in strs)
	{
		trim(str);
		if (str.length() < (uint)cDictionary::MIN_WORD_LEN)
			continue; // 너무 짧은 단어는 제외한다.

		ss << ((isFirst)? str : string(" ")+str);
		isFirst = false;
	}

	m_str = ss.str();
	return true;
}


bool cScanner::IsEmpty()
{
	return m_str.empty();
}
