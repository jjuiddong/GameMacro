
#include "global.h"



// "  skdfjskdjf  "
// "$$$skdfjskdjf$$$"
// "skdfjskdjf"
string& trim(string &str)
{
	// �տ������� �˻�
	for (int i = 0; i < (int)str.length(); ++i)
	{
		if ((str[i] == '\n') || (str[i] == '\t') || (str[i] == '\r') || (str[i] == ' '))
		{
			rotatepopvector(str, i);
			--i;
		}
		else
			break;
	}

	// �ڿ������� �˻�
	for (int i = (int)str.length() - 1; i >= 0; --i)
	{
		if ((str[i] == '\n') || (str[i] == '\t') || (str[i] == '\r') || (str[i] == ' '))
		{
			rotatepopvector(str, i);
		}
		else
			break;
	}

	return str;
}



// �Ѿ�� ���� str �� �ҹ��ڷ� �ٲ㼭 �����Ѵ�.
string& lowerCase(string &str)
{
	std::transform(str.begin(), str.end(), str.begin(), tolower);
	return str;
}


// ���� ���ڿ� str���� ������ delimeter �� �����ؼ� out �� �����ؼ� �����Ѵ�.
// delimeter �� ������� �ʴ´�.
void tokenizer(const string &str, const string &delimeter, const string &ignoreStr, OUT vector<string> &out)
{
	string tmp = str;
	int offset = 0;
	int first = 0;

	while (!tmp.empty())
	{
		const int pos = (int)tmp.find(delimeter, offset);
		if (string::npos == pos)
		{
			out.push_back(tmp.substr(first));
			break;
		}
		else
		{
			const string tok = tmp.substr(offset, pos - offset);
			offset += (int)tok.length() + (int)delimeter.length();
			if (tok != ignoreStr)
			{
				out.push_back(tmp.substr(first, pos - first));
				first = offset;
			}

		}
	}
}
