//
// 2016-07-31, jjuiddong
// ���ڿ� �ϳ��� �о ��ȯ�ϴ� ����� ����.
//
#pragma once


namespace tess
{

	class cDictionary;
	class cScanner
	{
	public:
		cScanner();
		cScanner(const cDictionary &dict, const string &str);
		virtual ~cScanner();
		bool Init(const cDictionary &dict, const string &str);
		bool IsEmpty();


	public:
		string m_str;
	};

}