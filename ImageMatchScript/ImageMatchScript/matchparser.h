#pragma once

#include "global.h"

namespace match
{

	class cParser
	{
	public:
		cParser();
		virtual ~cParser();
		bool Read(const string &fileName);
		void Clear();


	public:
		bool ScanLine();
		void UnScanLine();
		bool IsBlank(const char *str, const int size);
		char* passBlank(const char *str);
		const char* id();
		const char* number();
		char op();
		int assigned(const char *var);
		void collectTree(sParseTree *current, set<sParseTree*> &out);
		void removeTree(sParseTree *current);
		sParseTree* tree(sParseTree *current, const int depth);


	public:
		sParseTree *m_root;
		sParseTree *m_execRoot;
		std::ifstream m_ifs;
		map<string, string> m_symbolTable;
		char *m_lineStr;
		char m_scanLine[128];
		char m_tmpBuffer[128];
		bool m_isReverseScan = false; // 한라인을 다시 읽는다.
		bool m_isError = false;
		int m_lineNum = 0;
	};	

}
