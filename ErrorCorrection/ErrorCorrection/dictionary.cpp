
#include "global.h"
#include "dictionary.h"
#include "scanner.h"
#include "errorcorrectcompare.h"


using namespace tess;

cDictionary::cDictionary()
	: m_ambigId(1)
{
}

cDictionary::~cDictionary()
{
}


// ���� ��ũ��Ʈ�� �о, �ܾ� ������ �����.
bool cDictionary::Init(const string &fileName)
{
	using namespace std;

	Clear();

	ifstream ifs(fileName);
	if (!ifs.is_open())
		return false;
	
	int state = 0; // 0:word, 1:ambiguous
	string line;
	while (getline(ifs, line))
	{
		trim(line);
		if (line.empty())
			continue;

		if (line == "word:")
		{
			state = 0;
			continue;
		}
		else if (line == "ambiguous:")
		{
			state = 1;
			continue;
		}

		if (0 == state ) // word list command
		{
			string tmp = line;
			const string lower = lowerCase(tmp);

			if (m_sentenceLookUp.end() == m_sentenceLookUp.find(lower))
			{// ������ �߰�
				m_sentences.push_back({line, lower});
				m_sentenceLookUp[lower] = m_sentences.size() - 1; // word index
			}
			else
			{ // ������ �̹� ������ �Ѿ
				std::cout << "error already exist word = " << line << "]\n";
			}

			// ������ ������ �������� ���� �ܾ�� ������ ����.
			const int sentenceId = m_sentences.size() - 1;
			vector<string> words;
			tokenizer(lower, " ", "", words);
			for each (auto word in words)
			{
				trim(word);
				if (word.length() < MIN_WORD_LEN)
					continue; // �ʹ� ª�� �ܾ�� �����Ѵ�.

				int wordId = 0;
				const auto it = m_wordLookup.find(word);
				if (it == m_wordLookup.end())
				{
					m_words.push_back(word);
					wordId = m_words.size() - 1;
					m_wordLookup[word] = m_words.size()-1;
				}
				else
				{
					wordId = it->second;
				}

				if (wordId < MAX_WORD)
					m_sentenceWordSet[wordId].insert(sentenceId);
			}

		}
		else if (1 == state) // ambiguous command
		{
			// format = c1 - c2 - c3
			string tmp = line;
			const string lower = lowerCase(tmp);

			vector<string> words;
			tokenizer(lower, "-", "", words);
			for each (auto word in words)
			{
				trim(word);
				if (word.empty())
					continue;
				if (word.length() > 1) // allow only one character
					continue;
				m_ambiguousTable[word[0]] = m_ambigId; // ���� ID�� �ο��Ѵ�.
			}
			++m_ambigId;
		}
	}

	// dictionary script �� ��� ���� ��, ���̺��� �����Ѵ�.
	GenerateCharTable();
	
	return true;
}


// word �� ������ �ִ� �ܾ�θ� �����Ǿ� �ִٸ�, true�� �����Ѵ�.
// ��ġ�ϴ� �ܾ out�� ������ �����Ѵ�.
// ErrorCorrectionSearch() ���� ������.
string cDictionary::FastSearch(const string &sentence, OUT vector<string> &out)
{
	cScanner scanner(*this, sentence);// �ʿ���� ���� ����
	if (scanner.IsEmpty())
		return "";

	stringstream ss;
	set<int> sentenceSet; // words �ܾ ���Ե� ���� id�� ����

	// ������ �ܾ� ������ ���Ѵ�.
	vector<string> words;
	tokenizer(scanner.m_str, " ", "", words);
	for each (auto word in words)
	{
		trim(word);
		if (word.length() < MIN_WORD_LEN)
			continue; // �ʹ� ª�� �ܾ�� �����Ѵ�.
		
		auto it = m_wordLookup.find(word);
		if (it != m_wordLookup.end())
		{
			ss << (out.empty()? word : string(" ")+ word);
			out.push_back(word); // add maching word

			const int wordId = it->second;
			if (wordId >= MAX_WORD)
				continue;

			if (sentenceSet.empty())
			{
				sentenceSet = m_sentenceWordSet[wordId];
			}
			else
			{
				// �� �ܾ ���Ե� ����鳢���� �������� ���Ѵ�.
				vector<int> tmp;
				std::set_intersection(sentenceSet.begin(), sentenceSet.end(),
					m_sentenceWordSet[wordId].begin(), m_sentenceWordSet[wordId].end(),
					std::back_inserter(tmp));
				
				sentenceSet.clear();
				for each (auto id in tmp)
					sentenceSet.insert(id);
			}
		}
	}
	
	if (sentenceSet.empty())
		return "";

	if (sentenceSet.size() == 1)
		return m_sentences[*sentenceSet.begin()].src;

	return "";
}


void cDictionary::Clear()
{
	m_ambigId = 0;
	m_sentences.clear();
	m_words.clear();
	m_sentenceLookUp.clear();
	ZeroMemory(m_useChar, sizeof(m_useChar));

	for (uint i = 0; i < MAX_CHAR; ++i)
		for (uint k = 0; k < MAX_LEN; ++k)
			m_charTable[i][k].sentenceIds.clear();

	for (uint i = 0; i < MAX_WORD; ++i)
		m_sentenceWordSet[i].clear();
}


void cDictionary::GenerateCharTable()
{
	for (uint i = 0; i < m_sentences.size(); ++i)
	{
		const uint sentenceId = i;
		const sSentence &sentence = m_sentences[i];
		for (uint k=0; k < sentence.lower.length(); ++k)
		{
			if (k >= MAX_LEN)
				break;
 			const char c = sentence.lower[k];
			m_charTable[c][k].sentenceIds.insert(sentenceId);
			m_useChar[c] = true;
		}
	}
}


// ������ �����ϸ鼭, ������ �������� ã�´�.
// FastSearch() ���� ������.
string cDictionary::ErrorCorrectionSearch(const string &sentence)
{
	cScanner scanner(*this, sentence);// �ʿ���� ���� ����
	if (scanner.IsEmpty())
		return "";

	vector<bool> procSentenceIds(m_sentences.size(), false);

	int maxSentenceId = -1;
	int maxTotCount = -100;

	string src = scanner.m_str;
	for (uint i = 0; i < src.size(); ++i)
	{
		const uchar c = src[i];
		for (uint k = 0; k < MAX_LEN; ++k)
		{
			for each (auto sentenceId in m_charTable[c][k].sentenceIds)
			{
				if (procSentenceIds[sentenceId])
					continue;
				procSentenceIds[sentenceId] = true;

				cErrCorrectCompare cmp;
				string tmp = src;
				cmp.Compare(this, (char*)tmp.c_str(), (char*)m_sentences[sentenceId].lower.c_str(), i, k);
				if (cmp.m_result.tot > maxTotCount)
				{
					maxTotCount = cmp.m_result.tot;
					maxSentenceId = sentenceId;
				}
			}
		}
	}

	if (maxSentenceId >= 0)
		return m_sentences[maxSentenceId].src;

	return "";
}


// FastSearch() + ErrorCorrectionSearch()
string cDictionary::Search(const string &sentence, OUT vector<string> &out)
{
	string result = FastSearch(sentence, out);
	if (result.empty())
		result = ErrorCorrectionSearch(sentence);
	return result;
}