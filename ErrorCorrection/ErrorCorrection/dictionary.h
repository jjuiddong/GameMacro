//
// 2016-07-31, jjuiddong
// ���� ���
// �������� �ҹ��ڷ� ����ǰ�, �ҹ��ڷ� ��Ī�Ѵ�.
// �� ĭ �̻��� ������ �� ĭ���� �����Ѵ�.
// ������ �ʴ� ���ڳ� Ư�����ڴ� ��Ī���� ���� �ȴ�.
//
#pragma once


namespace tess
{

	class cDictionary
	{
	public:
		cDictionary();
		virtual ~cDictionary();

		bool Init(const string &fileName);
		string FastSearch(const string &sentence, OUT vector<string> &out);
		string ErrorCorrectionSearch(const string &sentence);
		string Search(const string &sentence, OUT vector<string> &out);
		void Clear();


	protected:
		void GenerateCharTable();


	public:
		struct sSentence
		{
			string src; // ���� �ܾ�
			string lower; // �ҹ��� �ܾ�
		};

		vector<sSentence> m_sentences;
		vector<string> m_words; // lower case words
		map<string, int> m_sentenceLookUp; // sentence (lower case), sentence id (m_sentences index)
		map<string, int> m_wordLookup; // all separate word (lower case), word id (m_words index)
		char m_ambiguousTable[256]; // alphabet + number + special char
		int m_ambigId;

		struct sCharTable
		{
			set<uint> sentenceIds;
		};

		enum {
			MAX_CHAR=256,
			MAX_LEN = 30,
			MAX_WORD = 1000, // �ִ� �ܾ� ����
			MIN_WORD_LEN=3,
		};
		sCharTable m_charTable[MAX_CHAR][MAX_LEN]; // character table
		bool m_useChar[MAX_CHAR]; // ���ǰ� �ִ� ���ڶ�� true ���ȴ�. GenerateCharTable() ���� �ʱ�ȭ

		set<int> m_sentenceWordSet[MAX_WORD]; // �ش��ϴ� �ܾ ���Ե� ���� id�� ����, index = m_words index
	};

}
