
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


// �ҹ��� ��ȯ, ���鹮�ڴ� �ִ� 1�� ����, 
// ������� �ʴ� Ư�� ���� ����
// ª�� �ܾ�� ���ܽ�Ų��.
bool cScanner::Init(const cDictionary &dict, const string &str)
{
	string src = str;
	trim(src);
	lowerCase(src);

	//m_str.clear();
	string dst;

	// ���̴� ����, ��ȣ�� ��ȣ�� ������ ���� ����
	for (uint i = 0; i < src.length(); ++i)
	{
		const char c = src[i];
		if ((dict.m_useChar[c]) || (dict.m_ambiguousTable[c] != 0))
			dst += c;
	}
	
	// ���� �ִ� 1��
	bool isBlank = false;
	for (uint i = 0; i < dst.length(); )
	{
		if (dst[i] == ' ')
		{
			if (isBlank)
			{ // ���鹮�ڸ� ����ڷ� �ű� ��, ����
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

	// ª�� �ܾ���� ���ܽ�Ų��.
	stringstream ss;
	bool isFirst = true;
	vector<string> strs;
	tokenizer(dst, " ", "", strs);
	for each (auto str in strs)
	{
		trim(str);
		if (str.length() < (uint)cDictionary::MIN_WORD_LEN)
			continue; // �ʹ� ª�� �ܾ�� �����Ѵ�.

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
