//
// 2016-07-31, jjuiddong
// ���ڿ� �ϳ��� �о ��ȯ�ϴ� ����� ����.
//
// 2016-08-27
//		- ��ȣ�� ���� �Ǻ� ����
//
#pragma once


namespace tess
{

	class cScanner
	{
	public:
		cScanner();
		cScanner(const string &str);
		virtual ~cScanner();
		bool Init(const string &str);
		bool IsEmpty();


	public:
		string m_str;
	};

}