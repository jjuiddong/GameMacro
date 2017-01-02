//
// 2016-07-31, jjuiddong
// ���� ���
// �������� �ҹ��ڷ� ����ǰ�, �ҹ��ڷ� ��Ī�Ѵ�.
// �� ĭ �̻��� ������ �� ĭ���� �����Ѵ�.
// ������ �ʴ� ���ڳ� Ư�����ڴ� ��Ī���� ���� �ȴ�.
//
// 2016-08-22
//	- ���ڿ� �˻��� levenshtein_distance �� ����
//		https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance
// 
//
#pragma once


namespace tess
{

	class cDictionary2
	{
	public:
		cDictionary2();
		virtual ~cDictionary2();

		bool Init(const string &fileName);
		string FastSearch(const string &sentence, OUT vector<string> &out);
		string ErrorCorrectionSearch(const string &sentence, OUT float &maxFitness);
		string Search(const string &sentence, OUT vector<string> &out, OUT float &maxFitness);
		void Clear();


	protected:
		unsigned int cDictionary2::levenshtein_distance(const std::string& s1, const std::string& s2);


	public:
		struct sSentence
		{
			string src; // ���� �ܾ�
			string lower; // �ҹ��� �ܾ�
			string output; // ��Ī�̵ǰ� ���ϵ� ���ڿ�, { ~~ } �� �����Ѵ�., �⺻���� ���� �ܾ�
		};

		vector<sSentence> m_sentences;
		vector<string> m_words; // lower case words
		map<string, int> m_sentenceLookUp; // sentence (lower case), sentence id (m_sentences index)
		map<string, int> m_wordLookup; // all separate word (lower case), word id (m_words index)
		std::vector<unsigned int> m_col, m_prevCol; // for levenshtein_distance function

		enum {
			MAX_CHAR=256,
			MAX_WORD = 1000, // �ִ� �ܾ� ����
			MIN_WORD_LEN=3,
		};

		set<int> m_sentenceWordSet[MAX_WORD]; // �ش��ϴ� �ܾ ���Ե� ���� id�� ����, index = m_words index
	};

}
