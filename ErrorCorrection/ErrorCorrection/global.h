#pragma once


#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator> 
#include <windows.h>

using std::string;
using std::vector;
using std::map;
using std::set;
using std::stringstream;


#define OUT

typedef unsigned int uint;
typedef unsigned char uchar;


string& trim(string &str);
string& lowerCase(string &str);
void tokenizer(const string &str, const string &delimeter, const string &ignoreStr, OUT vector<string> &out);


// elements를 회전해서 제거한다.
template <class Seq>
void rotatepopvector(Seq &seq, const unsigned int idx)
{
	if ((seq.size() - 1) > idx)
		std::rotate(seq.begin() + idx, seq.begin() + idx + 1, seq.end());
	seq.pop_back();
}
