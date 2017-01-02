
#include "stdafx.h"
#include "scanner.h"
#include "dictionary2.h"

namespace tess
{
	static const char *g_numStr = "1234567890.";
}

using namespace tess;


tess::cScanner::cScanner()
{
}

cScanner::cScanner(const string &str)
{
	Init(str);
}

tess::cScanner::~cScanner()
{
}


// �ҹ��� ��ȯ, ���鹮�ڴ� �ִ� 1�� ����, 
// ª�� �ܾ�� ���ܽ�Ų��.
bool cScanner::Init(const string &str)
{
	string src = str;
	trim(src);
	lowerCase(src);

	string dst = src;
	
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

	// ���� ���ڰ� 3���̻� �����ؼ� ������ ����, 
	// ���ڴ� ����
	for (int i = 0; i < (int)dst.length() - 3;)
	{
		const char *n = strchr(g_numStr, dst[i]);
		if (n) // ���ڴ� ����
		{
			++i;
			continue;
		}
		
		if ((dst[i] == dst[i+1]) 
			&& (dst[i] == dst[i + 2]))
		{
			for (int k = i+1; k < (int)dst.length(); )
			{
				if (dst[i] == dst[k])
					rotatepopvector(dst, k);
				else
					break;
			}
			rotatepopvector(dst, i);
		}
		else
		{
			++i;
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
		if (str.length() < (uint)cDictionary2::MIN_WORD_LEN)
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
