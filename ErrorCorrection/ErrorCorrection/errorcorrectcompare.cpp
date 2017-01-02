
#include "global.h"
#include "errorcorrectcompare.h"
#include "dictionary.h"


using namespace tess;

cErrCorrectCompare::cErrCorrectCompare()
{
	ZeroMemory(&m_result, sizeof(m_result));
}

cErrCorrectCompare::~cErrCorrectCompare()
{
}


bool cErrCorrectCompare::Compare(cDictionary *dict,
	char *src, char *dict_word, const int src_idx, const int dict_index)
{
	sInfo info1;
	ZeroMemory(&info1, sizeof(info1));	
	CompareSub(dict, src, dict_word, src_idx, dict_index, info1); // ���� �ܾ �ٲ㰡�鼭 ��

	sInfo info2;
	ZeroMemory(&info2, sizeof(info2));
	info2.flags = 1;
	CompareSub(dict, src, dict_word, src_idx, dict_index, info2); // ���� �ܾ �ٲ㰡�鼭 ��

	// �� ����� ����� �����ϰ� �����Ѵ�.
	m_result = (info1.tot < info2.tot) ? info2 : info1;
	return true;
}


bool cErrCorrectCompare::CompareSub(cDictionary *dict, 
	char *src, char *dict_word, const int src_idx, const int dict_index, 
	sInfo &info)
{
	const int MAX_GAP = 20;
	const int MAX_ERROR = 15;
	const int MAX_WORD_ERROR = 3;

	sInfo maxFitness;
	ZeroMemory(&maxFitness, sizeof(maxFitness));
	maxFitness.tot = -1000;

	int i = src_idx;
	int k = dict_index;
	int nextCount = 0;
	int storeIdx = 0;

	while (src[i])
	{
		while (dict_word[k])
		{
			if (src[i] == dict_word[k])
			{
				nextCount = 0;
				++info.hit;
				++i;
				++k;
			}
			else if (
				(0 != dict->m_ambiguousTable[src[i]]) &&
				(dict->m_ambiguousTable[ src[i]] == dict->m_ambiguousTable[ dict_word[i]]))
			{
				// ��ȣ�� �������� ������ ��, ������ ������ ����
				nextCount = 0;
				++info.rep;
				++i;
				++k;
			}
			else
			{
				++info.err;

				if (0 == nextCount)
					storeIdx = (info.flags==0)? i : k;

				if (info.flags ==0)
					++i; // �ν��� �ܾ ������Ų��. ���� �ν��� �߸��Ǽ�, ���� �ܾ ���� ������ ��찡 ����.
				else
					++k; // ���� �ܾ� �ε����� ������Ų��. �ν��� ���忡�� �ܾ ������ ��찡 ���� ����.

				++nextCount;

				// ���������� ��Ī�� �ȵȰ��, src index or dict index�� ������Ų��.
				// ��Ī�� �ȵ� ������ ��������� �ٽ� �˻��Ѵ�.
				if (nextCount > MAX_WORD_ERROR)
				{
					info.err -= (nextCount - 1);

					sInfo tmp = info;
					CompareSub(dict, src, dict_word, i, k, tmp);
					if (tmp.tot > maxFitness.tot)
						maxFitness = tmp;

					if (0 == info.flags)
					{
						i = storeIdx + 1;
						++k;
					}
					else
					{
						k = storeIdx + 1;
						++i;
					}

					nextCount = 0;
				}
			}

			if (!src[i] || !dict_word[k])
				break;
			if (abs(i - k) > MAX_GAP) // �� ������ ������ �ʹ�ũ�� ����.
				break;
			if (info.err >= MAX_ERROR)
				break;
		}

		if (!src[i] || !dict_word[k])
			break;
		if (abs(i - k) > MAX_GAP) // �� ������ ������ �ʹ�ũ�� ����.
			break;
		if (info.err >= MAX_ERROR)
			break;
	}

	// ���� ������ �ܾŭ ���� �߻�
	while (dict_word[k++])
		++info.err;
	while (src[i++])
		++info.err;

	info.tot = info.hit - info.err;

	if (maxFitness.tot > info.tot)
		info = maxFitness;

	return false;
}
