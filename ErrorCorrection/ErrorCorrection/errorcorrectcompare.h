//
// 2016-08-01, jjuiddong
// ������ ��ϵ� �����, �Է����� ���� ������ ���ؼ� �˻��Ѵ�.
// ������ �����ϸ鼭 �˻��Ѵ�.
//
#pragma once


namespace tess
{
	class cDictionary;
	class cErrCorrectCompare
	{
	public:
		struct sInfo
		{
			int tot;
			int hit;
			int err;
			int rep;
			int flags;
		};

		cErrCorrectCompare();
		virtual ~cErrCorrectCompare();
		bool Compare(cDictionary *dict, char *src, char *dict_word, const int src_idx, const int dict_index);


	protected:
		bool CompareSub(cDictionary *dict, char *src, char *dict_word, const int src_idx, const int dict_index, 
			sInfo &info);


	public:
		sInfo m_result;
	};
}

