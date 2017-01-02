
#include "matchparser.h"

using namespace match;
using namespace std;

const char *g_numStr = "1234567890.";
const int g_numLen = 11;
const char *g_strStr = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@$_/.";
const int g_strLen = 57;
const char *g_strStr2 = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@$_/.1234567890";
const int g_strLen2 = 67;
const char *g_strOp = "+-*/=:()";
const int g_opLen = 7;


cParser::cParser()
	: m_root(NULL)
	, m_execRoot(NULL)
{
}

cParser::~cParser()
{
	Clear();
}


bool cParser::IsBlank(const char *str, const int size)
{
	for (int i = 0; i < size; ++i)
	{
		if (!*str)
			break;
		if (('\n' == *str) || ('\r' == *str) || ('\t' == *str) || (' ' == *str))
			++str;
		else
			return false;
	}
	return true;
}


// pass through blank character
char* cParser::passBlank(const char *str)
{
	RETV(!str, NULL);

	while (1)
	{
		if (!*str)
			return NULL;
		if ((*str == ' ') || (*str == '\t'))
			++str;
		else
			break;
	}
	return (char*)str;
}

// id -> alphabet + {alphabet | number}
const char* cParser::id()
{
	// comma check
	m_lineStr = passBlank(m_lineStr);
	if (!m_lineStr)
		return "";

	// find first char
	const char *n = strchr(g_strStr, *m_lineStr);
	if (!n)
		return "";

	char *dst = m_tmpBuffer;
	*dst++ = *m_lineStr++;

	while (1)
	{
		const char *n = strchr(g_strStr2, *m_lineStr);
		if (!n || !*n)
			break;
		*dst++ = *m_lineStr++;
	}

	*dst = NULL;
	return m_tmpBuffer;
}

const char* cParser::number()
{
	// comma check
	m_lineStr = passBlank(m_lineStr);
	if (!m_lineStr)
		return "";

	char *dst = m_tmpBuffer;
	if ('+' == *m_lineStr)
		*dst++ = *m_lineStr++;
	else if ('-' == *m_lineStr)
		*dst++ = *m_lineStr++;

	while (1)
	{
		const char *n = strchr(g_numStr, *m_lineStr);
		if (!n || !*n)
			break;
		*dst++ = *m_lineStr++;
	}

	*dst = NULL;
	return m_tmpBuffer;
}

char cParser::op()
{
	m_lineStr = passBlank(m_lineStr);
	if (!m_lineStr)
		return NULL;
	const char *n = strchr(g_strOp, *m_lineStr);
	if (!n)
		return NULL;
	return *m_lineStr++;
}

int cParser::assigned(const char *var)
{
	m_lineStr = passBlank(m_lineStr);
	if (!m_lineStr)
	{
		std::cout << "line {" << m_lineNum << "} error!! assigned \n";
		return 0;
	}

	m_symbolTable[var] = m_lineStr;
	return 1;
}


// 한 라인을 읽는다.
bool cParser::ScanLine()
{
	if (m_isReverseScan)
	{
		--m_lineNum;
		m_lineStr = m_scanLine;
		m_isReverseScan = false;
		return true;
	}

	do
	{
		++m_lineNum;
		if (!m_ifs.getline(m_scanLine, sizeof(m_scanLine)))
			return false;
	} while (IsBlank(m_scanLine, sizeof(m_scanLine)));

	m_lineStr = m_scanLine;
	return true;
}


// 읽는 위치를 한 라인위로 옮긴다.
void cParser::UnScanLine()
{
	m_isReverseScan = true;
}


void cParser::collectTree(sParseTree *current, set<sParseTree*> &out)
{
	if (!current)
		return;
	out.insert(current);
	collectTree(current->child, out);
	collectTree(current->next, out);
}


void cParser::removeTree(sParseTree *current)
{
	set<sParseTree*> rmNodes;// 중복된 노드를 제거하지 않기위해 사용됨
	collectTree(current, rmNodes);
	for each (auto &node in rmNodes)
		delete node;
}


sParseTree* cParser::tree(sParseTree *current, const int depth)
{
	if (!ScanLine())
		return NULL;

	int cntDepth = 0;
	while (1)
	{
		if (!*m_lineStr)
			break; // error

		if (*m_lineStr == '\t')
		{
			++cntDepth;
			++m_lineStr;
		}
		else
		{
			break; // end
		}
	}

	if (cntDepth <= 0)
		return NULL; // error

	sParseTree *node = new sParseTree;
	memset(node, 0, sizeof(sParseTree));
	node->lineNum = m_lineNum;
	node->depth = cntDepth;
	strcpy_s(node->str, m_lineStr);

	sParseTree *reval = NULL;
	if (cntDepth == depth)
	{
		current->next = node;
		reval = tree(node, cntDepth);
	}
	else if (cntDepth == (depth + 1))
	{
		current->child = node;
		reval = tree(node, cntDepth);
	}
	else if (cntDepth > depth + 1)
	{
		// error
		std::cout << "line {" << m_lineNum << "} tree depth error!! " << node->str << std::endl;
		return NULL;
	}
	else
	{
		return node;
	}

	// search parent tree node
	while (reval)
	{
		if (depth == reval->depth)
		{
			current->next = reval;
			reval = tree(reval, depth);
		}
		else
		{
			return reval;
		}
	}

	return NULL;
}


// 스크립트를 읽고, 파스트리를 생성한다.
bool cParser::Read(const string &fileName)
{
	Clear();

	m_ifs.close();
	m_ifs.open(fileName.c_str());
	if (!m_ifs.is_open())
		return false;

	sParseTree *currentTree = NULL;
	sParseTree *currentExecTree = NULL;
	while (ScanLine())
	{
		const char *str = id();
		if (!*str)
			continue;

		const char c = op();

		if (c == ':') // tree head
		{
			sParseTree *head = new sParseTree;
			memset(head, 0, sizeof(sParseTree));
			head->type = 0;
			strcpy_s(head->str, str);

			if (!m_root)
			{
				m_root = head;
				currentTree = head;
			}
			else
			{
				currentTree->next = head;
				currentTree = head;
			}

 			tree(head, 0); // build tree
			UnScanLine();
		}
		else if (c == '=')
		{
 			if (!assigned(str))
 				break;
		}
		else
		{
			// exec command
			if (string(str) == "exec")
			{
				sParseTree *node = new sParseTree;
				memset(node, 0, sizeof(sParseTree));
				node->type = 1;
				strcpy_s(node->str, m_lineStr);

				if (!m_execRoot)
				{
					m_execRoot = node;
					currentExecTree = node;
				}
				else
				{
					currentExecTree->next = node;
					currentExecTree = node;
				}
			}
			else
			{
				// error occur
				std::cout << "line {" << m_lineNum << "} error!! operator >> ";
				if (m_lineStr)
					std::cout << m_lineStr;
				std::cout << "[" << c << "]" << std::endl;
				m_isError = true;
				break;
			}
		}
	}

	m_ifs.close();

	if (m_isError)
		return false;

	return true;
}


void cParser::Clear()
{
	removeTree(m_root);
	removeTree(m_execRoot);
	m_root = NULL;
	m_execRoot = NULL;
	m_isReverseScan = false;
	m_isError = false;
	m_lineNum = 0;

	m_symbolTable.clear();
}
